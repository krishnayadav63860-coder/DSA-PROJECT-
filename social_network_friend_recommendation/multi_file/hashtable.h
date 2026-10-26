#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "common.h"

/* Node for separate chaining collision resolution */
typedef struct HashNode {
    char *key;
    int userId;
    struct HashNode *next;
} HashNode;

/* Hash table mapping username -> userId */
typedef struct HashTable {
    HashNode **buckets;
    int capacity;
    int size;
} HashTable;

/* Hash table API */
HashTable* ht_create(int capacity);
unsigned long ht_hash(const char *str, int capacity);
bool ht_insert(HashTable *ht, const char *key, int userId);
int ht_search(const HashTable *ht, const char *key);
bool ht_delete(HashTable *ht, const char *key);
void ht_clear(HashTable *ht);
void ht_free(HashTable *ht);

#endif /* HASHTABLE_H */
