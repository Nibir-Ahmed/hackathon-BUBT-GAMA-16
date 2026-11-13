import { initializeApp, getApps } from 'firebase/app';
import {
  getFirestore,
  doc,
  collection,
  query,
  orderBy,
  limit,
  onSnapshot,
  setDoc,
  addDoc
} from 'firebase/firestore';
import { SensorData, RemoteConfig, EventLog } from './types';

// Firebase Project Configuration (smart-classroom-bubt)
const firebaseConfig = {
  projectId: "smart-classroom-bubt",
  authDomain: "smart-classroom-bubt.firebaseapp.com",
};

const app = getApps().length === 0 ? initializeApp(firebaseConfig) : getApps()[0];
export const firestore = getFirestore(app);

export interface TelemetryRecord {
  id: string;
  device_id: string;
  room_id: string;
  seq: number;
  ts: number;
  temp_c: number;
  rh: number;
  heat_index_c: number;
  flame: boolean;
  door: string;
  status: string;
  rssi: number;
}

export const subscribeToDeviceStatus = (
  deviceId: string,
  callback: (data: Partial<SensorData> & { room_id?: string; ts?: number; seq?: number }) => void
) => {
  try {
    const devRef = doc(firestore, 'devices', deviceId);
    return onSnapshot(devRef, (snap) => {
      if (snap.exists()) {
        const d = snap.data() as Record<string, any>;
        const tsMs = d.ts ? Number(d.ts) * 1000 : Date.now();
        const isOnline = (Date.now() - tsMs) < 25000;
        const statusStr = String(d.status || 'SAFE').toUpperCase();
        callback({
          temperature: Number(d.temp_c ?? 0),
          humidity: Number(d.rh ?? 0),
          flameDetected: Boolean(d.flame),
          doorState: String(d.door || 'LOCKED') as 'LOCKED' | 'UNLOCKED',
          wifiRSSI: Number(d.rssi ?? -127),
          lastSeen: tsMs,
          isOnline,
          activeAlert: d.flame ? 'FIRE' : 'NONE',
          room_id: d.room_id,
          ts: Number(d.ts ?? 0),
          seq: Number(d.seq ?? 0)
        });
      }
    }, (err) => {
      console.warn('Firestore device subscription error:', err);
    });
  } catch (err) {
    console.warn('Firestore offline:', err);
    return () => {};
  }
};

export const subscribeToTelemetryRecords = (
  callback: (records: TelemetryRecord[]) => void
) => {
  try {
    const q = query(collection(firestore, 'telemetry'), orderBy('seq', 'desc'), limit(20));
    return onSnapshot(q, (snap) => {
      const list: TelemetryRecord[] = snap.docs.map((docSnap) => {
        const d = docSnap.data() as Record<string, any>;
        return {
          id: docSnap.id,
          device_id: String(d.device_id || ''),
          room_id: String(d.room_id || ''),
          seq: Number(d.seq || 0),
          ts: Number(d.ts || 0),
          temp_c: Number(d.temp_c || 0),
          rh: Number(d.rh || 0),
          heat_index_c: Number(d.heat_index_c || 0),
          flame: Boolean(d.flame),
          door: String(d.door || ''),
          status: String(d.status || ''),
          rssi: Number(d.rssi || 0)
        };
      });
      callback(list);
    }, (err) => {
      console.warn('Firestore telemetry subscription error:', err);
    });
  } catch (err) {
    return () => {};
  }
};

export const subscribeToEvents = (
  callback: (events: EventLog[]) => void
) => {
  try {
    const q = query(collection(firestore, 'events'), orderBy('seq', 'desc'), limit(25));
    return onSnapshot(q, (snap) => {
      const list: EventLog[] = snap.docs.map((docSnap) => {
        const d = docSnap.data() as Record<string, any>;
        const rawType = String(d.type || 'EVENT');
        const tsSec = Number(d.ts || 0);
        const timeStr = tsSec > 1700000000
          ? new Date(tsSec * 1000).toLocaleTimeString('en-US', { timeZone: 'Asia/Dhaka' })
          : new Date().toLocaleTimeString('en-US', { timeZone: 'Asia/Dhaka' });
        return {
          id: docSnap.id,
          type: rawType.toLowerCase() as any,
          title: rawType,
          details: String(d.detail || ''),
          timestamp: timeStr,
          severity: rawType === 'FIRE' || rawType === 'PANIC' ? 'danger' : (rawType === 'BOOT' ? 'info' : 'success')
        };
      });
      callback(list);
    }, (err) => {
      console.warn('Firestore events subscription error:', err);
    });
  } catch (err) {
    return () => {};
  }
};

export const pushRemoteConfig = async (roomId: string, config: RemoteConfig) => {
  try {
    await setDoc(doc(firestore, 'configs', roomId), {
      ...config,
      updatedAt: Date.now()
    });
    return true;
  } catch (err) {
    console.error('Failed to push config to Firestore:', err);
    return false;
  }
};

export const pushEventLog = async (roomId: string, event: { type: string; title: string; details: string }) => {
  try {
    await addDoc(collection(firestore, 'events'), {
      room_id: roomId,
      type: event.type,
      detail: event.details,
      ts: Math.floor(Date.now() / 1000),
      source: 'web_dashboard'
    });
  } catch (err) {
    console.warn('Event push to Firestore skipped:', err);
  }
};
