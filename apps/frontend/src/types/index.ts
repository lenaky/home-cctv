export enum CameraStatus {
  Inactive = 1,
  Active = 2,
  Error = 3,
}

export enum RtspTransport {
  Tcp = 0,
  Udp = 1,
}

export enum StreamStatus {
  Idle = 1,
  Connecting = 2,
  Streaming = 3,
  Error = 4,
}

export interface CameraSettings {
  transport: RtspTransport
  segment_duration_sec: number
  hls_list_size: number
  recording_enabled: boolean
  recording_path: string
}

export interface Camera {
  id: string
  name: string
  rtsp_url: string
  status: CameraStatus
  settings: CameraSettings
  created_at: number
  updated_at: number
  last_error: string
}

export interface StreamInfo {
  camera_id: string
  status: StreamStatus
  codec_name: string
  width: number
  height: number
  fps: number
  hls_playlist_url: string
  last_error: string
}

export const defaultSettings = (): CameraSettings => ({
  transport: RtspTransport.Tcp,
  segment_duration_sec: 2,
  hls_list_size: 10,
  recording_enabled: false,
  recording_path: '',
})
