#include <game.h>

#define KEYNAME(key) \
  [AM_KEY_##key] = #key,
static const char *key_names[] = {
  AM_KEYS(KEYNAME)
};

/*
typedef struct segment {
  int st, len;
  bool status;
  struct segment * next;
} seg;
*/
void game_progress();
void logic_for_end();
void kbd_process(const char *keyword);
void new_seg_create(bool status, bool towards);
void dead();
extern int w, h;
int next_frame = 1;
void uptime(){
  while (io_read(AM_TIMER_UPTIME).us < (1000000 / FPS) * next_frame);
  printf("It's going to change!\n");
  screen_update(BLACK);
  game_progress();
  screen_update(WHITE);
  next_frame ++;
}
 



void game_progress() {
    
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE };
  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");
    kbd_process(key_names[event.keycode]);
    }
  else if (event.keycode == AM_KEY_NONE) {
    // judge :
    if((beg -> towards == 0) && beg -> st - SIDE * ((beg -> len) + 1) < 0) dead(2);
    else if (beg -> towards == 1){
      if(beg -> status && beg -> st + (SIDE * ((beg -> len) + 1)) > w)
      {printf("Now ed is %d, w is %d\n", beg->st+(SIDE*((beg->len)+1)), w); 
        printf("The length is %d\n", beg -> len);
        printf("The start of segment is %d\n", beg -> st);
        printf("Increment is %d\n", SIDE * (beg -> len + 1));
      dead(3);}
      if(!beg -> status && beg -> st + SIDE * ((beg -> len) + 1) > h) dead(4);
    } 
    // judge end
    if(beg == end){
      if(beg -> towards)beg -> st += SIDE;
      else beg -> st -= SIDE;
    }
    else {
      beg -> len += 1;
      logic_for_end();
    }
  }
}



void kbd_process(const char * keyword){
    if (!strcmp(keyword, "ESCAPE")) halt(1);
    else if (!strcmp(keyword, "W")){ 
      if(beg -> status == 1) new_seg_create(0, 0);}
    else if (!strcmp(keyword, "S")){
      if(beg -> status == 1) new_seg_create(0, 1);}    
    else if (!strcmp(keyword, "D")){
      if(beg -> status == 0) new_seg_create(1, 1);}
    else if (!strcmp(keyword, "A")){
      if(beg -> status == 0) new_seg_create(1, 0);
    }
}


void new_seg_create(bool status, bool towards){
      if(!towards)
        {if(beg -> another - SIDE < 0) dead(5); }
      else {
        if(status && beg -> another + SIDE > w) dead(6);
        else if((!status) && beg -> another + SIDE > h) dead(7);
      } 
        seg * newseg = malloc(sizeof(seg));
        beg -> front = newseg;
        newseg -> st = beg -> another;
        newseg -> len = 1;
        newseg -> status =  status;
        newseg -> towards = towards;
        if(beg -> towards){
        newseg -> another = beg -> st + beg -> len * SIDE;
        }
        else{newseg -> another = beg -> st - beg -> len * SIDE;}
        newseg -> front = NULL;
        beg = newseg;
        // create a new segment of snake's body;
       logic_for_end();
        // deal with the motion logic of the last segment of the snake;
  
}

void logic_for_end() {
        end -> len --;
        if(end -> len <= 0) end = end -> front;
        else{
            if(end -> towards == 1) end -> st += SIDE;
            else end -> st -= SIDE;
          } 
}

void dead(int x){
  printf("YOU ARE DEAD!\n");
  halt(x);
}