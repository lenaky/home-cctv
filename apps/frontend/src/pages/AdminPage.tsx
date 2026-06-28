import { useState, useEffect, useCallback } from 'react'
import type { Camera } from '../types'
import { camerasApi } from '../api/cameras'
import { streamsApi } from '../api/streams'
import CameraCard from '../components/CameraCard'
import CameraForm from '../components/CameraForm'
import { defaultSettings } from '../types'

type Modal = { type: 'add' } | { type: 'edit'; camera: Camera } | null

export default function AdminPage() {
  const [cameras, setCameras] = useState<Camera[]>([])
  const [modal, setModal] = useState<Modal>(null)
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState('')

  const fetchCameras = useCallback(async () => {
    try {
      const data = await camerasApi.list()
      setCameras(data)
      setError('')
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e))
    } finally {
      setLoading(false)
    }
  }, [])

  useEffect(() => {
    void fetchCameras()
    const id = setInterval(() => void fetchCameras(), 5000)
    return () => clearInterval(id)
  }, [fetchCameras])

  const handleCreate = async (data: Parameters<typeof camerasApi.create>[0]) => {
    await camerasApi.create(data)
    setModal(null)
    await fetchCameras()
  }

  const handleUpdate = async (id: string, data: Parameters<typeof camerasApi.update>[1]) => {
    await camerasApi.update(id, data)
    setModal(null)
    await fetchCameras()
  }

  const handleDelete = async (id: string) => {
    if (!confirm('카메라를 삭제하시겠습니까?')) return
    await camerasApi.delete(id)
    await fetchCameras()
  }

  const handleStart = async (id: string) => {
    await streamsApi.start(id)
    await fetchCameras()
  }

  const handleStop = async (id: string) => {
    await streamsApi.stop(id)
    await fetchCameras()
  }

  return (
    <div className="max-w-4xl mx-auto">
      <div className="flex items-center justify-between mb-6">
        <h1 className="text-xl font-bold text-white">카메라 관리</h1>
        <button
          onClick={() => setModal({ type: 'add' })}
          className="bg-blue-600 hover:bg-blue-500 px-4 py-2 rounded text-sm font-medium transition-colors"
        >
          + 카메라 추가
        </button>
      </div>

      {error && (
        <div className="bg-red-900/40 border border-red-700 text-red-300 px-4 py-3 rounded mb-4 text-sm">
          {error}
        </div>
      )}

      {loading ? (
        <p className="text-gray-500 text-sm">불러오는 중...</p>
      ) : cameras.length === 0 ? (
        <div className="text-center py-16 text-gray-600">
          <p className="text-lg mb-2">등록된 카메라가 없습니다</p>
          <p className="text-sm">오른쪽 상단 버튼으로 카메라를 추가하세요</p>
        </div>
      ) : (
        <div className="grid gap-4 sm:grid-cols-2">
          {cameras.map(cam => (
            <CameraCard
              key={cam.id}
              camera={cam}
              onStart={() => void handleStart(cam.id)}
              onStop={() => void handleStop(cam.id)}
              onEdit={() => setModal({ type: 'edit', camera: cam })}
              onDelete={() => void handleDelete(cam.id)}
            />
          ))}
        </div>
      )}

      {modal && (
        <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50 p-4">
          <div className="bg-gray-900 border border-gray-700 rounded-xl p-6 w-full max-w-lg">
            <h2 className="text-lg font-semibold mb-4">
              {modal.type === 'add' ? '카메라 추가' : '카메라 편집'}
            </h2>
            {modal.type === 'add' ? (
              <CameraForm
                onSubmit={d => handleCreate({ ...d, settings: d.settings ?? defaultSettings() })}
                onCancel={() => setModal(null)}
              />
            ) : (
              <CameraForm
                initial={modal.camera}
                onSubmit={d => handleUpdate(modal.camera.id, d)}
                onCancel={() => setModal(null)}
              />
            )}
          </div>
        </div>
      )}
    </div>
  )
}
