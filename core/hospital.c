/*
 * CarePoint Hospital Management System
 * DSA Core Implementation in C
 *
 * Data Structures Used:
 *   1. Array           – Patient record store
 *   2. Doubly Linked List – Action history / audit log
 *   3. Stack (LIFO)    – Emergency patient handling
 *   4. Queue (FIFO)    – Regular waiting list
 *   5. Min-Heap        – Priority-based treatment queue
 *   6. BST             – Fast patient lookup by ID
 *   7. Hash Table      – Fast lookup by name (separate chaining)
 *   8. Graph           – Ward adjacency + BFS/DFS transfer paths
 *
 * Algorithms:
 *   - Merge Sort   (by name)
 *   - Quick Sort   (by age)
 *   - Heap Sort    (by priority)
 *   - Insertion Sort (by ID)
 *   - Bubble Sort  (by bill, demonstration)
 *   - Linear Search
 *   - Binary Search (on sorted ID array)
 *   - BFS / DFS on ward graph
 *
 * Build:
 *   gcc -Wall -O2 -o hospital hospital.c
 *
 * Usage:
 *   hospital <command> [args...]
 */

#include "hospital.h"

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 0 – JSON Helpers
 * ════════════════════════════════════════════════════════════════════════════*/

void json_string(const char *s) {
    putchar('"');
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') putchar('\\');
        if (*s == '\n')      printf("\\n");
        else if (*s == '\r') printf("\\r");
        else if (*s == '\t') printf("\\t");
        else if ((unsigned char)*s >= 32) putchar(*s);
    }
    putchar('"');
}

void patient_json(Patient *p) {
    printf("{\"id\":%d,\"name\":", p->id);
    json_string(p->name);
    printf(",\"age\":%d,\"gender\":", p->age);
    json_string(p->gender);
    printf(",\"phone\":");
    json_string(p->phone);
    printf(",\"condition\":");
    json_string(p->condition);
    printf(",\"status\":");
    json_string(p->status);
    printf(",\"ward\":");
    json_string(p->ward);
    printf(",\"priority\":%d,\"bill\":%.2f,\"admit_date\":", p->priority, p->bill);
    json_string(p->admit_date);
    printf("}");
}

void ok_msg(const char *msg) {
    printf("{\"ok\":true,\"message\":");
    json_string(msg);
    printf("}\n");
}

void error_msg(const char *msg) {
    printf("{\"ok\":false,\"message\":");
    json_string(msg);
    printf("}\n");
}

static char *now_str(char *buf, int len) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
    return buf;
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 1 – Array Operations
 * ════════════════════════════════════════════════════════════════════════════*/

int array_add_patient(PatientArray *arr, Patient *p) {
    if (arr->count >= MAX_PATIENTS) return -1;
    p->id = arr->next_id++;
    p->active = 1;
    arr->data[arr->count] = *p;
    return arr->count++;
}

int array_find_by_id(PatientArray *arr, int id) {
    for (int i = 0; i < arr->count; i++)
        if (arr->data[i].id == id && arr->data[i].active) return i;
    return -1;
}

void array_delete_patient(PatientArray *arr, int idx) {
    arr->data[idx].active = 0;
    strcpy(arr->data[idx].status, "Discharged");
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 2 – Doubly Linked List (History / Audit Log)
 * ════════════════════════════════════════════════════════════════════════════*/

HistoryList *history_create(void) {
    HistoryList *list = (HistoryList *)malloc(sizeof(HistoryList));
    list->head = list->tail = NULL;
    list->size = 0;
    return list;
}

void history_push(HistoryList *list, const char *action) {
    LogNode *node = (LogNode *)malloc(sizeof(LogNode));
    strncpy(node->action, action, 255);
    node->action[255] = '\0';
    now_str(node->timestamp, sizeof(node->timestamp));
    node->prev = node->next = NULL;

    if (!list->tail) {
        list->head = list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

void history_print_all(HistoryList *list) {
    printf("{\"ok\":true,\"history\":[");
    LogNode *cur = list->head;
    int first = 1;
    while (cur) {
        if (!first) putchar(',');
        first = 0;
        printf("{\"time\":");
        json_string(cur->timestamp);
        printf(",\"action\":");
        json_string(cur->action);
        printf("}");
        cur = cur->next;
    }
    printf("],\"count\":%d}\n", list->size);
}

void history_print_recent(HistoryList *list, int n) {
    /* traverse from tail backwards */
    printf("{\"ok\":true,\"recent_history\":[");
    LogNode *cur = list->tail;
    int count = 0, first = 1;
    while (cur && count < n) {
        if (!first) putchar(',');
        first = 0;
        printf("{\"time\":");
        json_string(cur->timestamp);
        printf(",\"action\":");
        json_string(cur->action);
        printf("}");
        cur = cur->prev;
        count++;
    }
    printf("]}\n");
}

void history_free(HistoryList *list) {
    LogNode *cur = list->head;
    while (cur) {
        LogNode *next = cur->next;
        free(cur);
        cur = next;
    }
    free(list);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 3 – Stack (Emergency Patients, LIFO)
 * ════════════════════════════════════════════════════════════════════════════*/

void stack_init(EmergencyStack *s) { s->top = -1; }

int stack_push(EmergencyStack *s, int idx) {
    if (s->top >= MAX_PATIENTS - 1) return 0;
    s->items[++s->top] = idx;
    return 1;
}

int stack_pop(EmergencyStack *s) {
    if (s->top < 0) return -1;
    return s->items[s->top--];
}

int stack_peek(EmergencyStack *s) {
    if (s->top < 0) return -1;
    return s->items[s->top];
}

int stack_is_empty(EmergencyStack *s) { return s->top < 0; }

void stack_print(EmergencyStack *s, PatientArray *arr) {
    printf("{\"ok\":true,\"emergency_stack\":[");
    for (int i = s->top; i >= 0; i--) {
        if (i < s->top) putchar(',');
        patient_json(&arr->data[s->items[i]]);
    }
    printf("],\"size\":%d}\n", s->top + 1);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 4 – Circular Queue (Regular Waiting List, FIFO)
 * ════════════════════════════════════════════════════════════════════════════*/

void queue_init(WaitingQueue *q) {
    q->front = 0; q->rear = -1; q->size = 0;
}

int queue_enqueue(WaitingQueue *q, int idx) {
    if (q->size >= MAX_PATIENTS) return 0;
    q->rear = (q->rear + 1) % MAX_PATIENTS;
    q->items[q->rear] = idx;
    q->size++;
    return 1;
}

int queue_dequeue(WaitingQueue *q) {
    if (q->size == 0) return -1;
    int val = q->items[q->front];
    q->front = (q->front + 1) % MAX_PATIENTS;
    q->size--;
    return val;
}

int queue_is_empty(WaitingQueue *q) { return q->size == 0; }

void queue_print(WaitingQueue *q, PatientArray *arr) {
    printf("{\"ok\":true,\"regular_queue\":[");
    for (int i = 0; i < q->size; i++) {
        if (i) putchar(',');
        int idx = q->items[(q->front + i) % MAX_PATIENTS];
        patient_json(&arr->data[idx]);
    }
    printf("],\"size\":%d}\n", q->size);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 5 – Min-Heap Priority Queue (Priority 1=Critical first)
 * ════════════════════════════════════════════════════════════════════════════*/

void heap_init(PriorityHeap *h) { h->size = 0; }

static int heap_priority(PriorityHeap *h, int i, PatientArray *arr) {
    return arr->data[h->items[i]].priority;
}

static void heap_swap(PriorityHeap *h, int i, int j) {
    int tmp = h->items[i]; h->items[i] = h->items[j]; h->items[j] = tmp;
}

static void heapify_up(PriorityHeap *h, int i, PatientArray *arr) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap_priority(h, parent, arr) > heap_priority(h, i, arr)) {
            heap_swap(h, parent, i);
            i = parent;
        } else break;
    }
}

static void heapify_down(PriorityHeap *h, int i, PatientArray *arr) {
    int n = h->size;
    while (1) {
        int smallest = i, l = 2*i+1, r = 2*i+2;
        if (l < n && heap_priority(h, l, arr) < heap_priority(h, smallest, arr)) smallest = l;
        if (r < n && heap_priority(h, r, arr) < heap_priority(h, smallest, arr)) smallest = r;
        if (smallest == i) break;
        heap_swap(h, i, smallest);
        i = smallest;
    }
}

void heap_insert(PriorityHeap *h, int idx, PatientArray *arr) {
    if (h->size >= MAX_PATIENTS) return;
    h->items[h->size++] = idx;
    heapify_up(h, h->size - 1, arr);
}

int heap_extract_min(PriorityHeap *h, PatientArray *arr) {
    if (h->size == 0) return -1;
    int min_idx = h->items[0];
    h->items[0] = h->items[--h->size];
    heapify_down(h, 0, arr);
    return min_idx;
}

void heap_print(PriorityHeap *h, PatientArray *arr) {
    printf("{\"ok\":true,\"priority_queue\":[");
    /* print in priority order by extracting (use a copy) */
    PriorityHeap copy = *h;
    int first = 1;
    while (copy.size > 0) {
        if (!first) putchar(',');
        first = 0;
        int idx = heap_extract_min(&copy, arr);
        patient_json(&arr->data[idx]);
    }
    printf("],\"size\":%d}\n", h->size);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 6 – Binary Search Tree (Patient ID index)
 * ════════════════════════════════════════════════════════════════════════════*/

BSTNode *bst_insert(BSTNode *root, int idx, int id) {
    if (!root) {
        BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
        node->patient_idx = idx;
        node->id = id;
        node->left = node->right = NULL;
        return node;
    }
    if (id < root->id) root->left  = bst_insert(root->left,  idx, id);
    else               root->right = bst_insert(root->right, idx, id);
    return root;
}

BSTNode *bst_search(BSTNode *root, int id) {
    if (!root || root->id == id) return root;
    if (id < root->id) return bst_search(root->left,  id);
    else               return bst_search(root->right, id);
}

void bst_inorder_json(BSTNode *root, PatientArray *arr, int *first) {
    if (!root) return;
    bst_inorder_json(root->left, arr, first);
    if (!*first) putchar(',');
    *first = 0;
    patient_json(&arr->data[root->patient_idx]);
    bst_inorder_json(root->right, arr, first);
}

void bst_free(BSTNode *root) {
    if (!root) return;
    bst_free(root->left);
    bst_free(root->right);
    free(root);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 7 – Hash Table (Separate Chaining by Name)
 * ════════════════════════════════════════════════════════════════════════════*/

HashTable *hash_create(void) {
    HashTable *ht = (HashTable *)calloc(1, sizeof(HashTable));
    return ht;
}

unsigned hash_fn(const char *key) {
    unsigned h = 5381;
    for (; *key; key++) h = ((h << 5) + h) ^ (unsigned char)*key;
    return h % HASH_SIZE;
}

void hash_insert(HashTable *ht, int idx, const char *name) {
    unsigned slot = hash_fn(name);
    HashEntry *entry = (HashEntry *)malloc(sizeof(HashEntry));
    entry->patient_idx = idx;
    strncpy(entry->key, name, NAME_LEN - 1);
    entry->key[NAME_LEN - 1] = '\0';
    entry->occupied = 1;
    entry->next = ht->buckets[slot];
    ht->buckets[slot] = entry;
}

int hash_search(HashTable *ht, const char *name) {
    unsigned slot = hash_fn(name);
    HashEntry *e = ht->buckets[slot];
    while (e) {
        if (strcmp(e->key, name) == 0) return e->patient_idx;
        e = e->next;
    }
    return -1;
}

void hash_free(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *e = ht->buckets[i];
        while (e) {
            HashEntry *next = e->next;
            free(e); e = next;
        }
    }
    free(ht);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 8 – Graph (Ward Adjacency List) + BFS + DFS
 * ════════════════════════════════════════════════════════════════════════════*/

void graph_init(WardGraph *g) {
    g->ward_count = 0;
    memset(g->adj, 0, sizeof(g->adj));
    /* Seed default wards */
    const char *wards[] = {"ICU","General","Pediatrics","Orthopedics",
                           "Cardiology","Neurology","Oncology","Emergency"};
    int caps[]          = { 10,   50,       20,          30,
                             25,     20,          15,         15};
    for (int i = 0; i < 8; i++) graph_add_ward(g, wards[i], caps[i]);

    /* Add adjacency edges (bidirectional) */
    int edges[][3] = {
        {0,1,2},{0,7,1},{1,2,3},{1,3,3},{1,4,2},{1,5,4},
        {2,3,5},{3,4,3},{4,5,2},{5,6,3},{6,7,4},{7,0,1}
    };
    for (int i = 0; i < 12; i++) {
        graph_add_edge(g, edges[i][0], edges[i][1], edges[i][2]);
        graph_add_edge(g, edges[i][1], edges[i][0], edges[i][2]);
    }
}

void graph_add_ward(WardGraph *g, const char *name, int cap) {
    if (g->ward_count >= MAX_WARDS) return;
    int i = g->ward_count++;
    g->wards[i].ward_id = i;
    strncpy(g->wards[i].ward_name, name, 31);
    g->wards[i].capacity = cap;
    g->wards[i].current_load = 0;
    g->wards[i].adj_next = NULL;
    g->adj[i] = NULL;
}

void graph_add_edge(WardGraph *g, int from, int to, int weight) {
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    node->to_ward = to;
    node->weight  = weight;
    node->next    = g->adj[from];
    g->adj[from]  = node;
}

int graph_find_ward(WardGraph *g, const char *name) {
    for (int i = 0; i < g->ward_count; i++)
        if (strcasecmp(g->wards[i].ward_name, name) == 0) return i;
    return -1;
}

/* BFS from a start ward – returns JSON array of visited wards */
void graph_bfs(WardGraph *g, int start) {
    int visited[MAX_WARDS] = {0};
    int q[MAX_WARDS], front = 0, rear = 0;
    q[rear++] = start;
    visited[start] = 1;
    printf("{\"ok\":true,\"bfs_from\":");
    json_string(g->wards[start].ward_name);
    printf(",\"traversal\":[");
    int first = 1;
    while (front < rear) {
        int curr = q[front++];
        if (!first) putchar(',');
        first = 0;
        json_string(g->wards[curr].ward_name);
        AdjNode *adj = g->adj[curr];
        while (adj) {
            if (!visited[adj->to_ward]) {
                visited[adj->to_ward] = 1;
                q[rear++] = adj->to_ward;
            }
            adj = adj->next;
        }
    }
    printf("]}\n");
}

void graph_dfs(WardGraph *g, int start, int visited[]) {
    visited[start] = 1;
    printf("{\"ward\":");
    json_string(g->wards[start].ward_name);
    printf(",\"load\":%d,\"capacity\":%d}",
           g->wards[start].current_load, g->wards[start].capacity);
    AdjNode *adj = g->adj[start];
    while (adj) {
        if (!visited[adj->to_ward])
            graph_dfs(g, adj->to_ward, visited);
        adj = adj->next;
    }
}

void graph_print_json(WardGraph *g) {
    printf("{\"ok\":true,\"wards\":[");
    for (int i = 0; i < g->ward_count; i++) {
        if (i) putchar(',');
        printf("{\"id\":%d,\"name\":", g->wards[i].ward_id);
        json_string(g->wards[i].ward_name);
        printf(",\"capacity\":%d,\"load\":%d,\"adjacent\":[",
               g->wards[i].capacity, g->wards[i].current_load);
        AdjNode *adj = g->adj[i];
        int first = 1;
        while (adj) {
            if (!first) putchar(',');
            first = 0;
            printf("{\"ward\":");
            json_string(g->wards[adj->to_ward].ward_name);
            printf(",\"weight\":%d}", adj->weight);
            adj = adj->next;
        }
        printf("]}");
    }
    printf("]}\n");
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 9 – Sorting Algorithms
 * ════════════════════════════════════════════════════════════════════════════*/

/* --- Merge Sort by Name --- */
static void merge_by_name(Patient *arr, int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    Patient *L = (Patient *)malloc(n1 * sizeof(Patient));
    Patient *R = (Patient *)malloc(n2 * sizeof(Patient));
    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (strcasecmp(L[i].name, R[j].name) <= 0) arr[k++] = L[i++];
        else                                         arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    free(L); free(R);
}

static void merge_sort_name(Patient *arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        merge_sort_name(arr, l, m);
        merge_sort_name(arr, m + 1, r);
        merge_by_name(arr, l, m, r);
    }
}

void sort_by_name(Patient *arr, int n) {
    if (n > 1) merge_sort_name(arr, 0, n - 1);
}

/* --- Quick Sort by Age --- */
static int partition_age(Patient *arr, int low, int high) {
    int pivot = arr[high].age;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j].age <= pivot) {
            i++;
            Patient tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    }
    Patient tmp = arr[i+1]; arr[i+1] = arr[high]; arr[high] = tmp;
    return i + 1;
}

static void quick_sort_age(Patient *arr, int low, int high) {
    if (low < high) {
        int pi = partition_age(arr, low, high);
        quick_sort_age(arr, low, pi - 1);
        quick_sort_age(arr, pi + 1, high);
    }
}

void sort_by_age(Patient *arr, int n) {
    if (n > 1) quick_sort_age(arr, 0, n - 1);
}

/* --- Heap Sort by Priority --- */
static void heapify_priority(Patient *arr, int n, int i) {
    int largest = i, l = 2*i+1, r = 2*i+2;
    if (l < n && arr[l].priority > arr[largest].priority) largest = l;
    if (r < n && arr[r].priority > arr[largest].priority) largest = r;
    if (largest != i) {
        Patient tmp = arr[i]; arr[i] = arr[largest]; arr[largest] = tmp;
        heapify_priority(arr, n, largest);
    }
}

void sort_by_priority(Patient *arr, int n) {
    for (int i = n/2 - 1; i >= 0; i--) heapify_priority(arr, n, i);
    for (int i = n - 1; i > 0; i--) {
        Patient tmp = arr[0]; arr[0] = arr[i]; arr[i] = tmp;
        heapify_priority(arr, i, 0);
    }
}

/* --- Insertion Sort by ID --- */
void sort_by_id(Patient *arr, int n) {
    for (int i = 1; i < n; i++) {
        Patient key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].id > key.id) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/* --- Bubble Sort by Bill (demonstration) --- */
void sort_by_bill(Patient *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j].bill > arr[j+1].bill) {
                Patient tmp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 10 – Search Algorithms
 * ════════════════════════════════════════════════════════════════════════════*/

void linear_search_print(PatientArray *arr, const char *keyword) {
    printf("{\"ok\":true,\"results\":[");
    int first = 1;
    for (int i = 0; i < arr->count; i++) {
        if (!arr->data[i].active) continue;
        if (strcasestr(arr->data[i].name, keyword) ||
            strcasestr(arr->data[i].condition, keyword) ||
            strcasestr(arr->data[i].ward, keyword)) {
            if (!first) putchar(',');
            first = 0;
            patient_json(&arr->data[i]);
        }
    }
    printf("]}\n");
}

int binary_search_by_id(Patient *sorted, int n, int id) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (sorted[mid].id == id) return mid;
        if (sorted[mid].id < id)  lo = mid + 1;
        else                       hi = mid - 1;
    }
    return -1;
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 11 – File Persistence
 * ════════════════════════════════════════════════════════════════════════════*/

void save_hospital(Hospital *h) {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) { fprintf(stderr, "Cannot save data\n"); return; }
    fwrite(&h->patients,       sizeof(h->patients),       1, f);
    fwrite(&h->emergency,      sizeof(h->emergency),      1, f);
    fwrite(&h->regular_queue,  sizeof(h->regular_queue),  1, f);
    fwrite(&h->priority_queue, sizeof(h->priority_queue), 1, f);
    /* Ward loads are dynamic, save them */
    fwrite(h->wards.wards, sizeof(GraphNode), h->wards.ward_count, f);
    fclose(f);
}

void load_hospital(Hospital *h) {
    memset(h, 0, sizeof(*h));
    h->patients.next_id = 1001;
    stack_init(&h->emergency);
    queue_init(&h->regular_queue);
    heap_init(&h->priority_queue);
    graph_init(&h->wards);

    FILE *f = fopen(DATA_FILE, "rb");
    if (!f) return;
    if (fread(&h->patients,       sizeof(h->patients),       1, f) != 1) goto done;
    if (fread(&h->emergency,      sizeof(h->emergency),      1, f) != 1) goto done;
    if (fread(&h->regular_queue,  sizeof(h->regular_queue),  1, f) != 1) goto done;
    if (fread(&h->priority_queue, sizeof(h->priority_queue), 1, f) != 1) goto done;
    fread(h->wards.wards, sizeof(GraphNode), h->wards.ward_count, f);
done:
    fclose(f);
}

void log_action(const char *action) {
    FILE *f = fopen(LOG_FILE, "a");
    if (!f) return;
    char ts[32]; now_str(ts, sizeof(ts));
    fprintf(f, "[%s] %s\n", ts, action);
    fclose(f);
}

/* ════════════════════════════════════════════════════════════════════════════
 * SECTION 12 – Main Entry Point (CLI command dispatcher)
 * ════════════════════════════════════════════════════════════════════════════*/

int main(int argc, char **argv) {
    static Hospital h;
    load_hospital(&h);

    /* Rebuild BST and Hash Table in memory from patient array */
    BSTNode  *bst = NULL;
    HashTable *ht = hash_create();
    for (int i = 0; i < h.patients.count; i++) {
        if (h.patients.data[i].active) {
            bst = bst_insert(bst, i, h.patients.data[i].id);
            hash_insert(ht, i, h.patients.data[i].name);
        }
    }

    if (argc < 2) { error_msg("Missing command"); goto cleanup; }

    const char *cmd = argv[1];

    /* ── register <name> <age> <gender> <phone> <condition> <priority> <ward> ── */
    if (strcmp(cmd, "register") == 0) {
        if (argc < 9) { error_msg("Usage: register <name> <age> <gender> <phone> <condition> <priority 1-3> <ward>"); goto cleanup; }
        Patient p; memset(&p, 0, sizeof(p));
        strncpy(p.name, argv[2], NAME_LEN - 1);
        p.age = atoi(argv[3]);
        strncpy(p.gender, argv[4], 15);
        strncpy(p.phone, argv[5], PHONE_LEN - 1);
        strncpy(p.condition, argv[6], COND_LEN - 1);
        p.priority = atoi(argv[7]);
        strncpy(p.ward, argv[8], 31);
        now_str(p.admit_date, sizeof(p.admit_date));
        strcpy(p.status, "Registered");
        p.bill = 500.0;  /* base registration fee */

        if (!p.name[0] || p.age < 1 || p.age > 120) { error_msg("Invalid name or age"); goto cleanup; }
        if (p.priority < 1 || p.priority > 3) { error_msg("Priority must be 1 (Critical), 2 (Urgent), or 3 (Normal)"); goto cleanup; }

        int idx = array_add_patient(&h.patients, &p);
        if (idx < 0) { error_msg("Hospital at full capacity"); goto cleanup; }

        /* Update BST and Hash */
        bst = bst_insert(bst, idx, h.patients.data[idx].id);
        hash_insert(ht, idx, h.patients.data[idx].name);

        /* Update ward load */
        int ward_id = graph_find_ward(&h.wards, p.ward);
        if (ward_id >= 0) h.wards.wards[ward_id].current_load++;

        char log_buf[256];
        snprintf(log_buf, sizeof(log_buf), "Registered patient %s (ID:%d, Priority:%d, Ward:%s)",
                 p.name, h.patients.data[idx].id, p.priority, p.ward);
        log_action(log_buf);

        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient registered successfully\",\"patient\":");
        patient_json(&h.patients.data[idx]);
        printf("}\n");
    }

    /* ── queue <patient_id>  – add to regular FIFO queue ── */
    else if (strcmp(cmd, "queue") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        int id = atoi(argv[2]);
        int i = array_find_by_id(&h.patients, id);
        if (i < 0) { error_msg("Patient not found"); goto cleanup; }
        if (strcmp(h.patients.data[i].status, "Registered") != 0) {
            error_msg("Patient already queued or under treatment"); goto cleanup;
        }
        if (!queue_enqueue(&h.regular_queue, i)) { error_msg("Queue is full"); goto cleanup; }
        strcpy(h.patients.data[i].status, "Regular Queue");
        char buf[128]; snprintf(buf, sizeof(buf), "Patient ID:%d added to regular queue", id);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient added to regular waiting queue\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── emergency <patient_id>  – push onto emergency LIFO stack ── */
    else if (strcmp(cmd, "emergency") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        int id = atoi(argv[2]);
        int i = array_find_by_id(&h.patients, id);
        if (i < 0) { error_msg("Patient not found"); goto cleanup; }
        if (!stack_push(&h.emergency, i)) { error_msg("Emergency stack full"); goto cleanup; }
        strcpy(h.patients.data[i].status, "Emergency");
        char buf[128]; snprintf(buf, sizeof(buf), "Patient ID:%d pushed to emergency stack", id);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient added to emergency stack\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── priority_queue <patient_id>  – insert into min-heap by priority ── */
    else if (strcmp(cmd, "priority_queue") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        int id = atoi(argv[2]);
        int i = array_find_by_id(&h.patients, id);
        if (i < 0) { error_msg("Patient not found"); goto cleanup; }
        heap_insert(&h.priority_queue, i, &h.patients);
        strcpy(h.patients.data[i].status, "Priority Queue");
        char buf[128]; snprintf(buf, sizeof(buf), "Patient ID:%d inserted in priority queue (priority=%d)", id, h.patients.data[i].priority);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient inserted into priority queue\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── treat  – treat next patient (emergency stack first, then regular queue) ── */
    else if (strcmp(cmd, "treat") == 0) {
        int i = -1;
        const char *src = "";
        if (!stack_is_empty(&h.emergency)) {
            i = stack_pop(&h.emergency); src = "Emergency Stack";
        } else if (!queue_is_empty(&h.regular_queue)) {
            i = queue_dequeue(&h.regular_queue); src = "Regular Queue";
        }
        if (i < 0) { error_msg("No patients waiting for treatment"); goto cleanup; }
        strcpy(h.patients.data[i].status, "Under Treatment");
        h.patients.data[i].bill += 1500.0;  /* treatment fee */
        char buf[256]; snprintf(buf, sizeof(buf), "Patient ID:%d treated (from %s)", h.patients.data[i].id, src);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient sent for treatment\",\"source\":");
        json_string(src);
        printf(",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── treat_priority  – treat from min-heap priority queue ── */
    else if (strcmp(cmd, "treat_priority") == 0) {
        int i = heap_extract_min(&h.priority_queue, &h.patients);
        if (i < 0) { error_msg("Priority queue is empty"); goto cleanup; }
        strcpy(h.patients.data[i].status, "Under Treatment");
        h.patients.data[i].bill += 2000.0;
        char buf[128]; snprintf(buf, sizeof(buf), "Patient ID:%d treated (from priority queue)", h.patients.data[i].id);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Highest-priority patient sent for treatment\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── discharge <patient_id>  ── */
    else if (strcmp(cmd, "discharge") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        int id = atoi(argv[2]);
        int i = array_find_by_id(&h.patients, id);
        if (i < 0) { error_msg("Patient not found"); goto cleanup; }
        array_delete_patient(&h.patients, i);
        h.patients.data[i].bill += 200.0; /* discharge fee */
        int ward_id = graph_find_ward(&h.wards, h.patients.data[i].ward);
        if (ward_id >= 0 && h.wards.wards[ward_id].current_load > 0)
            h.wards.wards[ward_id].current_load--;
        char buf[128]; snprintf(buf, sizeof(buf), "Patient ID:%d discharged", id);
        log_action(buf);
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Patient discharged\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── add_bill <patient_id> <amount>  ── */
    else if (strcmp(cmd, "add_bill") == 0) {
        if (argc < 4) { error_msg("Usage: add_bill <id> <amount>"); goto cleanup; }
        int id = atoi(argv[2]);
        double amount = atof(argv[3]);
        int i = array_find_by_id(&h.patients, id);
        if (i < 0) { error_msg("Patient not found"); goto cleanup; }
        h.patients.data[i].bill += amount;
        save_hospital(&h);
        printf("{\"ok\":true,\"message\":\"Bill updated\",\"patient\":");
        patient_json(&h.patients.data[i]);
        printf("}\n");
    }

    /* ── list  – all active patients ── */
    else if (strcmp(cmd, "list") == 0) {
        printf("{\"ok\":true,\"patients\":[");
        int first = 1;
        for (int i = 0; i < h.patients.count; i++) {
            if (!h.patients.data[i].active) continue;
            if (!first) putchar(',');
            first = 0;
            patient_json(&h.patients.data[i]);
        }
        printf("],\"total\":%d}\n", h.patients.count);
    }

    /* ── search <patient_id>  – BST lookup ── */
    else if (strcmp(cmd, "search") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        int id = atoi(argv[2]);
        BSTNode *node = bst_search(bst, id);
        if (!node) { error_msg("Patient not found"); goto cleanup; }
        printf("{\"ok\":true,\"patient\":");
        patient_json(&h.patients.data[node->patient_idx]);
        printf("}\n");
    }

    /* ── search_name <keyword>  – linear search ── */
    else if (strcmp(cmd, "search_name") == 0) {
        if (argc < 3) { error_msg("Keyword required"); goto cleanup; }
        linear_search_print(&h.patients, argv[2]);
    }

    /* ── hash_lookup <name>  – O(1) average name lookup ── */
    else if (strcmp(cmd, "hash_lookup") == 0) {
        if (argc < 3) { error_msg("Name required"); goto cleanup; }
        int idx = hash_search(ht, argv[2]);
        if (idx < 0) { error_msg("Patient not found in hash table"); goto cleanup; }
        printf("{\"ok\":true,\"patient\":");
        patient_json(&h.patients.data[idx]);
        printf("}\n");
    }

    /* ── bst_list  – in-order BST traversal (sorted by ID) ── */
    else if (strcmp(cmd, "bst_list") == 0) {
        printf("{\"ok\":true,\"patients_sorted_by_id\":[");
        int first = 1;
        bst_inorder_json(bst, &h.patients, &first);
        printf("]}\n");
    }

    /* ── sort <field>  – sort and return patients ── */
    else if (strcmp(cmd, "sort") == 0) {
        if (argc < 3) { error_msg("Sort field required: name|age|priority|id|bill"); goto cleanup; }

        /* Build a copy of active patients */
        Patient copy[MAX_PATIENTS]; int n = 0;
        for (int i = 0; i < h.patients.count; i++)
            if (h.patients.data[i].active) copy[n++] = h.patients.data[i];

        const char *field = argv[2];
        if      (strcmp(field, "name")     == 0) sort_by_name(copy, n);
        else if (strcmp(field, "age")      == 0) sort_by_age(copy, n);
        else if (strcmp(field, "priority") == 0) sort_by_priority(copy, n);
        else if (strcmp(field, "id")       == 0) sort_by_id(copy, n);
        else if (strcmp(field, "bill")     == 0) sort_by_bill(copy, n);
        else { error_msg("Unknown sort field. Use: name|age|priority|id|bill"); goto cleanup; }

        printf("{\"ok\":true,\"sorted_by\":");
        json_string(field);
        printf(",\"patients\":[");
        for (int i = 0; i < n; i++) {
            if (i) putchar(',');
            patient_json(&copy[i]);
        }
        printf("]}\n");
    }

    /* ── binary_search <id>  – binary search on id-sorted array ── */
    else if (strcmp(cmd, "binary_search") == 0) {
        if (argc < 3) { error_msg("Patient ID required"); goto cleanup; }
        Patient copy[MAX_PATIENTS]; int n = 0;
        for (int i = 0; i < h.patients.count; i++)
            if (h.patients.data[i].active) copy[n++] = h.patients.data[i];
        sort_by_id(copy, n);
        int pos = binary_search_by_id(copy, n, atoi(argv[2]));
        if (pos < 0) { error_msg("Patient not found (binary search)"); goto cleanup; }
        printf("{\"ok\":true,\"algorithm\":\"Binary Search\",\"patient\":");
        patient_json(&copy[pos]);
        printf("}\n");
    }

    /* ── waiting  – show emergency stack + regular queue ── */
    else if (strcmp(cmd, "waiting") == 0) {
        printf("{\"ok\":true,\"emergency_stack\":[");
        for (int j = h.emergency.top; j >= 0; j--) {
            if (j < h.emergency.top) putchar(',');
            patient_json(&h.patients.data[h.emergency.items[j]]);
        }
        printf("],\"regular_queue\":[");
        for (int j = 0; j < h.regular_queue.size; j++) {
            if (j) putchar(',');
            int idx = h.regular_queue.items[(h.regular_queue.front + j) % MAX_PATIENTS];
            patient_json(&h.patients.data[idx]);
        }
        printf("],\"priority_queue\":");
        heap_print(&h.priority_queue, &h.patients);
    }

    /* ── wards  – show ward graph ── */
    else if (strcmp(cmd, "wards") == 0) {
        graph_print_json(&h.wards);
    }

    /* ── bfs <ward_name>  – BFS traversal of ward graph ── */
    else if (strcmp(cmd, "bfs") == 0) {
        if (argc < 3) { error_msg("Ward name required"); goto cleanup; }
        int start = graph_find_ward(&h.wards, argv[2]);
        if (start < 0) { error_msg("Ward not found"); goto cleanup; }
        graph_bfs(&h.wards, start);
    }

    /* ── dfs  – DFS traversal of ward graph from ICU ── */
    else if (strcmp(cmd, "dfs") == 0) {
        int visited[MAX_WARDS] = {0};
        printf("{\"ok\":true,\"dfs_traversal\":[");
        graph_dfs(&h.wards, 0, visited);
        printf("]}\n");
    }

    /* ── stats  – summary statistics ── */
    else if (strcmp(cmd, "stats") == 0) {
        int active = 0, treated = 0, discharged = 0, critical = 0, urgent = 0, normal = 0;
        double total_bill = 0;
        for (int i = 0; i < h.patients.count; i++) {
            Patient *p = &h.patients.data[i];
            if (p->active) {
                active++;
                if (p->priority == 1) critical++;
                if (p->priority == 2) urgent++;
                if (p->priority == 3) normal++;
            }
            if (!strcmp(p->status, "Under Treatment") || !strcmp(p->status, "Treated")) treated++;
            if (!strcmp(p->status, "Discharged")) discharged++;
            total_bill += p->bill;
        }
        printf("{\"ok\":true,\"stats\":{\"total_registered\":%d,\"active\":%d,\"under_treatment\":%d,"
               "\"discharged\":%d,\"critical\":%d,\"urgent\":%d,\"normal\":%d,"
               "\"emergency_stack_size\":%d,\"regular_queue_size\":%d,\"priority_queue_size\":%d,"
               "\"total_revenue\":%.2f}}\n",
               h.patients.count, active, treated, discharged, critical, urgent, normal,
               h.emergency.top + 1, h.regular_queue.size, h.priority_queue.size, total_bill);
    }

    /* ── reset  – wipe all data ── */
    else if (strcmp(cmd, "reset") == 0) {
        remove(DATA_FILE);
        log_action("SYSTEM RESET - all patient data cleared");
        ok_msg("All patient data has been reset");
    }

    else {
        error_msg("Unknown command. Valid: register|queue|emergency|priority_queue|treat|treat_priority|discharge|add_bill|list|search|search_name|hash_lookup|bst_list|sort|binary_search|waiting|wards|bfs|dfs|stats|reset");
    }

cleanup:
    bst_free(bst);
    hash_free(ht);
    return 0;
}
