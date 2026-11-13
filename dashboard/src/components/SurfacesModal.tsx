import React from 'react';
import { Layers, X, CheckCircle2 } from 'lucide-react';

interface SurfacesModalProps {
  isOpen: boolean;
  onClose: () => void;
}

export const SurfacesModal: React.FC<SurfacesModalProps> = ({ isOpen, onClose }) => {
  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 bg-slate-900/40 backdrop-blur-sm flex items-center justify-center p-4">
      <div className="bg-white rounded-3xl max-w-lg w-full p-6 shadow-2xl space-y-4">
        <div className="flex items-center justify-between pb-3 border-b border-slate-100">
          <div className="flex items-center space-x-2">
            <Layers className="w-5 h-5 text-blue-600" />
            <h3 className="font-bold text-slate-900 text-base">5 Required Display Surfaces Checklist</h3>
          </div>
          <button onClick={onClose} className="btn btn-sm btn-circle btn-ghost text-slate-400">
            <X className="w-4 h-4" />
          </button>
        </div>

        <p className="text-xs text-slate-500">
          Judges will inspect all 5 surfaces during evaluations. Verify all 5 are active before calling the judge:
        </p>

        <div className="space-y-2.5 text-xs">
          <div className="p-3 rounded-2xl bg-slate-50 flex items-center justify-between">
            <div>
              <strong className="text-slate-900 block font-bold">1. Serial Monitor (Laptop)</strong>
              <span className="text-slate-500">115200 baud, prints events in real-time</span>
            </div>
            <span className="badge badge-success text-emerald-950 font-bold gap-1">
              <CheckCircle2 className="w-3 h-3" /> Ready
            </span>
          </div>

          <div className="p-3 rounded-2xl bg-slate-50 flex items-center justify-between">
            <div>
              <strong className="text-slate-900 block font-bold">2. Local Web Page (ESP32 IP)</strong>
              <span className="text-slate-500">Loads on browser at http://&lt;local-ip&gt;/</span>
            </div>
            <span className="badge badge-success text-emerald-950 font-bold gap-1">
              <CheckCircle2 className="w-3 h-3" /> Ready
            </span>
          </div>

          <div className="p-3 rounded-2xl bg-slate-50 flex items-center justify-between">
            <div>
              <strong className="text-slate-900 block font-bold">3. Live Web Dashboard (Public HTTPS)</strong>
              <span className="text-slate-500">Current website running on Vercel/Netlify</span>
            </div>
            <span className="badge badge-success text-emerald-950 font-bold gap-1">
              <CheckCircle2 className="w-3 h-3" /> Active
            </span>
          </div>

          <div className="p-3 rounded-2xl bg-slate-50 flex items-center justify-between">
            <div>
              <strong className="text-slate-900 block font-bold">4. Hardware Outputs (LED + Buzzer)</strong>
              <span className="text-slate-500">NeoPixel Green/Red + Active Buzzer beeps</span>
            </div>
            <span className="badge badge-success text-emerald-950 font-bold gap-1">
              <CheckCircle2 className="w-3 h-3" /> Verified
            </span>
          </div>

          <div className="p-3 rounded-2xl bg-slate-50 flex items-center justify-between">
            <div>
              <strong className="text-slate-900 block font-bold">5. Database Records (Firebase)</strong>
              <span className="text-slate-500">Firestore / RTDB documents updating live</span>
            </div>
            <span className="badge badge-success text-emerald-950 font-bold gap-1">
              <CheckCircle2 className="w-3 h-3" /> Connected
            </span>
          </div>
        </div>

        <button 
          onClick={onClose} 
          className="btn btn-sm w-full bg-slate-900 hover:bg-slate-800 text-white rounded-xl font-bold mt-2"
        >
          Close & Return to Dashboard
        </button>
      </div>
    </div>
  );
};

