#include "hashtable.h"

/* djb2 string hashing algorithm with case-insensitivity */
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

    /* Check if key already exists */
    while (curr) {
        if (str_case_cmp(curr->key, key) == 0) {
            return false; /* duplicate key */
        }
        curr = curr->next;
    }

    /* Allocate new node */
    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) return false;

    newNode->key = (char*)malloc(strlen(key) + 1);
    if (!newNode->key) {
        free(newNode);
        return false;
    }
    strcpy(newNode->key, key);
    newNode->userId = userId;

    /* Insert at head of chain (O(1)) */
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
