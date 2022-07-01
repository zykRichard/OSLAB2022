#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
	assert(fmt);
	int i;
	va_list ap;
	char buf_[1025];
	va_start(ap, fmt);
	i = vsprintf(buf_, fmt, ap);
	int cnt = 0;
	while(cnt <= i){
		putch(buf_[cnt]);
		cnt++;
	}
	va_end(ap);
	return i;


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

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	int i;
	va_start(ap, fmt);
	i = vsprintf(out, fmt, ap);
	va_end(ap);
	return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
