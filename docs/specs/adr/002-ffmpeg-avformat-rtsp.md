# ADR 002 — FFmpeg avformat for RTSP ingestion

**날짜:** 2026-06-28  
**상태:** 확정

## 결정

RTSP 수신을 직접 구현하지 않고 FFmpeg `libavformat`을 사용한다.

## 이유

- RTSP TCP/UDP 전환 옵션 한 줄로 처리 (`rtsp_transport`)
- UDP RTP 패킷 jitter buffer, 재조립, 순서 보정 내장
- H.264, H.265, MJPEG 등 코덱 파악 자동화 (`avformat_find_stream_info`)
- 동일한 avformat 파이프라인으로 HLS muxer, fMP4 muxer 모두 처리 가능
- live555, 직접 RFC 2326 구현 대비 유지보수 비용 없음

## 거절한 대안

| 대안 | 거절 이유 |
|------|-----------|
| live555 | 오래된 C 스타일 API, 별도 muxer 연동 필요 |
| 직접 RTSP 구현 | 구현량 과대, jitter buffer 직접 구현 필요 |

## 제약

- FFmpeg LGPL 라이선스 — dynamic linking으로 MIT 프로젝트와 공존
- vcpkg `ffmpeg` 패키지로 관리
