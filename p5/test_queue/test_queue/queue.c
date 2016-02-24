#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


void Queue_Init(queue_t* q){
    
    q->head = NULL;
    q->tail = NULL;
}

void Queue_Push(queue_t* q, int fd){
    
    node_t* new = malloc(sizeof(node_t));
    new->fd = fd;
    new->next = NULL;
    new->length = 0;
    
    if (q->tail == NULL || q->head == NULL) {
        q->head = q->tail = new;
    }else if (q->head == q->tail){
        q->tail = new;
        q->head->next = new;
    }else{
        q->tail->next = new;
        q->tail = new;
    }
}

void p_Queue_Push(queue_t* q, int fd, unsigned int length){
    
    node_t* new = malloc(sizeof(node_t));
    new->fd = fd;
    new->next = NULL;
    new->length = length;
    
    if (q->tail == NULL || q->head == NULL) {
        q->head = q->tail = new;
    }else if (q->head == q->tail){
        if (length > q->head->length) {
            q->tail = new;
            q->head->next = new;
        }else{
            q->head = new;
            q->head->next = q->tail;
        }
    }else{
        node_t* prev, * cur;
        cur = prev = q->head;
        
        if (length <= q->head->length) {
            new->next = q->head;
            q->head = new;
        }else{
            
            while (cur != NULL) {
                if (length > cur->length) {
                    prev = cur;
                    cur = cur->next;
                }else{
                    prev->next = new;
                    new->next = cur;
                    return;
                }
            }
            q->tail->next = new;
            q->tail = new;
    
        }
    }
}

int Queue_Pop(queue_t* q){
    
    int r = -1;
    if (q->head != NULL) {
        r = q->head->fd;
        if (q->head == q->tail) {
            q->head = q->tail = NULL;
        }else{
            node_t* tmp = q->head;
            q->head = q->head->next;
            free(tmp);
        }
    }
    return r;
}