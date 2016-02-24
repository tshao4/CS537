#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "spin.h"
#include "list.h"



void List_Init(list_t *list){

    list->head = NULL;
    list->lk = 0;
}

void List_Insert(list_t *list, void *element, unsigned int key){
    
    spinlock_acquire(&list->lk);
    node_t* new = malloc(sizeof(node_t));
    new->key = key;
    new->element = element;
    new->next = NULL;
    
    new->next = list->head;
    list->head = new;
    spinlock_release(&list->lk);
}

void List_Delete(list_t *list, unsigned int key){
    
    spinlock_acquire(&list->lk);
    node_t* tmp = list->head;
    
    if (tmp != NULL) {
        if (key == tmp->key) {
            
            list->head = tmp->next;
            free(tmp);
            spinlock_release(&list->lk);
            return;
        }else{
            
            while (tmp->next != NULL) {
                if (key == tmp->next->key) {
                    
                    node_t *f = tmp->next;
                    tmp->next = tmp->next->next;
                    free(f);
                    spinlock_release(&list->lk);
                    return;
                }
                tmp = tmp->next;
            }
        }
    }
    spinlock_release(&list->lk);
}

void *List_Lookup(list_t *list, unsigned int key){
    
    spinlock_acquire(&list->lk);
    node_t* tmp = list->head;
    while (tmp != NULL) {
        if (key == tmp->key) {
            void* rt = tmp->element;
            spinlock_release(&list->lk);
            return rt;
        }
        tmp = tmp->next;
    }
    spinlock_release(&list->lk);
    return NULL;
}