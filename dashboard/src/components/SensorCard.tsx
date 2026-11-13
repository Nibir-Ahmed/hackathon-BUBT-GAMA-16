import React from 'react';
import { Thermometer, Droplets } from 'lucide-react';

interface SensorCardProps {
  temperature: number;
  humidity: number;
}

export const SensorCard: React.FC<SensorCardProps> = ({ temperature, humidity }) => {
  const isHighTemp = temperature >= 40;

  return (
    <div className="card-elevated p-6 flex flex-col justify-between">
      <div>
        <div className="flex justify-between items-center mb-3">
          <span className="text-xs font-bold uppercase tracking-wider text-slate-400">DHT11 Climate</span>
          <Thermometer className="w-5 h-5 text-orange-500" />
        </div>
        
        <div className="text-sm text-slate-500 font-medium mb-1">Ambient Temperature</div>
        
        <div className="flex items-baseline space-x-2">
          <span className={`text-4xl font-extrabold tracking-tight ${isHighTemp ? 'text-red-600 animate-pulse' : 'text-slate-900'}`}>
            {temperature.toFixed(1)}
          </span>
          <span className="text-lg font-bold text-slate-400">°C</span>
        </div>
      </div>

      <div className="mt-4 pt-3 border-t border-slate-100 flex items-center justify-between text-xs">
        <div className="flex items-center space-x-1.5 text-slate-600">
          <Droplets className="w-4 h-4 text-blue-500" />
          <span>Humidity:</span>
          <strong className="text-slate-900 font-bold">{humidity.toFixed(1)}%</strong>
        </div>
        <span className={`badge badge-sm font-semibold ${isHighTemp ? 'badge-error text-white' : 'badge-ghost text-slate-600'}`}>
          {isHighTemp ? 'High Temp Alarm' : 'Normal'}
        </span>
      </div>
    </div>
  );
};

