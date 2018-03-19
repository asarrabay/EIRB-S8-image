#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>

void copy(pnm imd, unsigned short v, int a, int i, int j, int k){
    for (int y = 0; y < a; y++) {
        for (int x = 0; x < a; x++) {
            pnm_set_component(imd, a*i + y, a*j + x, k, v);
        }
    }
}

void process(int a, char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    pnm imd = pnm_new(cols * a, rows * a, PnmRawPpm);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            for (int k = 0; k < 3; k++) {
                unsigned short v = pnm_get_component(ims, i, j, k);
                copy(imd, v, a, i, j, k);
            }
        }
    }

    pnm_save(imd, PnmRawPpm, imd_name);
    pnm_free(ims);
    pnm_free(imd);
}

void usage(char* s) {
    fprintf(stderr,"%s <factor> <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char* argv[]) {
    if(argc != PARAM+1)
    usage(argv[0]);

    process(atoi(argv[1]), argv[2], argv[3]);

    return EXIT_SUCCESS;
}
