#include <stdio.h>
#include <stdlib.h>
#include "spin.h"

void spinlock_acquire(spinlock_t *lock){
    
    while (xchg(lock, 1));
}

void spinlock_release(spinlock_t *lock){
    
    *lock = 0;
}