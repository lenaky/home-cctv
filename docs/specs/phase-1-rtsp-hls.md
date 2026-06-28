# Phase 1 Spec — RTSP Ingestion + HLS Egress + Admin UI

## 목표

가정용 IP 카메라의 RTSP 스트림을 수신하여 HLS로 변환, 브라우저에서 실시간으로 시청할 수 있는 최소 동작 시스템.

---

## 컴포넌트

### 1. ingestion-server (C++, port 8080)

**역할:** RTSP 수신, HLS 세그먼트 생성, 카메라 설정 관리, REST API 제공.

#### REST API

| Method | Path | 설명 |
|--------|------|------|
| GET | `/health` | 헬스체크 |
| GET | `/api/cameras` | 카메라 목록 |
| GET | `/api/cameras/:id` | 카메라 단건 조회 |
| POST | `/api/cameras` | 카메라 등록 |
| PUT | `/api/cameras/:id` | 카메라 수정 |
| DELETE | `/api/cameras/:id` | 카메라 삭제 |
| POST | `/api/streams/:camera_id/start` | 스트림 시작 |
| POST | `/api/streams/:camera_id/stop` | 스트림 중지 |
| GET | `/api/streams/:camera_id/status` | 스트림 상태 조회 |
| GET | `/hls/*` | HLS 세그먼트/플레이리스트 서빙 |

#### Camera 등록 요청 body
```json
{
  "name": "거실 카메라",
  "rtsp_url": "rtsp://192.168.1.100:554/stream",
  "settings": {
    "transport": 0,
    "segment_duration_sec": 2,
    "hls_list_size": 10,
    "recording_enabled": false,
    "recording_path": ""
  }
}
```

`transport`: `0` = TCP (기본), `1` = UDP

#### Camera 상태값
| 값 | 의미 |
|----|------|
| 1 | Inactive |
| 2 | Active (스트리밍 중) |
| 3 | Error |

#### RTSP 수신 동작
- FFmpeg `avformat`으로 RTSP 연결 (`avformat_open_input`)
- TCP/UDP 전송 방식 설정 가능 (카메라 settings에서)
- FFmpeg 내장 jitter buffer로 UDP 패킷 재조립 처리
- 연결 실패/끊김 시 자동 재연결 (최대 5회, 3초 간격)
- 비디오 스트림만 처리 (`av_find_best_stream AVMEDIA_TYPE_VIDEO`)
- 첫 패킷 수신 시 HLS writer 초기화 (codec params 확정 후)

#### HLS 출력
- FFmpeg HLS muxer 사용 (`avformat "hls"`)
- 세그먼트: `.ts` 파일
- 플레이리스트: `index.m3u8`
- 저장 경로: `~/.home-cctv/hls/<camera_id>/`
- 세그먼트 길이, list size: 카메라별 설정값 사용
- HLS flags: `delete_segments+append_list+independent_segments`

#### 녹화 (recording_enabled=true 시)
- H.264 / H.265 코덱: fMP4 remux (재인코딩 없음)
  - movflags: `frag_keyframe+empty_moov+default_base_moof+faststart`
- 기타 코덱: MPEG-TS 원본 저장 (`.ts`)
- 세그먼트 단위: 기본 1시간 (3600초)
- 저장 경로: `recording_path` 설정값, 비어있으면 `~/.home-cctv/recordings/<camera_id>/`

#### 데이터 저장
- SQLite3 (`~/.home-cctv/ingestion.db`, WAL 모드)
- cameras 테이블에 모든 카메라 설정, 상태, 에러 메시지 저장

#### 환경변수
| 변수 | 기본값 | 설명 |
|------|--------|------|
| `INGESTION_PORT` | `8080` | HTTP API 포트 |
| `EGRESS_BASE_URL` | `http://localhost:8090` | HLS URL prefix |

---

### 2. egress-server (C++, port 8090)

**역할:** HLS 파일 HTTP 서빙, (선택) React 프론트엔드 static 서빙.

| Path | 설명 |
|------|------|
| GET `/health` | 헬스체크 |
| GET `/hls/<camera_id>/index.m3u8` | HLS 플레이리스트 |
| GET `/hls/<camera_id>/seg*.ts` | HLS 세그먼트 |
| GET `/*` | React 빌드 결과 (FRONTEND_DIST 설정 시) |

- CORS 헤더 전체 허용 (`Access-Control-Allow-Origin: *`)
- SPA fallback: 알 수 없는 경로 → `index.html`

#### 환경변수
| 변수 | 기본값 | 설명 |
|------|--------|------|
| `EGRESS_PORT` | `8090` | HTTP 포트 |
| `FRONTEND_DIST` | (없음) | React 빌드 경로 |

---

### 3. frontend (React + TypeScript, dev: port 5173)

**역할:** 카메라 관리 Admin UI + HLS 라이브 뷰어.

#### 라우팅
| Path | 페이지 |
|------|--------|
| `/admin` | 카메라 목록, 추가/편집/삭제, 스트림 시작/중지 |
| `/viewer/:cameraId` | HLS 라이브 뷰어 + 스트림 메타 정보 |

#### Admin 페이지 기능
- 카메라 목록 (5초 폴링으로 상태 자동 갱신)
- 카메라 추가/편집 모달 (이름, RTSP URL, transport, 세그먼트 길이, 녹화 설정)
- 스트림 시작/중지 버튼
- "라이브 보기" 버튼 → `/viewer/:cameraId` 이동

#### Viewer 페이지 기능
- hls.js 기반 HLS 플레이어
- Safari: 네이티브 HLS 지원
- 스트림 메타 정보 표시: 코덱, 해상도, FPS
- 3초 폴링으로 스트림 상태 갱신

#### Vite proxy (개발)
```
/api  → http://localhost:8080
/hls  → http://localhost:8090
```

---

## 데이터 디렉토리

```
~/.home-cctv/          (macOS)
%APPDATA%\home-cctv\   (Windows)
  ingestion.db
  hls/
    <camera_id>/
      index.m3u8
      seg00001.ts
      ...
  recordings/
    <camera_id>/
      <camera_id>_0001_<timestamp>.mp4   (fMP4)
      <camera_id>_0001_<timestamp>.ts    (raw fallback)
```

---

## 구현 범위 제외 (Phase 2 이후)

- 인증/권한 (JWT)
- WebRTC 출력
- gRPC inter-server 통신
- 트랜스코딩
- RTMP / WebRTC 입력
- 다중 노드
