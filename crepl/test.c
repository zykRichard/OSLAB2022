#include <stdio.h>
#include <stdlib.h>

int a = 1;
#define gen(i) __expr_wrapper_##i 



void gen(0) (void ){
    printf("Hello, world\n");
}

void gen(1) (void ){
    printf("hello world\n");

}

typedef struct func_map {
    int id; 
    void (*func)(void);
} func_map;

static func_map maps[10] = {
{0, gen(0)},
{1, gen(1)},
{2, gen(2)}
} ;



int main(){
    sleep(1);
   for(int i = 0 ; i <= 1; i++){
       maps[i].func();
   } 
    return 0;
}