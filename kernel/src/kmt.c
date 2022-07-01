#ifndef TEST

#include <common.h>

int intenb[MAX_CPU]; // indicate the interrput condition of CPU
int nest[MAX_CPU];   // indicate nest number of every CPU


void k_init(){
    for(int i = 0; i < cpu_count(); i++){
        task_t *idle = pmm -> alloc(sizeof(task_t));
        idle -> name = "idle";
        idle -> entry = os -> run;
        idle -> next = idle;
        idle -> prev = idle;
        idle -> nc = 0;
        percpu[i].curTask = idle;
        percpu[i].numTask = 1;
        currents[i] = idle;    
    } 
    
    for(int i = 0; i < cpu_count(); i++) {
        kmt -> spin_init(&(locks[i]), "cpulock");
        nest[i] = 0;
    }
    cpurec = 0;
    lockinit(&(glock));
}

void spin_init(spinlock_t *lk, const char *name) {
    lk -> flag = 0;
    lk -> name = name;
};

void spin_lock(spinlock_t *lk) {
    int c = cpu_current();
    //printf("cpu id is %d\n", c);
    int i = ienabled();

    iset(false);

    nest[c]++;
    if(nest[c] == 1){
        intenb[c] = i;
        while(atomic_xchg(&(lk -> flag), 1));   
    }
}

void spin_unlock(spinlock_t *lk) {
    int c = cpu_current();
    //printf("cpu id is %d\n", c);
    nest[c]--;
    assert(nest[c] >= 0);
    if(nest[c] == 0){
        atomic_xchg(&(lk -> flag), 0);
        assert(!ienabled());
        if(intenb[c]) iset(true);
    }
    
}

void sem_init(sem_t *sem, const char *name, int value) {
    lockinit(&(sem -> lk));
    sem -> value = value;
    sem -> name = name;
}

void sem_wait(sem_t *sem) {
    int acquire = 0;
    while(!acquire) {
        lock(&(sem -> lk));
        if(sem -> value > 0) {
            sem -> value --;
            acquire = 1;
        }
        unlock(&(sem -> lk));
        if(!acquire) yield();
    }
}

void sem_signal(sem_t *sem) {
    lock(&(sem -> lk));
    sem -> value ++;
    unlock(&(sem -> lk));
}

int kcreate(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
    //printf("task addr is %p \n", (void *)task);
    task -> name = name;
    task -> entry = entry;
    Area stack = (Area) {&(task -> kstack), task + 1};
    task -> context[0] = *kcontext(stack, entry, arg);
    task -> nc = 1;
    task -> status = RUNNING;
    //printf("it's going to spinlock\n");
    kmt -> spin_lock(&locks[cpu_current()]); 
    lock(&(glock));
    int cpu = cpurec;
    cpurec = (cpurec + 1) % cpu_count();
           
    // add this task into CPU queue
    percpu[cpu].numTask ++ ;
    // insert task into current thread and its next thread
    task -> next = currents[cpu] -> next;
    task -> prev = currents[cpu] ;
    
    currents[cpu] -> next = task;
    task -> next -> prev = task;
    unlock(&(glock));
    kmt -> spin_unlock(&locks[cpu_current()]);
    return 1;
}

void kteardown(task_t *task) {
    spin_lock(&locks[cpu_current()]);
    task -> next -> prev = task -> prev;
    task -> prev -> next = task -> next;
    spin_unlock(&(locks[cpu_current()]));
    pmm -> free(task);
    return ;
}


MODULE_DEF(kmt) = {
   .init = k_init,
   .sem_init = sem_init,
   .sem_signal = sem_signal,
   .sem_wait = sem_wait,
   .create = kcreate,
   .teardown = kteardown,
   .spin_init = spin_init,
   .spin_lock = spin_lock,
   .spin_unlock = spin_unlock, 
};

#endif