#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash_init();

while(1){
  uptime();
}
  return 0;
}
