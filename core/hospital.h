/*
 * CarePoint Hospital Management System - DSA Core Header
 * Implements: Array, Linked List, Stack, Queue, Priority Queue (Min-Heap),
 *             BST, Hashing, Graph (Ward Adjacency), Sorting, File Persistence
 */

#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Constants ─────────────────────────────────────────────────────────── */
#define MAX_PATIENTS     1000
#define MAX_WARDS        20
#define HASH_SIZE        503      /* prime for hash table */
#define NAME_LEN         80
#define PHONE_LEN        24
#define COND_LEN         120
#define DATA_FILE        "hospital.dat"
#define LOG_FILE         "hospital_log.txt"

/* ─── Patient Record ─────────────────────────────────────────────────────── */
typedef struct Patient {
    int    id;
    int    age;
    int    priority;          /* 1=Critical, 2=Urgent, 3=Normal */
    char   name[NAME_LEN];
    char   gender[16];
    char   phone[PHONE_LEN];
    char   condition[COND_LEN];
    char   status[32];
    char   ward[32];
    char   admit_date[32];
    double bill;
    int    active;            /* 1=in system, 0=discharged/deleted */
} Patient;

/* ─── 1. Array Store ─────────────────────────────────────────────────────── */
typedef struct {
    Patient data[MAX_PATIENTS];
    int     count;
    int     next_id;
} PatientArray;

/* ─── 2. Doubly Linked List (Patient History Log) ───────────────────────── */
typedef struct LogNode {
    char           action[256];
    char           timestamp[32];
    struct LogNode *prev;
    struct LogNode *next;
} LogNode;

typedef struct {
    LogNode *head;
    LogNode *tail;
    int      size;
} HistoryList;

/* ─── 3. Stack (Emergency Patients - LIFO) ──────────────────────────────── */
typedef struct {
    int  items[MAX_PATIENTS];   /* indices into PatientArray */
    int  top;
} EmergencyStack;

/* ─── 4. Queue (Regular Waiting - FIFO) ─────────────────────────────────── */
typedef struct {
    int  items[MAX_PATIENTS];
    int  front;
    int  rear;
    int  size;
} WaitingQueue;

/* ─── 5. Min-Heap Priority Queue (Priority-based Treatment) ─────────────── */
typedef struct {
    int items[MAX_PATIENTS];    /* patient indices */
    int size;
} PriorityHeap;

/* ─── 6. BST (Search patients by ID) ────────────────────────────────────── */
typedef struct BSTNode {
    int            patient_idx;
    int            id;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

/* ─── 7. Hash Table (Quick lookup by name) ──────────────────────────────── */
typedef struct HashEntry {
    int              patient_idx;
    char             key[NAME_LEN];
    int              occupied;
    struct HashEntry *next;    /* chaining */
} HashEntry;

typedef struct {
    HashEntry *buckets[HASH_SIZE];
} HashTable;

/* ─── 8. Graph (Ward Adjacency - for patient transfer paths) ────────────── */
typedef struct GraphNode {
    int              ward_id;
    char             ward_name[32];
    int              capacity;
    int              current_load;
    struct GraphNode *adj_next;   /* adjacency list pointer */
} GraphNode;

typedef struct AdjNode {
    int             to_ward;
    int             weight;       /* transfer cost/distance */
    struct AdjNode *next;
} AdjNode;

typedef struct {
    GraphNode wards[MAX_WARDS];
    AdjNode  *adj[MAX_WARDS];
    int       ward_count;
} WardGraph;

/* ─── Hospital Master State ──────────────────────────────────────────────── */
typedef struct {
    PatientArray  patients;
    EmergencyStack emergency;
    WaitingQueue   regular_queue;
    PriorityHeap   priority_queue;
    WardGraph      wards;
    /* BST and Hash Table are rebuilt in-memory on load */
} Hospital;

/* ─── Function Prototypes ────────────────────────────────────────────────── */

/* JSON output helpers */
void json_string(const char *s);
void patient_json(Patient *p);
void ok_msg(const char *msg);
void error_msg(const char *msg);

/* Array operations */
int  array_add_patient(PatientArray *arr, Patient *p);
int  array_find_by_id(PatientArray *arr, int id);
void array_delete_patient(PatientArray *arr, int idx);

/* Linked List (History) */
HistoryList *history_create(void);
void         history_push(HistoryList *list, const char *action);
void         history_print_all(HistoryList *list);
void         history_print_recent(HistoryList *list, int n);
void         history_free(HistoryList *list);

/* Stack */
void stack_init(EmergencyStack *s);
int  stack_push(EmergencyStack *s, int idx);
int  stack_pop(EmergencyStack *s);
int  stack_peek(EmergencyStack *s);
int  stack_is_empty(EmergencyStack *s);
void stack_print(EmergencyStack *s, PatientArray *arr);

/* Queue */
void queue_init(WaitingQueue *q);
int  queue_enqueue(WaitingQueue *q, int idx);
int  queue_dequeue(WaitingQueue *q);
int  queue_is_empty(WaitingQueue *q);
void queue_print(WaitingQueue *q, PatientArray *arr);

/* Min-Heap */
void heap_init(PriorityHeap *h);
void heap_insert(PriorityHeap *h, int idx, PatientArray *arr);
int  heap_extract_min(PriorityHeap *h, PatientArray *arr);
void heap_print(PriorityHeap *h, PatientArray *arr);

/* BST */
BSTNode *bst_insert(BSTNode *root, int idx, int id);
BSTNode *bst_search(BSTNode *root, int id);
void     bst_inorder_json(BSTNode *root, PatientArray *arr, int *first);
void     bst_free(BSTNode *root);

/* Hash Table */
HashTable *hash_create(void);
unsigned   hash_fn(const char *key);
void       hash_insert(HashTable *ht, int idx, const char *name);
int        hash_search(HashTable *ht, const char *name);
void       hash_free(HashTable *ht);

/* Graph (Ward) */
void graph_init(WardGraph *g);
void graph_add_ward(WardGraph *g, const char *name, int cap);
void graph_add_edge(WardGraph *g, int from, int to, int weight);
void graph_bfs(WardGraph *g, int start);    /* BFS traversal */
void graph_dfs(WardGraph *g, int start, int visited[]); /* DFS */
int  graph_find_ward(WardGraph *g, const char *name);
void graph_print_json(WardGraph *g);

/* Sorting */
void sort_by_name(Patient *arr, int n);       /* Merge Sort */
void sort_by_age(Patient *arr, int n);        /* Quick Sort */
void sort_by_priority(Patient *arr, int n);   /* Heap Sort */
void sort_by_id(Patient *arr, int n);         /* Insertion Sort */
void sort_by_bill(Patient *arr, int n);       /* Bubble Sort (demo) */

/* File persistence */
void save_hospital(Hospital *h);
void load_hospital(Hospital *h);
void log_action(const char *action);

/* Search */
void linear_search_print(PatientArray *arr, const char *keyword);
int  binary_search_by_id(Patient *sorted, int n, int id);

#endif /* HOSPITAL_H */
