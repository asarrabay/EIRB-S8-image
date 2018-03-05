/**
* @file color-transfert
* @brief transfert color from source image to target image.
*        Method from Reinhard et al. :
*        Erik Reinhard, Michael Ashikhmin, Bruce Gooch and Peter Shirley,
*        'Color Transfer between Images', IEEE CGA special issue on
*        Applied Perception, Vol 21, No 5, pp 34-41, September - October 2001
*/

//source from wget http://www.labri.fr/perso/ta/tmp-files/color-transfer.tar.gz

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bcl.h>

#define D 3
#define NB_SAMPLES_EXPECTED_H 12
#define NB_SAMPLES_EXPECTED_W 17
#define NB_SAMPLES_EXPECTED NB_SAMPLES_EXPECTED_H*NB_SAMPLES_EXPECTED_W


struct sample {
    float l;
    float a;
    float b;
    float standard_deviation;
};

void product_matrix_vector(float matrix[D][D], float vector[D], float res[D]);
void print_matrix(float matrix[D][D]);
void print_vector(float vector[D]);
void transform_buf(pnm p, float *buf, float matrix[D][D]);
float center_float_value(float min, float max, float value);
void fill_buffer_from_img(pnm p, float *buf);
void fill_img_from_buffer(pnm p, float *buf);
void apply_log_buffer(float *buf, int size);//size should be w * h
void apply_exp_buffer(float *buf, int size);//size should be w * h
void apply_add_value_buffer(float *buf, float value, int canal, int size);
void apply_mul_value_buffer(float *buf, float value, int canal, int size);
float get_max_value(float *buf, int l);
float get_min_value(float *buf, int l);
void normalize(int max, int min, float *buf, int l);
float get_neighborhood_standard_deviation(pnm img_src, float *data,int i,int j);
struct sample matching_sample(pnm img_t, float *data_t, int i, int j,
                              struct sample **samples, int nb_samples,
                              float sdl_max, float l_max);

float RGB2LMS[D][D] = {
    {0.3811, 0.5783, 0.0402},
    {0.1967, 0.7244, 0.0782},
    {0.0241, 0.1288, 0.8444}
};

float LMS2RGB[D][D] = {
    {4.4679, -3.5873, 0.1193},
    {-1.2186, 2.3809, -0.1624},
    {0.0497, -0.2439, 1.2045}
};

float LMS2LAB[D][D] = {
    {0.5773, 0.5773, 0.5773},
    {0.4082, 0.4082, -0.8164},
    {0.7071, -0.7071, 0}
};

float LAB2LMS[D][D] = {
    {0.5773, 0.4082, 0.7071},
    {0.5773, 0.4082, -0.7071},
    {0.5773, -0.8164, 0}
};

void product_matrix_vector(float matrix[D][D], float vector[D], float res[D]){
    for(int i = 0; i < D; i++){
        res[i] = 0;
        for(int k = 0; k < D; k++){
            res[i] += matrix[i][k] * vector[k];
        }
    }
}

void print_matrix(float matrix[D][D]){
    printf("(");
    for(int i = 0; i < D; i++){
        for(int j = 0; j < D; j++){
            printf("%f", matrix[i][j]);
            if(j < D - 1)
            printf(", ");
        }
        if(i < D - 1)
        printf("\n");
    }
    printf(")\n");
}

void print_vector(float vector[D]){
    printf("(");
    for(int i = 0; i < D; i++){
        printf("%f", vector[i]);
        if(i < 2)
        printf(", ");
    }
    printf(")\n");
}

float center_float_value(float min, float max, float value){
    if(value < min)
    value = min;
    else if(value > max)
    value = max;

    return value;
}



void fill_buffer_from_img(pnm p, float *buf){
    int w = pnm_get_width(p);
    int h = pnm_get_height(p);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            int index = pnm_offset(p, i, j);
            for(int c = 0; c < D; c++){
                buf[index + c] = pnm_get_component(p, i, j, c);
            }
        }
    }
}

void fill_img_from_buffer(pnm p, float *buf){
    int w = pnm_get_width(p);
    int h = pnm_get_height(p);

    normalize(255, 0, buf, w * h * 3);

    for(int i = 0; i < w; i++){
        for(int j = 0; j < h; j++){
            int index = pnm_offset(p, j, i);
            for(int c = 0; c < D; c++){
                pnm_set_component(p, j, i, c, center_float_value(0, 255, buf[index + c]));
            }
        }
    }
}

void apply_log_buffer(float *buf, int size){
    for(int i = 0; i < size * 3; i++){
        buf[i] = log10(buf[i] + 1);
    }
}

void apply_exp_buffer(float *buf, int size){
    for(int i = 0; i < size * 3; i++){
        buf[i] = powf(10, buf[i]) - 1;
    }
}



void apply_add_value_buffer(float *buf, float value, int canal, int size){
    for(int i = 0; i < size; i++){
        buf[i * 3 + canal] += value;
    }
}

void apply_mul_value_buffer(float *buf, float value, int canal, int size){
    for(int i = 0; i < size; i++){
        buf[i * 3 + canal] *= value;
    }
}

void transform_buf(pnm p, float *buf, float matrix[D][D]){
    int w = pnm_get_width(p);
    int h = pnm_get_height(p);
    float tmp_src[D];
    float tmp_dst[D];

    for(int i = 0; i < w; i++){
        for(int j = 0; j < h; j++){
            int index = pnm_offset(p, j, i);
            for(int c = 0; c < D; c++){
                tmp_src[c] = buf[index + c];
            }
            product_matrix_vector(matrix, tmp_src, tmp_dst);
            for(int c = 0; c < D; c++){
                buf[index + c] = tmp_dst[c];
            }
        }
    }
}

float get_max_value(float *buf, int l){
    float m = 0;
    for(int i = 0; i < l; i++){
        if(m < buf[i])
        m = buf[i];
    }

    return m;
}

float get_min_value(float *buf, int l){
    float m = 255;
    for(int i = 0; i < l; i++){
        if(m > buf[i])
        m = buf[i];
    }

    return m;
}

void normalize(int max, int min, float *buf, int l){
    float max_buf = get_max_value(buf, l);
    float min_buf = get_min_value(buf, l);

    if(max_buf == min_buf){
        printf("Error : from img : max = min\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < l; i++){
        buf[i] = ((double)max - min) / (max_buf - min_buf) * buf[i] +
        ((double)min * max_buf - max * min_buf) / (max_buf - min_buf);
    }
}


float get_neighborhood_standard_deviation(pnm img_src, float *data,int i,int j){
    float mean = 0;
    int n = 0;
    for (int ii = i-2; ii <= i+2; ii++) {
        for (int jj = j-2; jj <= j+2; jj++) {
            if (ii >= 0 && ii < pnm_get_height(img_src) && jj >= 0 && jj < pnm_get_width(img_src)){
                mean += data[pnm_offset(img_src, ii, jj)];
                n++;
            }

        }
    }
    mean /= n;
    float sd = 0;
    for (int ii = i-2; ii <= i+2; ii++) {
        for (int jj = j-2; jj < j+2; jj++) {
            if (ii >= 0 && ii < pnm_get_height(img_src) && jj >= 0 && jj < pnm_get_width(img_src)){
                sd += powf(data[pnm_offset(img_src, ii, jj)]-mean, 2);
            }
        }
    }
    return sqrtf(sd/n);
}

struct sample matching_sample(pnm img_t, float *data_t, int i, int j, struct sample **samples, int nb_samples, float sdl_max, float l_max){
    float dist_min;
    int i_min;
    float l = data_t[pnm_offset(img_t, i, j)];
    float sdl_t = get_neighborhood_standard_deviation(img_t, data_t, i, j);
    for (int ii = 0; ii < nb_samples; ii++) {
        //compute distance in a normalized vector space (luminance,neighborhood_standard_deviation)
        float d = pow(l - samples[ii]->l, 2) + pow((sdl_t - samples[ii]->standard_deviation) * l_max/sdl_max, 2);
        if (ii == 0) {
            dist_min = d;
            i_min = ii;
        } else {
            if (dist_min > d) {
                dist_min = d;
                i_min = ii;
            }
        }
    }
    return *samples[i_min];
}

void
process(char *ims, char *imt, char* imd){
    pnm img_src = pnm_load(ims);
    pnm img_t = pnm_load(imt);

    int w_src = pnm_get_width(img_src);
    int h_src = pnm_get_height(img_src);

    int w_t = pnm_get_width(img_t);
    int h_t = pnm_get_height(img_t);

    pnm img_dst = pnm_new(w_t, h_t, PnmRawPpm);
    float *buf_src = malloc(w_src * h_src * 3 * sizeof(float));
    float *buf_t = malloc(w_t * h_t  * 3 * sizeof(float));

    if(!buf_src || !buf_t){
        fprintf(stderr, "Error : cannot allocate buffer memory\n");
        exit(EXIT_FAILURE);
    }


    fill_buffer_from_img(img_src, buf_src);
    fill_buffer_from_img(img_t, buf_t);

    //transforming src rgb buf to lab buf
    transform_buf(img_src, buf_src, RGB2LMS);
    apply_log_buffer(buf_src, w_src * h_src);
    transform_buf(img_src, buf_src, LMS2LAB);

    //transforming t rgb buf to lab buf
    transform_buf(img_t, buf_t, RGB2LMS);
    apply_log_buffer(buf_t, w_t * h_t);
    transform_buf(img_t, buf_t, LMS2LAB);

    /****************************
    *  1-                      *
    *  Jittered sampling       *
    * Choose NB_SAMPLES_EXPECTED pixels in ims
    * and compute stat *
    ****************************/

    struct sample **samples = malloc(sizeof(struct sample*) * NB_SAMPLES_EXPECTED);
    for (int i = 0; i < NB_SAMPLES_EXPECTED; i++) {
        samples[i] = malloc(sizeof(struct sample));
    }
    int p = 0;
    float sdl_max = -1;
    float l_max = -1;


    float r = (float)h_src / w_src;
    //compute the real number of samples
    int nb_samples_w = sqrt(NB_SAMPLES_EXPECTED/r);
    int nb_samples_h = r * nb_samples_w;
    int nb_samples = nb_samples_h * nb_samples_w;
    //division of the image in nb_sample rectangles
    int samples_w = w_src/nb_samples_w;
    int samples_h = h_src/nb_samples_h;
    srand(time(NULL));
    int i_rand, j_rand;;
    for (int i = 0; i < nb_samples_h; i++) {
        for (int j = 0; j < nb_samples_w; j++) {
            //random selection in a rectangle
            i_rand = rand()%samples_h;
            j_rand = rand()%samples_w;
            int index = pnm_offset(img_src, i*samples_h + i_rand, j*samples_w + j_rand);
            //filling the sample
            samples[p]->l = buf_src[index];
            samples[p]->a = buf_src[index+1];
            samples[p]->b = buf_src[index+2];
            samples[p]->standard_deviation = get_neighborhood_standard_deviation(img_src, buf_src, i*samples_h, j*samples_w);
            //compute neighbourhood standard_deviation max and luminance max for later (normalization in matching_sample)
            if (sdl_max < samples[p]->standard_deviation) {
                sdl_max = samples[p]->standard_deviation;
            }
            if (l_max < samples[p]->l) {
                l_max = samples[p]->l;
            }
            p++;
        }
    }

    /*
    * 2-
    * matching each imt's pixels
    * with the best  sample
    * and transfert chromatic ims to imd
    */

    for (int i = 0; i < h_t; i++) {
        for (int j = 0; j < w_t; j++) {
            int index = pnm_offset(img_t, i, j);
            struct sample best_sample = matching_sample(img_t, buf_t, i, j, samples, nb_samples, sdl_max, l_max);
            buf_t[index+1] = best_sample.a;
            buf_t[index+2] = best_sample.b;
        }
    }


    transform_buf(img_t, buf_t, LAB2LMS);
    apply_exp_buffer(buf_t, w_t * h_t);
    transform_buf(img_t, buf_t, LMS2RGB);

    fill_img_from_buffer(img_dst, buf_t);

    pnm_save(img_dst, PnmRawPpm, imd);

    free(buf_src);
    free(buf_t);
    pnm_free(img_src);
    pnm_free(img_t);
    pnm_free(img_dst);
}

void
usage (char *s){
    fprintf(stderr, "Usage: %s <ims> <imt> <imd> \n", s);
    exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char *argv[]){
    if (argc != PARAM+1) usage(argv[0]);
    process(argv[1], argv[2], argv[3]);
    return EXIT_SUCCESS;
}
