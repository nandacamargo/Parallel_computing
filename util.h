#ifndef UTIL_H
#define UTIL_H

/*Pode ser que fique melhor se RGB tiverem as componentes
Rx, Ry, Bx, By, Gx, Gy*/
typedef struct {
  float R;
  float Rx;
  float Ry;
  float G;
  float B;
  float Bx;
  float By;
} Pixel;

float horizontal_component(float comp, float angle);
float vertical_component(float comp, float angle);
float transfer(float neighbor, float current);
int isBorder(int i, int j);
void cp(Pixel **M, Pixel **M2, int lines, int columns);

#endif
