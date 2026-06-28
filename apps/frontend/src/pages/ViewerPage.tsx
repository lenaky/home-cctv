import { useEffect, useState, useCallback } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import type { Camera, StreamInfo } from '../types'
import { StreamStatus } from '../types'
import { camerasApi } from '../api/cameras'
import { streamsApi } from '../api/streams'
import HlsPlayer from '../components/HlsPlayer'

function formatBitrate(bps: number): string {
  if (bps <= 0) return '-'
  if (bps >= 1_000_000) return `${(bps / 1_000_000).toFixed(1)} Mbps`
  return `${(bps / 1_000).toFixed(0)} Kbps`
}

export default function ViewerPage() {
  const { cameraId } = useParams<{ cameraId: string }>()
  const navigate = useNavigate()
  const [camera, setCamera] = useState<Camera | null>(null)
  const [streamInfo, setStreamInfo] = useState<StreamInfo | null>(null)
  const [error, setError] = useState('')
  const [liveBitrate, setLiveBitrate] = useState(0)

  useEffect(() => {
    if (!cameraId) return

    const fetchAll = async () => {
      try {
        const [cam, info] = await Promise.all([
          camerasApi.get(cameraId),
          streamsApi.status(cameraId),
        ])
        setCamera(cam)
        setStreamInfo(info)
      } catch (e) {
        setError(e instanceof Error ? e.message : String(e))
      }
    }

    void fetchAll()
    const id = setInterval(() => void fetchAll(), 3000)
    return () => clearInterval(id)
  }, [cameraId])

  const handleBitrateUpdate = useCallback((bps: number) => setLiveBitrate(bps), [])

  const isStreaming = streamInfo?.status === StreamStatus.Streaming
  const hlsSrc = isStreaming ? streamInfo.hls_playlist_url : ''

  // Use live bitrate from hls.js if available, fall back to server-reported
  const displayBitrate = liveBitrate > 0 ? liveBitrate : (streamInfo?.bitrate ?? 0)

  const stats = isStreaming && streamInfo
    ? [
        { label: '코덱', value: streamInfo.codec_name || '-' },
        { label: '해상도', value: streamInfo.width ? `${streamInfo.width}×${streamInfo.height}` : '-' },
        { label: 'FPS', value: streamInfo.fps > 0 ? streamInfo.fps.toFixed(1) : '-' },
        { label: '비트레이트', value: formatBitrate(displayBitrate) },
      ]
    : []

  return (
    <div className="max-w-5xl mx-auto">
      <div className="flex items-center gap-3 mb-4">
        <button
          onClick={() => navigate('/admin')}
          className="text-gray-400 hover:text-white text-sm transition-colors"
        >
          &larr; 목록
        </button>
        <h1 className="text-lg font-bold text-white">{camera?.name ?? '로딩 중...'}</h1>
        {streamInfo && (
          <span className={`text-xs px-2 py-0.5 rounded-full ${
            isStreaming ? 'bg-green-500 text-white' : 'bg-gray-600 text-gray-300'
          }`}>
            {isStreaming ? 'LIVE' : '대기 중'}
          </span>
        )}
      </div>

      {error && (
        <div className="bg-red-900/40 border border-red-700 text-red-300 px-4 py-3 rounded mb-4 text-sm">
          {error}
        </div>
      )}

      <div className="bg-black rounded-xl overflow-hidden aspect-video">
        {isStreaming ? (
          <HlsPlayer
            src={hlsSrc}
            className="w-full h-full"
            onBitrateUpdate={handleBitrateUpdate}
          />
        ) : (
          <div className="flex items-center justify-center h-full text-gray-600">
            <p>스트림을 시작하려면 카메라 관리 페이지에서 시작하세요</p>
          </div>
        )}
      </div>

      {stats.length > 0 && (
        <div className="mt-4 grid grid-cols-4 gap-3">
          {stats.map(({ label, value }) => (
            <div key={label} className="bg-gray-900 border border-gray-800 rounded-lg px-3 py-2 text-center">
              <p className="text-xs text-gray-500">{label}</p>
              <p className="text-sm font-mono font-medium text-white">{value}</p>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}
