/*
 * CarePoint – Node.js / Express bridge
 * Compiles the C core on first run, then proxies HTTP requests to it.
 */

'use strict';
const express  = require('express');
const { execFileSync, execSync } = require('child_process');
const path     = require('path');
const fs       = require('fs');

const app  = express();
const PORT = process.env.PORT || 3000;

const isWin  = process.platform === 'win32';
const coreSrc = path.join(__dirname, 'core', 'hospital.c');
const hdrSrc  = path.join(__dirname, 'core', 'hospital.h');
const binName = isWin ? 'hospital.exe' : 'hospital';
const binPath = path.join(__dirname, 'core', binName);

/* ── Compile C on startup ───────────────────────────────────────────────── */
function compile() {
  try {
    console.log('[CarePoint] Compiling C core …');
    execSync(`gcc -Wall -O2 -o "${binPath}" "${coreSrc}"`, { stdio: 'inherit' });
    console.log('[CarePoint] Compile OK →', binPath);
  } catch (e) {
    console.error('[CarePoint] Compile FAILED. Make sure GCC is installed.');
    process.exit(1);
  }
}

if (!fs.existsSync(binPath)) compile();

/* ── Run C binary with args ─────────────────────────────────────────────── */
function runCore(args) {
  try {
    const out = execFileSync(binPath, args, {
      cwd: path.join(__dirname, 'core'),
      timeout: 8000
    });
    return JSON.parse(out.toString());
  } catch (e) {
    const raw = e.stdout ? e.stdout.toString() : '';
    try { return JSON.parse(raw); } catch (_) {}
    return { ok: false, message: 'Internal C error: ' + e.message };
  }
}

/* ── Middleware ─────────────────────────────────────────────────────────── */
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

/* ── REST API Routes ────────────────────────────────────────────────────── */

// POST /api/register
app.post('/api/register', (req, res) => {
  const { name, age, gender, phone, condition, priority, ward } = req.body;
  if (!name || !age || !gender || !phone || !condition || !priority || !ward)
    return res.json({ ok: false, message: 'All fields are required' });
  res.json(runCore(['register', name, String(age), gender, phone, condition, String(priority), ward]));
});

// POST /api/queue/:id
app.post('/api/queue/:id', (req, res) =>
  res.json(runCore(['queue', req.params.id])));

// POST /api/emergency/:id
app.post('/api/emergency/:id', (req, res) =>
  res.json(runCore(['emergency', req.params.id])));

// POST /api/priority_queue/:id
app.post('/api/priority_queue/:id', (req, res) =>
  res.json(runCore(['priority_queue', req.params.id])));

// POST /api/treat
app.post('/api/treat', (req, res) =>
  res.json(runCore(['treat'])));

// POST /api/treat_priority
app.post('/api/treat_priority', (req, res) =>
  res.json(runCore(['treat_priority'])));

// POST /api/discharge/:id
app.post('/api/discharge/:id', (req, res) =>
  res.json(runCore(['discharge', req.params.id])));

// POST /api/add_bill/:id
app.post('/api/add_bill/:id', (req, res) => {
  const { amount } = req.body;
  res.json(runCore(['add_bill', req.params.id, String(amount || 0)]));
});

// GET /api/patients
app.get('/api/patients', (req, res) =>
  res.json(runCore(['list'])));

// GET /api/search/:id  (BST)
app.get('/api/search/:id', (req, res) =>
  res.json(runCore(['search', req.params.id])));

// GET /api/search_name/:keyword  (Linear)
app.get('/api/search_name/:keyword', (req, res) =>
  res.json(runCore(['search_name', req.params.keyword])));

// GET /api/hash_lookup/:name
app.get('/api/hash_lookup/:name', (req, res) =>
  res.json(runCore(['hash_lookup', req.params.name])));

// GET /api/bst_list
app.get('/api/bst_list', (req, res) =>
  res.json(runCore(['bst_list'])));

// GET /api/sort/:field
app.get('/api/sort/:field', (req, res) =>
  res.json(runCore(['sort', req.params.field])));

// GET /api/binary_search/:id
app.get('/api/binary_search/:id', (req, res) =>
  res.json(runCore(['binary_search', req.params.id])));

// GET /api/waiting
app.get('/api/waiting', (req, res) =>
  res.json(runCore(['waiting'])));

// GET /api/wards
app.get('/api/wards', (req, res) =>
  res.json(runCore(['wards'])));

// GET /api/bfs/:ward
app.get('/api/bfs/:ward', (req, res) =>
  res.json(runCore(['bfs', req.params.ward])));

// GET /api/dfs
app.get('/api/dfs', (req, res) =>
  res.json(runCore(['dfs'])));

// GET /api/stats
app.get('/api/stats', (req, res) =>
  res.json(runCore(['stats'])));

// POST /api/reset
app.post('/api/reset', (req, res) =>
  res.json(runCore(['reset'])));

/* ── Start ──────────────────────────────────────────────────────────────── */
app.listen(PORT, () =>
  console.log(`[CarePoint] Server running at http://localhost:${PORT}`));
