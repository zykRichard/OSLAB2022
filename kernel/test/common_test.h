
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <lock_test.h>
#include <am_test.h>
#include <stddef.h>
#include <stddef.h>
#include <stdbool.h>



#define MAGIC          (int)0x1234ffff
#define PAGE_SIZE      (1 << 12)
#define MPAGE_SIZE     (1 << 13)
#define HEAPSIZE       (1 << 27)
#define STARTADDR(i)   (uintptr_t)heap.start + (HEAPSIZE >> 4) * (i)
#define PAGESTART      (uintptr_t)heap.start + (HEAPSIZE >> 1)
#define MPAGESTART     PAGESTART + (HEAPSIZE >> 2)
//#define PAGE(i)        PAGESTART + (HEAPSIZE >> 3) * (i)
#define HUGESTART      MPAGESTART + (HEAPSIZE >> 3)
// linklist:
typedef struct list {
    struct list *nxt;
} list ; 

// memory manage list 
typedef struct kmem {
    lock_t m;
    list *freelist;
} kmem;


// global variables:
lock_t g_lock;             // global lock
kmem smallmem[8];          // small memory allocator managers
kmem pagemem[2];              // page memory allocator managers
uintptr_t hbound;          // huge memory allocator bound;

// subroutines:
void    *kalloc       (size_t size);
void     kfree        (void *ptr);
void    *smallalloc   (size_t sz); 
void    *pagealloc    (size_t sz);
void    *hugealloc    (size_t sz);
void     sfree        (void *ptr);
void     hfree        (void *ptr);
void     kinit        ();

     