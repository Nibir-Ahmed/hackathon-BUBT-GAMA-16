import React from 'react';
import { ShieldAlert, CheckCircle2, Flame, HandMetal } from 'lucide-react';

interface SafetyCardProps {
  flameDetected: boolean;
  panicActive: boolean;
}

export const SafetyCard: React.FC<SafetyCardProps> = ({ flameDetected, panicActive }) => {
  return (
    <div className="card-elevated p-6 flex flex-col justify-between">
      <div>
        <div className="flex justify-between items-center mb-3">
          <span className="text-xs font-bold uppercase tracking-wider text-slate-400">Safety Sensors</span>
          <ShieldAlert className={`w-5 h-5 ${flameDetected || panicActive ? 'text-red-500 animate-bounce' : 'text-emerald-500'}`} />
        </div>
        
        <div className="text-sm text-slate-500 font-medium mb-2">Flame Sensor Status</div>

        <div className={`badge badge-lg gap-1.5 font-bold py-3.5 px-3 border-0 ${
          flameDetected 
            ? 'badge-error text-white animate-pulse' 
            : 'badge-success text-emerald-950 bg-emerald-100'
        }`}>
          {flameDetected ? (
            <>
              <Flame className="w-4 h-4 animate-bounce" />
              <span>FIRE DETECTED!</span>
            </>
          ) : (
            <>
              <CheckCircle2 className="w-4 h-4 text-emerald-600" />
              <span>Safe (No Flame)</span>
            </>
          )}
        </div>
      </div>

      <div className="mt-4 pt-3 border-t border-slate-100 flex items-center justify-between text-xs">
        <div className="flex items-center space-x-1.5 text-slate-600">
          <HandMetal className="w-4 h-4 text-slate-400" />
          <span>Touch Panic Pad:</span>
        </div>
        <span className={`badge badge-sm font-semibold ${
          panicActive ? 'badge-warning text-amber-950 font-bold animate-pulse' : 'badge-ghost text-slate-500'
        }`}>
          {panicActive ? 'TRIGGERED (SOS)' : 'Clear (Inactive)'}
        </span>
      </div>
    </div>
  );
};

