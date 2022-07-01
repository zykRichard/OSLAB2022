#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef int BOOL;
#define FALSE 0
#define TRUE 1
BOOL n, p, v = FALSE;
const char version[] = "pstree (PSmisc) UNKNOWN\n版权所有 © 1993-2019 Werner Almesberger 和 Craig Small\n\nPSmisc 不提供任何保证.\n该程序为自由软件，欢迎你在 GNU 通用公共许可证 (GPL) 下重新发布。\n详情可参阅 COPYING 文件。\n";


struct proc {
  pid_t pid;
  struct proc* child;
  struct proc* bros;
  char spid[6];
  char name[30];
} ;

struct proc procs[1000], *root;


/*int children[40000];// record the number of child processes of PID process into children[PID];
int *child_pid[40000];
char name_rec[40000][50];*/

int num = -1; // record the number of PIDs;

void init(){
  DIR *dir;
  FILE *fp;
  char path[50];
  dir = opendir("/proc");
  //printf("That's OK\n");
  struct dirent *ptr;
  while((ptr = readdir(dir)) != NULL){
    char s[30];
    strcpy(s, ptr -> d_name);
    //printf("That's OK, and string is %s\n", s);
    BOOL flag = TRUE; // Whether is this dir desired?(consist of all number characters)
    int pid = 0;
    for(int i = 0; s[i] != '\0'; i++){
       if(!isdigit(s[i])) {flag = FALSE; break;}
       pid = pid * 10 + (int)(s[i] - '0');
    }
    if(flag){
    num ++ ;
    procs[num] = (struct proc) {.pid = pid, .bros = NULL, .child = NULL}; 
    strcpy(procs[num].spid, s);
    if(pid == 1) root = &procs[num];
    //printf("The PID of this process is %d\n", pid);
    }
  }
  // This part is to record PID.


    for(int i = 0; i<=num; i++){
      //printf("Let's goto second part!\n");
      strcpy(path, "/proc/\0");
      strcat(path, procs[i].spid);
      strcat(path, "/stat\0");
      //printf("Path is %s\n", path);
      if((fp = fopen(path, "r"))!= NULL){
        //printf("OK to read file\n");
        char ch;
        BOOL f = TRUE; // Whether to start to record process name?
        BOOL st = FALSE; //Start to record ?
        BOOL end = FALSE; //Whether PPID has been recorded?
        int cntt = 0;
        int ppid = 0;

        //calculate PPID
        while((ch = fgetc(fp)) != EOF){
          if(f){
            if(ch == '(') {st = TRUE; continue;}
            if(ch == ')') {f = FALSE; st = FALSE; continue;}
            if(st){
              procs[i].name[cntt] = ch;
              cntt ++;
            }
          }
          else {
              if(isdigit(ch)){
                ppid = ppid * 10 + (int)(ch - '0');
                end = TRUE;
              }
              if(end)
                if(!isdigit(ch)) break;
            }
          }

          // Tree Building
          int idx;
          for(idx = 0; idx <= num; idx++)
            if(procs[idx].pid == ppid) break;           
          if(procs[idx].child == NULL)
            procs[idx].child = &procs[i];
          else{
           struct proc *tmp = procs[idx].child;
           while(tmp -> bros != NULL) tmp = tmp -> bros;
           tmp -> bros = &procs[i]; 
          } 

        }
      
      // This part is to record the relation between process and its parent process;

      else {printf("No Such FILE %s\n", path); assert(0);}
    }
  
}

void P(struct proc *r, int depth){
    if(v){
      fprintf(stderr, version);
      return ;
    }
    //printf("That's OK !");
    if(p)
    printf("%s(%d)", r -> name, r -> pid);
    else printf("%s", r -> name);
    if(r -> child == NULL) return ;
    else {
      struct proc *tmp = r -> child;
      for( ;tmp != NULL; tmp = tmp -> bros){
        printf("\n");
        for(int i = 0; i <= depth; i++)
        printf("  ");
        P(tmp, depth + 1);
      }
    }
  }

int main(int argc, char *argv[]) {
  //memset(children, 0, sizeof(children));
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    if(!(strcmp(argv[i], "-p")) || !(strcmp(argv[i], "--show-pids"))) p = TRUE;
    if(!(strcmp(argv[i], "-n")) || !(strcmp(argv[i], "--numeric-sort"))) n = TRUE;
    if(!(strcmp(argv[i], "-V")) || !(strcmp(argv[i], "--version"))) v = TRUE;
    //if(!(strcmp(argv[i], ">"))) {printf("It's going to be changed.");}
    //printf("argv[%d] = %s\n", i, argv[i]);
  }
  init();
 // printf("%s\n",name_rec[1]);
  P(root, 1);
  assert(!argv[argc]);
  return 0;
}
