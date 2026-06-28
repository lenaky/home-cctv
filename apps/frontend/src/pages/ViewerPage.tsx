import { useEffect, useState } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import type { Camera, StreamInfo } from '../types'
import { StreamStatus } from '../types'
import { camerasApi } from '../api/cameras'
import { streamsApi } from '../api/streams'
import HlsPlayer from '../components/HlsPlayer'

export default function ViewerPage() {
  const { cameraId } = useParams<{ cameraId: string }>()
  const navigate = useNavigate()
  const [camera, setCamera] = useState<Camera | null>(null)
  const [streamInfo, setStreamInfo] = useState<StreamInfo | null>(null)
  const [error, setError] = useState('')

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

  const isStreaming = streamInfo?.status === StreamStatus.Streaming
  const hlsSrc = isStreaming ? streamInfo.hls_playlist_url : ''

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
          <HlsPlayer src={hlsSrc} className="h-full" />
        ) : (
          <div className="flex items-center justify-center h-full text-gray-600">
            <p>스트림을 시작하려면 카메라 관리 페이지에서 시작하세요</p>
          </div>
        )}
      </div>

      {streamInfo && isStreaming && (
        <div className="mt-4 grid grid-cols-3 gap-3">
          {[
            ['코덱', streamInfo.codec_name],
            ['해상도', streamInfo.width ? `${streamInfo.width}×${streamInfo.height}` : '-'],
            ['FPS', streamInfo.fps ? streamInfo.fps.toFixed(1) : '-'],
          ].map(([label, value]) => (
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
