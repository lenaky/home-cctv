import { useNavigate } from 'react-router-dom'
import type { Camera } from '../types'
import { CameraStatus } from '../types'

const statusLabel: Record<CameraStatus, string> = {
  [CameraStatus.Inactive]: '비활성',
  [CameraStatus.Active]: '스트리밍',
  [CameraStatus.Error]: '오류',
}

const statusColor: Record<CameraStatus, string> = {
  [CameraStatus.Inactive]: 'bg-gray-600',
  [CameraStatus.Active]: 'bg-green-500',
  [CameraStatus.Error]: 'bg-red-500',
}

interface CameraCardProps {
  camera: Camera
  pending?: boolean
  connecting?: boolean
  onStart: () => void
  onStop: () => void
  onEdit: () => void
  onDelete: () => void
}

function Spinner() {
  return (
    <svg className="animate-spin h-3.5 w-3.5" viewBox="0 0 24 24" fill="none">
      <circle className="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" strokeWidth="4" />
      <path className="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8v4a4 4 0 00-4 4H4z" />
    </svg>
  )
}

export default function CameraCard({ camera, pending = false, connecting = false, onStart, onStop, onEdit, onDelete }: CameraCardProps) {
  const navigate = useNavigate()
  const isActive = camera.status === CameraStatus.Active
  const showAsActive = isActive || connecting

  const badgeColor = pending || connecting
    ? 'bg-yellow-600'
    : statusColor[camera.status]
  const badgeLabel = pending
    ? '처리중...'
    : connecting
    ? '연결 중'
    : statusLabel[camera.status]

  return (
    <div className="bg-gray-900 border border-gray-800 rounded-lg p-4 flex flex-col gap-3">
      <div className="flex items-start justify-between">
        <div>
          <h3 className="font-semibold text-white">{camera.name}</h3>
          <p className="text-xs text-gray-500 font-mono mt-0.5 break-all">{camera.rtsp_url}</p>
        </div>
        <span className={`text-xs px-2 py-0.5 rounded-full text-white shrink-0 ml-2 flex items-center gap-1.5 ${badgeColor}`}>
          {(pending || connecting) && <Spinner />}
          {badgeLabel}
        </span>
      </div>

      {camera.last_error && (
        <p className="text-xs text-red-400 bg-red-900/20 px-2 py-1 rounded">{camera.last_error}</p>
      )}

      <div className="flex gap-2 flex-wrap">
        {showAsActive ? (
          <>
            <button
              onClick={() => navigate(`/viewer/${camera.id}`)}
              disabled={pending}
              className="bg-blue-600 hover:bg-blue-500 disabled:opacity-50 disabled:cursor-not-allowed px-3 py-1.5 rounded text-xs font-medium transition-colors"
            >
              라이브 보기
            </button>
            <button
              onClick={onStop}
              disabled={pending || connecting}
              className="bg-gray-700 hover:bg-gray-600 disabled:opacity-50 disabled:cursor-not-allowed px-3 py-1.5 rounded text-xs font-medium transition-colors flex items-center gap-1.5"
            >
              {pending ? <><Spinner />중지 중</> : '중지'}
            </button>
          </>
        ) : (
          <button
            onClick={onStart}
            disabled={pending}
            className="bg-green-700 hover:bg-green-600 disabled:opacity-50 disabled:cursor-not-allowed px-3 py-1.5 rounded text-xs font-medium transition-colors flex items-center gap-1.5"
          >
            {pending ? <><Spinner />시작 중</> : '시작'}
          </button>
        )}
        <button
          onClick={onEdit}
          className="bg-gray-700 hover:bg-gray-600 px-3 py-1.5 rounded text-xs font-medium transition-colors"
        >
          편집
        </button>
        <button
          onClick={onDelete}
          className="bg-red-900/60 hover:bg-red-800 px-3 py-1.5 rounded text-xs font-medium transition-colors text-red-300"
        >
          삭제
        </button>
      </div>
    </div>
  )
}
