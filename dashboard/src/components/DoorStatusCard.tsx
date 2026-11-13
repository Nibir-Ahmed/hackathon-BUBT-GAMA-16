import React from 'react';
import { DoorClosed, RefreshCw } from 'lucide-react';
import { DoorState } from '../types';

interface DoorStatusCardProps {
  doorState: DoorState;
  lastChanged: string;
  onToggle: () => void;
}

export const DoorStatusCard: React.FC<DoorStatusCardProps> = ({ doorState, lastChanged, onToggle }) => {
  const isUnlocked = doorState === 'UNLOCKED';
  const isEmergency = doorState === 'EMERGENCY_OPEN';

  return (
    <div className="card-elevated p-6 flex flex-col justify-between">
      <div>
        <div className="flex justify-between items-center mb-3">
          <span className="text-xs font-bold uppercase tracking-wider text-slate-400">Classroom Access</span>
          <DoorClosed className="w-5 h-5 text-slate-400" />
        </div>
        
        <div className="text-sm text-slate-500 font-medium mb-2">Simulated Door</div>

        {/* DaisyUI Status Badge */}
        <div className={`badge badge-lg gap-2 font-bold py-4 px-4 rounded-xl border-0 ${
          isEmergency 
            ? 'badge-error text-white animate-bounce' 
            : isUnlocked 
              ? 'badge-success text-emerald-950 bg-emerald-100' 
              : 'badge-error text-red-950 bg-red-100'
        }`}>
          <span className={`w-2.5 h-2.5 rounded-full ${
            isEmergency ? 'bg-white' : isUnlocked ? 'bg-emerald-600' : 'bg-red-600'
          }`}></span>
          <span>{doorState}</span>
        </div>
      </div>

      <div className="mt-6 pt-4 border-t border-slate-100 flex items-center justify-between">
        <span className="text-[11px] text-slate-400">{lastChanged}</span>
        <button 
          onClick={onToggle}
          className="btn btn-sm bg-slate-900 hover:bg-slate-800 text-white rounded-xl font-semibold gap-1 text-xs"
        >
          <RefreshCw className="w-3 h-3" />
          <span>Toggle Door</span>
        </button>
      </div>
    </div>
  );
};

