# 002 — vcpkg 미설치 환경에서는 brew로 즉시 검증하고 CMake를 pkg-config로 통일

- **날짜:** 2026-10-12
- **관련 작업:** Phase 1 C++ 스켈레톤 생성 및 빌드/테스트/린트 검증

## 무슨 일이 있었는가

프로젝트 공식 빌드 체계는 CMake+vcpkg(메모리에 확정)지만, 이 머신에는 vcpkg가 설치돼
있지 않았다. vcpkg로 FFmpeg를 새로 부트스트랩하면 소스 빌드 특성상 매우 오래 걸릴 수
있어, 사용자 확인을 받아 Homebrew에 이미 설치된 ffmpeg/sqlite3/cpp-httplib로 먼저
검증하기로 했다.

이 과정에서 두 서브에이전트(ingestion-server, egress-server)가 병렬로 스캐폴딩하면서
FFmpeg/sqlite3 CMake 타겟 이름을 서로 다르게 선택했다 (`PkgConfig::FFMPEG` vs
`FFmpeg::avformat`/`unofficial::sqlite3::sqlite3`). vcpkg의 FFmpeg 포트는 CMake config
패키지를 안정적으로 내보내지 않고, sqlite3는 vcpkg/brew 둘 다 일관된 CMake config 이름이
없다 — 반면 둘 다 pkg-config(.pc 파일)는 제공한다.

추가로: egress-server의 `tests/CMakeLists.txt`에 `find_package(GTest CONFIG REQUIRED)`가
빠져 있었고(ingestion-server에는 있었음), web-admin의 `.eslintrc.cjs`가 package.json에
선언되지 않은 `eslint-plugin-react-refresh`를 참조했고, `tsconfig.json`이 존재하지 않는
`tsconfig.node.json`을 참조했다 — 모두 "실제로 빌드/린트를 돌려보기 전까지" 드러나지 않는
종류의 불일치였다.

## 왜 그랬는가

- FFmpeg/sqlite3는 패키지 매니저마다 CMake 통합 방식이 달라서, 에이전트에게 "vcpkg의
  타겟 이름"만 스펙으로 주면 실제 설치 환경(vcpkg vs brew)에 따라 깨질 수 있는 선택을
  하게 된다 — **pkg-config로 통일하는 게 vcpkg/brew 양쪽에서 더 안정적**이라는 걸 build
  전까지는 몰랐다.
- 병렬로 scaffolding한 두 서브에이전트는 서로의 결과물을 보지 못하므로, 공유해야 할
  관례(CMake 타겟 링크 방식)가 자연스럽게 갈라졌다.
- web-admin처럼 "최소 골격"을 만들 때도, 실제로 `npm install && npm run build && npm run
  lint`를 돌리기 전에는 설정 파일 간의 숨은 참조 누락(tsconfig.node.json, eslint 플러그인)
  을 알 수 없었다.

## 다음에는 어떻게 할 것인가

- 여러 서비스가 같은 서드파티 라이브러리(FFmpeg, sqlite3 등)를 쓸 때는, 병렬 스캐폴딩
  에이전트들에게 "정확한 CMake 타겟 이름"을 각자 판단하게 하지 말고, **pkg-config
  기반으로 통일하라고 공통 스펙에 명시**해 애초에 갈라지지 않게 한다.
- "완료 조건을 직접 실행해서 확인"하는 5단계(검증)는 생략 불가다 — 이번에도 실제로
  `cmake --build`와 `npm run build`를 돌리기 전까지는 두 가지 구성 버그(GTest
  find_package 누락, eslint 플러그인 미선언)를 못 찾았다.
- vcpkg가 설치 안 된 로컬 환경에서는, 전체 vcpkg 부트스트랩을 기다리기보다 먼저
  사용자에게 "이미 설치된 패키지 매니저(brew 등)로 빠르게 검증할지"를 물어보는 게
  낫다 — 단, 이건 로컬 검증 경로일 뿐이고 vcpkg.json(공식 매니페스트)은 그대로 유지한다.
