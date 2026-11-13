import React from 'react';
import { Camera, Video } from 'lucide-react';

interface CameraVisionCardProps {
  lastCaptureTime: string;
  onTriggerCapture: () => void;
}

export const CameraVisionCard: React.FC<CameraVisionCardProps> = ({ lastCaptureTime, onTriggerCapture }) => {
  return (
    <div className="card-elevated p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center space-x-2">
          <Camera className="w-5 h-5 text-indigo-600" />
          <h2 className="text-base font-bold text-slate-900">ESP32-S3-CAM Vision Snapshot</h2>
        </div>
        <button 
          onClick={onTriggerCapture}
          className="btn btn-xs bg-indigo-50 hover:bg-indigo-100 text-indigo-700 font-bold rounded-lg border-0 gap-1"
        >
          <Camera className="w-3 h-3" />
          <span>Trigger Snapshot</span>
        </button>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <div className="aspect-video bg-slate-900 rounded-2xl overflow-hidden relative flex items-center justify-center text-slate-400 shadow-inner">
          <div className="text-center p-4">
            <Video className="w-8 h-8 mx-auto mb-2 opacity-50 text-indigo-400" />
            <span className="text-xs font-medium text-slate-300">Live Snapshot Preview (VGA / QVGA)</span>
            <div className="text-[10px] text-slate-400 mt-1 font-mono">Last Captured: {lastCaptureTime}</div>
          </div>
          <span className="badge badge-sm badge-success absolute top-2.5 left-2.5 font-bold text-[10px] text-white">
            OV2640 Ready
          </span>
        </div>

        <div className="p-4 bg-slate-50/80 rounded-2xl flex flex-col justify-between text-xs space-y-3">
          <div>
            <h3 className="font-bold text-slate-800 text-sm mb-1">Vision Attendance & Intruder Capture</h3>
            <p className="text-slate-500 leading-relaxed">
              The camera snaps visual evidence automatically on RFID tap (Levels 5, 6) and logs unauthorized intruders (Level 8).
            </p>
          </div>

          <div className="space-y-1.5 border-t border-slate-200/80 pt-3 text-slate-600">
            <div className="flex justify-between"><span>Frame Size:</span><strong>640x480 (VGA)</strong></div>
            <div className="flex justify-between"><span>JPEG Quality:</span><strong>12 (High)</strong></div>
            <div className="flex justify-between"><span>Capture Latency:</span><strong className="text-emerald-600">&lt; 0.4s</strong></div>
          </div>
        </div>
      </div>
    </div>
  );
};

