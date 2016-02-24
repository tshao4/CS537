#ifndef p4_list_h
#define p4_list_h

typedef struct node{
    
    int fd;
    unsigned int length;
    struct node* next;
}node_t;

typedef struct queue{
    
    node_t* head;
    node_t* tail;
}queue_t;

void Queue_Init(queue_t* q);
void Queue_Push(queue_t* q, int fd);
void p_Queue_Push(queue_t* q, int fd, unsigned int length);
int Queue_Pop(queue_t* q);

#endif
