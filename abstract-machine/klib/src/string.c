#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  assert(s);
  for(size_t i = 0; ; i++){
  	if(s[i] == '\0') return i;
  	}
}

char *strcpy(char *dst, const char *src) {
  assert(dst);
  assert(src);
  for(size_t i = 0;;i++){
  	if(src[i]!='\0') 
  	dst[i] = src[i];
  	else {dst[i] = '\0'; return dst;}
  	}
}

char *strncpy(char *dst, const char *src, size_t n) {
   assert(dst);
   assert(src);
   size_t i = 0;
   for ( i = 0; i < n; i++)
   	dst[i] = src[i];
   dst[i] = '\0';
   return dst;
}

char *strcat(char *dst, const char *src) {
   int len1 = 0, len2 = 0;
   while(dst[len1] != '\0')
   	len1++;
   while(src[len2] != '\0'){
   	dst[len1+len2] = src[len2];
   	len2++;
   	}
   	dst[len1+len2] = '\0';
   	return dst;
}

int strcmp(const char *s1, const char *s2) {
    assert(s1);
    assert(s2);
    const char *tmp1 = s1;
    const char *tmp2 = s2;
    while(*tmp1 == *tmp2){
    	if (*tmp1 == '\0') return 0;
    	tmp1++, tmp2++;
    	}
    	if(*tmp1 == '\0'|| *tmp1 < *tmp2) return -1;
    	if(*tmp2 == '\0'|| *tmp1 > *tmp2) return 1;
    	else return 0; 
}

int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1);
  assert(s2);
  size_t cnt = 0;
  while(*s1 == *s2 && cnt < n){
  	s1++;
  	s2++;
  	cnt++;
  }
  if(cnt == n) return 0;
  else if(*s1 == '\0' || *s1 < *s2) return -1;
  else if(*s2 == '\0' || *s1 > *s2) return 1;
  else return 0;
  
}

void *memset(void *s, int c, size_t n) {
  //怎样控制每次转移一个字节；
  //当然是char型
  char *tmp = s;
  while(n--){
  	*tmp = (char) c;
  	tmp++;
  	}
  	return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  assert(dst && src);
  char *tmp1 = dst;
  const char *tmp2 = src;
  if(tmp2 < tmp1)
  	while(n--){
  		*tmp1 = *tmp2;
  		tmp1++, tmp2++;
  		}
  else if(tmp2 > tmp1){
  	tmp2 += n-1;
  	tmp1 += n-1;
  	while(n--){
  		*tmp1 = *tmp2;
  		tmp1--, tmp2--;
  		}
}
return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  	char *tmp1 = out;
  	const char *tmp2 = in;
  	while(n--){
  		*tmp1 = *tmp2;
  		tmp1++;
  		tmp2++;
  		}
  	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
        assert(s1);
  	assert(s2);
  	const char *tmp1 = s1;
  	const char *tmp2 = s2;
  		while(n-- && tmp1){
  			if(*tmp1 != *tmp2){
  				int ret = *(unsigned char*)s1 - *(unsigned char*)s2;
  				if(ret > 0)
  					return 1;
  				else 
  					return -1;
  					}
  			tmp1++;
  			tmp2++;
  			}
  			return 0;
}

#endif
