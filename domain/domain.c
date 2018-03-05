#include <math.h>
#include <stdio.h>
#include <domain.h>

unsigned short bilinear_interpolation(float x, float y, pnm ims, int c) {
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    int i = floor(y);
    int j = floor(x);
    float dx = x - j;
    float dy = y - i;
    float interpolation = 0;
    if (i >= 0 && j >= 0 && i < rows && j < cols) {
        interpolation = interpolation + (1-dx) * (1-dy) * pnm_get_component(ims, i, j, c);
    }
    if (i >= 0 && j+1 >= 0 && i < rows && j+1 < cols) {
        interpolation = interpolation + dx * (1-dy) * pnm_get_component(ims, i, j+1, c);
    }
    if (i+1 >= 0 && j >= 0 && i+1 < rows && j < cols) {
        interpolation = interpolation + (1-dx) * dy * pnm_get_component(ims, i+1, j, c);
    }
    if (i+1 >= 0 && j+1 >= 0 && i+1 < rows && j+1 < cols) {
        interpolation = interpolation + dx * dy * pnm_get_component(ims, i+1, j+1, c);
    }
    return (unsigned short)interpolation;
}
