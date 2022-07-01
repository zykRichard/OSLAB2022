#include <stdio.h>
#include <pthread.h>


int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void thr_exit(){
    pthread_mutex_lock(&m);
    done = 1;
    pthread_cond_signal(&c);
    printf("If this has been printed, which means that m will be unlocked in child process\n");
   // pthread_mutex_unlock(&m);
}

void *child(){
    printf("child here\n");
    thr_exit();
    return NULL;
}

void thr_join(){
    pthread_mutex_lock(&m);

    printf("Hello\n");
    while(done == 0)
        pthread_cond_wait(&c, &m);
    if(done == 0) printf("lock will be released in parent process\n");
    pthread_mutex_unlock(&m);
}

int main(){
    printf("parent begin\n");
    pthread_t th;
    pthread_create(&th, NULL, child, NULL);
    thr_join();
    printf("parent end\n");
    return 0;
}