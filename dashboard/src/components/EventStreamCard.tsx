import React from 'react';
import { Activity, Bell, AlertTriangle, ShieldCheck, DoorClosed, Sliders } from 'lucide-react';
import { EventLog } from '../types';

interface EventStreamCardProps {
  events: EventLog[];
  onClear: () => void;
}

export const EventStreamCard: React.FC<EventStreamCardProps> = ({ events, onClear }) => {
  const getIcon = (type: EventLog['type']) => {
    switch (type) {
      case 'fire_alarm':
      case 'panic_pad':
        return <AlertTriangle className="w-3.5 h-3.5 text-red-600" />;
      case 'door_toggle':
        return <DoorClosed className="w-3.5 h-3.5 text-blue-600" />;
      case 'config_push':
        return <Sliders className="w-3.5 h-3.5 text-indigo-600" />;
      case 'rfid_student':
      case 'rfid_teacher':
        return <ShieldCheck className="w-3.5 h-3.5 text-emerald-600" />;
      default:
        return <Bell className="w-3.5 h-3.5 text-slate-500" />;
    }
  };

  return (
    <div className="card-elevated p-6 flex flex-col h-[460px]">
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center space-x-2">
          <Activity className="w-5 h-5 text-slate-700" />
          <h2 className="text-base font-bold text-slate-900">Event Stream</h2>
        </div>
        <button 
          onClick={onClear}
          className="btn btn-xs btn-ghost text-slate-400 hover:text-slate-600"
        >
          Clear Feed
        </button>
      </div>

      <div className="flex-1 overflow-y-auto space-y-2.5 pr-1 text-xs">
        {events.length === 0 ? (
          <div className="h-full flex items-center justify-center text-slate-400">
            No events logged yet.
          </div>
        ) : (
          events.map((evt) => (
            <div 
              key={evt.id} 
              className="p-2.5 rounded-xl bg-slate-50 flex items-start space-x-2.5 transition hover:bg-slate-100/70"
            >
              <span className="p-1.5 rounded-lg bg-white shadow-xs">
                {getIcon(evt.type)}
              </span>
              <div className="flex-1 min-w-0">
                <div className="flex justify-between items-baseline">
                  <div className="font-bold text-slate-800 truncate">{evt.title}</div>
                  <span className="text-[10px] text-slate-400 whitespace-nowrap ml-2">{evt.timestamp}</span>
                </div>
                <div className="text-[11px] text-slate-500 truncate mt-0.5">{evt.details}</div>
              </div>
            </div>
          ))
        )}
      </div>
    </div>
  );
};

