# 005 — 녹화 저장 위치는 시스템 전역 설정 (카메라별 아님)

- **날짜:** 2026-10-11
- **관련 코드:** `services/ingestion-server/domain/system_config.h`,
  `services/ingestion-server/domain/camera.h`

## 상황 (Context)

녹화 파일을 로컬/원격 중 어디에 저장할지를 카메라 엔티티에 둘지(`Camera.recording_storage`),
시스템 전역 설정으로 둘지 결정이 필요했다.

## 검토한 대안

1. 카메라별 설정 — 카메라마다 다른 저장 위치를 쓸 수 있어 유연하지만, 지금 요구사항에는
   그런 니즈가 없고 Camera 엔티티가 인프라 설정을 떠안게 된다.
2. 시스템 전역 설정 — 모든 카메라가 같은 저장 정책을 공유. `Camera` 엔티티는 순수 도메인
   정보(이름, RTSP 주소)만 가진다.

## 결정

옵션 2. `Camera`에서 `recording_storage` 필드를 제거하고, 별도 전역 설정
(`SystemConfig` — 기본 로컬 경로 또는 원격 타겟)을 둔다. `FfmpegIngestPipeline`과
storage writer가 이 전역 설정을 직접 참조한다.

## 근거 (왜)

사용자 확인. 지금 요구사항에 카메라별 차등 저장 니즈가 없으므로, 불필요한 유연성을
Camera 엔티티에 넣지 않는다 (CLAUDE.md의 "요구사항에 없는 추상화를 만들지 마라" 원칙과
일치).

## 재검토 조건

특정 카메라만 다른 위치(예: 민감한 카메라는 로컬만, 나머지는 원격)에 저장해야 하는
요구사항이 생기면, 그때 카메라별 오버라이드를 추가.
