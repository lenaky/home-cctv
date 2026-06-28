# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Home CCTV / VMS (Video Management System) — C++ monorepo.  
Receives RTSP streams → segments to HLS → serves to browser via React frontend.

**Target platforms:** macOS + Windows (no Linux).  
**C++ standard:** C++20 (`co_await`, `std::jthread`, concepts, `std::format`).

---

## Documentation structure

```
docs/
  specs/              # 기능 스펙 (구현 전 작성 필수)
    phase-1-*.md
    adr/              # Architecture Decision Records
  impl-notes/         # 구현 후 기록 — LLM이 어려웠던 점, 애매했던 점
    phase-1-*.md
  test-log/           # 자체 테스트 로그 — 실패 케이스 및 수정 이력
    phase-1-*.md
```

---

## Spec storage

모든 기능 스펙과 설계 결정은 `docs/specs/` 디렉토리에 마크다운으로 저장한다.

```
docs/
  specs/
    phase-1-rtsp-hls.md        # Phase 1 스펙 (RTSP 수신, HLS 출력, 카메라 CRUD)
    phase-2-webrtc-grpc.md     # Phase 2 스펙 (WebRTC, gRPC, JWT)
    phase-3-transcoding.md     # Phase 3 스펙
    adr/                       # Architecture Decision Records
      001-hexagonal-arch.md
      002-ffmpeg-avformat.md
      ...
```

새로운 기능을 구현하기 전에 반드시 해당 스펙 파일을 먼저 작성하거나 업데이트한다.  
스펙이 없는 기능은 구현하지 않는다.

---

## Self-testing rule (MANDATORY)

**구현이 완료된 후에는 반드시 자체 테스트를 수행하여 정상 동작을 확인해야 한다.**

아래 절차를 순서대로 따른다:

### 1. C++ 빌드 검증
```bash
cmake --preset macos-debug
cmake --build --preset macos-debug 2>&1 | tail -20
# 빌드 에러가 없어야 한다
```

### 2. 서버 기동 확인
```bash
# 각 서버를 백그라운드로 기동 후 /health 엔드포인트로 응답 확인
curl -s http://localhost:8080/health   # ingestion → {"status":"ok"}
curl -s http://localhost:8090/health   # egress    → {"status":"ok","service":"egress"}
```

### 3. API 동작 확인
```bash
# 카메라 등록
curl -s -X POST http://localhost:8080/api/cameras \
  -H 'Content-Type: application/json' \
  -d '{"name":"test","rtsp_url":"rtsp://test","settings":{"transport":0,"segment_duration_sec":2,"hls_list_size":10,"recording_enabled":false,"recording_path":""}}' \
  | python3 -m json.tool

# 목록 조회
curl -s http://localhost:8080/api/cameras | python3 -m json.tool
```

### 4. 프론트엔드 타입 검증
```bash
cd apps/frontend && npm run build
# TypeScript 컴파일 에러가 없어야 한다
```

### 5. 기록 후 보고

테스트가 끝나면 반드시 아래 두 파일을 업데이트한다.

**`docs/impl-notes/<phase-or-feature>.md`** — 구현 중 어려웠던 점, 애매했던 결정, 미구현/스텁 항목.  
**`docs/test-log/<phase-or-feature>.md`** — 각 테스트 케이스의 결과(통과/실패), 실패 원인, 수정 내용.

실패한 항목은 수정 후 해당 로그에 "수정됨" 표시하고 재확인한다.  
"빌드 성공"만으로는 완료로 간주하지 않는다 — 반드시 런타임 동작까지 확인한다.

→ 현재까지의 impl-notes: [`docs/impl-notes/`](docs/impl-notes/)  
→ 현재까지의 test-log: [`docs/test-log/`](docs/test-log/)

---

## Build (C++ servers)

**Prerequisites:**
- CMake ≥ 3.25, Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) — set `VCPKG_ROOT` env var
- macOS: Xcode CLT / Clang; Windows: Visual Studio 2022 + MSVC

```bash
# macOS — debug
cmake --preset macos-debug
cmake --build --preset macos-debug

# macOS — release
cmake --preset macos-release
cmake --build --preset macos-release

# Windows — debug (run in VS Developer Command Prompt)
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Binaries land in `build/<preset>/apps/ingestion-server/` and `build/<preset>/apps/egress-server/`.

---

## Frontend

```bash
cd apps/frontend
npm install
npm run dev        # Vite dev server on :5173 (proxies /api → :8080, /hls → :8090)
npm run build      # Production build → dist/
```

---

## Running Phase 1

```bash
# Terminal 1 — ingestion server (REST API + RTSP → HLS)
INGESTION_PORT=8080 EGRESS_BASE_URL=http://localhost:8090 ./ingestion-server

# Terminal 2 — egress server (serves HLS segments)
EGRESS_PORT=8090 ./egress-server

# Terminal 3 — frontend dev server
cd apps/frontend && npm run dev
```

Open `http://localhost:5173` → Admin page → add RTSP camera → start stream → live viewer.

**Data directories (auto-created):**
- macOS: `~/.home-cctv/` (DB, HLS segments, recordings)
- Windows: `%APPDATA%\home-cctv\`

**Override via env:**
- `INGESTION_PORT` — ingestion HTTP API port (default: 8080)
- `EGRESS_PORT` — egress HTTP port (default: 8090)
- `EGRESS_BASE_URL` — base URL for HLS playlist URLs returned by ingestion
- `FRONTEND_DIST` — path to built frontend for egress to serve statically

---

## Architecture

### Monorepo structure

```
docs/specs/             # 기능 스펙 및 ADR (구현 전 작성 필수)
packages/core/          # Shared: Result<T,E> type, proto definitions (gRPC Phase 2)
apps/ingestion-server/  # C++ — receives RTSP, writes HLS, exposes REST API
apps/egress-server/     # C++ — serves HLS files + static frontend via HTTP
apps/frontend/          # React + TypeScript — admin UI + live viewer
```

### Hexagonal architecture (ingestion-server)

```
src/
  domain/
    entities/           # Camera, Stream — pure data, no dependencies
    ports/
      inbound/          # ICameraService, IStreamService — use case interfaces
      outbound/         # ICameraRepository, ISegmentWriter, IStreamEventPublisher
  application/          # Use case implementations: CameraService, StreamManager
  adapters/
    inbound/
      rtsp/             # RtspReceiver — FFmpeg avformat, handles TCP/UDP + reconnect
      http/             # HttpApiServer — cpp-httplib REST API + HLS file mount
    outbound/
      sqlite/           # CameraRepository — SQLite3 persistence
      hls/              # HlsSegmentWriter — FFmpeg HLS muxer
      storage/          # FMp4Writer — fMP4 remux (H.264/H.265) or raw passthrough
      events/           # LogEventPublisher — stream lifecycle events
```

**Dependency rule:** domain has zero external dependencies. Adapters depend on domain ports, never the reverse.  
**Composition root:** `apps/ingestion-server/src/main.cpp` wires all dependencies.

### Data flow (Phase 1)

```
RTSP camera
  └─► RtspReceiver (FFmpeg avformat, jitter buffer built-in)
        ├─► HlsSegmentWriter (FFmpeg HLS muxer → ~/.home-cctv/hls/<camera_id>/)
        └─► FMp4Writer (fMP4 if H.264/H.265, else raw .ts — ~/.home-cctv/recordings/)

Browser ─► GET /hls/<camera_id>/index.m3u8 ─► egress-server (shared dir)
Browser ─► GET /api/cameras ─► ingestion-server REST API
```

### Inter-server communication

- **Control plane:** REST HTTP (Phase 1) → gRPC (Phase 2, proto in `packages/core/proto/`)
- **Data plane:** shared filesystem directory (`~/.home-cctv/hls/`)

### Key design decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Async I/O | Boost.Asio + C++20 coroutines | Ready for Phase 2 WebRTC/gRPC |
| HTTP server | cpp-httplib | Single-header, simple REST + static file serving |
| Storage | SQLite3 (WAL mode) | Embedded; DAL abstracted for future PostgreSQL migration |
| RTSP | FFmpeg avformat | Built-in jitter buffer, RTP reassembly, TCP/UDP toggle |
| HLS muxer | FFmpeg HLS muxer | Reuses same avformat pipeline as ingestion |
| Recording | fMP4 remux (H.264/H.265 only) | Zero re-encode; falls back to raw .ts for other codecs |

---

## Dependencies (vcpkg)

`ffmpeg`, `grpc` (Phase 2), `boost-asio`, `boost-beast`, `unofficial-sqlite3`, `nlohmann-json`, `spdlog`, `cpp-httplib`

Frontend: `hls.js`, `react`, `react-router-dom`, `tailwindcss`

---

## Phase roadmap

| Phase | Scope |
|-------|-------|
| 1 (current) | RTSP → HLS pipeline, camera CRUD, live viewer, fMP4 recording |
| 2 | WebRTC egress (libdatachannel), gRPC inter-server, JWT auth |
| 3 | RTMP/WebRTC ingestion, transcoding, multi-node scale-out |
