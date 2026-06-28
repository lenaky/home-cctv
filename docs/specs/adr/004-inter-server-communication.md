# ADR 004 — Inter-server communication

**날짜:** 2026-06-28  
**상태:** 확정 (Phase 1), 진화 예정

## Phase 1 결정

| 레이어 | 방식 |
|--------|------|
| 컨트롤 플레인 (카메라 설정, 스트림 제어) | REST HTTP (ingestion-server :8080) |
| 데이터 플레인 (HLS 세그먼트) | 공유 파일시스템 (`~/.home-cctv/hls/`) |

프론트엔드는 ingestion-server REST API에 직접 접근.  
egress-server는 공유 디렉토리에서 HLS 파일을 읽어 서빙.

## Phase 2 계획

| 레이어 | 방식 |
|--------|------|
| 컨트롤 플레인 | gRPC (proto 정의: `packages/core/proto/`) |
| 데이터 플레인 | 공유 파일시스템 유지 (단일 노드) |

## Phase 3 계획 (수평 확장)

| 레이어 | 방식 |
|--------|------|
| 컨트롤 플레인 | gRPC |
| 데이터 플레인 | 공유 스토리지 (NFS/S3) 또는 미디어 버스 (NATS) |

## 이유

단계적 도입. gRPC 프로토 파일은 Phase 1부터 `packages/core/proto/`에 정의해두어 Phase 2 전환 시 재설계 없이 구현만 추가.
