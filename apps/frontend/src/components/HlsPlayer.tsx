import { useEffect, useRef, useState, useCallback } from 'react'
import Hls from 'hls.js'

interface HlsPlayerProps {
  src: string
  className?: string
  onBitrateUpdate?: (bps: number) => void
}

// --- SVG icons ---
const IconPlay = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-6 h-6">
    <path d="M8 5v14l11-7z" />
  </svg>
)
const IconPause = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-6 h-6">
    <path d="M6 19h4V5H6zm8-14v14h4V5z" />
  </svg>
)
const IconVolume = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-5 h-5">
    <path d="M3 9v6h4l5 5V4L7 9H3zm13.5 3c0-1.77-1.02-3.29-2.5-4.03v8.05c1.48-.73 2.5-2.25 2.5-4.02z" />
  </svg>
)
const IconVolumeOff = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-5 h-5">
    <path d="M16.5 12c0-1.77-1.02-3.29-2.5-4.03v2.21l2.45 2.45c.03-.2.05-.41.05-.63zm2.5 0c0 .94-.2 1.82-.54 2.64l1.51 1.51C20.63 14.91 21 13.5 21 12c0-4.28-2.99-7.86-7-8.77v2.06c2.89.86 5 3.54 5 6.71zM4.27 3L3 4.27 7.73 9H3v6h4l5 5v-6.73l4.25 4.25c-.67.52-1.42.93-2.25 1.18v2.06c1.38-.31 2.63-.95 3.69-1.81L19.73 21 21 19.73l-9-9L4.27 3zM12 4L9.91 6.09 12 8.18V4z" />
  </svg>
)
const IconFullscreen = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-5 h-5">
    <path d="M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z" />
  </svg>
)
const IconFullscreenExit = () => (
  <svg viewBox="0 0 24 24" fill="currentColor" className="w-5 h-5">
    <path d="M5 16h3v3h2v-5H5v2zm3-8H5v2h5V5H8v3zm6 11h2v-3h3v-2h-5v5zm2-11V5h-2v5h5V8h-3z" />
  </svg>
)

export default function HlsPlayer({ src, className = '', onBitrateUpdate }: HlsPlayerProps) {
  const containerRef = useRef<HTMLDivElement>(null)
  const videoRef = useRef<HTMLVideoElement>(null)
  const hlsRef = useRef<Hls | null>(null)
  const hideTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null)
  const bitrateIntervalRef = useRef<ReturnType<typeof setInterval> | null>(null)

  const [playing, setPlaying] = useState(false)
  const [muted, setMuted] = useState(true)
  const [volume, setVolume] = useState(1)
  const [showVolume, setShowVolume] = useState(false)
  const [fullscreen, setFullscreen] = useState(false)
  const [showControls, setShowControls] = useState(true)

  // Auto-hide controls after 3s of inactivity
  const resetHideTimer = useCallback(() => {
    setShowControls(true)
    if (hideTimerRef.current) clearTimeout(hideTimerRef.current)
    hideTimerRef.current = setTimeout(() => setShowControls(false), 3000)
  }, [])

  const handleMouseMove = useCallback(() => resetHideTimer(), [resetHideTimer])
  const handleMouseLeave = useCallback(() => {
    if (hideTimerRef.current) clearTimeout(hideTimerRef.current)
    setShowControls(false)
    setShowVolume(false)
  }, [])

  // hls.js setup
  useEffect(() => {
    const video = videoRef.current
    if (!video || !src) return

    const onPlay = () => setPlaying(true)
    const onPause = () => setPlaying(false)
    video.addEventListener('play', onPlay)
    video.addEventListener('pause', onPause)

    if (Hls.isSupported()) {
      const hls = new Hls({
        lowLatencyMode: true,
        // PART-HOLD-BACK=0.6s → target latency must be ≥ 0.6s
        // 3 × PART-TARGET(0.2s) = 0.6s → aligns with hold-back
        liveSyncDurationCount: 3,
        // Give 3s headroom before catch-up kicks in; prevents constant seek attempts
        liveMaxLatencyDurationCount: 15,
        liveDurationInfinity: true,
        enableWorker: true,
        // Limit buffer to avoid drifting further from live edge
        maxBufferLength: 4,
        maxMaxBufferLength: 4,
      })
      hlsRef.current = hls
      hls.loadSource(src)
      hls.attachMedia(video)
      hls.on(Hls.Events.MANIFEST_PARSED, () => void video.play())
      hls.on(Hls.Events.ERROR, (_e, data) => {
        if (!data.fatal) return
        if (data.type === Hls.ErrorTypes.MEDIA_ERROR) hls.recoverMediaError()
        else if (data.type === Hls.ErrorTypes.NETWORK_ERROR) hls.startLoad()
        else hls.destroy()
      })
      if (onBitrateUpdate) {
        let bpsAccum = 0
        let fragCount = 0
        bitrateIntervalRef.current = setInterval(() => {
          if (fragCount > 0) {
            onBitrateUpdate(bpsAccum / fragCount)
            bpsAccum = 0
            fragCount = 0
          }
        }, 1000)
        hls.on(Hls.Events.FRAG_LOADED, (_e, data) => {
          const dur = data.frag.duration
          const loaded = data.frag.stats.loaded
          if (dur > 0 && loaded > 0) {
            bpsAccum += (loaded * 8) / dur
            fragCount++
          }
        })
      }
    } else if (video.canPlayType('application/vnd.apple.mpegurl')) {
      video.src = src
      video.addEventListener('loadedmetadata', () => void video.play())
    }

    resetHideTimer()

    return () => {
      video.removeEventListener('play', onPlay)
      video.removeEventListener('pause', onPause)
      if (bitrateIntervalRef.current) {
        clearInterval(bitrateIntervalRef.current)
        bitrateIntervalRef.current = null
      }
      hlsRef.current?.destroy()
      hlsRef.current = null
      if (hideTimerRef.current) clearTimeout(hideTimerRef.current)
    }
  }, [src, onBitrateUpdate, resetHideTimer])

  // Fullscreen change listener
  useEffect(() => {
    const onFsChange = () => setFullscreen(!!document.fullscreenElement)
    document.addEventListener('fullscreenchange', onFsChange)
    return () => document.removeEventListener('fullscreenchange', onFsChange)
  }, [])

  const togglePlay = () => {
    const v = videoRef.current
    if (!v) return
    v.paused ? void v.play() : v.pause()
  }

  const toggleMute = () => {
    const v = videoRef.current
    if (!v) return
    v.muted = !v.muted
    setMuted(v.muted)
  }

  const changeVolume = (val: number) => {
    const v = videoRef.current
    if (!v) return
    v.volume = val
    v.muted = val === 0
    setVolume(val)
    setMuted(val === 0)
  }

  const jumpToLive = () => {
    const v = videoRef.current
    if (!v || !isFinite(v.duration)) return
    v.currentTime = v.duration
    void v.play()
  }

  const toggleFullscreen = () => {
    const el = containerRef.current
    if (!el) return
    if (!document.fullscreenElement) void el.requestFullscreen()
    else void document.exitFullscreen()
  }

  // Click on video toggles play (YouTube behavior)
  const handleVideoClick = () => {
    togglePlay()
    resetHideTimer()
  }

  const controlsVisible = showControls || showVolume

  return (
    <div
      ref={containerRef}
      className={`relative bg-black select-none ${className}`}
      onMouseMove={handleMouseMove}
      onMouseLeave={handleMouseLeave}
    >
      <video
        ref={videoRef}
        className="w-full h-full"
        muted={muted}
        playsInline
        onClick={handleVideoClick}
        style={{ cursor: controlsVisible ? 'default' : 'none' }}
      />

      {/* Bottom gradient + controls */}
      <div
        className="absolute inset-x-0 bottom-0 transition-opacity duration-200"
        style={{ opacity: controlsVisible ? 1 : 0, pointerEvents: controlsVisible ? 'auto' : 'none' }}
      >
        {/* Gradient */}
        <div className="h-20 bg-gradient-to-t from-black/80 to-transparent" />

        {/* Controls bar */}
        <div className="absolute bottom-0 left-0 right-0 flex items-center gap-3 px-4 pb-3">
          {/* Play / Pause */}
          <button
            onClick={togglePlay}
            className="text-white hover:text-gray-200 transition-colors flex-shrink-0"
          >
            {playing ? <IconPause /> : <IconPlay />}
          </button>

          {/* Volume */}
          <div
            className="flex items-center gap-2 flex-shrink-0"
            onMouseEnter={() => setShowVolume(true)}
            onMouseLeave={() => setShowVolume(false)}
          >
            <button onClick={toggleMute} className="text-white hover:text-gray-200 transition-colors">
              {muted || volume === 0 ? <IconVolumeOff /> : <IconVolume />}
            </button>
            <div
              className="overflow-hidden transition-all duration-200"
              style={{ width: showVolume ? '72px' : '0px' }}
            >
              <input
                type="range"
                min={0}
                max={1}
                step={0.05}
                value={muted ? 0 : volume}
                onChange={e => changeVolume(Number(e.target.value))}
                className="w-[72px] h-1 accent-white cursor-pointer"
              />
            </div>
          </div>

          {/* LIVE badge */}
          <button
            onClick={jumpToLive}
            className="flex items-center gap-1.5 flex-shrink-0 group"
            title="라이브로 이동"
          >
            <span className="w-2 h-2 rounded-full bg-red-500 group-hover:bg-red-400" />
            <span className="text-white text-sm font-bold tracking-wide">LIVE</span>
          </button>

          <div className="flex-1" />

          {/* Fullscreen */}
          <button
            onClick={toggleFullscreen}
            className="text-white hover:text-gray-200 transition-colors flex-shrink-0"
          >
            {fullscreen ? <IconFullscreenExit /> : <IconFullscreen />}
          </button>
        </div>
      </div>
    </div>
  )
}
