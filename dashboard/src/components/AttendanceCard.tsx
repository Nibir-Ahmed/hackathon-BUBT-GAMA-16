import React from 'react';
import { Users, Download, UserCheck } from 'lucide-react';
import { AttendanceRecord } from '../types';

interface AttendanceCardProps {
  records: AttendanceRecord[];
  onExportCSV: () => void;
}

export const AttendanceCard: React.FC<AttendanceCardProps> = ({ records, onExportCSV }) => {
  return (
    <div className="card-elevated p-6 flex flex-col justify-between">
      <div>
        <div className="flex justify-between items-center mb-3">
          <span className="text-xs font-bold uppercase tracking-wider text-slate-400">Daily Presence</span>
          <Users className="w-5 h-5 text-blue-500" />
        </div>
        
        <div className="text-sm text-slate-500 font-medium mb-1">Total Check-ins</div>
        
        <div className="flex items-baseline space-x-2">
          <span className="text-4xl font-extrabold text-slate-900 tracking-tight">
            {records.length}
          </span>
          <span className="text-xs font-semibold text-slate-400">Attendees</span>
        </div>
      </div>

      <div className="mt-4 pt-3 border-t border-slate-100 flex items-center justify-between text-xs">
        <span className="text-slate-400">Student & Admin</span>
        <button 
          onClick={onExportCSV}
          className="btn btn-xs btn-ghost text-blue-600 hover:text-blue-800 font-bold gap-1"
        >
          <Download className="w-3 h-3" />
          <span>Export CSV</span>
        </button>
      </div>
    </div>
  );
};

export const AttendanceTable: React.FC<{ records: AttendanceRecord[] }> = ({ records }) => {
  return (
    <div className="card-elevated p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center space-x-2">
          <UserCheck className="w-5 h-5 text-blue-600" />
          <h2 className="text-base font-bold text-slate-900">Live Attendance Record</h2>
        </div>
        <span className="badge badge-sm badge-ghost text-slate-500">Auto-logged</span>
      </div>

      <div className="overflow-x-auto">
        <table className="table table-sm w-full text-slate-600">
          <thead>
            <tr className="text-slate-400 text-xs border-b border-slate-100 uppercase">
              <th>Role</th>
              <th>Card UID</th>
              <th>Verification Method</th>
              <th>Time</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-100 text-xs font-medium">
            {records.length === 0 ? (
              <tr>
                <td colSpan={5} className="py-6 text-center text-slate-400">
                  No attendance records logged yet. Tap White Card on RC522 reader.
                </td>
              </tr>
            ) : (
              records.map((rec) => (
                <tr key={rec.id} className="hover:bg-slate-50/60 transition">
                  <td className="font-bold text-slate-900 flex items-center space-x-2">
                    <span className="w-2 h-2 rounded-full bg-emerald-500"></span>
                    <span>{rec.role}</span>
                  </td>
                  <td className="font-mono text-slate-500">{rec.cardUID}</td>
                  <td>
                    <span className="badge badge-sm badge-info badge-outline font-semibold">
                      {rec.method}
                    </span>
                  </td>
                  <td className="text-slate-500">{rec.checkInTime}</td>
                  <td>
                    <span className="badge badge-sm badge-success font-bold text-[10px]">
                      {rec.status}
                    </span>
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};

