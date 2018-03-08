#include <float.h>
#include <stdlib.h>
#include <math.h>

#include <fft.h>

#define SHIFT(val, i, rows, cols) int x = i%cols;\
                        int y = (i - x) / rows;\
                        if ((x+y) % 2 != 0) {\
                            val = -1 * val;\
                        }

fftw_complex * forward(int rows, int cols, unsigned short* g_img) {
    int size = rows * cols;
    fftw_complex *img_c = malloc(sizeof(fftw_complex) * size);
    for (int i = 0; i < size; i++) {
        img_c[i] = g_img[3*i]; // induit : + 0*I
        SHIFT(img_c[i], i, rows, cols)
    }
    fftw_complex *res_c = malloc(sizeof(fftw_complex) * size);
    fftw_plan plan = fftw_plan_dft_2d(rows, cols, img_c, res_c, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    free(img_c);
    return res_c;
}

unsigned short * backward(int rows, int cols, fftw_complex* freq_repr) {
    int size = rows * cols;
    fftw_complex *res_c = malloc(sizeof(fftw_complex) * size);
    fftw_plan plan = fftw_plan_dft_2d(rows, cols, freq_repr, res_c, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    unsigned short *img = malloc(sizeof(unsigned short) * size);
    for (int i = 0; i < size; i++) {
        double tmp = creal(res_c[i]) / size;
        SHIFT(tmp, i, rows, cols)
        if (tmp < 0.0) {
            tmp = 0;
        } else if(tmp > 255){
            tmp = 255;
        }
        img[i] = tmp;
    }
    free(res_c);
    return img;
}

void freq2spectra(int rows, int cols, fftw_complex* freq_repr, float* as, float* ps) {
    int size = cols * rows;
    for (int i = 0; i < size; i++) {
        as[i] = sqrt(pow(creal(freq_repr[i]), 2) + pow(cimag(freq_repr[i]), 2));
        // ps[i] = atan(cimag(freq_repr[i]) / creal(freq_repr[i]));
        ps[i] = atan2(cimag(freq_repr[i]), creal(freq_repr[i]));
    }
}

void spectra2freq(int rows, int cols, float* as, float* ps, fftw_complex* freq_repr) {
    int size = rows * cols;
    for (int i = 0; i < size; i++) {
        freq_repr[i] = as[i] * cos(ps[i]) + I * as[i] * sin(ps[i]);
    }
}
