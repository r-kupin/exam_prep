#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define KO 1

const float e0 = 0.0000f;
const float e1 = 1.0001f;

float myfabs(float f) {
  if (f < 0)
    return f * -1;
  return f;
}

int process(FILE *f) {
  int w, h;
  char b;

  if (fscanf(f, "%d %d %c\n", &w, &h, &b) != 3)
    return KO;
  if (w <= 0 || w > 300 || h <= 0 || h > 300) 
    return KO;
  
  char plot[h][w];
  memset(plot, b, h * w);
  
  int status;
  float Xtl, Ytl, Xbr, Ybr, W, H;
  char Rr, c;
  while ((status = fscanf(f, "%c %f %f %f %f %c\n", &Rr, &Xtl, &Ytl, &W, &H, &c)) != -1) {
    if (status != 6 || !(Rr == 'r' || Rr == 'R') || W <= e0 || H <= e0)
      return KO;
    Xbr = Xtl + W;
    Ybr = Ytl + H;
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        if ((myfabs(x - Xbr) < e1 || myfabs(x - Xtl) < e1 || myfabs(y - Ybr) < e1 || myfabs(y - Ytl) < e1) &&
            (Xtl <= x && x <= Xbr && Ytl <= y && y <= Ybr)) {
          plot[y][x] = c;
        }
        if (Rr == 'R' && Xtl <= x && x <= Xbr && Ytl <= y && y <= Ybr)
          plot[y][x] = c;
      }
    }
  }
  for (int i = 0; i < h; i++ ) {
    write(1, plot[i], w);
    write(1, "\n", 1);
  }
  return OK;
}

int main(int ac, char **av) {
  if (ac != 2) {
    write(1, "Error: argument\n", 17);
    return KO;
  }
  FILE *f = fopen(av[1], "r");
  if (!f) {
    write(1, "Error: Operation file corrupted\n", 33);
    return KO;
  }
  if (process(f) == KO) {
    write(1, "Error: Operation file corrupted\n", 33);
    return KO;
  }
  return OK;
}
