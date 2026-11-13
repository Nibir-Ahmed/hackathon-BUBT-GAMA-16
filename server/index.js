import 'dotenv/config';
import express from 'express';
import cors from 'cors';
import { DatabaseSync } from 'node:sqlite';

const PORT = Number(process.env.PORT || 3000);
const DEVICE_TOKEN = process.env.DEVICE_TOKEN || '';
const DB_PATH = process.env.DB_PATH || './smart_classroom.db';
const OFFLINE_TIMEOUT_MS = Number(process.env.OFFLINE_TIMEOUT_MS || 25000);

const db = new DatabaseSync(DB_PATH);

db.exec(`
  PRAGMA journal_mode = WAL;

  CREATE TABLE IF NOT EXISTS devices (
    device_id TEXT PRIMARY KEY,
    room_id TEXT NOT NULL,
    online INTEGER NOT NULL DEFAULT 0,
    last_seen_ms INTEGER NOT NULL,
    last_status TEXT,
    fw TEXT
  );

  CREATE TABLE IF NOT EXISTS telemetry (
    device_id TEXT NOT NULL,
    room_id TEXT NOT NULL,
    seq INTEGER NOT NULL,
    ts INTEGER NOT NULL,
    uptime_s INTEGER NOT NULL,
    temp_c REAL NOT NULL,
    rh INTEGER NOT NULL,
    heat_index_c REAL NOT NULL,
    flame INTEGER NOT NULL,
    door TEXT NOT NULL,
    status TEXT NOT NULL,
    heat_trend TEXT NOT NULL,
    rssi INTEGER NOT NULL,
    heap_kb INTEGER NOT NULL,
    chip_temp_c REAL NOT NULL,
    fw TEXT NOT NULL,
    quality TEXT NOT NULL,
    received_at TEXT NOT NULL,
    PRIMARY KEY (device_id, seq)
  );

  CREATE TABLE IF NOT EXISTS events (
    device_id TEXT NOT NULL,
    room_id TEXT NOT NULL,
    seq INTEGER NOT NULL,
    ts INTEGER NOT NULL,
    type TEXT NOT NULL,
    detail TEXT NOT NULL,
    received_at TEXT NOT NULL,
    PRIMARY KEY (device_id, seq)
  );
`);

const stmtUpsertDevice = db.prepare(`
  INSERT INTO devices (device_id, room_id, online, last_seen_ms, last_status, fw)
  VALUES (?, ?, 1, ?, ?, ?)
  ON CONFLICT(device_id) DO UPDATE SET
    room_id = excluded.room_id,
    online = 1,
    last_seen_ms = excluded.last_seen_ms,
    last_status = COALESCE(excluded.last_status, devices.last_status),
    fw = COALESCE(excluded.fw, devices.fw)
`);

const stmtInsertTelemetry = db.prepare(`
  INSERT INTO telemetry (
    device_id, room_id, seq, ts, uptime_s, temp_c, rh, heat_index_c,
    flame, door, status, heat_trend, rssi, heap_kb, chip_temp_c, fw, quality, received_at
  ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
`);

const stmtInsertEvent = db.prepare(`
  INSERT INTO events (device_id, room_id, seq, ts, type, detail, received_at)
  VALUES (?, ?, ?, ?, ?, ?, ?)
`);

const stmtMarkOffline = db.prepare(`
  UPDATE devices SET online = 0 WHERE online = 1 AND (? - last_seen_ms) > ?
`);

setInterval(() => {
  stmtMarkOffline.run(Date.now(), OFFLINE_TIMEOUT_MS);
}, 5000);

const app = express();
app.use(cors());
app.use(express.json({ limit: '1mb' }));

function requireDeviceToken(req, res, next) {
  if (!DEVICE_TOKEN) {
    return res.status(500).json({ error: 'server_misconfigured_missing_device_token' });
  }
  const auth = req.headers.authorization || '';
  const token = auth.startsWith('Bearer ') ? auth.slice(7).trim() : '';
  if (!token || token !== DEVICE_TOKEN) {
    return res.status(401).json({ error: 'unauthorized' });
  }
  next();
}

function isUniqueViolation(err) {
  return err && (String(err.message).includes('UNIQUE constraint') || String(err.message).includes('PRIMARY KEY'));
}

app.get('/health', (_req, res) => {
  res.json({ ok: true, now: new Date().toISOString() });
});

app.post('/api/telemetry', requireDeviceToken, (req, res) => {
  const b = req.body || {};
  if (!b.device_id || b.seq === undefined) {
    return res.status(400).json({ error: 'missing_device_id_or_seq' });
  }

  try {
    stmtInsertTelemetry.run(
      String(b.device_id),
      String(b.room_id || '-'),
      Number(b.seq),
      Number(b.ts || 0),
      Number(b.uptime_s || 0),
      Number(b.temp_c || 0),
      Number(b.rh || 0),
      Number(b.heat_index_c || 0),
      b.flame ? 1 : 0,
      String(b.door || 'LOCKED'),
      String(b.status || 'SAFE'),
      String(b.heat_trend || 'STABLE'),
      Number(b.rssi || 0),
      Number(b.heap_kb || 0),
      Number(b.chip_temp_c || 0),
      String(b.fw || '-'),
      String(b.quality || 'good'),
      new Date().toISOString()
    );

    stmtUpsertDevice.run(
      String(b.device_id),
      String(b.room_id || '-'),
      Date.now(),
      String(b.status || 'SAFE'),
      String(b.fw || '-')
    );

    return res.status(201).json({ ok: true, seq: Number(b.seq) });
  } catch (err) {
    if (isUniqueViolation(err)) {
      return res.status(409).json({ error: 'replay_rejected', device_id: b.device_id, seq: Number(b.seq) });
    }
    return res.status(500).json({ error: err.message });
  }
});

app.post('/api/events', requireDeviceToken, (req, res) => {
  const b = req.body || {};
  if (!b.device_id || b.seq === undefined || !b.type) {
    return res.status(400).json({ error: 'missing_fields' });
  }

  try {
    stmtInsertEvent.run(
      String(b.device_id),
      String(b.room_id || '-'),
      Number(b.seq),
      Number(b.ts || 0),
      String(b.type),
      String(b.detail || ''),
      new Date().toISOString()
    );

    stmtUpsertDevice.run(
      String(b.device_id),
      String(b.room_id || '-'),
      Date.now(),
      null,
      null
    );

    return res.status(201).json({ ok: true, seq: Number(b.seq) });
  } catch (err) {
    if (isUniqueViolation(err)) {
      return res.status(409).json({ error: 'replay_rejected', device_id: b.device_id, seq: Number(b.seq) });
    }
    return res.status(500).json({ error: err.message });
  }
});

app.get('/api/devices', (_req, res) => {
  stmtMarkOffline.run(Date.now(), OFFLINE_TIMEOUT_MS);
  const rows = db.prepare('SELECT * FROM devices ORDER BY device_id').all();
  res.json(rows.map((r) => ({ ...r, state: r.online ? 'ONLINE' : 'OFFLINE' })));
});

app.get('/api/telemetry', (req, res) => {
  const limit = Math.min(Number(req.query.limit || 50), 500);
  const rows = db.prepare('SELECT * FROM telemetry ORDER BY seq DESC LIMIT ?').all(limit);
  res.json(rows);
});

app.get('/api/events', (req, res) => {
  const limit = Math.min(Number(req.query.limit || 50), 500);
  const rows = db.prepare('SELECT * FROM events ORDER BY seq DESC LIMIT ?').all(limit);
  res.json(rows);
});

app.get('/', (_req, res) => {
  res.send(`<!DOCTYPE html>
<html><head><meta charset="UTF-8"><title>Smart Classroom Cloud DB Viewer</title>
<style>body{font-family:sans-serif;padding:20px;background:#f8fafc;color:#0f172a}
table{border-collapse:collapse;width:100%;background:#fff;margin-bottom:20px;font-size:13px}
th,td{border:1px solid #e2e8f0;padding:6px 8px;text-align:left}th{background:#f1f5f9}</style></head>
<body><h2>Smart Classroom Backend (Levels 2–3)</h2>
<div id="out">Loading...</div>
<script>
async function load(){
  const [d,t,e] = await Promise.all([
    fetch('/api/devices').then(r=>r.json()),
    fetch('/api/telemetry?limit=15').then(r=>r.json()),
    fetch('/api/events?limit=20').then(r=>r.json())
  ]);
  document.getElementById('out').innerHTML =
    '<h3>Devices</h3>' + tbl(d,['device_id','room_id','state','last_status','fw']) +
    '<h3>Latest Telemetry</h3>' + tbl(t,['seq','ts','room_id','temp_c','rh','heat_index_c','heat_trend','status','door','flame','quality','rssi']) +
    '<h3>Latest Events</h3>' + tbl(e,['seq','ts','room_id','type','detail']);
}
function tbl(rows,cols){
  if(!rows.length) return '<p>No records yet</p>';
  return '<table><tr>'+cols.map(c=>'<th>'+c+'</th>').join('')+'</tr>' +
    rows.map(r=>'<tr>'+cols.map(c=>'<td>'+r[c]+'</td>').join('')+'</tr>').join('') + '</table>';
}
load(); setInterval(load, 2000);
</script></body></html>`);
});

app.listen(PORT, () => {
  console.log(`Smart Classroom server listening on port ${PORT}`);
});

