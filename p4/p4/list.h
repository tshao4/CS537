#ifndef p4_list_h
#define p4_list_h

#include "spin.h"

typedef struct node{
    
    unsigned int key;
    void* element;
    struct node* next;
}node_t;

typedef struct list{
    
    node_t* head;
    spinlock_t lk;
}list_t;

void List_Init(list_t *list);
void List_Insert(list_t *list, void *element, unsigned int key);
void List_Delete(list_t *list, unsigned int key);
void *List_Lookup(list_t *list, unsigned int key);

#endif
