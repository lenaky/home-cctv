import { api } from './client'
import type { Camera, CameraSettings } from '../types'

export interface CreateCameraDto {
  name: string
  rtsp_url: string
  settings: CameraSettings
}

export interface UpdateCameraDto {
  name: string
  rtsp_url: string
  settings: CameraSettings
}

export const camerasApi = {
  list: () => api.get<Camera[]>('/cameras'),
  get: (id: string) => api.get<Camera>(`/cameras/${id}`),
  create: (dto: CreateCameraDto) => api.post<Camera>('/cameras', dto),
  update: (id: string, dto: UpdateCameraDto) => api.put<Camera>(`/cameras/${id}`, dto),
  delete: (id: string) => api.delete<void>(`/cameras/${id}`),
}
