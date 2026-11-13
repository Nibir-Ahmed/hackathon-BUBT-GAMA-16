import React from 'react';
import { Flame, AlertTriangle, ShieldAlert } from 'lucide-react';

interface EmergencyBannerProps {
  alertType: 'NONE' | 'FIRE' | 'PANIC' | 'INTRUDER';
  onDismiss: () => void;
}

export const EmergencyBanner: React.FC<EmergencyBannerProps> = ({ alertType, onDismiss }) => {
  if (alertType === 'NONE') return null;

  const isFire = alertType === 'FIRE';
  const isPanic = alertType === 'PANIC';

  return (
    <div className={`alert ${isFire ? 'alert-error' : isPanic ? 'alert-warning' : 'alert-error'} shadow-xl rounded-2xl p-4 text-white animate-pulse mb-6 flex justify-between items-center`}>
      <div className="flex items-center space-x-3">
        <div className="p-2.5 bg-black/15 rounded-xl">
          {isFire && <Flame className="w-7 h-7 text-white animate-bounce" />}
          {isPanic && <AlertTriangle className="w-7 h-7 text-white animate-bounce" />}
          {!isFire && !isPanic && <ShieldAlert className="w-7 h-7 text-white animate-bounce" />}
        </div>
        <div>
          <div className="font-extrabold text-base tracking-wide uppercase">
            {isFire && "CRITICAL EMERGENCY: FIRE DETECTED!"}
            {isPanic && "PANIC ALARM: CAPACITIVE TOUCH WIRE TRIGGERED!"}
            {!isFire && !isPanic && "SECURITY ALERT: UNAUTHORIZED INTRUDER TAP!"}
          </div>
          <div className="text-xs opacity-90">
            {isFire && "Active evacuation siren sounding on ESP32-S3 node. Classroom door set to EMERGENCY OPEN."}
            {isPanic && "Occupant touched panic wire. Audio alert SOS sounding on node."}
            {!isFire && !isPanic && "Unregistered RFID card rejected with denial alarm."}
          </div>
        </div>
      </div>

      <button 
        onClick={onDismiss}
        className="btn btn-sm btn-ghost bg-white/20 hover:bg-white/30 text-white rounded-xl text-xs font-bold"
      >
        Acknowledge
      </button>
    </div>
  );
};

