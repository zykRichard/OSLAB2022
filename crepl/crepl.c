#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/wait.h>


#define JUDGE (sizeof(long) == 8)
typedef int (*EXE_FUNC)(void);

typedef struct fun_map {
  int id;
  const char fun_name[32];
} fun_map;

typedef struct func_map {
  int id;
  int (*func)(void);
} func_map;


// char *funcname[100] = {"__expr_wrapper_0", "__expr_wrapper_1", NULL};
// char *libname[100] = {"tmplib0.so", "tmplib1.so", NULL};
int cnt = 0;
// char pathname[32] = "/tmp/";
// static fun_map maps [100]= {
//   {0, "__expr_wrapper_0"}, {1, "__expr_wrapper_1"}
// };

// static func_map fmaps[100] = {
//   {0, expr(0)}, {1, expr(1)}, 
// };
void error_handler() {
  perror("fault");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[], char *envp[])
{
  static char line[4096];
  size_t len = 0;
  char rubbish[32] = "/tmp/null.XXXXXX";
  int fk = mkstemp(rubbish);
  while (1)
  {
    printf("crepl> ");
    fflush(stdout);
    char pathname[32] = "/tmp/aXXXXXX.c";
    int fd = mkstemps(pathname, 2);      // open a new temporary file in every loop  
    // strcat(pathname, name);     
    char inttype[4] = "int ";   
    // generate a library name :
    char libname[32] = "/tmp/tmplib";
    char str[4];
    sprintf(str, "%d", cnt);
    strcat(libname, str);
    strcat(libname, ".so");

    // generate a function name:
    char funcname[32] = "__expr_wrapper_";
    strcat(funcname, str);
    #if __x86_64__
    char *machine = "-m64";
    #else 
    char *machine = "-m32";
    #endif
    char *exec_argv[] = {"/usr/bin/gcc", "-fPIC", "-shared", "-w", machine , pathname, "-o" ,libname, NULL};
    int flag = 0;
    // int rdbytes = 0;
    // rdbytes = getline(&line, &len, stdin);
    if(fgets(line, sizeof(line), stdin))
    {
    
      if (strncmp(inttype, line, 4) == 0)
         { 
          //  printf("it's a function\n");
           write(fd, line, sizeof(line));
           write(fd, "\n", 1);
    }
      
    else {
      flag = 1;
      // char funame[32]; 
      // strcpy(funame, funcname[cnt]);
      write(fd, inttype, sizeof(inttype));
      write(fd, funcname, sizeof(funcname));
      write(fd, "()", 2);
      write(fd, "{", 1);
      write(fd, "return (", 8);
      write(fd, line, sizeof(line));
      write(fd, ");};\n", 4);   
    }
    // printf("Got %zu chars.\n", strlen(line));
    }

    else { printf("exit\n"); break; }

    pid_t pid = fork();

    if(pid < 0){
      error_handler();
    }

    else if(pid == 0) {
        // close(2);
        execve(exec_argv[0], exec_argv, envp);  
    }
    // child processor which compiles wrapper into shared library

    else {
      int status = 0;
      pid_t st = wait(&status);
      // printf("the ret value of wait is %d\n", st);
      if(!WEXITSTATUS(status)){
        void* handler = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
        if(!handler){
          cnt ++;
          fprintf(stderr, "%s\n", dlerror());
          exit(EXIT_FAILURE);
        }
        if(flag) {
          EXE_FUNC exe_func = NULL;
          exe_func = (EXE_FUNC) dlsym(handler, funcname);
          printf("%d\n", exe_func());
          cnt ++;
        }
        else cnt++;
    }
    // child correctly returns, loading... 
    else {
        printf("compile error\n");
        continue;
    }
    // compile error
    memset(line, 0, sizeof(line));
    }
    //parent processor which is responsible for judging and loading
}
  return 0;
}
