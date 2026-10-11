# home-cctv — Phase 1 설계 요약

> 2026-10-11~12 아키텍처 제안 세션의 산출물. 전체 판단 근거는 [`docs/decisions/001`](decisions/001-error-handling-result-type.md)~[`006`](decisions/006-phase1-ops-scope.md) 참고.

## 개요

가정용 NVR/VMS. RTSP만 입력받아 LL-HLS + WebRTC로 출력. C++20, 하나의 맥북에어 M3에서
동시 8채널 수집, 10명 미만 동시 시청. macOS + Windows 타겟. 장기적으로 32채널+ 수평 확장을
염두에 둔 구조를 Phase 1부터 유지한다(→ [[docs/decisions/002-ingestion-egress-full-separation]]).

## 디렉토리 구조와 모듈 책임

```
services/ingestion-server/   — RTSP 수집, 카메라 CRUD, 녹화(fMP4), LL-HLS 세그먼트 생성. 포트 8080.
services/egress-server/      — 실시간 시청(LL-HLS/WebRTC) 서빙, 녹화 재생. 포트 8090.
web-admin/                   — React+TS+Vite+Tailwind 웹 어드민 (최소 골격, 세부 UI는 범위 밖)
deploy/                      — macOS launchd / Windows 서비스 등록 설정 (데몬 자동 재시작)
docs/                        — 설계 요약, 판단 로그(decisions/), 버그 로그(fixes/), 검증된 패턴(learnings/)
compound/                    — 이 프로젝트의 작업 절차(CLAUDE.md 6단계)를 거치며 얻은 교훈
```

각 서비스 내부(hexagonal):
```
domain/            — 엔티티, 값 타입, Result<T,E>, port 인터페이스. 외부 의존 없음.
application/        — usecase. domain의 port만 의존, 구현체는 모름.
adapters/inbound/   — HTTP 컨트롤러. port + usecase를 호출해 HTTP로 번역.
adapters/outbound/  — SQLite, FFmpeg, 파일시스템, IPC 등 실제 구현.
main.cpp            — 합성 루트. concrete adapter를 생성해 usecase에 주입.
```

`ingestion-server`와 `egress-server`는 컴파일 타임 의존이 전혀 없다. 런타임으로만
(REST, 공유 파일시스템, SQLite 파일, 로컬 IPC) 연결된다.

## 핵심 설계 결정 (요약, 상세는 docs/decisions/)

| 번호 | 결정 | 한 줄 요약 |
|---|---|---|
| 001 | 에러 처리 | 직접 구현한 `Result<T,E>`, 예외 없음 |
| 002 | 서비스 분리 | ingestion/egress 완전 분리, 타입도 공유 안 함 |
| 003 | WebRTC 저지연 | HLS 세그먼트 재사용 대신 로컬 IPC(UDS/Named Pipe)로 실시간 패킷 전달 |
| 004 | SQLite 동시 접근 | WAL 모드 + 각자 커넥션 + 쓰기 충돌 재시도 |
| 005 | 녹화 저장 위치 | 카메라별이 아니라 시스템 전역 설정 (`SystemConfig`) |
| 006 | 운영 범위 | OS 네이티브 데몬(launchd/서비스), Phase 1 인증 없음 |

## Phase 1 스켈레톤 범위 (2026-10-12)

구조(타입/시그니처/모듈 경계)만 확정하고 구현은 전부 보류한 상태. 모든 함수 본문은
`Result<T>::Err(Error{ErrorCode::NotImplemented, ...})`를 반환한다(예외 없음 —
→ [[docs/decisions/001-error-handling-result-type]]). 유일한 예외는 `domain/result.h` —
모든 스텁이 이걸 통해 에러를 반환하므로 실제로 동작해야 한다.

다음 단계(Phase 1 구현)에서 시그니처가 바뀌지 않는 것이 목표. 실제 비즈니스 로직(RTSP 디코드,
SQLite 쿼리, FFmpeg mux, IPC 소켓 등)은 이 스켈레톤 위에 TDD로 채워나간다
(→ CLAUDE.md의 `test-driven-development` 스킬).
