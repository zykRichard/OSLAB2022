#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#include <../framework/syscall.h>
#include <../framework/user.h>
#include <lock.h>

#define MAX_CPU        8
#define MAGIC          (int)0x1234ffff
#define PAGE_SIZE      (1 << 12)
#define MPAGE_SIZE     (1 << 13)
#define HEAPSIZE       (uintptr_t)(heap.end - heap.start)
#define STARTADDR(i)   (uintptr_t)heap.start + (HEAPSIZE >> 4) * (i)
#define PAGESTART      (uintptr_t)heap.start + (HEAPSIZE >> 1)
#define MPAGESTART     PAGESTART + (HEAPSIZE >> 2)
//#define PAGE(i)        PAGESTART + (HEAPSIZE >> 3) * (i)
#define HUGESTART      MPAGESTART + (HEAPSIZE >> 3)

// pmm:
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
kmem pagemem[2];           // page memory allocator managers
uintptr_t hbound;          // huge memory allocator bound;

// subroutines:
void    *smallalloc   (size_t sz); 
void    *pagealloc    (size_t sz);
void    *hugealloc    (size_t sz);
void     sfree        (void *ptr);
void     hfree        (void *ptr);
void     kinit        ();


// kmt :

struct task {
  struct 
  {
  const char  *name;
  void        (*entry)(void *arg);
  Context     context[4];
  int         nc;
  struct task *next;
  struct task *prev;
  };

  enum {
  RUNNING = 0, SLEEPING, WAITING, ZOMBIE, 
  } status;
  
  int         pid;
  struct task *children;
  struct task *parent;
  struct task *bros;
  
  int         nchildren;
  int         excode;
  
  int         np;   
  int         npneed;          
  void        *va[64];
  void        *pa[64];
  AddrSpace   as;
  uint8_t     kstack[4096];
} ;

struct spinlock {
  const char *name;
  int        flag;
} ;

struct semaphore {
  lock_t     lk;
  int        value;
  const char *name;  
} ;

struct cpustream {
  //task_t *tasks[64];
  task_t *curTask; // loop tasks linklist
  int    numTask;
} percpu[MAX_CPU] ;
// CPU threads queue

task_t *currents[MAX_CPU];
spinlock_t locks[MAX_CPU];
lock_t glock;
int cpurec;

// irq_handlers :
void irq_handlers    (Event e, Context *c);

// PAGEFAULT :
void PF_handler      (Event e, Context *c);
void pgmap           (task_t *task, void *va, void *pa, int prot);
// SYSCALL :
void SYS_handler     (Event e, Context *c);


// uproc :
task_t *ucreate  (int cpu, const char *name);
void   uteardown (task_t *task);
void   pgmap     (task_t *task, void *va, void *pa, int prot);
task_t *pidtables[32768];
int    pidrec;
// slab header: the metadata for every memory allocator in one slab
// typedef struct slab_header {
//   int magic; // which indicates whether this allocator has been allocated, if not, default by MAGIC
//   struct slab_header *nex_slab; // pointer to next free slab allocator
// } slab_header;

// // slab: the metadata for one slab block
// typedef struct slab {
//   int sz; // indicating the size of slab by 2^sz bytes
//   slab_header *freelist; // a linklist that indicate the slab allocators which are free 
//   struct slab *nxtslab; // pointer to next free slab of specific size
//   lock_t m; // spinlock for one block
//   bool isfull; // indicate whether this slab is full, if that, default by 0
// } slab;

// // linklist for free slab
// // typedef struct list {
// //   slab *slabfree; // pointer to a slab
// //   list *next;
// // } list;

// // cache for different size slab allocators
// typedef struct cache {
//   int sz;  // which indicates 2^sz bytes slab allocator
//   lock_t m; // spinlock for one cache 
//   slab *freeslab; // linklist for free slabs
// } cache;

// //global variables:
// // for lock rest:
// // global lock > cache lock > slab lock
// lock_t g_lock; // global lock
// lock_t b_lock; // lock for huge memory allocation
// uintptr_t boundnow; // indicates the bound of pages allocated 
// uintptr_t meta; // indicates the metadata usage bound;
// cache *cachelist; // cachelist, which is an array on the heap.
// //int pagenum = 0; // indicates the number of pages allocated
// uint8_t *intree ; // 64bit - bitmap for buddy systems
// // 0 for free; 1 for partial; 2 for full;

// //subroutines:

// // samllalloc: allocate memory in range between 16 bytes and 1024 bytes
// // this function will call cache locking and slab locking;
// void *smallalloc(size_t sz);

// // pagealloc: allocate a page and initialize the slab;
// // this function will call global locking and slab locking;
// slab *pagealloc(size_t sz);

// // hugealloc: allocate a memory block of which size is no less than 4096bytes
// // this function will call buddy locking
// void *hugealloc(size_t sz);

// void smallfree(void *ptr);
// void hugefree(void *ptr);

// //initialization functions:

// void global_init();

// //assistance function:

// //dfs: search the buddy tree to find the possible memory block
// // sz = 25 - log(real_sz) the full block is no less than 32MiB
// int dfs(int dep, int id, size_t sz);




