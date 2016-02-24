#include <stdio.h>
#include <stdlib.h>
#include "spin.h"
#include "list.h"
#include "hash.h"

int bk;

void Hash_Init(hash_t *hash, int buckets){
    
    hash->lists = (list_t*)malloc(sizeof(list_t) * buckets);
    bk = buckets;
    
    int i;
    for (i = 0; i < buckets; i++){
        List_Init(&hash->lists[i]);
    }
}

void Hash_Insert(hash_t *hash, void *element, unsigned int key){
    
    int bucket = key % bk;
    List_Insert(&hash->lists[bucket], element, key);
}

void Hash_Delete(hash_t *hash, unsigned int key){
    
    int bucket = key % bk;
    List_Delete(&hash->lists[bucket], key);
}

void *Hash_Lookup(hash_t *hash, unsigned int key){
    
    int bucket = key % bk;
    return List_Lookup(&hash->lists[bucket], key);
}