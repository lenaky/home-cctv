# compound/ — 누적 교훈 로그

CLAUDE.md의 "Mandatory procedure" 3~5단계(설계 리뷰, 구현, 검증)를 거치며 얻은 교훈을
남기는 기록. `docs/learnings/`가 "검증된 패턴"을 남기는 것과 달리, 여기는 그 과정에서 겪은
실패·재시도·Opus 설계 리뷰 반려 사유 등 절차 자체에 대한 교훈을 남긴다.

새 항목 작성 시 [`TEMPLATE.md`](TEMPLATE.md) 사용. 파일명: `NNN-짧은-제목.md`.
새 항목을 추가하면 반드시 루트 `CLAUDE.md`의 "compound/ — 누적 교훈 인덱스" 섹션에도
한 줄 추가한다.

## 인덱스

| # | 제목 | 어떤 상황에 적용 가능한가 |
|---|---|---|
| 001 | [Opus 5.5 설계 리뷰 루프는 라운드가 늘수록 희귀 엣지케이스로 수렴한다](001-opus-review-loop-diminishing-returns.md) | 3단계(설계 리뷰) 반복이 3라운드를 넘기고 지적 사항이 이 리포 실제 환경과 무관해질 때 |
