# 구현 노트 — Phase 1 (RTSP Ingestion + HLS Egress + Admin UI)

**작업일:** 2026-06-28 ~ 2026-06-29  
**구현 범위:** ingestion-server, egress-server, frontend scaffold

---

## 어려웠던 점

### 1. StreamManager first_packet 캡처 위험 (수정됨)

최초 구현에서 `first_packet`을 `startStream()` 스택에서 람다가 참조 캡처했고, 함수 리턴 후 댕글링 참조가 발생할 수 있었다.

**수정:** `ActiveStream` 구조체 내에 `std::atomic<bool> first_packet{true}` 필드를 두고, 포인터로 캡처. `compare_exchange_strong`으로 정확히 한 번만 실행 보장.

### 2. StreamManager 락 데드락 (수정됨)

최초 구현에서 `unique_lock`을 잡은 채 `receiver->start()`를 호출, 패킷 콜백 내부에서 동일 mutex를 잡으려 했다.

**수정:** `streams_mutex_`를 `std::shared_mutex`로 변경. `startStream()`에서 `unique_lock`은 map 삽입 시에만 사용. 패킷 콜백에서는 `unique_lock`으로 info 업데이트 후 바로 해제. `stopStream()`에서 `ActiveStream`을 map에서 꺼낸 후 lock 해제, 그 다음 `stop()` 호출로 콜백 완료 대기.

### 3. SQLite3 namespace 충돌 (수정됨)

헤더에서 `struct sqlite3_stmt`를 namespace 내부에서 전방선언 시 컴파일러가 `homecctv::adapters::sqlite3_stmt`라는 별개 타입을 만들어 `sqlite3.h`의 전역 `sqlite3_stmt`와 타입 불일치가 발생했다.

**수정:** 헤더에서 직접 `#include <sqlite3.h>`.

### 4. FFmpeg pkg-config 링크 경로 (수정됨)

`pkg_check_modules` 결과로 `FFMPEG_LIBRARIES = -lavformat -lavcodec...` 만 얻어져서 링커가 라이브러리 위치를 찾지 못했다.

**수정:** `IMPORTED_TARGET` 옵션으로 `PkgConfig::FFMPEG` 타겟 사용. 이 타겟에는 include 경로, 라이브러리 경로, 링크 플래그가 모두 포함된다.

### 5. httplib::Response 전방선언 불가 (수정됨)

namespace 내 클래스에 대해 `struct httplib::Response&` 방식의 전방선언은 동작하지 않는다. 메서드 시그니처에 사용하려면 완전한 타입 정보가 필요하다.

**수정:** `HttpApiServer.hpp`에서 `<httplib.h>`를 직접 include.

---

## 애매했던 점

### 1. HLS URL 생성 위치

`HlsSegmentWriter::getPlaylistUrl()`이 `egress_base_url + "/hls/" + camera_id` 형태로 URL을 만든다. 이 URL이 실제로 egress-server에서 서빙되려면 두 서버가 같은 머신에서 실행되거나 같은 공유 스토리지를 써야 한다.

Phase 3 수평 확장 시 이 가정이 깨지므로, URL은 egress-server가 생성하고 ingestion-server는 파일 경로만 관리하는 구조로 변경 필요.

### 2. vcpkg vs Homebrew CMake 타겟명 차이

vcpkg 기준으로 설계한 타겟명(예: `unofficial::sqlite3::sqlite3`, `FFMPEG::avformat`)이 Homebrew 환경에서는 다르다. 현재 CMakeLists는 Homebrew 타겟으로 교체되어 있는데, vcpkg 환경에서 빌드 시 다시 교체 필요.

**TODO:** `vcpkg.json` 정비 후 CMakeLists를 조건 분기(`if(VCPKG_TOOLCHAIN)`)로 처리하거나 vcpkg 환경을 표준으로 확립.

### 3. StreamManager의 ActiveStream 소유권

최초 구현에서 `streams_`가 `unordered_map<string, ActiveStream>`이었는데, `ActiveStream`이 non-copyable 멤버(`unique_ptr`, `atomic`)를 갖고 있어 이동 의미론 처리가 복잡했다.

**수정:** `unordered_map<string, unique_ptr<ActiveStream>>`으로 변경. 포인터 기반이라 콜백 캡처 시 안정적.

---

### 6. HEVC timestamp 미설정 → 서버 크래시 (수정됨)

실제 HEVC(hvc1) 카메라(ONVIF RTSP, 2880x1620)에서 수신한 패킷의 PTS/DTS가 `AV_NOPTS_VALUE`로 오는 경우가 있었다. `av_interleaved_write_frame`이 첫 패킷에서 경고를 출력하고 timestamp를 임의 생성했지만, 이후 HLS context 내부 상태가 깨져 두 번째 패킷 쓰기 시 프로세스가 크래시했다.

추가로 HEVC 코덱 태그가 `hev1`로 설정되어 있어 HLS muxer가 `hvc1`을 사용하라는 경고를 출력했다.

**수정 (HlsSegmentWriter):**
- `open()`: 코덱이 HEVC이면 `codec_tag = MKTAG('h','v','c','1')` 설정
- `writePacket()`: rescale 전 pts/dts 크로스-보정, 둘 다 NOPTS면 `next_dts_` 카운터로 합성
- `bool errored_` 플래그 추가 — write 실패 후 추가 호출 차단

### 7. CameraRepository SQLite 스레드 안전성 (수정됨)

RTSP 수신 백그라운드 스레드가 `camera_repo_->updateStatus()` 호출, HTTP 핸들러 스레드가 동시에 `findAll()` 호출. 같은 `sqlite3*` 핸들을 mutex 없이 공유해 크래시 가능.

**수정:** `CameraRepository.hpp`에 `mutable std::mutex db_mutex_` 추가. 모든 public 메서드 진입 시 `std::lock_guard<std::mutex> lock(db_mutex_)` 획득.

---

### 8. LL-HLS keyframe 정렬 미흡으로 hls.js "오류" (수정됨)

초기 구현에서 세그먼트 분할 기준이 시간 단위(`current_seg_duration_ >= cfg_.segment_duration`)였다. GOP 경계와 무관하게 분할되면 새 세그먼트 첫 part가 IDR 프레임으로 시작하지 않아 hls.js가 독립 재생 불가로 "오류"를 표시했다.

또한 keyframe 직전 flush 없이 keyframe 패킷을 쓰면 keyframe이 현재 part 끝에 붙어 다음 part에서 `INDEPENDENT=YES`가 되어야 할 part가 non-independent로 표시됐다.

**수정:**
- 세그먼트 분할 조건: `(시간 초과 && is_key) || 2× 초과(하드 리밋)`
- keyframe 도착 전 현재 part flush → keyframe이 새 part의 첫 프레임 → `INDEPENDENT=YES`
- `EXT-X-TARGETDURATION`: 완료 세그먼트 중 `ceil(duration)` 최대값으로 동적 계산

### 9. ReadyCallback 도입으로 오디오 초기화 타이밍 문제 해결 (수정됨)

기존 `first_packet` 패턴에서 `avformat_write_header` 호출 전에 오디오 스트림을 추가할 수 없었다. 최초 패킷이 도달했을 때 이미 `write_header`가 실행됐거나 오디오 스트림 정보가 없었기 때문이다.

**수정:** `ReadyCallback(video_st, audio_st)` 도입. RTSP 연결 직후, 패킷 루프 진입 전에 호출. `setAudioStream()` → `open()` 순서로 호출하면 `avformat_write_header` 시 오디오 트랙이 init.mp4에 포함된다.

### 10. IPTIME C500G 카메라 오디오 없음 (확인됨)

테스트 카메라(IPTIME C500G) 두 대 모두 RTSP 스트림에 오디오가 없음. `av_find_best_stream(AVMEDIA_TYPE_AUDIO)` 반환값 -1. 오디오 파이프라인 코드는 정상 동작하나 실제 오디오 스트림이 없으므로 `init.mp4`에 오디오 트랙 없음.

---

## 미구현 / 스텁 처리된 항목

| 항목 | 상태 | 비고 |
|------|------|------|
| FPS 계산 | 구현됨 | `ReadyCallback`에서 `avg_frame_rate`로 채움 |
| 비트레이트 표시 | 구현됨 | 서버: `codecpar->bit_rate`, 클라이언트: `FRAG_LOADED` 실시간 |
| 오디오 지원 | 구현됨 | 카메라에 오디오 스트림이 있을 경우 자동 포함 |
| 스트림 이벤트 WebSocket 푸시 | 미구현 | 현재 로그만 출력 |
| JWT 인증 미들웨어 | 미구현 | Phase 2 |
| RTMP / WebRTC 수신 | 미구현 | Phase 3 |
| fMP4 녹화 실제 검증 | 미검증 | 실제 RTSP 카메라 필요 |

---

## macOS 개발 환경 비고

- `std::jthread` / `std::stop_token` — Apple Clang 17에서 미지원. `std::thread`로 대체.
- `CMAKE_OSX_ARCHITECTURES: arm64;x86_64` — Homebrew 라이브러리가 arm64 전용이라 유니버설 빌드 불가. 개발은 arm64 단독으로 진행.
- vcpkg 도입 시 유니버설 바이너리 빌드 재검토 가능.
