/**
* @file color-transfert
* @brief transfert color from source image to target image.
*        Method from Reinhard et al. :
*        Erik Reinhard, Michael Ashikhmin, Bruce Gooch and Peter Shirley,
*        'Color Transfer between Images', IEEE CGA special issue on
*        Applied Perception, Vol 21, No 5, pp 34-41, September - October 2001
*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <bcl.h>

#define D 3

static float RGB2LMS[D][D] = {
    {0.3811, 0.5783, 0.0402},
    {0.1967, 0.7244, 0.0782},
    {0.0241, 0.1288, 0.8444}
};

static float LMS2RGB[D][D] = {
    {4.4679, -3.5873, 0.1193},
    {-1.2186, 2.3809, -0.1624},
    {0.0497, -0.2439, 1.2045}
};

static float LMS2RAB[D][D] = {
    {1/sqrtf(3), 1/sqrtf(3), 1/sqrtf(3)},
    {1/sqrtf(6), 1/sqrtf(6), -2/sqrtf(6)},
    {1/sqrtf(2), -1/sqrtf(2), 0}
};

static float RAB2LMS[D][D] = {
    {sqrtf(3)/3, sqrtf(6)/6, sqrtf(2)/2},
    {sqrtf(3)/3, sqrtf(6)/6, -sqrtf(2)/2},
    {sqrtf(3)/3, -sqrtf(6)/3, 0}
};


void normalize(int min, int max, float *data, int size){
    float max_I = *data;
    float min_I = *data;

    for (int i = 0; i < size; i++) {
        if (data[i] < min_I) {
            min_I = data[i];
        }
        if (data[i] > max_I) {
            max_I = data[i];
        }
    }


    for(int i = 0; i < size; i++){
        data[i] = ((double)max - min) / (max_I - min_I) * data[i] +
        ((double)min * max_I - max * min_I) / (max_I - min_I);
    }
}



static void mean(float * data, int size, float res[D]) {
    float sum[D];
    int s = size / D;

    for (int k = 0; k < D; k++) {
        sum[k] = 0;
        for (int i = 0; i < size; i += D) {
            sum[k] += *(data+i+k);
        }
    }

    for (int i = 0; i < D; i++) {
        res[i] = sum[i] / s;
    }
}


static void standard_deviation(float * data, int size, float m[D], float res[D]) {
    int s = size / D;
    for (int i = 0; i < D; i++) {
        float sum = 0;
        for(int j = 0; j < size; j++){
            sum += powf(data[j * 3 + i] - m[i], 2);
        }
        res[i] =  sqrtf(sum/s);
    }
}


/**
* produit matrice vecteur dans res
*/
static void prod_mat_vect(float mat[D][D], float vect[D], float res[D]){
    for (int i = 0; i < D; i++) {
        res[i] = 0;
        for (int j = 0; j < D; j++) {
            res[i] += mat[i][j] * vect[j];
        }
    }
}

void vect_log(float vect[D]) {
    for (int i = 0; i < D; i++) {
        vect[i] = log10(vect[i] + 1);
    }
}

void vect_power(float vect[D]) {
    for (int i = 0; i < D; i++) {
        vect[i] = powf(10, vect[i]) - 1;
    }
}

/**
* convertion usigned int --> float
*/
static void data_ustof(unsigned short *data_src, float *data_dst, int size){
    for (int i = 0; i < size; i++) {
        data_dst[i] = data_src[i];
    }
}

/**
* convertion float --> usigned int
*/
static void data_ftous(float *data_src, unsigned short *data_dst, int size){
    for (int i = 0; i < size; i++) {

        if (data_src[i] <= 0.0) {
            data_dst[i] = 0;
        } else if (data_src[i] >= 255.0) {
            data_dst[i] = 255;
        } else {
            data_dst[i] = floor(data_src[i]);
        }
    }
}

static void process(char * ims_name, char * imt_name, char * imd_name){
    pnm ims = pnm_load(ims_name);
    pnm imt = pnm_load(imt_name);

    int cols_ims = pnm_get_width(ims);
    int rows_ims = pnm_get_height(ims);
    int cols_imt = pnm_get_width(imt);
    int rows_imt = pnm_get_height(imt);

    pnm imd = pnm_new(cols_imt, rows_imt, PnmRawPpm);


    int size_ims = rows_ims * cols_ims * D;
    int size_imt = rows_imt * cols_imt * D;
    unsigned short *data_ims = pnm_get_image(ims);
    unsigned short *data_imt = pnm_get_image(imt);
    unsigned short *data_imd = pnm_get_image(imd);

    float *fdata_ims = malloc(sizeof(float) * size_ims);
    float *fdata_imt = malloc(sizeof(float) * size_imt);
    float *fdata_imd = malloc(sizeof(float) * size_imt);
    float *fdata_ims_lms = malloc(sizeof(float) * size_ims);
    float *fdata_imt_lms = malloc(sizeof(float) * size_imt);
    float *fdata_imd_lms = malloc(sizeof(float) * size_imt);
    float *fdata_ims_rab = malloc(sizeof(float) * size_ims);
    float *fdata_imt_rab = malloc(sizeof(float) * size_imt);
    float *fdata_imd_rab = malloc(sizeof(float) * size_imt);


    /*
    step 1 : RGB --> ραβ
    step 2 : stats
    step 3 : ραβ -> RGB
    */

    /* step 1 */
    data_ustof(data_ims, fdata_ims, size_ims);
    data_ustof(data_imt, fdata_imt, size_imt);

    for (int i = 0; i < size_ims; i += D) {
        prod_mat_vect(RGB2LMS, fdata_ims+i, fdata_ims_lms+i);
        vect_log(fdata_ims_lms+i);
        prod_mat_vect(LMS2RAB, fdata_ims_lms+i, fdata_ims_rab+i);
    }

    for (int i = 0; i < size_imt; i += D) {
        prod_mat_vect(RGB2LMS, fdata_imt+i, fdata_imt_lms+i);
        vect_log(fdata_imt_lms+i);
        prod_mat_vect(LMS2RAB, fdata_imt_lms+i, fdata_imt_rab+i);
    }

    /* step 2 */
    float mean_ims[D];
    float mean_imt[D];
    float sd_ims[D];
    float sd_imt[D];

    mean(fdata_ims_rab, size_ims, mean_ims);
    mean(fdata_imt_rab, size_imt, mean_imt);
    standard_deviation(fdata_ims_rab, size_ims, mean_ims, sd_ims);
    standard_deviation(fdata_imt_rab, size_imt, mean_imt, sd_imt);

    for (int i = 0; i < size_imt; i += D) {
        for (int k = 0; k < D; k++) {
            *(fdata_imd_rab+i+k) = ((sd_imt[k] / sd_ims[k])
            * (*(fdata_imt_rab+i+k) - mean_imt[k]))
            + mean_ims[k];
        }
    }



    /* step 3 */
    for (int i = 0; i < size_imt; i+=D) {
        prod_mat_vect(RAB2LMS, fdata_imd_rab+i, fdata_imd_lms+i);
        vect_power(fdata_imd_lms+i);
        prod_mat_vect(LMS2RGB, fdata_imd_lms+i, fdata_imd+i);
    }
    normalize(0, 255, fdata_imd, size_imt);
    data_ftous(fdata_imd, data_imd, size_imt);
    pnm_save(imd, PnmRawPpm, imd_name);

    free(fdata_ims);
    free(fdata_imt);
    free(fdata_imd);
    free(fdata_ims_lms);
    free(fdata_imt_lms);
    free(fdata_imd_lms);
    free(fdata_ims_rab);
    free(fdata_imt_rab);
    free(fdata_imd_rab);

    pnm_free(ims);
    pnm_free(imt);
    pnm_free(imd);
}

void usage (char *s){
    fprintf(stderr, "Usage: %s <ims> <imt> <imd> \n", s);
    exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[]){
    if (argc != PARAM+1) usage(argv[0]);
    process(argv[1], argv[2], argv[3]);
    return EXIT_SUCCESS;
}
