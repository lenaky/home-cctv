# ADR 003 — SQLite + fMP4 storage strategy

**날짜:** 2026-06-28  
**상태:** 확정

## 결정 1: 설정 저장소 — SQLite3 (WAL 모드)

카메라 설정, 상태, 에러 이력을 SQLite3에 저장한다.

**이유:**
- 단일 파일, 인프라 없음 (PostgreSQL 설치 불필요)
- WAL 모드로 동시 읽기/쓰기 안전
- `ICameraRepository` 인터페이스로 추상화 — Phase 3 수평 확장 시 PostgreSQL 어댑터로 교체 가능

## 결정 2: 녹화 포맷 — fMP4 조건부 remux

| 조건 | 저장 방식 |
|------|-----------|
| 코덱이 H.264 또는 H.265 | fMP4 (`.mp4`) remux — 재인코딩 없음 |
| 그 외 코덱 | MPEG-TS (`.ts`) 원본 passthrough |

**이유:**
- 재인코딩 없이 CPU 부담 최소화
- fMP4는 브라우저 MSE(Media Source Extensions) 직접 재생 가능
- 비호환 코덱은 원본 보존 우선 (트랜스코딩은 Phase 2+)
- 세그먼트 단위(기본 1시간)는 frontend에서 설정 가능

**fMP4 movflags:**
```
frag_keyframe+empty_moov+default_base_moof+faststart
```
