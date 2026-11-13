import { initializeApp, getApps } from 'firebase/app';
import { getDatabase, ref, onValue, set, push } from 'firebase/database';
import { SensorData, RemoteConfig } from './types';

// Default Firebase Configuration (Update with your project credentials)
const firebaseConfig = {
  databaseURL: "https://smart-classroom-bubt-default-rtdb.firebaseio.com",
  projectId: "smart-classroom-bubt",
};

const app = getApps().length === 0 ? initializeApp(firebaseConfig) : getApps()[0];
export const db = getDatabase(app);

export const subscribeToRoomStatus = (roomId: string, callback: (data: Partial<SensorData>) => void) => {
  try {
    const statusRef = ref(db, `rooms/${roomId}/status`);
    return onValue(statusRef, (snapshot) => {
      if (snapshot.exists()) {
        callback(snapshot.val());
      }
    });
  } catch (err) {
    console.warn("Firebase offline or not configured yet — using local state.", err);
    return () => {};
  }
};

export const pushRemoteConfig = async (roomId: string, config: RemoteConfig) => {
  try {
    const configRef = ref(db, `rooms/${roomId}/config`);
    await set(configRef, {
      ...config,
      updatedAt: Date.now()
    });
    return true;
  } catch (err) {
    console.error("Failed to push config to Firebase", err);
    return false;
  }
};

export const pushEventLog = async (roomId: string, event: { type: string; title: string; details: string }) => {
  try {
    const eventsRef = ref(db, `rooms/${roomId}/events`);
    await push(eventsRef, {
      ...event,
      timestamp: Date.now(),
      source: 'web_dashboard'
    });
  } catch (err) {
    console.warn("Event push to Firebase skipped (offline mode)", err);
  }
};

