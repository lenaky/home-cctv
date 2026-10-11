# decisions/ — 구현 판단 로그

브레인스토밍/계획/구현 중 여러 선택지가 있었고 LLM이 하나를 골랐을 때 남기는 기록. 코드 리뷰나 이후 작업에서
"왜 이렇게 했지?"를 다시 조사하지 않도록 하는 것이 목적이다.

새 항목 작성 시 [`TEMPLATE.md`](TEMPLATE.md) 사용. 파일명: `NNN-짧은-제목.md`.

## 인덱스

| # | 제목 | 요약 |
|---|---|---|
| 001 | [에러 처리: 직접 구현한 Result<T,E> 사용](001-error-handling-result-type.md) | tl::expected/Boost.Outcome 대신 의존성 없는 직접 구현 선택 |
| 002 | [ingestion/egress 완전 분리](002-ingestion-egress-full-separation.md) | 바이너리/타입 모두 분리 유지, 독립 재시작 가능하게 |
| 003 | [WebRTC 저지연: 로컬 IPC 패킷 전달](003-webrtc-low-latency-local-ipc.md) | HLS 세그먼트 재사용 대신 UDS/Named Pipe로 실시간 패킷 전달 |
| 004 | [SQLite 동시 접근: WAL + 각자 커넥션](004-sqlite-concurrent-access-wal.md) | 읽기전용 분리 대신 WAL+재시도로 양쪽 접근 허용 |
| 005 | [녹화 저장 위치는 전역 설정](005-recording-storage-global-config.md) | Camera 엔티티에서 storage 필드 제거, SystemConfig로 이동 |
| 006 | [Phase 1 운영 범위](006-phase1-ops-scope.md) | OS 네이티브 데몬 관리(launchd/서비스), 인증은 Phase 2로 유지 |
