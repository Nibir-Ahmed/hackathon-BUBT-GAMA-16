import React, { useState } from 'react';
import { Sliders, Send } from 'lucide-react';
import { RemoteConfig } from '../types';

interface RemoteConfigCardProps {
  initialConfig: RemoteConfig;
  onSave: (config: RemoteConfig) => void;
}

export const RemoteConfigCard: React.FC<RemoteConfigCardProps> = ({ initialConfig, onSave }) => {
  const [config, setConfig] = useState<RemoteConfig>(initialConfig);

  const handleSave = () => {
    onSave(config);
  };

  return (
    <div className="card-elevated p-6 space-y-4">
      <div className="flex items-center space-x-2">
        <Sliders className="w-5 h-5 text-slate-700" />
        <h2 className="text-base font-bold text-slate-900">Remote Settings (OTA)</h2>
      </div>

      <div className="space-y-4 text-xs">
        {/* High Temp Threshold Slider with DaisyUI Range */}
        <div>
          <div className="flex justify-between font-semibold mb-1.5 text-slate-700">
            <span>High Temp Alert Threshold:</span>
            <span className="badge badge-sm badge-neutral font-bold">{config.tempThresholdHigh} °C</span>
          </div>
          <input 
            type="range" 
            min="30" 
            max="60" 
            value={config.tempThresholdHigh} 
            onChange={(e) => setConfig({ ...config, tempThresholdHigh: Number(e.target.value) })}
            className="range range-xs range-primary w-full"
          />
        </div>

        {/* Auto-Lock Timeout with DaisyUI Range */}
        <div>
          <div className="flex justify-between font-semibold mb-1.5 text-slate-700">
            <span>Door Auto-lock Duration:</span>
            <span className="badge badge-sm badge-neutral font-bold">{config.autoLockTimeoutSec} sec</span>
          </div>
          <input 
            type="range" 
            min="5" 
            max="60" 
            value={config.autoLockTimeoutSec} 
            onChange={(e) => setConfig({ ...config, autoLockTimeoutSec: Number(e.target.value) })}
            className="range range-xs range-primary w-full"
          />
        </div>

        {/* Buzzer Mute Toggle with DaisyUI Toggle */}
        <div className="flex items-center justify-between pt-2 border-t border-slate-100">
          <div>
            <div className="font-semibold text-slate-800">Hardware Buzzer Audio</div>
            <div className="text-[11px] text-slate-400">Alarm and confirmation feedback</div>
          </div>
          <input 
            type="checkbox" 
            checked={config.buzzerEnabled} 
            onChange={(e) => setConfig({ ...config, buzzerEnabled: e.target.checked })}
            className="toggle toggle-sm toggle-success"
          />
        </div>
      </div>

      <button 
        onClick={handleSave}
        className="btn btn-sm w-full bg-slate-900 hover:bg-slate-800 text-white rounded-xl font-bold gap-1.5 shadow-sm"
      >
        <Send className="w-3.5 h-3.5" />
        <span>Push Settings to ESP32</span>
      </button>
    </div>
  );
};

