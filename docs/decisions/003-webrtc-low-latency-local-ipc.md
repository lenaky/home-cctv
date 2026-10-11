# 003 — WebRTC 저지연 경로: 로컬 IPC로 실시간 패킷 직접 전달

- **날짜:** 2026-10-11
- **관련 코드:** `services/ingestion-server/domain/ports/live_packet_feed.h`,
  `services/ingestion-server/adapters/outbound/ipc/*`,
  `services/egress-server/adapters/outbound/webrtc/ffmpeg_webrtc_sender.h/.cpp`

## 상황 (Context)

WebRTC egress(Phase 2)의 미디어 소스를 어디서 가져올지 결정이 필요했다. `egress-server`는
`ingestion-server`가 공유 파일시스템에 쓰는 LL-HLS 세그먼트를 읽을 수도 있고, 별도 경로로
실시간 패킷을 받을 수도 있었다. WebRTC를 도입하는 목적이 무엇인지가 핵심 변수였다.

## 검토한 대안

1. HLS 세그먼트 재사용 — 구현 단순. 그러나 세그먼트 길이(수백 ms~수 초)만큼 지연이 추가돼
   WebRTC의 저지연 장점이 사라진다.
2. ingestion→egress 로컬 IPC(Unix Domain Socket / Windows Named Pipe)로 인코딩된 패킷을
   카메라별로 실시간 전달 — 구현 복잡도는 올라가지만 진짜 저지연을 달성.

## 결정

옵션 2. 카메라별 로컬 IPC 채널(`ILivePacketFeed` 포트, macOS는 UDS, Windows는 Named
Pipe 구현체)로 ingestion이 패킷을 publish하고, egress의 `FfmpegWebRtcSender`가 세션
생성 시 연결해서 소비한다.

## 근거 (왜)

사용자가 WebRTC 도입 목적이 "초저지연"이라고 명시적으로 확인함. 세그먼트 재사용 방식은
이 목적과 직접 충돌하므로 제외.

소켓 경로/파이프 이름은 카메라 ID로부터 결정되는 고정 네이밍 규칙으로 두 서비스가 코드
의존 없이 맞출 수 있게 한다 (→ [[002-ingestion-egress-full-separation]]과 일관,
타입/코드 공유 없이 런타임 계약만 공유).

## 재검토 조건

플랫폼별 IPC 구현(UDS/Named Pipe) 유지보수 비용이 너무 커지면, 로컬 전용 경량 프로토콜
(예: 로컬 TCP 루프백)로 단순화할지 재검토.
