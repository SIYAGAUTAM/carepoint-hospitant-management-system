/* ══════════════════════════════════════════════════════════════════════════
 * CarePoint – app.js  (Frontend logic)
 * ══════════════════════════════════════════════════════════════════════════ */

'use strict';

/* ── Toast ───────────────────────────────────────────────────────────────── */
function toast(msg, type = 'info') {
  const el = document.getElementById('toast');
  el.textContent = msg;
  el.className = `toast show ${type}`;
  setTimeout(() => { el.className = 'toast'; }, 3200);
}

/* ── API helper ──────────────────────────────────────────────────────────── */
async function api(method, url, body) {
  try {
    const opts = { method, headers: { 'Content-Type': 'application/json' } };
    if (body) opts.body = JSON.stringify(body);
    const res = await fetch(url, opts);
    return await res.json();
  } catch (e) {
    return { ok: false, message: 'Network error: ' + e.message };
  }
}

/* ── Results panel ───────────────────────────────────────────────────────── */
function showResult(data) {
  const el = document.getElementById('results');
  if (!data) return;
  el.innerHTML = `<pre class="json-block">${JSON.stringify(data, null, 2)}</pre>`;
}

function showPatientResult(data) {
  const el = document.getElementById('results');
  if (!data || !data.ok) {
    el.innerHTML = `<div class="result-row" style="border-color:var(--danger)"><strong>❌ ${data?.message || 'Error'}</strong></div>`;
    return;
  }
  const p = data.patient || data;
  if (data.patients) {
    el.innerHTML = data.patients.map(renderPatientResult).join('');
  } else if (data.patient) {
    el.innerHTML = renderPatientResult(data.patient);
  } else {
    el.innerHTML = `<pre class="json-block">${JSON.stringify(data, null, 2)}</pre>`;
  }
}

function renderPatientResult(p) {
  const prioLabel = { 1: '🔴 Critical', 2: '🟡 Urgent', 3: '🟢 Normal' };
  return `
    <div class="result-row">
      <strong>#${p.id} – ${p.name}</strong>
      <span class="status-badge status-${statusClass(p.status)}" style="margin-left:8px">${p.status}</span><br/>
      Age: ${p.age} · ${p.gender} · ${p.ward} · ${prioLabel[p.priority] || p.priority}<br/>
      Condition: ${p.condition}<br/>
      Bill: ₹${p.bill?.toFixed(2)} · Admitted: ${p.admit_date}
    </div>`;
}

function statusClass(s) {
  if (!s) return 'registered';
  s = s.toLowerCase();
  if (s.includes('queue'))     return 'queue';
  if (s.includes('emergency')) return 'emergency';
  if (s.includes('treatment') || s.includes('treat')) return 'treatment';
  if (s.includes('discharge')) return 'discharged';
  if (s.includes('priority'))  return 'priority';
  return 'registered';
}

/* ── Stats ───────────────────────────────────────────────────────────────── */
async function loadStats() {
  const data = await api('GET', '/api/stats');
  if (!data.ok) return;
  const s = data.stats;
  document.getElementById('statTotal').textContent   = s.total_registered;
  document.getElementById('statActive').textContent  = s.active;
  document.getElementById('statEmergency').textContent = s.emergency_stack_size;
  document.getElementById('statQueue').textContent   = s.regular_queue_size;
  document.getElementById('statPriorityQ').textContent = s.priority_queue_size;
  document.getElementById('statRevenue').textContent = '₹' + s.total_revenue?.toLocaleString('en-IN', { minimumFractionDigits: 2 });
}

/* ── Register ────────────────────────────────────────────────────────────── */
document.getElementById('regForm').addEventListener('submit', async e => {
  e.preventDefault();
  const body = {
    name:      document.getElementById('rName').value.trim(),
    age:       parseInt(document.getElementById('rAge').value),
    gender:    document.getElementById('rGender').value,
    phone:     document.getElementById('rPhone').value.trim(),
    condition: document.getElementById('rCond').value.trim(),
    priority:  parseInt(document.getElementById('rPriority').value),
    ward:      document.getElementById('rWard').value,
  };
  if (!body.gender || !body.ward || !body.priority) {
    toast('Please fill all fields including gender, priority, and ward', 'error'); return;
  }
  const data = await api('POST', '/api/register', body);
  if (data.ok) {
    toast(`✅ Patient ${body.name} registered (ID: ${data.patient?.id})`, 'success');
    e.target.reset();
    loadPatients();
    loadStats();
  } else {
    toast('❌ ' + data.message, 'error');
  }
  showPatientResult(data);
});

/* ── Queue / Emergency / Priority ───────────────────────────────────────── */
async function addToQueue() {
  const id = document.getElementById('queueId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('POST', `/api/queue/${id}`);
  toast(data.ok ? '✅ ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadWaiting(); loadStats(); loadPatients(); }
  showPatientResult(data);
}

async function addEmergency() {
  const id = document.getElementById('emergId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('POST', `/api/emergency/${id}`);
  toast(data.ok ? '🚨 ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadWaiting(); loadStats(); loadPatients(); }
  showPatientResult(data);
}

async function addPriorityQueue() {
  const id = document.getElementById('priorId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('POST', `/api/priority_queue/${id}`);
  toast(data.ok ? '⚡ ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadWaiting(); loadStats(); loadPatients(); }
  showPatientResult(data);
}

async function dischargePatient() {
  const id = document.getElementById('dischargeId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('POST', `/api/discharge/${id}`);
  toast(data.ok ? '🏠 Patient discharged' : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadPatients(); loadStats(); }
  showPatientResult(data);
}

async function addBill() {
  const id  = document.getElementById('billId').value;
  const amt = parseFloat(document.getElementById('billAmt').value);
  if (!id || isNaN(amt)) { toast('Enter Patient ID and amount', 'error'); return; }
  const data = await api('POST', `/api/add_bill/${id}`, { amount: amt });
  toast(data.ok ? `💳 ₹${amt} added to bill` : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadPatients(); loadStats(); }
  showPatientResult(data);
}

async function treatNext() {
  const data = await api('POST', '/api/treat');
  toast(data.ok ? '🏥 ' + data.message + ' (' + (data.source || '') + ')' : '❌ ' + data.message,
        data.ok ? 'success' : 'error');
  if (data.ok) { loadWaiting(); loadStats(); loadPatients(); }
  showPatientResult(data);
}

async function treatPriority() {
  const data = await api('POST', '/api/treat_priority');
  toast(data.ok ? '⚡ ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadWaiting(); loadStats(); loadPatients(); }
  showPatientResult(data);
}

/* ── Search ──────────────────────────────────────────────────────────────── */
async function searchById() {
  const id = document.getElementById('searchId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('GET', `/api/search/${id}`);
  toast(data.ok ? '🔍 BST search complete' : '❌ ' + data.message, data.ok ? 'info' : 'error');
  showPatientResult(data);
}

async function binarySearchById() {
  const id = document.getElementById('searchId').value;
  if (!id) { toast('Enter a Patient ID', 'error'); return; }
  const data = await api('GET', `/api/binary_search/${id}`);
  toast(data.ok ? '🎯 Binary search complete' : '❌ ' + data.message, data.ok ? 'info' : 'error');
  showPatientResult(data);
}

async function searchByName() {
  const kw = document.getElementById('searchName').value.trim();
  if (!kw) { toast('Enter a search keyword', 'error'); return; }
  const data = await api('GET', `/api/search_name/${encodeURIComponent(kw)}`);
  toast(data.ok ? `🔍 Linear search: ${data.results?.length || 0} result(s)` : '❌ ' + data.message,
        data.ok ? 'info' : 'error');
  if (data.ok) {
    const el = document.getElementById('results');
    if (!data.results?.length) {
      el.innerHTML = '<p class="hint">No results found.</p>';
    } else {
      el.innerHTML = data.results.map(renderPatientResult).join('');
    }
  }
}

async function hashLookup() {
  const name = document.getElementById('searchName').value.trim();
  if (!name) { toast('Enter an exact patient name', 'error'); return; }
  const data = await api('GET', `/api/hash_lookup/${encodeURIComponent(name)}`);
  toast(data.ok ? '#️⃣ Hash lookup hit' : '❌ ' + data.message, data.ok ? 'info' : 'error');
  showPatientResult(data);
}

/* ── Sort ────────────────────────────────────────────────────────────────── */
async function sortBy(field) {
  const data = await api('GET', `/api/sort/${field}`);
  if (!data.ok) { toast('❌ ' + data.message, 'error'); return; }
  const algo = { name: 'Merge Sort', age: 'Quick Sort', priority: 'Heap Sort', id: 'Insertion Sort', bill: 'Bubble Sort' };
  toast(`🔃 ${algo[field] || 'Sort'} by ${field} complete`, 'info');
  const el = document.getElementById('results');
  el.innerHTML = `<strong>${algo[field] || 'Sort'} by ${field}:</strong><br/>` +
    (data.patients || []).map(renderPatientResult).join('');
  renderTable(data.patients || []);
}

async function loadBST() {
  const data = await api('GET', '/api/bst_list');
  if (!data.ok) { toast('❌ ' + data.message, 'error'); return; }
  toast('🌳 BST in-order traversal (sorted by ID)', 'info');
  renderTable(data.patients_sorted_by_id || []);
}

/* ── Waiting Lists ───────────────────────────────────────────────────────── */
let activeTab = 'emergency';

function switchTab(tab) {
  activeTab = tab;
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
  event.target.classList.add('active');
  renderWaiting(window._waitingData);
}

async function loadWaiting() {
  const data = await api('GET', '/api/waiting');
  window._waitingData = data;
  renderWaiting(data);
}

function renderWaiting(data) {
  const el = document.getElementById('waitingContent');
  if (!data || !data.ok) { el.innerHTML = '<p class="hint">Could not load waiting list.</p>'; return; }

  let patients = [];
  if (activeTab === 'emergency') patients = data.emergency_stack || [];
  else if (activeTab === 'queue') patients = data.regular_queue || [];
  else {
    // priority queue embedded JSON
    const pqData = data.priority_queue;
    patients = Array.isArray(pqData) ? pqData :
               (pqData && pqData.priority_queue) ? pqData.priority_queue : [];
  }

  if (!patients.length) {
    el.innerHTML = '<p class="hint">No patients in this list.</p>'; return;
  }

  el.innerHTML = patients.map(p => {
    const cls = activeTab === 'emergency' ? 'emergency' :
                (p.priority === 1 ? 'critical' : p.priority === 2 ? 'urgent' : 'normal');
    return `
      <div class="patient-pill ${cls}">
        <span class="pill-id">#${p.id}</span>
        <span class="pill-name">${p.name}</span>
        <span class="pill-cond">${p.condition}</span>
        <span class="pill-prio prio-${p.priority}">${['', 'Critical', 'Urgent', 'Normal'][p.priority] || ''}</span>
      </div>`;
  }).join('');
}

/* ── Patients Table ──────────────────────────────────────────────────────── */
async function loadPatients() {
  const data = await api('GET', '/api/patients');
  if (!data.ok) return;
  renderTable(data.patients || []);
}

function renderTable(patients) {
  const tbody = document.getElementById('patientBody');
  if (!patients.length) {
    tbody.innerHTML = '<tr><td colspan="10" style="text-align:center;color:var(--muted);padding:30px">No patient records found.</td></tr>';
    return;
  }
  tbody.innerHTML = patients.map(p => `
    <tr>
      <td><strong>#${p.id}</strong></td>
      <td>${p.name}</td>
      <td>${p.age}</td>
      <td>${p.gender}</td>
      <td title="${p.condition}">${p.condition.length > 20 ? p.condition.slice(0,20)+'…' : p.condition}</td>
      <td><span class="pill-prio prio-${p.priority}">${['', '🔴 Critical', '🟡 Urgent', '🟢 Normal'][p.priority] || p.priority}</span></td>
      <td>${p.ward}</td>
      <td><span class="status-badge status-${statusClass(p.status)}">${p.status}</span></td>
      <td>₹${p.bill?.toFixed(2)}</td>
      <td>
        <div class="action-btns">
          <button class="btn btn-blue btn-xs" onclick="quickQueue(${p.id})">Queue</button>
          <button class="btn btn-red btn-xs" onclick="quickEmerg(${p.id})">🚨</button>
          <button class="btn btn-green btn-xs" onclick="quickDischarge(${p.id})">🏠</button>
        </div>
      </td>
    </tr>`).join('');
}

async function quickQueue(id) {
  const data = await api('POST', `/api/queue/${id}`);
  toast(data.ok ? '✅ ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadPatients(); loadWaiting(); loadStats(); }
}

async function quickEmerg(id) {
  const data = await api('POST', `/api/emergency/${id}`);
  toast(data.ok ? '🚨 ' + data.message : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadPatients(); loadWaiting(); loadStats(); }
}

async function quickDischarge(id) {
  const data = await api('POST', `/api/discharge/${id}`);
  toast(data.ok ? '🏠 Discharged' : '❌ ' + data.message, data.ok ? 'success' : 'error');
  if (data.ok) { loadPatients(); loadStats(); }
}

/* ── Wards ───────────────────────────────────────────────────────────────── */
async function loadWards() {
  const data = await api('GET', '/api/wards');
  if (!data.ok) { toast('❌ Could not load wards', 'error'); return; }
  renderWards(data.wards || []);
}

function renderWards(wards) {
  const el = document.getElementById('wardContent');
  el.innerHTML = wards.map(w => {
    const pct = w.capacity > 0 ? Math.round(w.load / w.capacity * 100) : 0;
    return `
      <div class="ward-card">
        <div class="ward-name">${w.name}</div>
        <div class="ward-load">${w.load}/${w.capacity} beds (${pct}%)</div>
        <div class="ward-bar"><div class="ward-fill ${pct >= 90 ? 'full' : ''}" style="width:${pct}%"></div></div>
        <div style="font-size:.72rem;color:var(--muted);margin-top:4px">
          Links: ${(w.adjacent || []).map(a => a.ward).join(', ')}
        </div>
      </div>`;
  }).join('');
}

async function runBFS() {
  const ward = document.getElementById('bfsWard').value;
  const data = await api('GET', `/api/bfs/${encodeURIComponent(ward)}`);
  if (!data.ok) { toast('❌ ' + data.message, 'error'); return; }
  toast(`🔵 BFS from ${ward}: ${data.traversal?.join(' → ')}`, 'info');
  showResult(data);
}

async function runDFS() {
  const data = await api('GET', '/api/dfs');
  toast('🟣 DFS traversal complete', 'info');
  showResult(data);
}

/* ── Init ────────────────────────────────────────────────────────────────── */
async function init() {
  await Promise.all([loadPatients(), loadWaiting(), loadWards(), loadStats()]);
}

init();
setInterval(loadStats, 15000);  /* auto-refresh stats every 15s */
