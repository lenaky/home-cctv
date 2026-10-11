# 007 — FFmpeg/sqlite3는 CMake config 대신 PkgConfig로 링크

- **날짜:** 2026-10-12
- **관련 코드:** `services/ingestion-server/CMakeLists.txt`, `services/egress-server/CMakeLists.txt`

## 상황 (Context)

Phase 1 스켈레톤을 실제로 빌드/테스트/린트까지 돌려 검증하는 과정(→ [[compound/002-vcpkg-vs-brew-local-verification]])에서, FFmpeg와 sqlite3를 CMake에서 어떻게 찾을지 결정이 필요했다.
`cpp-httplib`와 `GTest`는 vcpkg/Homebrew 양쪽 모두 안정적인 CMake config 패키지
(`httplib::httplib`, `GTest::gtest`)를 내보내 `find_package(... CONFIG REQUIRED)`로
문제없이 해결됐다.

## 검토한 대안

1. vcpkg가 내보내는 타겟 이름을 그대로 가정 (`FFmpeg::avformat` 등, `unofficial::sqlite3::sqlite3`)
   — vcpkg 환경에서만 보장되고, FFmpeg는 vcpkg에서도 CMake config를 안정적으로
   내보내지 않는 경우가 있어 vcpkg 환경에서조차 깨질 수 있다. Homebrew 등 다른 패키지
   매니저에서는 전혀 맞지 않는다.
2. `find_package(PkgConfig REQUIRED)` + `pkg_check_modules(... IMPORTED_TARGET ...)`로
   FFmpeg(`libavformat`/`libavcodec`/`libavutil`)와 sqlite3를 찾기 — 두 라이브러리 모두
   vcpkg와 Homebrew 둘 다에서 `.pc` 파일을 설치하므로 패키지 매니저에 무관하게 동작한다.

## 결정

옵션 2. FFmpeg/sqlite3는 `pkg_check_modules`로 찾고 `PkgConfig::FFMPEG`/`PkgConfig::SQLITE3`
IMPORTED 타겟으로 링크한다. httplib/GTest는 그대로 `find_package(... CONFIG REQUIRED)`
유지 (이미 양쪽에서 잘 동작함).

## 근거 (왜)

실제로 vcpkg가 설치되지 않은 이 머신에서 Homebrew로 빌드를 검증하다가, 두 서브에이전트가
각자 다른 타겟 이름(`PkgConfig::FFMPEG` vs `FFmpeg::avformat`/`unofficial::sqlite3::sqlite3`)을
썼다는 게 drift로 드러났다. pkg-config 방식은 vcpkg manifest 모드에서도(vcpkg가 `.pc`
파일을 설치triplet에 포함) 동일하게 동작하므로, 패키지 매니저를 바꿔도 CMakeLists.txt를
고칠 필요가 없다.

## 재검토 조건

실제 vcpkg 설치 환경에서 빌드를 검증했을 때 `pkg_check_modules`가 vcpkg의 `.pc` 파일을
못 찾는 경우(triplet별 PKG_CONFIG_PATH 설정 문제 등)가 생기면, vcpkg 쪽에서 별도로
`CMAKE_PREFIX_PATH`/`PKG_CONFIG_PATH`를 vcpkg triplet의 `lib/pkgconfig`로 맞춰주는 사용
가이드를 CLAUDE.md에 추가. 그래도 안 되면 그때 vcpkg 전용 타겟 이름으로 분기.
