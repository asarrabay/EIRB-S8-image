#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>
#include <string.h>

typedef float(*filter_f)(float x);

float box(float x){
    if (x >= -0.5 && x < 0.5) {
        return 1;
    }
    return 0;
}
float tent(float x){
    float x_abs = fabsf(x);
    if( x_abs < 1){
        return 1 - x_abs;
    }
    return 0;
}
float bell(float x){
    float x_abs = fabsf(x);
    if (x_abs <= 0.5) {
        return -x*x + 0.75;
    } if (x_abs <= 1.5){
        float tmp = x_abs - 1.5;
        return 0.5*tmp*tmp;
    }
    return 0;
}

float mitch(float x){
    float x_abs = fabsf(x);
    if (x_abs < 1) {
        return 1.16666666667 * x_abs * x_abs * x_abs
            - 2 * x*x + 0.88888888888;
    } if (x_abs <= 2) {
        return -0.38888888888 * x_abs*x_abs*x_abs
            + 2 * x*x - 3.33333333333 * x_abs + 1.77777777778;
    }
    return 0;
}

pnm interpolation(filter_f filter, int wf, int a, int rows, int cols, pnm ims){
    int cols_p = cols * a;
    pnm imd = pnm_new(cols_p, rows, PnmRawPpm);
    for (int i = 0; i < rows; i++) {
        for (int j_p = 0; j_p < cols_p; j_p++) {
            float j = (float)j_p / a;
            int left = j - wf;
            if (left < 0) {
                left = 0;
            }
            int right = j + wf;
            if (right >= cols) {
                right = cols - 1;
            }
            for (int chnl = 0; chnl < 3; chnl++) {
                float s = 0;
                if (cols_p - j_p >= a + wf - 1) {
                    for (int k = left; k <= right; k++) {
                        s += pnm_get_component(ims, i, k, chnl) * filter(k-j);
                    }
                } else{
                    s = pnm_get_component(ims, i, cols-1, chnl);
                }
                if (s > 255) {
                    s = 255;
                } else if (s < 0) {
                    s = 0;
                }
                pnm_set_component(imd, i, j_p, chnl, s);
            }
        }
    }
    return imd;
}

pnm flip(pnm ims){
    int cols, rows;
    pnm imd;
    rows = pnm_get_height(ims);
    cols = pnm_get_width(ims);
    imd = pnm_new(rows, cols, PnmRawPpm);


    unsigned short *data_s = pnm_get_image(ims);
    unsigned short *data_d = pnm_get_image(imd);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int new_i = j;
            int new_j = i;
            int p = pnm_offset(ims, i, j);
            int q = pnm_offset(imd, new_i, new_j);
            for (size_t k = 0; k < 3; k++) {
                data_d[q+k] = data_s[p+k];
            }
        }

    }
    return imd;
}

void process(int a, char *filter_name, char *ims_name, char *imd_name){
    (void) filter_name;
    filter_f filter;
    int wf;
    if (strcmp(filter_name, "box") == 0) {
        filter = box;
        wf = 1;
    } else if (strcmp(filter_name, "tent") == 0) {
        filter = tent;
        wf = 2;
    } else if (strcmp(filter_name, "bell") == 0) {
        filter = bell;
        wf = 3;
    } else if (strcmp(filter_name, "mitch") == 0) {
        filter = mitch;
        wf = 4;
    }
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    pnm imd = interpolation(filter, wf, a, rows, cols, ims);
    imd = flip(imd);
    imd = interpolation(filter, wf, a, pnm_get_height(imd), rows, imd);
    imd = flip(imd);

    pnm_save(imd, PnmRawPpm, imd_name);
    pnm_free(ims);
    pnm_free(imd);
}

void usage(char* s) {
    fprintf(stderr,"%s <factor> <filter-name> <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 4
int main(int argc, char* argv[]) {
    if(argc != PARAM+1)
    usage(argv[0]);

    process(atoi(argv[1]), argv[2], argv[3], argv[4]);

    return EXIT_SUCCESS;
}
