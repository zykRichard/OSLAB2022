// #include <common.h>


// struct task {
//   const char  *name;
//   void        (*entry)(void *arg);
//   Context     *context;
//   uint8_t     kstack[4096];
// } ;

// struct spinlock {
//   const char *name;
//   int        flag;
// } ;

// struct semaphore {
//   spinlock_t lk;
//   int        value;  
// } ;

// struct cpustream {
//   task_t *tasks[64];
//   int    curThread;
//   int    numThread;
// } percpu[MAX_CPU] ;
// // CPU threads queue

