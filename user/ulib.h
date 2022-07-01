#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "../kernel/framework/syscall.h"
#include "../kernel/framework/user.h"

static inline long syscall(int num, long x1, long x2, long x3, long x4) {
  register long a0 asm ("rax") = num;
  register long a1 asm ("rdi") = x1;
  register long a2 asm ("rsi") = x2;
  register long a3 asm ("rdx") = x3;
  register long a4 asm ("rcx") = x4;
  asm volatile("int $0x80"
    : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4)
    : "memory");
  return a0;
}

static inline int kputc(char ch) {
  return syscall(SYS_kputc, ch, 0, 0, 0);
}

static inline int fork() {
  return syscall(SYS_fork, 0, 0, 0, 0);
}

static inline int wait(int *status) {
  return syscall(SYS_wait, (uint64_t)status, 0, 0, 0);
}

static inline int exit(int status) {
  return syscall(SYS_exit, status, 0, 0, 0);
}

static inline int kill(int pid) {
  return syscall(SYS_kill, pid, 0, 0, 0);
}

static inline void *mmap(void *addr, int length, int prot, int flags) {
  return (void *)syscall(SYS_mmap, (uint64_t)addr, length, prot, flags);
}

static inline int getpid() {
  return syscall(SYS_getpid, 0, 0, 0, 0);
}

static inline int sleep(int seconds) {
  return syscall(SYS_sleep, seconds, 0, 0, 0);
}

static inline int64_t uptime() {
  return syscall(SYS_uptime, 0, 0, 0, 0);
}




size_t strlen(const char *s) {
  for(size_t i = 0; ; i++){
  	if(s[i] == '\0') return i;
  	}
}



int vsprintf(char *out, const char *fmt, va_list ap) {
     int len;
     char *str;
     char *s;
     int temp;
     int i = 0;
     char buf[256];
     for(str = out; *fmt != '\0'; ++fmt){
	     if(*fmt != '%'){
		     *str++ = *fmt;
		     continue;
	     }
	     ++fmt; //skip '%'
             switch(*fmt) {
		     case 's' : //string
			     s = va_arg(ap, char*);
			     len = strlen(s);
			     for( i = 0; i < len; i++)
				     *str++ = *s++;
			     break;
		     case 'd' ://decimal
			     temp = va_arg(ap, int);
			     i = 0;
			     if(temp == 0)
				     *str++ = '0';
			     else{ 
			        for(i = 0; temp != 0; i++){
				    buf[i] = temp % 10 + '0';
				    temp = temp / 10;
				}
				i--;
				while(i>=0){
					*str = buf[i];
					str++;
					i--;
				}
			     }
				break;
		     case 'x' : //hexadecimal
			temp = va_arg(ap, int);
		        i = 0;
			if(temp == 0)
				*str++ = '0';
			else{ 
				for(i = 0; temp != 0; i++){
					buf[i] = temp % 16 + '0';
					temp = temp /16;
				}
				i--;
				while (i>=0){
					*str = buf[i];
					str++;
					i--;
				}
			     }
				break;
		      case '0' : //width
				fmt++;
				int len = *fmt - '0';
				fmt++;
				if (*fmt == 'd')
				{  
					temp = va_arg(ap, int);
				        i = 0;
					for(i = 0; temp != 0; i++){
						buf[i] = temp % 10 + '0';
						temp = temp / 10;
					}  
					i--;
					len = len - i - 1;
					if(len > 0)
						while(len > 0){
							*str = '0';
							str++;
							len--;
						}
					while(i >= 0){
						*str = buf[i];
						str++;
						i--;
					}
				}
			      else if(*fmt == 'x'){
					temp = va_arg(ap, int);
				        i = 0;
					for(i = 0; temp != 0; i++){
						buf[i] = temp % 16 + '0';
						temp = temp / 16;
					}
					i--;
					len = len - i - 1;
					if(len > 0)
						while(len > 0){
							*str = '0';
							str++;
							len--;
						}
					while(i >= 0){
						*str = buf[i];
						str++;
						i--;
					}
				}
			      break;
		      case '\\' :
				fmt++;
				if(*fmt == 'n')
					*str++ = '\n';
				else *str++ = *fmt;
				break;
		      default : break;
	     }
}
		*str = '\0';
		return (str - out);
		}


int printf(const char *fmt, ...) {
	//assert(fmt);
	int i;
	va_list ap;
	char buf_[1025];
	va_start(ap, fmt);
	i = vsprintf(buf_, fmt, ap);
	int cnt = 0;
	while(cnt <= i){
		kputc(buf_[cnt]);
		cnt++;
	}
	va_end(ap);
	return i;


}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	int i;
	va_start(ap, fmt);
	i = vsprintf(out, fmt, ap);
	va_end(ap);
	return i;
}