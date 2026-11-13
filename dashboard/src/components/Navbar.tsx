import React from 'react';
import { ShieldCheck, MapPin, Wifi, Layers } from 'lucide-react';

interface NavbarProps {
  roomId: string;
  isOnline: boolean;
  rssi: number;
  onOpenSurfaces: () => void;
}

export const Navbar: React.FC<NavbarProps> = ({ roomId, isOnline, rssi, onOpenSurfaces }) => {
  return (
    <header className="sticky top-0 z-40 bg-white/85 backdrop-blur-md shadow-sm mb-6 border-b border-slate-100">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-3.5 flex flex-wrap justify-between items-center gap-4">
        
        {/* Brand Logo & Event Details */}
        <div className="flex items-center space-x-3">
          <div className="w-10 h-10 rounded-2xl bg-blue-600 flex items-center justify-center text-white shadow-md shadow-blue-500/20">
            <ShieldCheck className="w-6 h-6" />
          </div>
          <div>
            <h1 className="text-lg font-bold tracking-tight text-slate-900 leading-none">Smart Classroom OS</h1>
            <span className="text-xs text-slate-500 font-medium">FASTROBOX 1.0 Techathon • BUBT Grand Finale</span>
          </div>
        </div>

        {/* Global Status Pills with DaisyUI Badges */}
        <div className="flex items-center space-x-3 text-sm">
          {/* Room Pill */}
          <div className="badge badge-lg gap-1.5 bg-slate-100 text-slate-700 font-semibold border-0 py-3.5 px-3">
            <MapPin className="w-3.5 h-3.5 text-slate-500" />
            <span>{roomId}</span>
          </div>

          {/* Online State Pill */}
          <div className={`badge badge-lg gap-1.5 font-semibold py-3.5 px-3 border-0 ${
            isOnline 
              ? 'badge-success text-emerald-950 bg-emerald-100' 
              : 'badge-warning text-amber-950 bg-amber-100'
          }`}>
            <span className={`w-2 h-2 rounded-full ${isOnline ? 'bg-emerald-600 animate-ping' : 'bg-amber-600'}`}></span>
            <span>{isOnline ? 'Live Cloud Sync' : 'Local / Offline Mode'}</span>
          </div>

          {/* Wi-Fi RSSI */}
          <div className="hidden sm:flex badge badge-lg gap-1.5 bg-slate-100 text-slate-600 font-semibold border-0 py-3.5 px-3">
            <Wifi className="w-3.5 h-3.5" />
            <span>{rssi} dBm</span>
          </div>

          {/* 5 Surfaces Modal Trigger */}
          <button 
            onClick={onOpenSurfaces}
            className="btn btn-sm bg-slate-900 hover:bg-slate-800 text-white rounded-xl gap-1.5 font-semibold shadow-sm"
          >
            <Layers className="w-3.5 h-3.5" />
            <span>5 Surfaces Check</span>
          </button>
        </div>
      </div>
    </header>
  );
};
