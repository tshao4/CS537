#ifndef p4_spin_h
#define p4_spin_h
#include <sys/types.h>

typedef volatile unsigned int spinlock_t;
static inline uint
xchg(volatile unsigned int *addr, unsigned int newval)
{
    uint result;
    asm volatile("lock; xchgl %0, %1" : "+m" (*addr), "=a" (result) : "1" (newval) : "cc");
    //xchg: exchange the two source operands
    //two outputs: *addr and result
    //"+m", m means the operand is directly from memory, + means read and write
    //"=a": suggest gcc to put the operand value into eax register; `=' means write only
    //one input: newval, "1" means it uses the same constraint as the earlier 1th, i.e., it
    // will be put into eax and then be overwritten
    //"cc" means the condition register might be altered
    return result;
}
void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);

#endif
