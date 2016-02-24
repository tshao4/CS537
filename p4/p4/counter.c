#include <stdio.h>
#include "pthread.h"
#include "spin.h"
#include "counter.h"

void Counter_Init(counter_t *c, int value){
    
    c->value = value;
    c->lock = 0;
}

int Counter_GetValue(counter_t *c){
    
    return c->value;
}

void Counter_Increment(counter_t *c){
    
    spinlock_acquire(&c->lock);
    c->value++;
    spinlock_release(&c->lock);
}

void Counter_Decrement(counter_t *c){
    
    spinlock_acquire(&c->lock);
    c->value--;
    spinlock_release(&c->lock);
}