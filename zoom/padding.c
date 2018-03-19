#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>

#include <fft.h>


void process(int a, char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    pnm imd = pnm_new(cols * a, rows * a, PnmRawPpm);


    unsigned short *data;
    fftw_complex *freq_repr;
    fftw_complex *new_freq_repr = malloc(sizeof(fftw_complex) * rows * cols * a * a);

    unsigned short *new_data;
    int i_offset = (a-1)*rows/2;
    int j_offset = (a-1)*cols/2;

    for (int k = 0; k < 3; k++) {
        data = pnm_get_channel(ims, NULL, k);
        freq_repr = forward(rows, cols, data);

        for (int i = 0; i < rows * a; i++) {
            for (int j = 0; j < cols * a; j++) {
                if (i < i_offset || j < j_offset || i >= rows * a - i_offset || j >= cols * a - j_offset) {
                    new_freq_repr[i*cols*a + j] = 0;
                } else {
                    new_freq_repr[i*cols*a + j] = freq_repr[(i - i_offset)*cols + (j - j_offset)] *a*a;
                }
            }
        }

        new_data = backward(rows * a, cols * a, new_freq_repr);

        pnm_set_channel(imd, new_data, k);
        free(data);
        free(new_data);
        free(freq_repr);
    }

    free(new_freq_repr);
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
