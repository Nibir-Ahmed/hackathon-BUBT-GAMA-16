export type DoorState = 'LOCKED' | 'UNLOCKED' | 'EMERGENCY_OPEN';

export interface SensorData {
  temperature: number;
  humidity: number;
  flameDetected: boolean;
  panicActive: boolean;
  wifiRSSI: number;
  doorState: DoorState;
  lastSeen: number;
  isOnline: boolean;
  activeAlert: 'NONE' | 'FIRE' | 'PANIC' | 'INTRUDER';
}

export interface AttendanceRecord {
  id: string;
  role: string;
  cardUID: string;
  method: 'RFID Tap' | 'Camera Face' | 'Admin Key';
  checkInTime: string;
  status: 'Verified' | 'Pending';
}

export interface EventLog {
  id: string;
  type: 'boot' | 'door_toggle' | 'rfid_student' | 'rfid_teacher' | 'fire_alarm' | 'panic_pad' | 'intruder' | 'config_push';
  title: string;
  details: string;
  timestamp: string;
  severity: 'info' | 'success' | 'warning' | 'danger';
}

export interface RemoteConfig {
  tempThresholdHigh: number;
  autoLockTimeoutSec: number;
  buzzerEnabled: boolean;
  faceDetectionEnabled: boolean;
}

