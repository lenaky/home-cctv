# ADR 001 — Hexagonal Architecture

**날짜:** 2026-06-28  
**상태:** 확정

## 결정

ingestion-server의 내부 구조를 헥사고날 아키텍처(Ports & Adapters)로 설계한다.

## 구조

```
domain/entities/    → 순수 도메인 모델 (외부 의존성 없음)
domain/ports/       → 인터페이스 정의 (inbound: 유스케이스, outbound: 외부 시스템)
application/        → inbound 포트를 구현하는 유스케이스
adapters/inbound/   → 외부에서 애플리케이션으로 들어오는 어댑터 (HTTP, RTSP)
adapters/outbound/  → 애플리케이션에서 외부로 나가는 어댑터 (SQLite, HLS, fMP4, 이벤트)
main.cpp            → Composition Root (모든 의존성 조립)
```

## 의존성 방향 규칙

```
adapters → application → domain
                        ↑
                   (ports만 의존)
```

domain은 어떤 외부 라이브러리도 include하지 않는다.  
adapters는 domain ports에만 의존하고, 다른 adapter를 직접 의존하지 않는다.

## 이유

- 테스트 시 adapter를 mock으로 교체 가능
- 새로운 ingestion 포맷(WebRTC, RTMP) 추가 시 도메인 변경 없이 adapter만 추가
- 새로운 egress 방식(WebRTC) 추가 시 동일한 ISegmentWriter 포트 활용
- Claude와 자연어로 기능 범위를 논의할 때 레이어 경계가 명확한 기준이 됨
