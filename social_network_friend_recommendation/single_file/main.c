/**
 * ==============================================================================
 * Project Title : Social Network Friend Recommendation System Using Graph,
 *                 BFS and Hashing
 * Course        : BTech Artificial Intelligence and Machine Learning (AIML)
 * Subject       : Data Structures and Algorithms (DSA)
 * Language      : C (C11 Standard, GCC)
 *
 * Description   : A robust, production-grade console application implementing:
 *                 - Dynamic Graph with Adjacency List representation
 *                 - Hash Table with djb2 hashing and separate chaining (O(1) avg)
 *                 - Dynamic FIFO Queue supporting Breadth-First Search (BFS)
 *                 - Friend Recommendation Engine (BFS Depth 2 + Mutual Scoring)
 *                 - Persistence to text files (network.txt)
 *                 - Comprehensive input validation and memory leak prevention
 * ==============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Global Constants */
#define MAX_NAME_LEN 64
#define MAX_INFO_LEN 128
#define INITIAL_GRAPH_CAPACITY 16
#define HASH_TABLE_CAPACITY 101

/* ==============================================================================
 *                          1. DATA STRUCTURE DEFINITIONS
 * ==============================================================================
 */

/* User Profile Record */
typedef struct User {
    int id;
    char name[MAX_NAME_LEN];
    char city[MAX_INFO_LEN];
    char interests[MAX_INFO_LEN];
    bool isDeleted;
} User;

/* Hash Table Node for Separate Chaining (Collision Resolution) */
typedef struct HashNode {
    char *key;
    int userId;
    struct HashNode *next;
} HashNode;

/* Hash Table mapping Username -> User ID */
typedef struct HashTable {
    HashNode **buckets;
    int capacity;
    int size;
} HashTable;

/* Queue Node for BFS traversal */
typedef struct QueueNode {
    int data;
    struct QueueNode *next;
} QueueNode;

/* FIFO Queue with front and rear pointers */
typedef struct Queue {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

/* Node in an Adjacency List (Represents an Undirected Edge) */
typedef struct AdjListNode {
    int destId;
    struct AdjListNode *next;
} AdjListNode;

/* Head of an Adjacency List for a single vertex */
typedef struct AdjList {
    AdjListNode *head;
    int degree;
} AdjList;

/* Dynamic Graph representation of the Social Network */
typedef struct Graph {
    User *users;             /* Dynamic array of User records */
    AdjList *adjLists;       /* Dynamic array of Adjacency Lists */
    int userCount;           /* Total user slots (active + tombstones) */
    int capacity;            /* Allocated capacity for users and adjLists */
    int activeCount;         /* Number of active (non-deleted) users */
    HashTable *userIndex;    /* Fast name-to-ID lookup hash table */
} Graph;

/* Friend Recommendation Candidate Record */
typedef struct Recommendation {
    int candidateId;
    char candidateName[MAX_NAME_LEN];
    int mutualCount;
    int *mutualFriendIds;
} Recommendation;

/* ==============================================================================
 *                       2. STRING & INPUT UTILITY FUNCTIONS
 * ==============================================================================
 */

/* Trims leading and trailing whitespace in-place */
void trim_whitespace(char *str) {
    if (!str) return;

    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *str = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* Portable case-insensitive string comparison (Windows & POSIX safe) */
int str_case_cmp(const char *s1, const char *s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;

    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Safe line reading using fgets with automatic newline removal and whitespace trimming */
void safe_read_line(char *buffer, size_t size) {
    if (!buffer || size == 0) return;

    if (fgets(buffer, (int)size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
        trim_whitespace(buffer);
    } else {
        buffer[0] = '\0';
        clearerr(stdin);
    }
}

/* Safe bounded integer reader (handles non-numeric strings and bounds gracefully) */
int safe_read_int(int min_val, int max_val) {
    char buf[64];
    long val;
    char *endptr;

    while (1) {
        printf("Choice (%d - %d): ", min_val, max_val);
        fflush(stdout);

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            clearerr(stdin);
            return min_val;
        }

        buf[strcspn(buf, "\r\n")] = '\0';
        trim_whitespace(buf);

        if (buf[0] == '\0') {
            printf("[!] Input cannot be empty. Please enter a valid number.\n");
            continue;
        }

        val = strtol(buf, &endptr, 10);
        if (*endptr != '\0') {
            printf("[!] Invalid input. Please enter numeric digits only.\n");
            continue;
        }

        if (val < min_val || val > max_val) {
            printf("[!] Value out of bounds. Please enter a number between %d and %d.\n", min_val, max_val);
            continue;
        }

        return (int)val;
    }
}

/* ==============================================================================
 *                     3. HASH TABLE IMPLEMENTATION (djb2)
 * ==============================================================================
 */

/* djb2 string hashing algorithm with case normalization */
unsigned long ht_hash(const char *str, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        c = tolower(c);
        /* hash * 33 + c */
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return hash % (unsigned long)capacity;
}

HashTable* ht_create(int capacity) {
    if (capacity <= 0) capacity = HASH_TABLE_CAPACITY;
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        perror("Failed to allocate hash table");
        exit(EXIT_FAILURE);
    }
    ht->capacity = capacity;
    ht->size = 0;
    ht->buckets = (HashNode**)calloc((size_t)capacity, sizeof(HashNode*));
    if (!ht->buckets) {
        perror("Failed to allocate hash table buckets");
        free(ht);
        exit(EXIT_FAILURE);
    }
    return ht;
}

bool ht_insert(HashTable *ht, const char *key, int userId) {
    if (!ht || !key || key[0] == '\0') return false;

    unsigned long index = ht_hash(key, ht->capacity);
    HashNode *curr = ht->buckets[index];

    /* Check for duplicate username */
    while (curr) {
        if (str_case_cmp(curr->key, key) == 0) {
            return false; /* Duplicate key rejected */
        }
        curr = curr->next;
    }

    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) return false;

    newNode->key = (char*)malloc(strlen(key) + 1);
    if (!newNode->key) {
        free(newNode);
        return false;
    }
    strcpy(newNode->key, key);
    newNode->userId = userId;

    /* Insert at head of linked chain (O(1)) */
    newNode->next = ht->buckets[index];
    ht->buckets[index] = newNode;
    ht->size++;
    return true;
}

int ht_search(const HashTable *ht, const char *key) {
    if (!ht || !key || key[0] == '\0') return -1;

    unsigned long index = ht_hash(key, ht->capacity);
    HashNode *curr = ht->buckets[index];

    while (curr) {
        if (str_case_cmp(curr->key, key) == 0) {
            return curr->userId;
        }
        curr = curr->next;
    }
    return -1; /* Not found */
}

bool ht_delete(HashTable *ht, const char *key) {
    if (!ht || !key || key[0] == '\0') return false;

    unsigned long index = ht_hash(key, ht->capacity);
    HashNode *curr = ht->buckets[index];
    HashNode *prev = NULL;

    while (curr) {
        if (str_case_cmp(curr->key, key) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                ht->buckets[index] = curr->next;
            }
            free(curr->key);
            free(curr);
            ht->size--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

void ht_clear(HashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < ht->capacity; i++) {
        HashNode *curr = ht->buckets[i];
        while (curr) {
            HashNode *temp = curr;
            curr = curr->next;
            free(temp->key);
            free(temp);
        }
        ht->buckets[i] = NULL;
    }
    ht->size = 0;
}

void ht_free(HashTable *ht) {
    if (!ht) return;
    ht_clear(ht);
    free(ht->buckets);
    free(ht);
}

/* ==============================================================================
 *                         4. QUEUE IMPLEMENTATION (BFS)
 * ==============================================================================
 */

Queue* queue_create(void) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        perror("Failed to allocate queue");
        exit(EXIT_FAILURE);
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    return q;
}

void queue_enqueue(Queue *q, int data) {
    if (!q) return;
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        perror("Failed to allocate queue node");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

int queue_dequeue(Queue *q) {
    if (!q || q->front == NULL) {
        return -1;
    }
    QueueNode *temp = q->front;
    int data = temp->data;

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return data;
}

bool queue_is_empty(const Queue *q) {
    return (q == NULL || q->front == NULL);
}

void queue_free(Queue *q) {
    if (!q) return;
    while (!queue_is_empty(q)) {
        queue_dequeue(q);
    }
    free(q);
}

/* ==============================================================================
 *                  5. GRAPH & RECOMMENDATION IMPLEMENTATION
 * ==============================================================================
 */

static int compare_recommendations(const void *a, const void *b) {
    const Recommendation *rA = (const Recommendation*)a;
    const Recommendation *rB = (const Recommendation*)b;

    /* Primary: descending order of mutual friends */
    if (rB->mutualCount != rA->mutualCount) {
        return rB->mutualCount - rA->mutualCount;
    }
    /* Tie-breaker: alphabetical order by name */
    return str_case_cmp(rA->candidateName, rB->candidateName);
}

Graph* graph_create(int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = INITIAL_GRAPH_CAPACITY;
    Graph *g = (Graph*)malloc(sizeof(Graph));
    if (!g) {
        perror("Failed to allocate graph");
        exit(EXIT_FAILURE);
    }
    g->capacity = initial_capacity;
    g->userCount = 0;
    g->activeCount = 0;
    g->users = (User*)malloc((size_t)g->capacity * sizeof(User));
    g->adjLists = (AdjList*)malloc((size_t)g->capacity * sizeof(AdjList));
    if (!g->users || !g->adjLists) {
        perror("Failed to allocate graph arrays");
        free(g->users);
        free(g->adjLists);
        free(g);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < g->capacity; i++) {
        g->adjLists[i].head = NULL;
        g->adjLists[i].degree = 0;
    }
    g->userIndex = ht_create(HASH_TABLE_CAPACITY);
    return g;
}

static void ensure_graph_capacity(Graph *g) {
    if (g->userCount >= g->capacity) {
        int new_capacity = g->capacity * 2;
        User *new_users = (User*)realloc(g->users, (size_t)new_capacity * sizeof(User));
        if (!new_users) {
            perror("Failed to reallocate users array");
            exit(EXIT_FAILURE);
        }
        g->users = new_users;

        AdjList *new_adj = (AdjList*)realloc(g->adjLists, (size_t)new_capacity * sizeof(AdjList));
        if (!new_adj) {
            perror("Failed to reallocate adjacency lists");
            exit(EXIT_FAILURE);
        }
        g->adjLists = new_adj;

        for (int i = g->capacity; i < new_capacity; i++) {
            g->adjLists[i].head = NULL;
            g->adjLists[i].degree = 0;
        }
        g->capacity = new_capacity;
    }
}

int graph_register_user(Graph *g, const char *name, const char *city, const char *interests) {
    if (!g || !name) return -1;

    char clean_name[MAX_NAME_LEN];
    strncpy(clean_name, name, sizeof(clean_name) - 1);
    clean_name[sizeof(clean_name) - 1] = '\0';
    trim_whitespace(clean_name);

    if (clean_name[0] == '\0') {
        return -1; /* Empty name rejected */
    }

    if (ht_search(g->userIndex, clean_name) != -1) {
        return -2; /* Duplicate username rejected */
    }

    ensure_graph_capacity(g);

    int newId = g->userCount;
    g->users[newId].id = newId;
    strncpy(g->users[newId].name, clean_name, MAX_NAME_LEN - 1);
    g->users[newId].name[MAX_NAME_LEN - 1] = '\0';

    if (city && city[0] != '\0') {
        strncpy(g->users[newId].city, city, MAX_INFO_LEN - 1);
        g->users[newId].city[MAX_INFO_LEN - 1] = '\0';
        trim_whitespace(g->users[newId].city);
    } else {
        strcpy(g->users[newId].city, "Not Specified");
    }

    if (interests && interests[0] != '\0') {
        strncpy(g->users[newId].interests, interests, MAX_INFO_LEN - 1);
        g->users[newId].interests[MAX_INFO_LEN - 1] = '\0';
        trim_whitespace(g->users[newId].interests);
    } else {
        strcpy(g->users[newId].interests, "General");
    }

    g->users[newId].isDeleted = false;
    g->adjLists[newId].head = NULL;
    g->adjLists[newId].degree = 0;

    ht_insert(g->userIndex, clean_name, newId);
    g->userCount++;
    g->activeCount++;

    return newId;
}

bool graph_has_friendship(const Graph *g, int u, int v) {
    if (!g) return false;
    if (u < 0 || u >= g->userCount || v < 0 || v >= g->userCount) return false;
    if (g->users[u].isDeleted || g->users[v].isDeleted) return false;

    AdjListNode *curr = g->adjLists[u].head;
    while (curr) {
        if (curr->destId == v) return true;
        curr = curr->next;
    }
    return false;
}

bool graph_add_friendship(Graph *g, int u, int v) {
    if (!g) return false;
    if (u == v) return false; /* No self-friending */
    if (u < 0 || u >= g->userCount || v < 0 || v >= g->userCount) return false;
    if (g->users[u].isDeleted || g->users[v].isDeleted) return false;
    if (graph_has_friendship(g, u, v)) return false; /* Duplicate friendship rejected */

    /* Add v to u's adjacency list */
    AdjListNode *nodeU = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!nodeU) return false;
    nodeU->destId = v;
    nodeU->next = g->adjLists[u].head;
    g->adjLists[u].head = nodeU;
    g->adjLists[u].degree++;

    /* Add u to v's adjacency list (undirected edge) */
    AdjListNode *nodeV = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!nodeV) {
        g->adjLists[u].head = nodeU->next;
        free(nodeU);
        g->adjLists[u].degree--;
        return false;
    }
    nodeV->destId = u;
    nodeV->next = g->adjLists[v].head;
    g->adjLists[v].head = nodeV;
    g->adjLists[v].degree++;

    return true;
}

bool graph_remove_friendship(Graph *g, int u, int v) {
    if (!g) return false;
    if (u < 0 || u >= g->userCount || v < 0 || v >= g->userCount) return false;
    if (g->users[u].isDeleted || g->users[v].isDeleted) return false;

    bool removedU = false;
    AdjListNode *curr = g->adjLists[u].head;
    AdjListNode *prev = NULL;
    while (curr) {
        if (curr->destId == v) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g->adjLists[u].head = curr->next;
            }
            free(curr);
            g->adjLists[u].degree--;
            removedU = true;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    bool removedV = false;
    curr = g->adjLists[v].head;
    prev = NULL;
    while (curr) {
        if (curr->destId == u) {
            if (prev) {
                prev->next = curr->next;
            } else {
                g->adjLists[v].head = curr->next;
            }
            free(curr);
            g->adjLists[v].degree--;
            removedV = true;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    return removedU && removedV;
}

bool graph_delete_user(Graph *g, int userId) {
    if (!g) return false;
    if (userId < 0 || userId >= g->userCount || g->users[userId].isDeleted) {
        return false;
    }

    /* Remove userId from all its friends' adjacency lists */
    AdjListNode *curr = g->adjLists[userId].head;
    while (curr) {
        int friendId = curr->destId;
        AdjListNode *fCurr = g->adjLists[friendId].head;
        AdjListNode *fPrev = NULL;
        while (fCurr) {
            if (fCurr->destId == userId) {
                if (fPrev) {
                    fPrev->next = fCurr->next;
                } else {
                    g->adjLists[friendId].head = fCurr->next;
                }
                free(fCurr);
                g->adjLists[friendId].degree--;
                break;
            }
            fPrev = fCurr;
            fCurr = fCurr->next;
        }
        curr = curr->next;
    }

    /* Free userId's own adjacency list */
    curr = g->adjLists[userId].head;
    while (curr) {
        AdjListNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    g->adjLists[userId].head = NULL;
    g->adjLists[userId].degree = 0;

    /* Remove from hash table */
    ht_delete(g->userIndex, g->users[userId].name);

    /* Mark user as deleted */
    g->users[userId].isDeleted = true;
    g->activeCount--;

    return true;
}

int graph_get_user_id(const Graph *g, const char *name) {
    if (!g || !name) return -1;
    return ht_search(g->userIndex, name);
}

void graph_display_friends(const Graph *g, int userId) {
    if (!g || userId < 0 || userId >= g->userCount || g->users[userId].isDeleted) {
        printf("[ERROR] User not found or inactive.\n");
        return;
    }

    printf("\n============================================================\n");
    printf(" Friends of %s (ID: %d, City: %s)\n", g->users[userId].name, userId, g->users[userId].city);
    printf("============================================================\n");

    AdjListNode *curr = g->adjLists[userId].head;
    if (!curr) {
        printf("  (No friends yet. Add friends using menu option 2!)\n");
        printf("============================================================\n");
        return;
    }

    int count = 0;
    printf("%-5s | %-20s | %-15s | %-15s\n", "No.", "Name", "City", "Interests");
    printf("------------------------------------------------------------\n");
    while (curr) {
        int fId = curr->destId;
        if (!g->users[fId].isDeleted) {
            count++;
            printf("%-5d | %-20s | %-15s | %-15s\n", count, g->users[fId].name, g->users[fId].city, g->users[fId].interests);
        }
        curr = curr->next;
    }
    printf("============================================================\n");
    printf("Total Friends: %d\n", count);
}

int graph_get_mutual_friends(const Graph *g, int u, int v, int **outMutualIds) {
    *outMutualIds = NULL;
    if (!g) return 0;
    if (u < 0 || u >= g->userCount || v < 0 || v >= g->userCount) return 0;
    if (g->users[u].isDeleted || g->users[v].isDeleted) return 0;

    int max_possible = g->adjLists[u].degree < g->adjLists[v].degree ? g->adjLists[u].degree : g->adjLists[v].degree;
    if (max_possible <= 0) return 0;

    int *temp = (int*)malloc((size_t)max_possible * sizeof(int));
    if (!temp) return 0;

    int count = 0;
    AdjListNode *curr = g->adjLists[u].head;
    while (curr) {
        int friendId = curr->destId;
        if (!g->users[friendId].isDeleted && graph_has_friendship(g, friendId, v)) {
            temp[count++] = friendId;
        }
        curr = curr->next;
    }

    if (count > 0) {
        *outMutualIds = temp;
    } else {
        free(temp);
    }
    return count;
}

Recommendation* graph_recommend_friends(const Graph *g, int userId, int *outCount) {
    *outCount = 0;
    if (!g || userId < 0 || userId >= g->userCount || g->users[userId].isDeleted) {
        return NULL;
    }

    int *dist = (int*)malloc((size_t)g->userCount * sizeof(int));
    if (!dist) return NULL;
    for (int i = 0; i < g->userCount; i++) {
        dist[i] = -1;
    }

    Queue *q = queue_create();
    dist[userId] = 0;
    queue_enqueue(q, userId);

    while (!queue_is_empty(q)) {
        int u = queue_dequeue(q);
        if (dist[u] >= 2) continue;

        AdjListNode *curr = g->adjLists[u].head;
        while (curr) {
            int v = curr->destId;
            if (!g->users[v].isDeleted && dist[v] == -1) {
                dist[v] = dist[u] + 1;
                if (dist[v] <= 2) {
                    queue_enqueue(q, v);
                }
            }
            curr = curr->next;
        }
    }
    queue_free(q);

    Recommendation *recs = (Recommendation*)malloc((size_t)g->userCount * sizeof(Recommendation));
    if (!recs) {
        free(dist);
        return NULL;
    }

    int recCount = 0;
    for (int candidate = 0; candidate < g->userCount; candidate++) {
        if (candidate != userId && !g->users[candidate].isDeleted && dist[candidate] == 2) {
            int *mutualIds = NULL;
            int mCount = graph_get_mutual_friends(g, userId, candidate, &mutualIds);
            if (mCount >= 1) {
                recs[recCount].candidateId = candidate;
                strncpy(recs[recCount].candidateName, g->users[candidate].name, MAX_NAME_LEN - 1);
                recs[recCount].candidateName[MAX_NAME_LEN - 1] = '\0';
                recs[recCount].mutualCount = mCount;
                recs[recCount].mutualFriendIds = mutualIds;
                recCount++;
            }
        }
    }

    free(dist);

    if (recCount > 1) {
        qsort(recs, (size_t)recCount, sizeof(Recommendation), compare_recommendations);
    }

    *outCount = recCount;
    return recs;
}

void graph_free_recommendations(Recommendation *recs, int count) {
    if (!recs) return;
    for (int i = 0; i < count; i++) {
        free(recs[i].mutualFriendIds);
    }
    free(recs);
}

void graph_show_stats(const Graph *g) {
    if (!g) return;

    int totalUsers = g->activeCount;
    int totalEdges = 0;
    int maxDegree = -1;
    int mostConnectedId = -1;

    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            totalEdges += g->adjLists[i].degree;
            if (g->adjLists[i].degree > maxDegree) {
                maxDegree = g->adjLists[i].degree;
                mostConnectedId = i;
            }
        }
    }
    int totalFriendships = totalEdges / 2;
    double avgFriends = (totalUsers > 0) ? (2.0 * totalFriendships / totalUsers) : 0.0;
    double density = 0.0;
    if (totalUsers > 1) {
        density = (2.0 * totalFriendships) / ((double)totalUsers * (totalUsers - 1));
    }

    printf("\n============================================================\n");
    printf("                 NETWORK STATISTICS & METRICS               \n");
    printf("============================================================\n");
    printf("  Total Active Users       : %d\n", totalUsers);
    printf("  Total Friendships (Edges): %d\n", totalFriendships);
    printf("  Average Friends / User   : %.2f\n", avgFriends);
    printf("  Network Graph Density    : %.4f (%.2f%% connected)\n", density, density * 100.0);
    if (mostConnectedId != -1 && maxDegree > 0) {
        printf("  Most Connected User      : %s (ID: %d) with %d friends\n",
               g->users[mostConnectedId].name, mostConnectedId, maxDegree);
    } else {
        printf("  Most Connected User      : None (no friendships established yet)\n");
    }
    printf("============================================================\n");
}

void graph_display_network(const Graph *g) {
    if (!g) return;

    printf("\n======================================================================\n");
    printf("                     COMPLETE SOCIAL NETWORK GRAPH                    \n");
    printf("======================================================================\n");
    if (g->activeCount == 0) {
        printf("  Network is empty. Please register users or load demo data.\n");
        printf("======================================================================\n");
        return;
    }

    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            printf("[%02d] %-14s (%-11s) -> ", g->users[i].id, g->users[i].name, g->users[i].city);
            AdjListNode *curr = g->adjLists[i].head;
            if (!curr) {
                printf("[No friends]\n");
            } else {
                printf("[");
                bool first = true;
                while (curr) {
                    int fId = curr->destId;
                    if (!g->users[fId].isDeleted) {
                        if (!first) printf(", ");
                        printf("%s", g->users[fId].name);
                        first = false;
                    }
                    curr = curr->next;
                }
                printf("]\n");
            }
        }
    }
    printf("======================================================================\n");
}

void graph_clear(Graph *g) {
    if (!g) return;
    for (int i = 0; i < g->userCount; i++) {
        AdjListNode *curr = g->adjLists[i].head;
        while (curr) {
            AdjListNode *temp = curr;
            curr = curr->next;
            free(temp);
        }
        g->adjLists[i].head = NULL;
        g->adjLists[i].degree = 0;
        g->users[i].isDeleted = true;
    }
    ht_clear(g->userIndex);
    g->userCount = 0;
    g->activeCount = 0;
}

void graph_load_demo_data(Graph *g) {
    if (!g) return;
    graph_clear(g);

    int idRahul = graph_register_user(g, "Rahul", "Delhi", "AI, Machine Learning");
    int idAmit  = graph_register_user(g, "Amit", "Bangalore", "DSA, Web Development");
    int idPriya = graph_register_user(g, "Priya", "Mumbai", "Data Science, Cloud");
    int idNeha  = graph_register_user(g, "Neha", "Delhi", "UI/UX, Cybersecurity");
    int idKaran = graph_register_user(g, "Karan", "Pune", "Gaming, IoT");
    (void)idKaran; /* Intentionally unlinked for disconnected vertex demonstration */

    /* Friendships: Rahul-Amit, Rahul-Neha, Amit-Priya */
    graph_add_friendship(g, idRahul, idAmit);
    graph_add_friendship(g, idRahul, idNeha);
    graph_add_friendship(g, idAmit, idPriya);

    printf("\n[SUCCESS] Demo Data Loaded Successfully!\n");
    printf("  Registered 5 Users: Rahul, Amit, Priya, Neha, Karan\n");
    printf("  Created 3 Friendships: Rahul <-> Amit, Rahul <-> Neha, Amit <-> Priya\n");
    printf("  (Notice: Karan is intentionally isolated for edge-case demonstration)\n");
}

void graph_free(Graph *g) {
    if (!g) return;
    graph_clear(g);
    ht_free(g->userIndex);
    free(g->users);
    free(g->adjLists);
    free(g);
}

/* ==============================================================================
 *                         6. FILE PERSISTENCE (STORAGE)
 * ==============================================================================
 */

bool storage_save_network(const Graph *g, const char *filename) {
    if (!g || !filename || filename[0] == '\0') return false;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("[ERROR] Failed to open file for saving");
        return false;
    }

    fprintf(fp, "# Social Network Friend Recommendation System Database\n");
    fprintf(fp, "# Format: USERS_COUNT, followed by Name|City|Interests\n");
    fprintf(fp, "# Followed by FRIENDSHIPS_COUNT, followed by User1|User2\n");
    fprintf(fp, "USERS_COUNT: %d\n", g->activeCount);

    /* Write active users */
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            fprintf(fp, "%s|%s|%s\n", g->users[i].name, g->users[i].city, g->users[i].interests);
        }
    }

    /* Count unique friendships */
    int friendshipCount = 0;
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            AdjListNode *curr = g->adjLists[i].head;
            while (curr) {
                if (curr->destId > i && !g->users[curr->destId].isDeleted) {
                    friendshipCount++;
                }
                curr = curr->next;
            }
        }
    }

    fprintf(fp, "FRIENDSHIPS_COUNT: %d\n", friendshipCount);
    for (int i = 0; i < g->userCount; i++) {
        if (!g->users[i].isDeleted) {
            AdjListNode *curr = g->adjLists[i].head;
            while (curr) {
                if (curr->destId > i && !g->users[curr->destId].isDeleted) {
                    fprintf(fp, "%s|%s\n", g->users[i].name, g->users[curr->destId].name);
                }
                curr = curr->next;
            }
        }
    }

    fclose(fp);
    return true;
}

bool storage_load_network(Graph *g, const char *filename) {
    if (!g || !filename || filename[0] == '\0') return false;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return false;
    }

    char line[512];
    int expectedUsers = -1;
    int expectedFriendships = -1;
    int usersLoaded = 0;
    int friendshipsLoaded = 0;

    graph_clear(g);

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        trim_whitespace(line);

        if (line[0] == '\0' || line[0] == '#') continue;

        if (strncmp(line, "USERS_COUNT:", 12) == 0) {
            expectedUsers = atoi(line + 12);
            continue;
        }

        if (strncmp(line, "FRIENDSHIPS_COUNT:", 18) == 0) {
            expectedFriendships = atoi(line + 18);
            continue;
        }

        if (expectedFriendships == -1) {
            char *pipe1 = strchr(line, '|');
            if (pipe1) {
                *pipe1 = '\0';
                char *name = line;
                char *rest = pipe1 + 1;
                char *pipe2 = strchr(rest, '|');
                char *city = rest;
                char *interests = "";
                if (pipe2) {
                    *pipe2 = '\0';
                    interests = pipe2 + 1;
                }
                graph_register_user(g, name, city, interests);
                usersLoaded++;
            }
        } else {
            char *pipe = strchr(line, '|');
            if (pipe) {
                *pipe = '\0';
                char *userA = line;
                char *userB = pipe + 1;
                trim_whitespace(userA);
                trim_whitespace(userB);

                int idA = graph_get_user_id(g, userA);
                int idB = graph_get_user_id(g, userB);
                if (idA != -1 && idB != -1) {
                    graph_add_friendship(g, idA, idB);
                    friendshipsLoaded++;
                }
            }
        }
    }

    fclose(fp);
    (void)expectedUsers;
    (void)expectedFriendships;
    return (usersLoaded > 0 || friendshipsLoaded > 0 || expectedUsers == 0);
}

/* ==============================================================================
 *                         7. USER INTERFACE & MENUS
 * ==============================================================================
 */

static void print_banner(void) {
    printf("\n");
    printf("======================================================================\n");
    printf("   SOCIAL NETWORK FRIEND RECOMMENDATION SYSTEM USING GRAPH & BFS      \n");
    printf("        BTech AIML | Data Structures and Algorithms Laboratory        \n");
    printf("======================================================================\n");
}

static void print_menu(void) {
    printf("\n-------------------------- [ MAIN MENU ] ---------------------------\n");
    printf("  1.  Register New User\n");
    printf("  2.  Add Friendship (Create Undirected Edge)\n");
    printf("  3.  Remove Friendship\n");
    printf("  4.  Delete User Profile (Clean up edges)\n");
    printf("  5.  View a User's Friend List\n");
    printf("  6.  Find Mutual Friends Between Two Users\n");
    printf("  7.  Recommend Friends (BFS Depth 2 + Mutual Scoring + Quick Add)\n");
    printf("  8.  Search User by Name (O(1) Hash Table Lookup)\n");
    printf("  9.  Show Network Statistics & Metrics\n");
    printf("  10. Display Entire Network (Graph Adjacency View)\n");
    printf("  11. File Storage (Save / Load Network Database)\n");
    printf("  12. Load Official Presentation Demo Data\n");
    printf("  0.  Exit System\n");
    printf("--------------------------------------------------------------------\n");
}

static int prompt_user_id(const Graph *g, const char *prompt_label) {
    char input[MAX_NAME_LEN];
    printf("%s: ", prompt_label);
    safe_read_line(input, sizeof(input));

    if (input[0] == '\0') {
        printf("[ERROR] Input cannot be blank.\n");
        return -1;
    }

    int id = graph_get_user_id(g, input);
    if (id != -1) return id;

    char *endptr;
    long num = strtol(input, &endptr, 10);
    if (*endptr == '\0' && num >= 0 && num < g->userCount) {
        if (!g->users[num].isDeleted) {
            return (int)num;
        }
    }

    printf("[ERROR] User '%s' does not exist in the network.\n", input);
    return -1;
}

static void handle_register_user(Graph *g) {
    char name[MAX_NAME_LEN];
    char city[MAX_INFO_LEN];
    char interests[MAX_INFO_LEN];

    printf("\n--- [1. REGISTER NEW USER] ---\n");
    printf("Enter Username: ");
    safe_read_line(name, sizeof(name));

    if (name[0] == '\0') {
        printf("[ERROR] Registration failed: Username cannot be empty.\n");
        return;
    }

    printf("Enter City (optional, press Enter to skip): ");
    safe_read_line(city, sizeof(city));

    printf("Enter Interests / Hobbies (optional, press Enter to skip): ");
    safe_read_line(interests, sizeof(interests));

    int res = graph_register_user(g, name, city, interests);
    if (res == -2) {
        printf("[ERROR] Registration failed: User '%s' already exists (case-insensitive duplicate).\n", name);
    } else if (res == -1) {
        printf("[ERROR] Registration failed: Invalid name format.\n");
    } else {
        printf("[SUCCESS] User '%s' successfully registered with User ID: %d\n", g->users[res].name, res);
    }
}

static void handle_add_friendship(Graph *g) {
    printf("\n--- [2. ADD FRIENDSHIP] ---\n");
    if (g->activeCount < 2) {
        printf("[INFO] At least 2 active users are needed to create a friendship.\n");
        return;
    }

    int u = prompt_user_id(g, "Enter First Username / ID");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second Username / ID");
    if (v == -1) return;

    if (u == v) {
        printf("[ERROR] Self-friending is not allowed. A user cannot be friends with themselves.\n");
        return;
    }

    if (graph_has_friendship(g, u, v)) {
        printf("[WARNING] Friendship already exists between %s and %s.\n", g->users[u].name, g->users[v].name);
        return;
    }

    if (graph_add_friendship(g, u, v)) {
        printf("[SUCCESS] Friendship established between %s and %s (undirected edge added)!\n",
               g->users[u].name, g->users[v].name);
    } else {
        printf("[ERROR] Failed to add friendship.\n");
    }
}

static void handle_remove_friendship(Graph *g) {
    printf("\n--- [3. REMOVE FRIENDSHIP] ---\n");
    int u = prompt_user_id(g, "Enter First Username / ID");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second Username / ID");
    if (v == -1) return;

    if (!graph_has_friendship(g, u, v)) {
        printf("[ERROR] %s and %s are not currently friends.\n", g->users[u].name, g->users[v].name);
        return;
    }

    if (graph_remove_friendship(g, u, v)) {
        printf("[SUCCESS] Friendship removed between %s and %s.\n", g->users[u].name, g->users[v].name);
    } else {
        printf("[ERROR] Failed to remove friendship.\n");
    }
}

static void handle_delete_user(Graph *g) {
    printf("\n--- [4. DELETE USER PROFILE] ---\n");
    int u = prompt_user_id(g, "Enter Username / ID to Delete");
    if (u == -1) return;

    char confirm[16];
    printf("Are you sure you want to delete '%s' and remove all their friendships? (y/n): ", g->users[u].name);
    safe_read_line(confirm, sizeof(confirm));

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        char deletedName[MAX_NAME_LEN];
        strncpy(deletedName, g->users[u].name, sizeof(deletedName) - 1);
        deletedName[sizeof(deletedName) - 1] = '\0';

        if (graph_delete_user(g, u)) {
            printf("[SUCCESS] User '%s' and all associated graph edges were permanently removed.\n", deletedName);
        } else {
            printf("[ERROR] Failed to delete user.\n");
        }
    } else {
        printf("[INFO] Deletion cancelled.\n");
    }
}

static void handle_view_friends(Graph *g) {
    printf("\n--- [5. VIEW USER FRIEND LIST] ---\n");
    int u = prompt_user_id(g, "Enter Username / ID");
    if (u == -1) return;

    graph_display_friends(g, u);
}

static void handle_find_mutual_friends(Graph *g) {
    printf("\n--- [6. FIND MUTUAL FRIENDS] ---\n");
    int u = prompt_user_id(g, "Enter First User");
    if (u == -1) return;

    int v = prompt_user_id(g, "Enter Second User");
    if (v == -1) return;

    if (u == v) {
        printf("[INFO] Both inputs refer to the same user (%s).\n", g->users[u].name);
        return;
    }

    int *mutualIds = NULL;
    int count = graph_get_mutual_friends(g, u, v, &mutualIds);

    printf("\n============================================================\n");
    printf(" Mutual Friends between %s and %s: (%d total)\n", g->users[u].name, g->users[v].name, count);
    printf("============================================================\n");

    if (count == 0) {
        printf("  No mutual friends found.\n");
    } else {
        for (int i = 0; i < count; i++) {
            int mId = mutualIds[i];
            printf("  %d. %-15s (City: %s)\n", i + 1, g->users[mId].name, g->users[mId].city);
        }
        free(mutualIds);
    }
    printf("============================================================\n");
}

static void handle_recommend_friends(Graph *g) {
    printf("\n--- [7. RECOMMEND FRIENDS (BFS DEPTH 2)] ---\n");
    int u = prompt_user_id(g, "Enter Target Username / ID");
    if (u == -1) return;

    int recCount = 0;
    Recommendation *recs = graph_recommend_friends(g, u, &recCount);

    if (recCount == 0) {
        printf("\n[INFO] No friend recommendations found for %s.\n", g->users[u].name);
        printf("       (Candidates must be at BFS distance 2 with >= 1 mutual friend).\n");
        graph_free_recommendations(recs, recCount);
        return;
    }

    printf("\n========================================================================================\n");
    printf("                   RECOMMENDED FRIENDS FOR: %-15s                      \n", g->users[u].name);
    printf("========================================================================================\n");
    printf(" %-4s | %-15s | %-15s | %-7s | %s\n", "Rank", "Candidate Name", "City", "Mutuals", "Mutual Friend Names");
    printf("----------------------------------------------------------------------------------------\n");

    for (int i = 0; i < recCount; i++) {
        char mutualList[256] = "";
        for (int m = 0; m < recs[i].mutualCount; m++) {
            int mId = recs[i].mutualFriendIds[m];
            if (m > 0) strncat(mutualList, ", ", sizeof(mutualList) - strlen(mutualList) - 1);
            strncat(mutualList, g->users[mId].name, sizeof(mutualList) - strlen(mutualList) - 1);
        }

        int cId = recs[i].candidateId;
        printf(" %-4d | %-15s | %-15s | %-7d | %s\n",
               i + 1, recs[i].candidateName, g->users[cId].city, recs[i].mutualCount, mutualList);
    }
    printf("========================================================================================\n");

    /* Quick Acceptance option */
    printf("\nAccept a recommendation? Enter Rank # to add friend (0 to return to menu): ");
    int choice = safe_read_int(0, recCount);

    if (choice > 0) {
        int selectedCandidateId = recs[choice - 1].candidateId;
        if (graph_add_friendship(g, u, selectedCandidateId)) {
            printf("[SUCCESS] Friendship established! %s and %s are now friends.\n",
                   g->users[u].name, g->users[selectedCandidateId].name);
        } else {
            printf("[ERROR] Failed to add friendship.\n");
        }
    } else {
        printf("[INFO] No friendship added.\n");
    }

    graph_free_recommendations(recs, recCount);
}

static void handle_search_user(Graph *g) {
    printf("\n--- [8. SEARCH USER BY NAME (HASH TABLE)] ---\n");
    char name[MAX_NAME_LEN];
    printf("Enter Username to Search: ");
    safe_read_line(name, sizeof(name));

    if (name[0] == '\0') {
        printf("[ERROR] Username cannot be empty.\n");
        return;
    }

    int id = graph_get_user_id(g, name);
    if (id == -1) {
        printf("[INFO] User '%s' NOT FOUND in the social network hash table.\n", name);
        return;
    }

    printf("\n============================================================\n");
    printf("                  USER PROFILE DETAILS                      \n");
    printf("============================================================\n");
    printf("  User ID       : %d\n", g->users[id].id);
    printf("  Username      : %s\n", g->users[id].name);
    printf("  City          : %s\n", g->users[id].city);
    printf("  Interests     : %s\n", g->users[id].interests);
    printf("  Total Friends : %d\n", g->adjLists[id].degree);
    printf("  Lookup Status : Found via djb2 Hash Table (O(1) average)\n");
    printf("============================================================\n");
}

static void handle_file_storage(Graph *g) {
    printf("\n--- [11. FILE STORAGE & PERSISTENCE] ---\n");
    printf("  1. Save Current Network to File\n");
    printf("  2. Load Network from File\n");
    int subChoice = safe_read_int(1, 2);

    char filename[128];
    printf("Enter filename (default: network.txt): ");
    safe_read_line(filename, sizeof(filename));
    if (filename[0] == '\0') {
        strcpy(filename, "network.txt");
    }

    if (subChoice == 1) {
        if (storage_save_network(g, filename)) {
            printf("[SUCCESS] Social network saved successfully to '%s'.\n", filename);
        } else {
            printf("[ERROR] Could not save network to '%s'.\n", filename);
        }
    } else {
        char confirm[16];
        printf("Loading from file will replace the current in-memory network. Continue? (y/n): ");
        safe_read_line(confirm, sizeof(confirm));
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            if (storage_load_network(g, filename)) {
                printf("[SUCCESS] Network successfully loaded from '%s'!\n", filename);
                printf("          Active Users: %d\n", g->activeCount);
            } else {
                printf("[ERROR] Failed to load from '%s'. Please check file existence/format.\n", filename);
            }
        } else {
            printf("[INFO] Load cancelled.\n");
        }
    }
}

int main(void) {
    Graph *network = graph_create(INITIAL_GRAPH_CAPACITY);
    print_banner();

    int choice = -1;
    while (choice != 0) {
        print_menu();
        choice = safe_read_int(0, 12);

        switch (choice) {
            case 1:
                handle_register_user(network);
                break;
            case 2:
                handle_add_friendship(network);
                break;
            case 3:
                handle_remove_friendship(network);
                break;
            case 4:
                handle_delete_user(network);
                break;
            case 5:
                handle_view_friends(network);
                break;
            case 6:
                handle_find_mutual_friends(network);
                break;
            case 7:
                handle_recommend_friends(network);
                break;
            case 8:
                handle_search_user(network);
                break;
            case 9:
                graph_show_stats(network);
                break;
            case 10:
                graph_display_network(network);
                break;
            case 11:
                handle_file_storage(network);
                break;
            case 12:
                graph_load_demo_data(network);
                break;
            case 0:
                printf("\nExiting Social Network System. Freeing all allocated memory...\n");
                break;
            default:
                printf("[ERROR] Unknown menu option.\n");
                break;
        }
    }

    graph_free(network);
    printf("All resources cleanly released. Program terminated safely.\n");
    return 0;
}
