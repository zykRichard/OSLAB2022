#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>


#define FPS 10
#define SIDE 5
#define LEN 10
#define WHITE 0xffffffff
#define YELLOW 0x00ffff00
#define RED 0x00ff4500
#define BLACK 0x00000000
int w, h; 



void uptime();
void screen_update(uint32_t color);
void splash_init();
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}

typedef struct segment {
  int st, len;
  bool status; // 1 for across and 0 for vertical;
  bool towards;
/*                                
  when status is 1: towards 1 means toward right, 0 means toward left; \
  when status is 0; toward 1 means toward down, 0 means toward up;
*/
  int another; // another oridinate;
  struct segment * front;
} seg;
// insert form front;
seg * beg ;
seg * end ;