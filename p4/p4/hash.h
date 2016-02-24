#ifndef p4_hash_h
#define p4_hash_h
#include "list.h"

typedef struct hash{
    
    list_t* lists;
}hash_t;

void Hash_Init(hash_t *hash, int buckets);
void Hash_Insert(hash_t *hash, void *element, unsigned int key);
void Hash_Delete(hash_t *hash, unsigned int key);
void *Hash_Lookup(hash_t *hash, unsigned int key);

#endif
