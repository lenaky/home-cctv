# 테스트 로그 — Phase 1 초기 구현

**날짜:** 2026-06-29  
**테스트 환경:** macOS (Apple Silicon, arm64), Apple Clang 17, Homebrew 패키지

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
| 재연결 동작 (RTSP 끊김) | 미확인 | 의도적 끊김 테스트 미진행 |
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
