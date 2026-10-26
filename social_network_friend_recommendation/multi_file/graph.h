#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "hashtable.h"
#include "queue.h"

/* User profile record */
typedef struct User {
    int id;
    char name[MAX_NAME_LEN];
    char city[MAX_INFO_LEN];
    char interests[MAX_INFO_LEN];
    bool isDeleted;
} User;

/* Node in an adjacency list (represents an undirected edge) */
typedef struct AdjListNode {
    int destId;
    struct AdjListNode *next;
} AdjListNode;

/* Head of an adjacency list for a single vertex */
typedef struct AdjList {
    AdjListNode *head;
    int degree;
} AdjList;

/* Dynamic Graph representation of the Social Network */
typedef struct Graph {
    User *users;             /* Dynamic array of User profiles */
    AdjList *adjLists;       /* Dynamic array of Adjacency Lists */
    int userCount;           /* Total user slots (active + tombstones) */
    int capacity;            /* Allocated memory capacity */
    int activeCount;         /* Number of active (non-deleted) users */
    HashTable *userIndex;    /* Fast name-to-ID lookup hash table */
} Graph;

/* Friend Recommendation data structure */
typedef struct Recommendation {
    int candidateId;
    char candidateName[MAX_NAME_LEN];
    int mutualCount;
    int *mutualFriendIds;
} Recommendation;

/* Graph API */
Graph* graph_create(int initial_capacity);
int graph_register_user(Graph *g, const char *name, const char *city, const char *interests);
bool graph_add_friendship(Graph *g, int u, int v);
bool graph_remove_friendship(Graph *g, int u, int v);
bool graph_delete_user(Graph *g, int userId);
bool graph_has_friendship(const Graph *g, int u, int v);
int graph_get_user_id(const Graph *g, const char *name);
void graph_display_friends(const Graph *g, int userId);
int graph_get_mutual_friends(const Graph *g, int u, int v, int **outMutualIds);
Recommendation* graph_recommend_friends(const Graph *g, int userId, int *outCount);
void graph_free_recommendations(Recommendation *recs, int count);
void graph_show_stats(const Graph *g);
void graph_display_network(const Graph *g);
void graph_load_demo_data(Graph *g);
void graph_clear(Graph *g);
void graph_free(Graph *g);

#endif /* GRAPH_H */
