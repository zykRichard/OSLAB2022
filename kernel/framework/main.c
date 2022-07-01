#include <common.h>
#include <klib.h>

// void idle() {
//   iset(1);
//   while(1)
//     yield();
// }

// void foo(void *s) {
//   char *ch = s;
//   while(1) { putch(*ch); }
//   //return NULL;
// }
void uproc_test() {
  for(int i = 1; i <= 1 ; i++)
    ucreate(0, "test");
  

}


int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  uproc_test();
  // kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "a", foo, "a");
  // kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "b", foo, "b");
  // kmt -> create((task_t *)(pmm -> alloc(sizeof(task_t))), "c", foo, "c");
  // //printf("new thread has been created\n");
  mpe_init(os -> run);
  return 1;
}
