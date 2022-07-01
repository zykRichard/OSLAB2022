#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define STACK_SIZE 32768 
#define ALIGN __attribute__((aligned(16)))
#ifdef LOCAL_MACHINE
    #define debug(...) printf(__VA_ARGS__)
#else 
    #define debug(...)
#endif


static inline void stack_switch_call (void *sp, void *entry, uintptr_t arg) {
  //debug("That's OK!\n");
  asm volatile (
  #if __x86_64__
      "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      :
      :"b"((uintptr_t)sp), "d"(entry), "a"(arg) 
      :"memory"
  #else 
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : 
      :"b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
  #endif
  );
}



enum co_status {
  CO_NEW = 1, // new coroutine
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD,
};

struct co {
  char name[64] ;// key-value pair is needed.
  void (*func) (void *) ;
  void *arg ;

  enum co_status status ;
  struct co *    waiter ;
  struct co *    next  ; 
  struct co *    front ;
  jmp_buf        context  ;
  uint8_t        stack[STACK_SIZE] ;
  uint8_t        buf[16];

};

// Global variables.
struct co *current = NULL;
// Ready for main function:
__attribute__((constructor))
      static inline void ready(){
        struct co *mainco = (struct co*)malloc(sizeof(struct co));
        strcpy(mainco -> name, "main");
        mainco -> func = NULL; // default by main 
        mainco -> arg = NULL;
        mainco -> status = CO_RUNNING;
        mainco -> waiter = NULL;
        mainco -> next = mainco;
        mainco -> front = mainco;
        current = mainco;
      }



static inline void *wrapper(uintptr_t arg){
    struct co* cur = (struct co*) arg;
    cur -> func(cur -> arg);
    cur -> status = CO_DEAD;
    co_yield();
    return NULL;
}


struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  struct co * newco = (struct co*)malloc(sizeof(struct co));
  strcpy(newco -> name, name);
  newco -> func = func;
  newco -> arg = arg;
  newco -> status = CO_NEW;
  newco -> waiter = NULL;
  if(current == NULL){
   assert(0);
   current = newco;
   current -> next = current;
   current -> front = current;
  }
  else {
    newco -> next = current -> next;
    newco -> front = current;
    current -> next -> front = newco;
    current -> next = newco;
    
  }
  return newco;
}
// loop coroutine pool : when inserting a new coroutine, maintain a loop structure.

void co_wait(struct co *co) {
  current -> status = CO_WAITING;
  co -> waiter = current;
  while(co -> status != CO_DEAD) 
    co_yield();
  co -> waiter -> status = CO_RUNNING;
  co -> front -> next = co -> next;
  co -> next -> front = co -> front;
  free(co);
  return;
}



void co_yield() {
  int ret = setjmp(current -> context);
  if(ret == 0) // which means we need to choose another coroutine to run;
  {
    current = current -> next;
    if(current -> status == CO_NEW) // New coroutine, hasn't been called 
    {
     current -> status = CO_RUNNING;
     stack_switch_call(current -> buf, wrapper, (uintptr_t)current); 
     debug("return here?\n");
    }
    else if(current -> status == CO_RUNNING || current -> status == CO_WAITING) // which means this coroutine has been called before
      longjmp(current -> context, 1);
  }
  else return ; // which means this coroutine has been called before and should be recovered now.
  
}

  __attribute__((destructor))
      static inline void join(){
        assert(current);
        assert(strcmp(current -> name, "main") == 0);
        while(current -> next != current)
          co_wait(current -> next);
      }