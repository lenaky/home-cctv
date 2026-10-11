# 004 — SQLite 동시 접근: 공유 파일 + WAL + 각자 커넥션

- **날짜:** 2026-10-11
- **관련 코드:** `services/ingestion-server/adapters/outbound/sqlite/*`,
  `services/egress-server/adapters/outbound/sqlite/*`

## 상황 (Context)

`ingestion-server`와 `egress-server`가 같은 SQLite 파일(카메라/녹화 메타데이터)에 어떻게
접근할지 결정이 필요했다. 둘 다 같은 프로세스가 아니므로(→ [[002-ingestion-egress-full-separation]])
각자 자기 커넥션을 연다.

## 검토한 대안

1. ingestion만 쓰기, egress는 읽기 전용 커넥션 — 책임이 명확하지만, egress가 쓰기 작업
   (예: 세션/통계 기록)이 필요해지면 구조를 다시 바꿔야 한다.
2. 양쪽 모두 자기 커넥션으로 자유롭게 접근, WAL 모드 + 쓰기 충돌(SQLITE_BUSY) 방어적
   재시도 — 당장 동시 쓰기는 일어나지 않지만 구조적으로 막아두지 않는다.

## 결정

옵션 2. WAL 모드를 켜고, 양쪽 서비스가 각자 커넥션으로 접근한다. 쓰기 작업은
`SQLITE_BUSY` 발생 시 짧은 backoff 재시도를 구현한다.

## 근거 (왜)

사용자 확인: 현재 시나리오에서 동시 쓰기는 실제로 발생하지 않지만(ingestion만 녹화
메타데이터를 쓰고, egress는 조회만 함), 향후 egress 쪽에 쓰기가 필요해질 가능성을 미리
차단하지 않는 쪽을 선택. WAL 모드는 다중 리더 상황에 적합하고, 쓰기 충돌 재시도는 비용이
낮다.

## 재검토 조건

실제로 양쪽에서 빈번한 쓰기가 발생해 `SQLITE_BUSY` 재시도가 유의미한 레이턴시를
일으키면, DB를 분리하거나 쓰기를 한쪽으로 모으는 방향(옵션 1)으로 재검토.
