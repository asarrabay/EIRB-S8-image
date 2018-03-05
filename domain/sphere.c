#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <bcl.h>

#include "domain.h"

#define PI 3.14159265359


pnm image(float Cx, float Cy, float ray, pnm ims, int rows, int cols) {
    pnm imd = pnm_new(cols, rows, PnmRawPpm);
    unsigned short * data = pnm_get_image(imd);
    size_t size = rows * cols * 3;
    while(size--) *data++ = 0;

    float d;
    float alpha;
    float x;
    float y;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /*
            x' = ray * arcsin(d / ray) * 2 / PI + Cx
            y' = ray * arcsin(d / ray) * 2 / PI + Cy
            */
            d = sqrtf(i * i + j * j);
            alpha = ray * asinf(d / ray) * 2 / PI;
            x = alpha + Cx;
            y = alpha + Cy;
            for (int k = 0; k < 3; k++) {
                pnm_set_component(imd, i, j, k, bilinear_interpolation(x, y, ims, k));
            }
        }
    }
    return imd;
}

void process(char * ims_name, char * imd_name) {
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);

    float Cx = cols / 2;
    float Cy = rows / 2;
    float ray = 0;
    if (cols >= rows) {
        ray = rows / 2;
    }
    if (cols < rows) {
        ray = cols / 2;
    }

    pnm imd = image(Cx, Cy, ray, ims, rows, cols);


    pnm_save(imd, PnmRawPpm, imd_name);

    pnm_free(imd);
    pnm_free(ims);
}

void
usage(char* s)
{
    fprintf(stderr,"%s <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 2
int
main(int argc, char* argv[])
{
    if(argc != PARAM+1)
    usage(argv[0]);

    char * ims_name = argv[1];
    char * imd_name = argv[2];

    process(ims_name, imd_name);

    return EXIT_SUCCESS;
}
