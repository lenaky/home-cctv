import { useState } from 'react'
import type { Camera, CameraSettings } from '../types'
import { RtspTransport, defaultSettings } from '../types'

interface CameraFormProps {
  initial?: Camera
  onSubmit: (data: { name: string; rtsp_url: string; settings: CameraSettings }) => Promise<void>
  onCancel: () => void
}

export default function CameraForm({ initial, onSubmit, onCancel }: CameraFormProps) {
  const [name, setName] = useState(initial?.name ?? '')
  const [rtspUrl, setRtspUrl] = useState(initial?.rtsp_url ?? '')
  const [settings, setSettings] = useState<CameraSettings>(initial?.settings ?? defaultSettings())
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState('')

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')
    setLoading(true)
    try {
      await onSubmit({ name, rtsp_url: rtspUrl, settings })
    } catch (err) {
      setError(err instanceof Error ? err.message : String(err))
    } finally {
      setLoading(false)
    }
  }

  return (
    <form onSubmit={handleSubmit} className="space-y-4">
      {error && (
        <div className="bg-red-900/40 border border-red-700 text-red-300 px-3 py-2 rounded text-sm">
          {error}
        </div>
      )}

      <div>
        <label className="block text-sm text-gray-400 mb-1">카메라 이름</label>
        <input
          className="w-full bg-gray-800 border border-gray-700 rounded px-3 py-2 text-sm focus:outline-none focus:border-blue-500"
          value={name}
          onChange={e => setName(e.target.value)}
          placeholder="거실 카메라"
          required
        />
      </div>

      <div>
        <label className="block text-sm text-gray-400 mb-1">RTSP URL</label>
        <input
          className="w-full bg-gray-800 border border-gray-700 rounded px-3 py-2 text-sm font-mono focus:outline-none focus:border-blue-500"
          value={rtspUrl}
          onChange={e => setRtspUrl(e.target.value)}
          placeholder="rtsp://192.168.1.100:554/stream"
          required
        />
      </div>

      <div className="grid grid-cols-2 gap-4">
        <div>
          <label className="block text-sm text-gray-400 mb-1">전송 방식</label>
          <select
            className="w-full bg-gray-800 border border-gray-700 rounded px-3 py-2 text-sm focus:outline-none focus:border-blue-500"
            value={settings.transport}
            onChange={e => setSettings(s => ({ ...s, transport: Number(e.target.value) }))}
          >
            <option value={RtspTransport.Tcp}>TCP (권장)</option>
            <option value={RtspTransport.Udp}>UDP</option>
          </select>
        </div>

        <div>
          <label className="block text-sm text-gray-400 mb-1">세그먼트 길이 (초)</label>
          <input
            type="number"
            min={1}
            max={10}
            className="w-full bg-gray-800 border border-gray-700 rounded px-3 py-2 text-sm focus:outline-none focus:border-blue-500"
            value={settings.segment_duration_sec}
            onChange={e => setSettings(s => ({ ...s, segment_duration_sec: Number(e.target.value) }))}
          />
        </div>
      </div>

      <div className="flex items-center gap-2">
        <input
          id="recording"
          type="checkbox"
          className="accent-blue-500"
          checked={settings.recording_enabled}
          onChange={e => setSettings(s => ({ ...s, recording_enabled: e.target.checked }))}
        />
        <label htmlFor="recording" className="text-sm text-gray-400">녹화 활성화</label>
      </div>

      {settings.recording_enabled && (
        <div>
          <label className="block text-sm text-gray-400 mb-1">녹화 경로 (비워두면 기본값)</label>
          <input
            className="w-full bg-gray-800 border border-gray-700 rounded px-3 py-2 text-sm font-mono focus:outline-none focus:border-blue-500"
            value={settings.recording_path}
            onChange={e => setSettings(s => ({ ...s, recording_path: e.target.value }))}
            placeholder="/mnt/storage/recordings"
          />
        </div>
      )}

      <div className="flex gap-3 pt-2">
        <button
          type="submit"
          disabled={loading}
          className="bg-blue-600 hover:bg-blue-500 disabled:opacity-50 px-4 py-2 rounded text-sm font-medium transition-colors"
        >
          {loading ? '저장 중...' : initial ? '수정' : '추가'}
        </button>
        <button
          type="button"
          onClick={onCancel}
          className="bg-gray-700 hover:bg-gray-600 px-4 py-2 rounded text-sm font-medium transition-colors"
        >
          취소
        </button>
      </div>
    </form>
  )
}
