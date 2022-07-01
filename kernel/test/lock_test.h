#include <stdint.h>
#include <stddef.h>
int ROUND_SELF(size_t x);      
//mutex lock section:


typedef struct __lock_t {
  int flag;
} lock_t;

void init (lock_t *m);

void lock(lock_t *m);

void unlock(lock_t *m);

