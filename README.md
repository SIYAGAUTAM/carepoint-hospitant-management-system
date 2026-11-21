# CarePoint – Hospital Management System
> **DSA Project in C** | Web UI · Express Bridge · Full DSA Implementation

---

## 📌 Project Overview

CarePoint is a **Data Structures & Algorithms (DSA) project** built entirely in **C**, with a modern web front-end powered by HTML/CSS/JavaScript and a lightweight Node.js + Express bridge.

The **entire DSA logic lives in `core/hospital.c`** — Node.js only compiles the C binary and proxies HTTP calls to it.

---

## 🧠 Data Structures Implemented (in C)

| # | Data Structure | Usage in System | Complexity |
|---|---------------|----------------|------------|
| 1 | **Array** | Patient record store (`PatientArray`) | O(1) access |
| 2 | **Doubly Linked List** | Audit log / action history | O(1) insert |
| 3 | **Stack (LIFO)** | Emergency patient handling | O(1) push/pop |
| 4 | **Circular Queue (FIFO)** | Regular waiting list | O(1) enqueue/dequeue |
| 5 | **Min-Heap** | Priority-based treatment queue | O(log n) insert/extract |
| 6 | **Binary Search Tree (BST)** | Patient ID index & lookup | O(log n) avg |
| 7 | **Hash Table** | Fast name lookup (separate chaining, djb2) | O(1) avg |
| 8 | **Graph (Adjacency List)** | Ward network + BFS + DFS | O(V+E) |

---

## ⚡ Algorithms Implemented (in C)

| Algorithm | Applied To | Complexity |
|-----------|-----------|-----------|
| **Merge Sort** | Sort patients by Name | O(n log n) |
| **Quick Sort** | Sort patients by Age | O(n log n) avg |
| **Heap Sort** | Sort patients by Priority | O(n log n) |
| **Insertion Sort** | Sort patients by ID | O(n²) worst, O(n) best |
| **Bubble Sort** | Sort patients by Bill (demo) | O(n²) |
| **Linear Search** | Search by name/condition/ward | O(n) |
| **Binary Search** | Search by ID on sorted array | O(log n) |
| **BFS** | Ward graph traversal | O(V+E) |
| **DFS** | Ward graph traversal | O(V+E) |

---

## 🗂️ Project Structure

```
carepoint-hospitant-management-system/
├── core/
│   ├── hospital.h       # Header: all struct definitions & prototypes
│   └── hospital.c       # Full DSA implementation in C (main file)
├── public/
│   ├── index.html       # Frontend UI
│   ├── style.css        # Modern responsive CSS
│   └── app.js           # JavaScript API bridge + UI logic
├── server.js            # Node.js + Express HTTP bridge
├── package.json
└── README.md
```

---

## 🚀 How to Run

### Prerequisites
- **GCC** (MinGW-w64 on Windows, or GCC on Linux/macOS)
- **Node.js** 18+

### Windows
```bash
npm install
npm start
```
Then open **http://localhost:3000** in your browser.

### Linux / macOS
```bash
npm install
npm start
```

### Manual C Compilation (optional)
```bash
# Windows
gcc -Wall -O2 -o core/hospital.exe core/hospital.c

# Linux / macOS
gcc -Wall -O2 -o core/hospital core/hospital.c
```

---

## 🌐 API Endpoints

| Method | Endpoint | DSA Used | Description |
|--------|----------|----------|-------------|
| POST | `/api/register` | Array, BST, Hash | Register new patient |
| POST | `/api/queue/:id` | Circular Queue | Add to regular waiting list |
| POST | `/api/emergency/:id` | Stack | Push to emergency stack |
| POST | `/api/priority_queue/:id` | Min-Heap | Insert by priority |
| POST | `/api/treat` | Stack + Queue | Treat next (emergency first) |
| POST | `/api/treat_priority` | Min-Heap | Treat highest priority |
| POST | `/api/discharge/:id` | Array | Discharge patient |
| POST | `/api/add_bill/:id` | Array | Add to patient bill |
| GET | `/api/patients` | Array | List all patients |
| GET | `/api/search/:id` | BST | O(log n) ID search |
| GET | `/api/binary_search/:id` | Binary Search | Binary search on sorted array |
| GET | `/api/search_name/:kw` | Linear Search | O(n) keyword search |
| GET | `/api/hash_lookup/:name` | Hash Table | O(1) avg name lookup |
| GET | `/api/bst_list` | BST In-order | Patients sorted by ID |
| GET | `/api/sort/:field` | 5 sort algos | Sort by name/age/priority/id/bill |
| GET | `/api/waiting` | Stack+Queue+Heap | All waiting lists |
| GET | `/api/wards` | Graph | Ward adjacency data |
| GET | `/api/bfs/:ward` | BFS | BFS ward traversal |
| GET | `/api/dfs` | DFS | DFS ward traversal |
| GET | `/api/stats` | — | System statistics |
| POST | `/api/reset` | — | Reset all data |

---

## 🏗️ Architecture

```
Browser (HTML + CSS + JS)
        │  HTTP REST
        ▼
  Node.js + Express  (server.js)
        │  execFileSync()
        ▼
  C Binary (core/hospital.c)
        │  fread/fwrite
        ▼
  hospital.dat  (binary persistence)
```

Node.js does **not** implement any DSA logic — it only:
1. Compiles the C core on first start
2. Receives HTTP requests and validates basic inputs
3. Calls the C binary with arguments
4. Returns the C program's JSON output to the browser

---

## 🔬 DSA Concept Deep-Dive

### Array (Patient Store)
```c
typedef struct {
    Patient data[MAX_PATIENTS];  // O(1) index access
    int     count;
    int     next_id;
} PatientArray;
```

### Doubly Linked List (History)
```c
typedef struct LogNode {
    char           action[256];
    char           timestamp[32];
    struct LogNode *prev;    // backward traversal
    struct LogNode *next;    // forward traversal
} LogNode;
```

### Stack (Emergency – LIFO)
```c
// Push: O(1)  |  Pop: O(1)
stack_push(&h.emergency, patient_idx);
int next = stack_pop(&h.emergency);  // last in = first out
```

### Circular Queue (Regular – FIFO)
```c
// Enqueue: O(1)  |  Dequeue: O(1)
q->rear = (q->rear + 1) % MAX_PATIENTS;  // wrap-around
```

### Min-Heap (Priority Queue)
```c
// Insert: O(log n)  |  Extract-min: O(log n)
// Priority 1 (Critical) is served before 2 (Urgent) before 3 (Normal)
heapify_up(h, h->size - 1, arr);    // bubble up after insert
heapify_down(h, 0, arr);             // sift down after extract
```

### BST (ID Index)
```c
// Avg O(log n) search
BSTNode *bst_search(BSTNode *root, int id) {
    if (!root || root->id == id) return root;
    return id < root->id ? bst_search(root->left, id)
                         : bst_search(root->right, id);
}
```

### Hash Table (Name → Index)
```c
// djb2 hash function, separate chaining collision resolution
unsigned hash_fn(const char *key) {
    unsigned h = 5381;
    for (; *key; key++) h = ((h << 5) + h) ^ (unsigned char)*key;
    return h % HASH_SIZE;  // HASH_SIZE = 503 (prime)
}
```

### Graph – Ward Adjacency List + BFS/DFS
```c
// 8 wards, 12 bidirectional edges
// BFS: level-order traversal using a queue
// DFS: depth-first using recursion
```

---

## 👤 Author
**SIYAGAUTAM** – DSA Project Submission

> ⚠️ **Academic Note:** The emergency stack (LIFO) is a simplified DSA demonstration. Real hospitals use clinical triage. This system is not intended for actual medical use.
