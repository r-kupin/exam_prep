#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define OK 0
#define KO 1

const float e0 = 0.0000f;
const float e1 = 1.0000f;

float gd(float Xa, float Ya, float Xb, float Yb) {
  return sqrtf((Xa - Xb) * (Xa - Xb) + (Ya - Yb) * (Ya - Yb));
}

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
  float X, Y, R;
  char Cc, c;

  while ((status = (fscanf(f, "%c %f %f %f %c\n", &Cc, &X, &Y, &R, &c))) != -1) {
    if (status != 5 || !(Cc == 'C' || Cc == 'c') || R <= e0)
      return KO;
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        if ( Cc == 'c' && myfabs(gd(x, y, X, Y) - R) < e1 && gd(x, y, X, Y) <= R)
          plot[y][x] = c;
        if (Cc == 'C' && (gd(x, y, X, Y) <= R))
          plot[y][x] = c;
      }
    }
  }
  for (int i = 0; i < h; i++) {
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
