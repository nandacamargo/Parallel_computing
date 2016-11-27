#ifndef UTIL_H
#define UTIL_H

/*Pode ser que fique melhor se RGB tiverem as componentes
Rx, Ry, Bx, By, Gx, Gy*/
typedef struct {
  double R;
  double Rx;
  double Ry;
  double G;
  double B;
  double Bx;
  double By;
  double ang;
} Pixel;

double horizontal_component(double comp, double angle);
double vertical_component(double comp, double angle);
double transfer(double neighbor, double current);
int isBorder(int i, int j);
void cp(Pixel **M, Pixel **M2, int lines, int columns);

#endif
