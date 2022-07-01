#ifndef TEST


#include <common.h>

//#include "initcode.inc"



static void uproc_init() {
    vme_init((void *(*)(int))(pmm -> alloc), (void (*)(void *))(pmm -> free));
    pidrec = 1;
    return ;
}

static int kputs(task_t *task, char ch) {
    putch(ch);
    int id = task -> nc - 1;
    return task -> context[id].GPRx;
}

static int fork(task_t *task){
    task_t *nproc = ucreate(cpu_current(), "children");
    int id = task -> nc - 1;
    uintptr_t rsp0 = nproc -> context[0].rsp0;
    void *cr3 = nproc -> context[0].cr3;

    nproc -> context[0] = task -> context[id];
    nproc -> context[0].rsp0 = rsp0;
    nproc -> context[0].cr3 = cr3;
    nproc -> context[0].GPRx = 0;

    for(int i = 0; i < task -> np; i++) {
        void *va = task -> va[i];
        void *pa = task -> pa[i];
        int sz = task -> as.pgsize;
        void *npa = pmm -> alloc(sz);
        memcpy(npa, pa, sz);
        pgmap(nproc, va, npa, PROT_READ|PROT_WRITE);
    }

    
    nproc -> parent = task;
    nproc -> bros = task -> children;
    task -> children = nproc;

    task -> nchildren ++ ;

    // return value:
    task -> context[task -> nc - 1].GPRx = nproc -> pid;
    nproc -> context[nproc -> nc - 1].GPRx = 0;
    return nproc -> pid;
}

static int wait(task_t *task, int *status) {
    return -1;
}

static int exit(task_t *task, int status) {
    task -> status = ZOMBIE;
    task -> excode = status;
    yield();
    panic("never return");
}

static int kill(task_t *task, int pid) {
    return -1;
}

static void *mmap(task_t *task, void *addr, int length, int prot, int flags) {
    return NULL;
}

static int getpid(task_t *task){
    int id = task -> nc - 1;
    task -> context[id].GPRx = task -> pid;
    return task -> pid;
}

static int sleep(task_t *task, int seconds) {
    uint64_t wakeup = io_read(AM_TIMER_UPTIME).us + 1000000L * seconds;
    int id = task -> nc - 1;
    while(io_read(AM_TIMER_UPTIME).us <= wakeup)
        yield();
    return task -> context[id].GPRx;
}

int64_t uptime(task_t *task) {
    return -1;
}


task_t *ucreate(int cpu, const char *name) {
   task_t *uproc = pmm -> alloc(sizeof(task_t));
   kmt -> spin_lock(&locks[cpu]);
   // atomic :
   // add this process to cpu queue
   protect(&(uproc -> as));
   percpu[cpu].numTask ++;
   uproc -> name = name;
   uproc -> entry = uproc -> as.area.start;
   uproc -> next = percpu[cpu].curTask -> next;
   uproc -> prev = percpu[cpu].curTask;
   uproc -> np = 0;
   uproc -> nc = 1;
   uproc -> status = RUNNING;
   uproc -> nchildren = 0;
   uproc -> parent = NULL;
   uproc -> children = NULL;
   uproc -> bros = NULL;
   uproc -> pid = pidrec;
   pidtables[pidrec] = uproc; 
   pidrec ++;
   percpu[cpu].curTask -> next -> prev = uproc;
   percpu[cpu].curTask -> next = uproc;
   uproc -> context[0] = *(ucontext(&(uproc -> as), (Area){&(uproc -> kstack), uproc + 1}, uproc -> entry));
   
   
   kmt -> spin_unlock(&locks[cpu]);
   return uproc;
}

void uteardown(task_t *task) {
    assert(task -> status == ZOMBIE);
    unprotect(&(task -> as));
    pmm -> free(task);
}

MODULE_DEF(uproc) = {
    .init    =  uproc_init,
    .kputc   =  kputs,
    .fork    =  fork,
    .wait    =  wait,
    .exit    =  exit,
    .kill    =  kill,
    .mmap    =  mmap,
    .getpid  =  getpid,
    .sleep   =  sleep,
    .uptime  =  uptime 
};

#endif