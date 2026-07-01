# 테스트 로그 — Phase 1 초기 구현 + 실사용 버그 수정

**날짜:** 2026-06-29 (초기) / 2026-06-30 ~ 2026-07-01 (실카메라 버그 수정)  
**테스트 환경:** macOS (Apple Silicon, arm64), Apple Clang 17, Homebrew 패키지  
**테스트 카메라 특성:**
- HEVC 2880×1620 / 20fps / GOP≈4s / pcm_alaw 오디오 / 파라미터셋 별도 패킷 주입 방식 (버그 #12 트리거)
- HEVC 2880×1620 / 20fps / GOP≈4s / 파라미터셋 IDR 합산 방식 (정상 동작)
- H.264 / 오디오 트랙 없음

---

## 빌드 검증

| 항목 | 결과 | 비고 |
|------|------|------|
| `cmake --preset macos-debug` | ✅ 통과 | gRPC 없음 — packages/core 스킵 (Phase 2) |
| `cmake --build --preset macos-debug` | ✅ 통과 | 수정 4회 후 최종 성공 |
| `cd apps/frontend && npm run build` | ✅ 통과 | TypeScript 에러 없음, hls.js 번들 경고 (무해) |

---

## 런타임 검증

| 테스트 케이스 | 결과 | 비고 |
|--------------|------|------|
| `GET /health` (ingestion :8080) | ✅ `{"status":"ok"}` | |
| `GET /health` (egress :8090) | ✅ `{"status":"ok","service":"egress"}` | |
| `POST /api/cameras` (카메라 등록) | ✅ status=1(Inactive) 반환 | 수정 후 — 아래 참조 |
| `GET /api/cameras` (목록 조회) | ✅ 등록된 카메라 1건 반환 | |
| `GET /api/cameras/:id` (단건 조회) | ✅ 정상 | |
| `PUT /api/cameras/:id` (수정) | ✅ name, segment_duration_sec 반영 | |
| `DELETE /api/cameras/:id` | ✅ HTTP 204 | |
| `GET /api/cameras` (삭제 후) | ✅ 빈 배열 `[]` | |
| `POST /api/streams/:id/start` | ✅ status=2(Connecting) + HLS URL 반환 | RTSP 연결 실패는 백그라운드에서 처리 |
| `GET /api/streams/:id/status` | ✅ status=2 + HLS URL 유지 | |
| HLS 플레이리스트 생성 | ✅ seg00003.ts ~ seg00023.ts 생성 확인 | HEVC hvc1 카메라 실측 |
| 브라우저 HLS 재생 | ✅ hls.js 재생 확인 | admin → viewer 이동 후 실시간 영상 출력 |
| LL-HLS 재생 (keyframe 정렬) | ✅ 수정 후 정상 | seg_time_reached 조건 + pre-flush 적용 후 "오류" 없음 |
| FPS / 비트레이트 표시 | ✅ 뷰어에서 확인 | ReadyCallback에서 avg_frame_rate 채움, FRAG_LOADED 실시간 비트레이트 |
| YouTube Live-style 플레이어 | ✅ 뷰어에서 확인 | 커스텀 오버레이, LIVE 배지, 볼륨 슬라이더 |
| 오디오 지원 코드 동작 확인 | ✅ 빌드+런타임 | setAudioStream/writeAudioPacket 정상 동작; 테스트 카메라 오디오 없음 (audio=none) |
| 재연결 동작 (RTSP 끊김) | ✅ 동작 확인 | max reconnect 후 zombie 버그 → 수정됨 (아래 참조) |
| fMP4 녹화 파일 생성 | 미확인 | recording_enabled=false (기본값) |
| SQLite DB 파일 생성 | ✅ `~/.home-cctv/ingestion.db` 생성 확인 | |

---

## 빌드 중 발생한 실패 케이스 (수정 완료)

| # | 실패 | 원인 | 수정 |
|---|------|------|------|
| 1 | `std::jthread` 컴파일 에러 | Apple Clang 17 / macOS SDK에서 `std::jthread` 미지원 | `std::thread` + `std::atomic<bool> running_` 로 교체 |
| 2 | `std::shared_lock` 컴파일 에러 | `<shared_mutex>` include 누락 + `std::mutex` 필드 타입 불일치 | `#include <shared_mutex>`, 필드를 `std::shared_mutex`로 변경 |
| 3 | `httplib::Response` 전방 선언 에러 | namespace 내 struct 전방 선언 불가 | `HttpApiServer.hpp`에 `<httplib.h>` 직접 include |
| 4 | SQLite3 namespace 충돌 | `CameraRepository.hpp`에서 `struct sqlite3_stmt` 네임스페이스 내 전방 선언 시 `homecctv::adapters::sqlite3_stmt` 생성 | header에서 `<sqlite3.h>` 직접 include |
| 5 | 링커: `-lavformat not found` | `pkg_check_modules` `FFMPEG_LIBRARIES` 변수가 라이브러리 경로 없이 `-l` 플래그만 제공 | `IMPORTED_TARGET` 옵션 사용 → `PkgConfig::FFMPEG` 타겟으로 변경 |
| 6 | 링커: `arm64/x86_64` 아키텍처 충돌 | `CMakePresets.json`에 `CMAKE_OSX_ARCHITECTURES: arm64;x86_64` 설정 + Homebrew는 arm64 전용 | 유니버설 바이너리 설정 제거 (arm64만 빌드) |
| 7 | `CameraStatus` enum 불일치 | C++ `enum class` 기본값 0-based, TypeScript/proto는 1-based | C++ enum에 명시적 값 지정: `Inactive=1, Active=2, Error=3`, `StreamStatus`도 동일 처리 |
| 8 | HEVC timestamp 미설정 → 서버 크래시 | 실제 ONVIF HEVC 카메라에서 PTS/DTS=AV_NOPTS_VALUE 패킷 수신 시 `av_interleaved_write_frame` 내부 상태 오염 후 크래시 | `HlsSegmentWriter::writePacket`에서 pts/dts 크로스-보정 + 합성 카운터 추가; HEVC 코덱 태그 `hvc1` 설정 |
| 9 | viewer → admin 뒤로 가기 시 Internal Server Error | RTSP 백그라운드 스레드의 `updateStatus()` + HTTP 핸들러의 `findAll()`이 mutex 없이 동일 `sqlite3*` 공유 | `CameraRepository`에 `std::mutex db_mutex_` 추가, 모든 메서드에 `lock_guard` 적용 |

---

---

## 실카메라 테스트 — 버그 수정 결과 (2026-06-30 ~ 07-01)

### HEVC 파라미터셋 패킷 trun 손상 버그

| 테스트 | 결과 | 비고 |
|--------|------|------|
| 수정 전 세그먼트 ffprobe: IDR duration | ❌ 0.128s (11520 ticks) | 정상=0.050s (4500 ticks) |
| 수정 전 세그먼트: IDR 이후 패킷 duration | ❌ 0.000011s (1 tick) / 0.020s | 파라미터셋 DTS 충돌 흔적 |
| 수정 후 세그먼트 IDR duration | ✅ 0.057s (~5130 ticks) | 정상 범위 |
| 수정 후 모든 패킷 duration | ✅ 0.040–0.053s | 전부 정상 프레임 간격 |
| 수정 후 `INDEPENDENT=YES` 파트 | ✅ 각 세그먼트 첫 파트에 부착 | 수정 전에는 누락됨 |
| 수정 후 세그먼트 파일 크기 | ✅ 178–182KB (기존 174–183KB와 유사) | 1차 시도(첫 NAL만 체크)에서 52KB로 축소됨 → VCL 전체 스캔으로 교정 |
| 수정 후 영상 재생 안정성 (파라미터셋 별도 주입 카메라) | ✅ 검은 화면/역재생 없음 | 브라우저에서 직접 확인 필요 |

**1차 시도 실패 케이스:** 첫 NAL 타입만 체크하는 방식 → VPS+SPS+PPS+IDR 합산 패킷의 첫 NAL이 VPS(32)이므로 IDR 전체가 필터링됨. 세그먼트에 K__ 플래그 없음, 파일 크기 1/3 수준으로 축소. 모든 NAL 스캔(VCL 포함 여부) 방식으로 재수정.

### Zombie 스트림 재시작 버그

| 테스트 | 결과 | 비고 |
|--------|------|------|
| Max reconnect 후 `startStream` API 호출 | ✅ 자동 정리 후 재시작 성공 | 수정 전: "Stream already active" 오류 |
| 정상 스트리밍 중 `startStream` 재호출 | ✅ "Stream already active" 반환 | 의도된 동작 유지 |
| Max reconnect 후 `GET /status` | ✅ status=4 (Error) 반환 | 수정 전: status=3 (Streaming, 오해 소지) |

### LL-HLS 딜레이 튜닝

| 테스트 | 결과 | 비고 |
|--------|------|------|
| hls.js 기본 설정 딜레이 | 측정 없음 | 추정 10–15s |
| setInterval 라이브엣지 강제 seek 후 딜레이 | ✅ ~3s | buffEnd - currentTime > 3s 시 seek |
| H.264 카메라 딜레이 | ✅ ~3s | |
| HEVC 카메라 딜레이 | 미확인 | 타임라인 점프 버그 수정 후 측정 필요 |

### HEVC 카메라 MSE 타임라인 점프 버그 (#16)

| 테스트 | 결과 | 비고 |
|--------|------|------|
| 수정 전 ingestion.log "out of range" 경고 횟수 | ❌ 617건 | `Packet duration: -810 / dts: ... out of range` |
| 수정 전 ingestion.log "pts has no value" 경고 | ❌ 617건 | out of range와 쌍으로 발생 |
| 수정 전 브라우저 타임라인 동작 | ❌ 점프 (1시1분→1시21분→1시20분→1시1분4초) | uint32 overflow로 ~47721s 점프 |
| 수정 후 빌드 | ✅ 오류 없음 | LlHlsWriter.cpp 1 파일만 재컴파일 |
| 수정 후 서버 기동 | ✅ /health 응답 정상 | |
| 수정 후 브라우저 재생 | 확인 필요 | ingestion-server 재시작 후 측정 |

---

## CMakeLists 변경 사항 (빌드 환경 적응)

**원래 설계 (vcpkg):**
- `find_package(FFMPEG REQUIRED)` → `FFMPEG::avformat` 타겟
- `find_package(unofficial-sqlite3 CONFIG REQUIRED)` → `unofficial::sqlite3::sqlite3`
- `CMAKE_OSX_ARCHITECTURES: arm64;x86_64`

**실제 (Homebrew):**
- `pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ...)` → `PkgConfig::FFMPEG`
- `find_package(SQLite3 REQUIRED)` → `SQLite3::SQLite3`
- 유니버설 바이너리 제거

vcpkg 환경에서는 원래 설계대로 복원 필요. Homebrew 대응은 macOS 개발 편의용.
