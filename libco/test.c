#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

#define STACK_SIZE 32768




enum co_status {
  CO_NEW = 1, // new coroutine
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD,
};


struct co {
  char *name ;// key-value pair is needed.
  void (*func) (void *) ;
  void *arg ;

  enum co_status status ;
  struct co *    waiter ;
  jmp_buf        context ;
  uint8_t        stack[STACK_SIZE] ;

  struct co *    next  ; 
};

int main(){
	printf("size is %ld\n", sizeof(struct co));
	return 0;
}
