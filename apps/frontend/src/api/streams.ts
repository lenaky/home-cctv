import { api } from './client'
import type { StreamInfo } from '../types'

export const streamsApi = {
  start: (cameraId: string) => api.post<StreamInfo>(`/streams/${cameraId}/start`),
  stop: (cameraId: string) => api.post<void>(`/streams/${cameraId}/stop`),
  status: (cameraId: string) => api.get<StreamInfo>(`/streams/${cameraId}/status`),
}
