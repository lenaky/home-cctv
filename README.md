# home-cctv

A self-hosted CCTV / Video Management System (VMS) that receives RTSP streams, segments them to Low-Latency HLS (LL-HLS), and serves them to a browser via a React frontend.

**Target platforms:** macOS, Windows  
**Live latency:** ~1 second (LL-HLS)

---

## Features

- **RTSP ingestion** — TCP/UDP transport, auto-reconnect, HEVC/H.264 passthrough
- **LL-HLS output** — fMP4 fragments, `#EXT-X-PART` byte-range parts, blocking playlist reload
- **Camera CRUD** — add/edit/delete cameras via REST API or Admin UI
- **Live viewer** — browser-based player with sub-second latency
- **fMP4 recording** — zero-transcode recording to disk for H.264/H.265 streams
- **SQLite persistence** — camera registry survives server restarts

---

## Supported cameras

| Manufacturer | Model   | Protocol | Notes |
|--------------|---------|----------|-------|
| IPTIME       | C500G   | RTSP/TCP | HEVC Main, 2880×1620 @ 20fps |

> PRs adding more cameras are welcome. Include the camera model, RTSP URL format, codec, and resolution.

---

## Architecture

```
RTSP camera
  └─► RtspReceiver (FFmpeg avformat)
        └─► LlHlsWriter (fMP4 fragments → ~/.home-cctv/hls/<id>/)

Browser ──► GET /hls/<id>/index.m3u8  ──► egress-server (blocking LL-HLS)
Browser ──► GET /hls/<id>/seg*.mp4    ──► egress-server (byte-range static)
Browser ──► GET /api/cameras          ──► ingestion-server (REST)
```

Two C++ servers share an HLS directory on the local filesystem:

| Server | Port | Role |
|--------|------|------|
| `ingestion-server` | 8080 | RTSP → fMP4 → HLS; camera REST API |
| `egress-server` | 8090 | LL-HLS playlist (blocking reload) + static segments |

The `apps/frontend` React app proxies `/api` → 8080 and `/hls` → 8090 during development.

A hexagonal architecture is used in `ingestion-server`: domain entities and ports are free of FFmpeg/SQLite dependencies; adapters wire everything together in `main.cpp`.

---

## Quick start

### Prerequisites

- CMake ≥ 3.25, Ninja
- [vcpkg](https://github.com/microsoft/vcpkg) with `VCPKG_ROOT` set
- macOS: Xcode Command Line Tools (Clang)
- Windows: Visual Studio 2022 (MSVC)
- Node.js ≥ 18 (frontend only)

### Build

```bash
# macOS
cmake --preset macos-debug
cmake --build --preset macos-debug

# Windows (VS Developer Command Prompt)
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Binaries are placed in `build/<preset>/apps/ingestion-server/` and `build/<preset>/apps/egress-server/`.

### Run

```bash
# Terminal 1 — ingestion server
INGESTION_PORT=8080 EGRESS_BASE_URL=http://localhost:8090 ./ingestion-server

# Terminal 2 — egress server
EGRESS_PORT=8090 ./egress-server

# Terminal 3 — frontend dev server
cd apps/frontend && npm install && npm run dev
```

Open `http://localhost:5173` → **Admin** → add a camera → **Live**.

Data is stored in `~/.home-cctv/` (macOS) or `%APPDATA%\home-cctv\` (Windows). Override paths via environment variables — see [CLAUDE.md](CLAUDE.md).

---

## Dependencies

Managed via vcpkg (`vcpkg.json`):

| Library | Use |
|---------|-----|
| FFmpeg | RTSP demux, fMP4 mux |
| Boost.Asio | Async I/O (Phase 2) |
| cpp-httplib | HTTP server (REST + static files) |
| SQLite3 | Camera registry |
| nlohmann/json | JSON serialization |
| spdlog | Logging |

Frontend: React, hls.js, Tailwind CSS, Vite.

---

## Roadmap

| Phase | Status | Scope |
|-------|--------|-------|
| 1 | ✅ Done | RTSP → LL-HLS pipeline, camera CRUD, live viewer, fMP4 recording |
| 2 | Planned | WebRTC egress, gRPC inter-server, JWT auth |
| 3 | Planned | RTMP/WebRTC ingestion, transcoding, multi-node |

---

## License

MIT — see [LICENSE](LICENSE).
