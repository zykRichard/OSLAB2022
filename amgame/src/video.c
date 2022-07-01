#include <game.h>

/*
#define WHITE 0xffffffff
#define YELLOW 0x00ffff00
#define RED 0x00ff4500
*/
extern int w, h;
static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}


static void gpu_am_init() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

void splash_init() {
  //srand(time(0));
  gpu_am_init();
  int x = w / 2;
  int y = h / 2;
  draw_tile(x, y, LEN * SIDE, SIDE, WHITE);
  /*seg seg_init = {
    .st = x, .len = LEN, .status = 1, .towards = 1, .another = y, .front = NULL,
  } ;*/
  beg = malloc(sizeof(seg));
  end = malloc(sizeof(seg));
  beg -> st = x;
  beg -> len = LEN;
  beg -> status = 1;
  beg -> towards = 1;
  beg -> another = y;
  beg -> front = NULL;
  end = beg;
}

static void draw_seg(seg * pen, uint32_t color){
  int x, y, w, h;
  if(pen -> status){
    if(pen -> towards){
      x = pen -> st;
      y = pen -> another;
      w = pen -> len * SIDE;
      h = SIDE;
    }
    else {
      x = pen -> st - pen -> len * SIDE;
      y = pen -> another;
      w = pen -> len * SIDE;
      h = SIDE;
    } 
 }
 else {
   if(pen -> towards){
     x = pen -> another;
     y = pen -> st;
     w = SIDE;
     h = pen -> len * SIDE;
   }
   else {
     x = pen -> another;
     y = pen -> st - pen -> len * SIDE;
     w = SIDE;
     h = pen -> len * SIDE;
   }
 }
  printf("Draw snakes at position (%d, %d) with width %d and length %d \n", x, y, w, h);
  draw_tile(x, y, w, h, color);
  return;
}

void screen_update(uint32_t color){
  printf("GOING to the draw function\n");
  seg * pen = end;
  while (pen != NULL){
    printf("OK to draw\n");
    draw_seg(pen, color);
    pen = pen -> front;
  } 
  return ;
}