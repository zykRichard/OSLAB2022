#include <assert.h>
#include <common_test.h>
#include <stdint.h>
#include <thread.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// int cpu_count () {
//     return thread_num();
// }
Area heap = {(void *)0, (void *)0};
#define SMP 1 
#define MAX_NUM 999999
Area has_alloc[1000001];
int num = 0;

int cpu_current () {
    return 0;
}

void Judge(void *ptr){
    for(int i = 0; i < num; i++){
        pthread_mutex_lock(&m);
        //printf("addr0x%x is from 0x%x to 0x%x\n",ptr, has_alloc[i].start, has_alloc[i].end);
        assert(!IN_RANGE(ptr, has_alloc[i]));
        pthread_mutex_unlock(&m);
    }
    // pthread_mutex_lock(&m);
    // printf("addr0x%x is from 0x%x to 0x%x\n",ptr, has_alloc[num].start, has_alloc[num].end);
    // pthread_mutex_unlock(&m);
}


void *test(){
    int a = 0;
        while(1){
            srand((unsigned int)time(0));
            sleep(1);
            a =  12 + rand() % 2; 
            void *p = kalloc((size_t)(1 << a));
            if(!p) {printf("There are no free space!\n"); return NULL;}
            //Judge(p);
            pthread_mutex_lock(&m);
            Area *newar = malloc(sizeof(Area));
            newar -> start = p;
            newar -> end = (void *)(p + (1 << a));
            has_alloc[num] = *newar;
            printf("Memory alloc %ld bytes from [0x%x, 0x%x)\n", (1 << a), newar -> start, newar -> end);
            num++;
            kfree(p);
            if(num >= MAX_NUM) {pthread_mutex_unlock(&m); break;}
            pthread_mutex_unlock(&m);

        }
    return 0;
}
int main()
{   pmm_init();
    for(int i = 1; i <= SMP; i++)
        create(test);
    join();
    
	return 0;
}