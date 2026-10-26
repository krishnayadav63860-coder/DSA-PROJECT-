#include "graph.h"

/* Comparator for sorting recommendations */
static int compare_recommendations(const void *a, const void *b) {
    const Recommendation *rA = (const Recommendation*)a;
    const Recommendation *rB = (const Recommendation*)b;

    /* Primary: descending order of mutual friend count */
    if (rB->mutualCount != rA->mutualCount) {
        return rB->mutualCount - rA->mutualCount;
    }
    /* Secondary tie-breaker: alphabetical order of name */
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
        perror("Failed to allocate graph storage arrays");
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

    /* Check duplicate username via hash table */
    if (ht_search(g->userIndex, clean_name) != -1) {
        return -2; /* Duplicate name rejected */
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
    if (graph_has_friendship(g, u, v)) return false; /* Already friends */

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
        /* Rollback nodeU on failure */
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
        /* Remove userId from friendId's list */
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

    /* Mark as deleted */
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

/*
 * BFS Friend Recommendation Engine:
 * Traverses graph up to depth 2 from userId.
 * Finds all candidates at depth 2 (friends-of-friends).
 * Calculates mutual friends with target user.
 * Filters candidates with mutualCount >= 1.
 * Sorts descending by mutualCount (tie-break alphabetically).
 */
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

    /* Allocate candidate recommendations array */
    Recommendation *recs = (Recommendation*)malloc((size_t)g->userCount * sizeof(Recommendation));
    if (!recs) {
        free(dist);
        return NULL;
    }

    int recCount = 0;
    for (int candidate = 0; candidate < g->userCount; candidate++) {
        /* Only consider active users at depth 2 */
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

void graph_free(Graph *g) {
    if (!g) return;
    graph_clear(g);
    ht_free(g->userIndex);
    free(g->users);
    free(g->adjLists);
    free(g);
}
