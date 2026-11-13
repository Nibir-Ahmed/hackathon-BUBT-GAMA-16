import React, { useState, useEffect } from 'react';
import { Navbar } from './components/Navbar';
import { EmergencyBanner } from './components/EmergencyBanner';
import { DoorStatusCard } from './components/DoorStatusCard';
import { SensorCard } from './components/SensorCard';
import { SafetyCard } from './components/SafetyCard';
import { AttendanceCard, AttendanceTable } from './components/AttendanceCard';
import { EventStreamCard } from './components/EventStreamCard';
import { RemoteConfigCard } from './components/RemoteConfigCard';
import { CameraVisionCard } from './components/CameraVisionCard';
import { SurfacesModal } from './components/SurfacesModal';
import { DoorState, SensorData, AttendanceRecord, EventLog, RemoteConfig } from './types';
import {
  subscribeToDeviceStatus,
  subscribeToTelemetryRecords,
  subscribeToEvents,
  pushRemoteConfig,
  pushEventLog,
  TelemetryRecord
} from './firebase';

export default function App() {
  const [roomId, setRoomId] = useState<string>("ROOM-301");
  const [telemetryRecords, setTelemetryRecords] = useState<TelemetryRecord[]>([]);
  const [isSurfacesModalOpen, setIsSurfacesModalOpen] = useState(false);

  // Sensor & Telemetry State
  const [sensorData, setSensorData] = useState<SensorData>({
    temperature: 26.5,
    humidity: 58.0,
    flameDetected: false,
    panicActive: false,
    wifiRSSI: -42,
    doorState: 'UNLOCKED',
    lastSeen: Date.now(),
    isOnline: true,
    activeAlert: 'NONE'
  });

  const [doorLastChanged, setDoorLastChanged] = useState<string>("Changed: Just now");
  const [lastCaptureTime, setLastCaptureTime] = useState<string>(new Date().toLocaleTimeString());

  // Attendance Records
  const [attendanceRecords, setAttendanceRecords] = useState<AttendanceRecord[]>([
    {
      id: 'att-1',
      role: 'Student White Card',
      cardUID: 'E3:4A:91:02',
      method: 'RFID Tap',
      checkInTime: '09:15:20 AM',
      status: 'Verified'
    }
  ]);

  // Live Event Feed
  const [eventLogs, setEventLogs] = useState<EventLog[]>([
    {
      id: 'evt-1',
      type: 'boot',
      title: 'System Booted',
      details: 'ESP32-S3 node active on network',
      timestamp: '09:00:00 AM',
      severity: 'info'
    },
    {
      id: 'evt-2',
      type: 'rfid_student',
      title: 'Student Check-in Recorded',
      details: 'Card UID E3:4A:91:02 verified',
      timestamp: '09:15:20 AM',
      severity: 'success'
    }
  ]);

  // Remote Config State
  const [remoteConfig, setRemoteConfig] = useState<RemoteConfig>({
    tempThresholdHigh: 40,
    autoLockTimeoutSec: 15,
    buzzerEnabled: true,
    faceDetectionEnabled: true
  });

  // Subscribe to Firestore real-time updates (device 16-D1, telemetry, events)
  useEffect(() => {
    const unsubDevice = subscribeToDeviceStatus('16-D1', (newData) => {
      if (newData.room_id) setRoomId(newData.room_id);
      setSensorData(prev => ({
        ...prev,
        ...newData,
        isOnline: newData.lastSeen ? (Date.now() - newData.lastSeen < 25000) : true
      }));
    });
    const unsubTel = subscribeToTelemetryRecords((records) => {
      setTelemetryRecords(records);
    });
    const unsubEvt = subscribeToEvents((events) => {
      if (events.length > 0) setEventLogs(events);
    });
    const offlineTimer = setInterval(() => {
      setSensorData(prev => ({
        ...prev,
        isOnline: (Date.now() - prev.lastSeen) < 25000
      }));
    }, 1000);
    return () => {
      unsubDevice();
      unsubTel();
      unsubEvt();
      clearInterval(offlineTimer);
    };
  }, []);

  // Toggle Door Action
  const handleToggleDoor = () => {
    const nextState: DoorState = sensorData.doorState === 'LOCKED' ? 'UNLOCKED' : 'LOCKED';
    setSensorData(prev => ({ ...prev, doorState: nextState }));
    setDoorLastChanged(`Changed: ${new Date().toLocaleTimeString()}`);

    const newLog: EventLog = {
      id: `evt-${Date.now()}`,
      type: 'door_toggle',
      title: `Door Manually ${nextState}`,
      details: `Action triggered from Web Dashboard`,
      timestamp: new Date().toLocaleTimeString(),
      severity: nextState === 'UNLOCKED' ? 'success' : 'warning'
    };
    setEventLogs(prev => [newLog, ...prev]);
    pushEventLog(roomId, { type: 'door_toggle', title: newLog.title, details: newLog.details });
  };

  // Push Remote Config
  const handleSaveConfig = async (newConfig: RemoteConfig) => {
    setRemoteConfig(newConfig);
    await pushRemoteConfig(roomId, newConfig);

    const newLog: EventLog = {
      id: `evt-${Date.now()}`,
      type: 'config_push',
      title: 'Remote Config Pushed (OTA)',
      details: `Temp: ${newConfig.tempThresholdHigh}°C | AutoLock: ${newConfig.autoLockTimeoutSec}s | Buzzer: ${newConfig.buzzerEnabled}`,
      timestamp: new Date().toLocaleTimeString(),
      severity: 'info'
    };
    setEventLogs(prev => [newLog, ...prev]);
    alert("Configuration pushed to Firebase! ESP32 will pick it up on the next poll cycle (<8s).");
  };

  // Trigger Simulated Camera Snapshot
  const handleTriggerCapture = () => {
    const timeStr = new Date().toLocaleTimeString();
    setLastCaptureTime(timeStr);
    const newLog: EventLog = {
      id: `evt-${Date.now()}`,
      type: 'boot',
      title: 'Manual Camera Snapshot',
      details: 'Captured VGA frame stored in session log',
      timestamp: timeStr,
      severity: 'info'
    };
    setEventLogs(prev => [newLog, ...prev]);
  };

  // Export Attendance to CSV
  const handleExportCSV = () => {
    let csv = "Role,Card_UID,Method,CheckIn_Time,Status\n";
    attendanceRecords.forEach(rec => {
      csv += `"${rec.role}","${rec.cardUID}","${rec.method}","${rec.checkInTime}","${rec.status}"\n`;
    });
    const blob = new Blob([csv], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `Classroom_Attendance_${new Date().toISOString().slice(0, 10)}.csv`;
    a.click();
  };

  return (
    <div className="min-h-screen bg-[#FAF9F6] pb-16 font-sans">
      {/* NAVBAR */}
      <Navbar 
        roomId={roomId}
        isOnline={sensorData.isOnline}
        rssi={sensorData.wifiRSSI}
        onOpenSurfaces={() => setIsSurfacesModalOpen(true)}
      />

      {/* MAIN CONTAINER */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 space-y-6">

        {/* EMERGENCY ALERT BANNER */}
        <EmergencyBanner 
          alertType={sensorData.activeAlert}
          onDismiss={() => setSensorData(prev => ({ ...prev, activeAlert: 'NONE', flameDetected: false, panicActive: false }))}
        />

        {/* TOP 4 HERO CARDS GRID */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
          <DoorStatusCard 
            doorState={sensorData.doorState}
            lastChanged={doorLastChanged}
            onToggle={handleToggleDoor}
          />
          <SensorCard 
            temperature={sensorData.temperature}
            humidity={sensorData.humidity}
          />
          <SafetyCard 
            flameDetected={sensorData.flameDetected}
            panicActive={sensorData.panicActive}
          />
          <AttendanceCard 
            records={attendanceRecords}
            onExportCSV={handleExportCSV}
          />
        </div>

        {/* TWO-COLUMN WORKSPACE */}
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* LEFT 2 COLUMNS: ATTENDANCE TABLE & CAMERA */}
          <div className="lg:col-span-2 space-y-6">
            <AttendanceTable records={attendanceRecords} />
            <CameraVisionCard 
              lastCaptureTime={lastCaptureTime}
              onTriggerCapture={handleTriggerCapture}
            />
          </div>

          {/* RIGHT 1 COLUMN: EVENT STREAM & REMOTE CONFIG */}
          <div className="space-y-6">
            <EventStreamCard 
              events={eventLogs}
              onClear={() => setEventLogs([])}
            />
            <RemoteConfigCard 
              initialConfig={remoteConfig}
              onSave={handleSaveConfig}
            />
          </div>
        </div>

        {/* LIVE FIRESTORE DATABASE RECORDS */}
        <section className="bg-white rounded-2xl border border-slate-200 shadow-sm p-6">
          <div className="flex items-center justify-between mb-4">
            <h2 className="text-lg font-bold text-slate-900">Live Firestore Telemetry Records (Latest 20)</h2>
            <span className="text-xs font-mono text-slate-500">Collection: telemetry · Device: 16-D1</span>
          </div>
          <div className="overflow-x-auto">
            <table className="w-full text-left text-xs font-mono">
              <thead className="bg-slate-50 text-slate-600 border-b border-slate-200">
                <tr>
                  <th className="py-2 px-3">seq</th>
                  <th className="py-2 px-3">device_id</th>
                  <th className="py-2 px-3">room_id</th>
                  <th className="py-2 px-3">temp_c</th>
                  <th className="py-2 px-3">rh</th>
                  <th className="py-2 px-3">flame</th>
                  <th className="py-2 px-3">door</th>
                  <th className="py-2 px-3">status</th>
                  <th className="py-2 px-3">ts</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-slate-100">
                {telemetryRecords.length === 0 ? (
                  <tr>
                    <td colSpan={9} className="py-4 text-center text-slate-400">
                      Waiting for Firestore telemetry records from 16-D1...
                    </td>
                  </tr>
                ) : (
                  telemetryRecords.map((r) => (
                    <tr key={r.id} className="hover:bg-slate-50">
                      <td className="py-2 px-3 font-bold text-slate-800">{r.seq}</td>
                      <td className="py-2 px-3">{r.device_id}</td>
                      <td className="py-2 px-3">{r.room_id}</td>
                      <td className="py-2 px-3">{r.temp_c}°C</td>
                      <td className="py-2 px-3">{r.rh}%</td>
                      <td className="py-2 px-3">{r.flame ? '1' : '0'}</td>
                      <td className="py-2 px-3">{r.door}</td>
                      <td className="py-2 px-3">{r.status}</td>
                      <td className="py-2 px-3">{r.ts}</td>
                    </tr>
                  ))
                )}
              </tbody>
            </table>
          </div>
        </section>

      </main>

      {/* 5 SURFACES READINESS MODAL */}
      <SurfacesModal 
        isOpen={isSurfacesModalOpen}
        onClose={() => setIsSurfacesModalOpen(false)}
      />
    </div>
  );
}

