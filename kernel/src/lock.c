#ifndef TEST
#include <lock.h>
#include <kernel.h>
#else 
#include <lock_test.h>

int atomic_xchg(int *addr, int newval) {
    int result;
    asm volatile(
        "lock xchg %0, %1":
        "+m"(*addr),
        "=a"(result):
        "1"(newval)
    );
    return result;
}
#endif
int ROUND_SELF(size_t x) {  
    int count = 0;   
    uint64_t i = 1;            
    while(i < x){          
        i = i << 1;
        count++;
    }
    return count;               
}                            
//mutex lock section:


void lockinit (lock_t *m){
    m -> flag = 0;
}

void lock(lock_t *m){
    while(atomic_xchg(&(m -> flag), 1));
}

void unlock(lock_t *m){
    atomic_xchg(&(m -> flag), 0);
}
