#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>
#include <domain.h>

#define PI 3.14159265359

float deg2rad(float deg) {
    return deg * PI / 180.0;
}

void process(float tx, float ax, float ty, float ay, char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    pnm imd = pnm_new(cols, rows, PnmRawPpm);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float x = j + ax * sinf(2*PI * i / tx);
            float y = i + ay * sinf(2*PI * j / ty);
            for (int c = 0; c < 3; c++) {
                pnm_set_component(imd, i, j, c, bilinear_interpolation(x, y, ims, c));
            }
        }
    }


    pnm_save(imd, PnmRawPpm, imd_name);
    pnm_free(ims);
    pnm_free(imd);
}

void usage(char* s) {
    fprintf(stderr,"%s <tx> <ax> <ty> <ay> <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 6
int main(int argc, char* argv[]) {
    if(argc != PARAM+1)
        usage(argv[0]);

    process(atof(argv[1]), atof(argv[2]), atof(argv[3]), atof(argv[4]), argv[5], argv[6]);

    return EXIT_SUCCESS;
}
