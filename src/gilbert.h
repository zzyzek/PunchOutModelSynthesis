#ifndef GILBERT_H
#define GILBERT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int gilbert_xy2d(int x, int y, int w, int h);
int gilbert_xyz2d(int x, int y, int z, int width, int height, int depth);

int gilbert_xyz2d(int x, int y, int z, int width, int height, int depth);
int gilbert_d2xyz(int *x, int *y, int *z, int idx, int width, int height, int depth);

#endif
