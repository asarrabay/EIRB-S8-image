#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <bcl.h>

#include "domain.h"

#define PI 3.14159265359

void process(char dir, int teta, char * ims_name, char * imd_name) {
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);

    float t = tanf(teta * PI / 180);

    pnm imd;
    unsigned short * data;
    size_t size;

    switch (dir) {
        case 'h':
        /*
        (x', y') = (x + y*tan(teta), y)
        (x, y) = (x' - y'*tan(teta), y')
        */
        imd = pnm_new(cols + rows * fabs(t), rows, PnmRawPpm);
        data = pnm_get_image(imd);
        size = rows * (cols + rows * fabs(t)) * 3;
        while(size--) *data++ = 0;

        for (int i = 0; i < rows - 1; i++) {
            for (int j = 0; j < (cols + rows * fabs(t)) - 1; j++) {
                for (int k = 0; k < 3; k++) {
                    if (t >= 0) {
                        pnm_set_component(imd, i, j, k, bilinear_interpolation(j - i * t, i, ims, k));
                    }
                    if (t < 0) {
                        pnm_set_component(imd, i, j, k, bilinear_interpolation((j - i * t) - (rows * fabs(t)), i, ims, k));
                    }
                }
            }
        }
        break;
        case 'v':
        /*
        (x', y') = (x, y + x*tan(teta))
        (x, y) = (x', y' - x'*tan(teta))
        */
        imd = pnm_new(cols, rows + cols * fabs(t), PnmRawPpm);
        data = pnm_get_image(imd);
        size = (rows + cols * fabs(t)) * cols * 3;
        while(size--) *data++ = 0;

        for (int i = 0; i < (rows + cols * fabs(t)) - 1; i++) {
            for (int j = 0; j < cols - 1; j++) {
                for (int k = 0; k < 3; k++) {
                    if (t >= 0) {
                        pnm_set_component(imd, i, j, k, bilinear_interpolation(j, i - j * t, ims, k));
                    }
                    if (t < 0) {
                        pnm_set_component(imd, i, j, k, bilinear_interpolation(j, (i - j * t) - (cols * fabs(t)), ims, k));
                    }
                }
            }
        }
        break;
        default:
        printf("First argument is not a valid direction\n");
        exit(EXIT_FAILURE);
    }


    pnm_save(imd, PnmRawPpm, imd_name);

    pnm_free(imd);
    pnm_free(ims);
}

static void usage(char* s){
    fprintf(stderr,"%s <dir>{h ,v} <angle> <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 4
int main(int argc, char* argv[]){
    if(argc != PARAM+1)
    usage(argv[0]);

    char * dir = argv[1];
    int teta = atoi(argv[2]);
    char * ims_name = argv[3];
    char * imd_name = argv[4];

    process(*dir, teta, ims_name, imd_name);
    /*
    just for the test, completely useless for the shear program
    for(int channel=0; channel<3; channel++){
    unsigned short v = bilinear_interpolation(0.0, 0.0, NULL, channel);
    (void)v;
}
*/
return EXIT_SUCCESS;
}
