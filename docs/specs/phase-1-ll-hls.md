# 스펙: Phase 1 LL-HLS (Low Latency HLS)

**작성일:** 2026-06-29  
**상태:** 구현 예정

---

## 목표

표준 HLS(~6s 딜레이)를 LL-HLS로 교체하여 **1초 이하** 딜레이 달성.

---

## 배경

FFmpeg 8.x HLS muxer는 Apple LL-HLS의 파셜 세그먼트(`#EXT-X-PART`)를 지원하지 않으므로, LlHlsWriter를 직접 구현한다.

---

## LL-HLS 핵심 요소

| 요소 | 설명 |
|------|------|
| 파셜 세그먼트 (Part) | 풀 세그먼트의 바이트레인지 구간. 200ms 단위로 플레이리스트에 공개 |
| 블로킹 플레이리스트 리로드 | `?_HLS_msn=N&_HLS_part=P` 요청 → 해당 파트가 써질 때까지 응답 보류 |
| 초기화 세그먼트 | `init.mp4` (ftyp+moov). `#EXT-X-MAP:URI="init.mp4"` |
| fMP4 세그먼트 | `seg%05d.mp4` (moof+mdat 연속). 파트는 바이트레인지로 참조 |

---

## 플레이리스트 형식

```m3u8
#EXTM3U
#EXT-X-VERSION:9
#EXT-X-TARGETDURATION:2
#EXT-X-PART-INF:PART-TARGET=0.200
#EXT-X-SERVER-CONTROL:CAN-BLOCK-RELOAD=YES,PART-HOLD-BACK=0.600
#EXT-X-MEDIA-SEQUENCE:5
#EXT-X-MAP:URI="init.mp4"

#EXT-X-PART:DURATION=0.200,INDEPENDENT=YES,URI="seg00005.mp4",BYTERANGE=12000@0
#EXT-X-PART:DURATION=0.200,URI="seg00005.mp4",BYTERANGE=11500@12000
...
#EXTINF:2.000,
seg00005.mp4

#EXT-X-PART:DURATION=0.200,INDEPENDENT=YES,URI="seg00006.mp4",BYTERANGE=11800@0
#EXT-X-PRELOAD-HINT:TYPE=PART,URI="seg00006.mp4",BYTERANGE-START=11800
```

---

## 컴포넌트 설계

### LlHlsWriter (ingestion-server, adapters/outbound/hls/)

- `ISegmentWriter` 포트 구현 (기존 인터페이스 유지)
- `open(AVStream*)`: init.mp4 생성 → seg00001.mp4 열기 → 초기 플레이리스트 작성
- `writePacket(AVPacket*, AVRational)`:
  - `av_write_frame(ctx_, pkt)` 로 패킷 축적 (단일 비디오 스트림, 인터리빙 불필요)
  - `current_part_duration_ >= part_target_(0.2s)` 또는 키프레임 경계 → `flushPart()` 호출
  - `current_seg_duration_ >= segment_duration_(2s)` 이고 키프레임 → `finalizeSegment()` + `openSegment()`
- `flushPart()`: `av_write_frame(ctx_, nullptr)` 로 moof+mdat flush → 바이트레인지 기록 → 플레이리스트 갱신
- `finalizeSegment()`: 세그먼트 파일 닫기 → completed_segs_ 추가 → 오래된 세그먼트 삭제
- `writePlaylist()`: 원자적 갱신 (tmp 파일 write → rename)

**FFmpeg 설정:**  
`movflags=frag_custom+empty_moov+default_base_moof`  
→ avformat_write_header = ftyp+moov (init.mp4에만 기록)  
→ av_write_frame(ctx, nullptr) = 하나의 moof+mdat 프래그먼트 flush  
→ ctx_->pb swap으로 세그먼트 파일 교체 (avformat_write_header 재호출 불필요)

### egress-server (블로킹 플레이리스트 리로드)

- `GET /hls/:cam_id/index.m3u8` 을 별도 핸들러로 분리
- `_HLS_msn`/`_HLS_part` 파라미터 파싱
- 50ms 폴링으로 플레이리스트 파일 읽기 → MSN/파트 조건 충족 시 반환
- 3초 타임아웃 → 현재 플레이리스트 반환

### hls.js (프론트엔드)

```typescript
new Hls({
    lowLatencyMode: true,
    liveSyncDurationCount: 1,
    liveMaxLatencyDurationCount: 3,
    enableWorker: true,
})
```

---

## 레이턴시 분석

| 단계 | 지연 |
|------|------|
| 카메라 GOP 경계 대기 | 0 ~ 200ms (파트 경계) |
| 파트 플러시 + 플레이리스트 갱신 | < 5ms |
| 블로킹 리로드 폴링 | 0 ~ 50ms |
| hls.js PART-HOLD-BACK | 600ms (= 3 × 200ms) |
| **총 목표 딜레이** | **~1초** |

---

## 구현 제외 사항

- HTTP/2 Server Push (PRELOAD-HINT 기반) — Phase 2
- 오디오 트랙 포함 LL-HLS — 현재 비디오 전용
- 블로킹 리로드를 inotify/kqueue로 교체 — 50ms 폴링으로 충분
