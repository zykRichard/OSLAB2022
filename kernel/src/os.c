#ifndef TEST
#include <common.h>
#include "initcode.inc"
sem_t empty1, fill1, empty2, fill2;
#define P kmt -> sem_wait
#define V kmt -> sem_signal
void producer1(void *arg) {while(1){P(&empty1); printf("("); V(&fill1);} }
void consumer1(void *arg) {while(1){P(&fill1); printf(")"); V(&empty1);} }
void producer2(void *arg) {while(1){P(&empty2); printf("["); V(&fill2);} }
void consumer2(void *arg) {while(1){P(&fill2); printf("]"); V(&empty2);} }
void foo(void *s) { char *ch = s; while(1) { putch(*ch); yield();} }
static inline task_t *task_create() {
  return (task_t *)pmm -> alloc(sizeof(task_t));
}
static void os_init() {
  pmm -> init();
  kmt -> init();
  uproc -> init();
  // kmt -> create(task_create(), "tty-reader", (void(*)(void *))(tty_ops.read), "tty1");
  //  kmt -> sem_init(&empty1, "empty1", 1);
  //  kmt -> sem_init(&fill1 , "fill1" , 0);
  //  kmt -> sem_init(&empty2, "empty2", 2);
  //  kmt -> sem_init(&fill2 , "fill2" , 0);
  //  kmt -> create(task_create(), "producer1", producer1, "");
  //  kmt -> create(task_create(), "consumer1", consumer1, "");
  //  kmt -> create(task_create(), "consumer1", consumer1, "");
  //  kmt -> create(task_create(), "producer2", producer2, "");
  //  kmt -> create(task_create(), "consumer2", consumer2, ""); 
  //  kmt -> create(task_create(), "a", foo, "a");
  //  kmt -> create(task_create(), "b", foo, "b");
}
static void os_run() {
 
  // for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
  //   putch(*s == '*' ? '0' + cpu_current() : *s);
  // }
  // kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "a", foo, "a");
  //  kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "b", foo, "b");
  //  kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "c", foo, "c");
  while (1) yield();
   
}

static Context *os_trap(Event ev, Context *context) {
  // saving cpntext (interrupt allowed)
  int cpuid = cpu_current();
  task_t *cur = currents[cpuid];
  cur -> context[cur -> nc ++] = *context;

  // irq handlers: 
  // some syscalls: interrupt allowed
  // else : interrupt forbidden
  irq_handlers(ev, context);
  
  // schedule: (should in atomic running) least priority
  kmt -> spin_lock(&locks[cpu_current()]);
  do{

  percpu[cpuid].curTask = percpu[cpuid].curTask -> next;
  
  }  while(percpu[cpuid].curTask -> status == ZOMBIE || 
           percpu[cpuid].curTask -> status == WAITING);

  currents[cpuid] = percpu[cpuid].curTask;
  //unlock(&glock);
  kmt -> spin_unlock(&(locks[cpuid]));
  return &(currents[cpuid] -> context[--(currents[cpuid] -> nc)]); 
}

void irq_handlers(Event e, Context *c) {
  switch (e.event) {

    case EVENT_PAGEFAULT : {
      // atomic:
      PF_handler(e, c);
      break;
    }
    
    case EVENT_SYSCALL : {
      SYS_handler(e, c);
      break;
    }

    case EVENT_ERROR : {
      panic("error occurs");
    }

    default : break;
  }
}

// all the handlers should make sense in trap handler
// which guarantees the atomic
void pgmap(task_t *task, void *va, void *pa, int prot) {
  int pages = task -> np;
  task -> va[pages] = va;
  task -> pa[pages] = pa;
  (task -> np) ++;
  assert(task -> np <= 64);
  // printf("map: 0x%x -> 0x%x\n", va, pa);

  map(&(task -> as), va, pa, prot);
}

void PF_handler(Event e, Context *c) {
  kmt -> spin_lock(&(locks[cpu_current()]));
  AddrSpace *as = &(percpu[cpu_current()].curTask -> as);
  void *pa = pmm -> alloc(as -> pgsize);
  void *va = (void *)ROUNDDOWN(e.ref, as ->pgsize);
  
  

  pgmap(percpu[cpu_current()].curTask, va, pa, MMAP_READ | MMAP_WRITE);
  kmt -> spin_unlock(&(locks[cpu_current()]));
}


void SYS_handler(Event e, Context *c) {
  task_t *curtask = percpu[cpu_current()].curTask;
  switch(c -> GPRx) {
      case SYS_kputc: {
        uproc -> kputc(curtask, c -> GPR1);
        break;
      } 

      case SYS_sleep: {
        //kmt -> spin_unlock(&(locks[cpu_current()])); // warning !!!
        uproc -> sleep(curtask, c -> GPR1);
        //kmt -> spin_lock(&(locks[cpu_current()]));
        break;
      }
      
      case SYS_exit: {
         uproc -> exit(curtask, c -> GPR1);
         break;
      }

      case SYS_fork: {
        // atomic:
        kmt -> spin_lock(&(locks[cpu_current()]));
        uproc -> fork(curtask);
        kmt -> spin_unlock(&(locks[cpu_current()]));
        break;
      }
      
      case SYS_getpid: {
        uproc -> getpid(curtask);
        break;
      }
      default : break;
  }
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
};
#endif