#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>
#include <domain.h>

#include <fft.h>

#define PI 3.14159265359
#define GAP_RATIO 1.5
#define SIZE_NEIGHBOURHOOD 7
#define TOLERANCE 100

struct coord {
    int i;
    int j;
};

struct coord find_circle(pnm img, int gap_min){
    (void) img;
    struct coord c;
    int rows = pnm_get_height(img);
    int cols = pnm_get_width(img);
    for(int i = 0; i < rows; i++){
        // unsigned short prev = pnm_get_component(img, i, 0, 0);
        int step = 0;
        int cpt = 0;
        int gap_size = 0;
        int start = 0;
        int end = 0;
        for (int j = 1; j < cols-1; j++) {
            unsigned short current = pnm_get_component(img, i, j, 0);
            unsigned short next = pnm_get_component(img, i, j+1, 0);
            switch (step) {
                case 0:
                if (current == 0 && next == 0) {
                    start = j;
                    step++;
                }
                break;

                case 1:
                if(current == 0){
                    cpt++;
                } else if (current == 255 && next == 255) {
                    if (cpt < gap_min) {
                        // printf("%d < gap_min\n", cpt);
                        step = 0;
                    } else {
                        // printf("STEP 1: %d, %d\n",i, j );
                        step++;
                        gap_size = cpt;
                    }
                    cpt = 0;
                }

                break;

                case 2:
                if (cpt > gap_size * GAP_RATIO) {
                    // printf("%d > gap_size*1.5 (%d)\n", cpt, gap_size);
                    cpt = 0;
                    step = 0;
                } else {
                    if(current == 255){
                        cpt++;
                    } else if (current == 0 && next == 0) {
                        if (cpt < gap_size) {
                            printf("STEP 2 : %d < gap_min\n", cpt);
                            step = 0;
                        } else {
                            printf("STEP 2 : %d, %d\n",i, j );
                            printf("STEP 2 cpt: %d\n",cpt);
                            printf("STEP 2 gap_size: %d\n",gap_size);
                            step++;
                        }
                        cpt = 0;
                    }
                }

                break;
                case 3:
                if (cpt > gap_size * GAP_RATIO) {
                    printf("STEP 3 : %d > gap_size$ (%d)\n", cpt, gap_size);
                    cpt = 0;
                    step = 0;
                } else {
                    if(current == 0){
                        cpt++;
                    } else if (current == 255 && next == 255) {
                        if (cpt < gap_size) {
                            step = 0;
                        } else {
                            end = j;
                            printf("%d, %d\n",i, j );
                            printf("######################################\n");
                            printf("start : %d,%d, end : %d,%d\n", i,start, i,end);
                            c.i = i;
                            c.j = start + (end-start)/2;
                            return c;
                            printf("###str###################################\n");
                            step = 0;
                            exit(0);
                        }
                        cpt = 0;
                    }
                }

                break;
            }
            // prev = current;
        }
    }
    return c;
}

float deg2rad(float deg) {
    return deg * PI / 180.0;
}

pnm thresholding(unsigned short threshold, pnm ims){
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    pnm imd = pnm_dup(ims);
    unsigned short *data_imd = pnm_get_image(imd);
    for (int i = 0; i < cols * rows * 3; i++) {
        if (data_imd[i] < threshold) {
            data_imd[i] = 0;
        } else {
            data_imd[i] = 255;
        }
    }
    return imd;
}

float get_mean_neighbourhood(pnm img, int i, int j){
    int cols = pnm_get_width(img);
    int rows = pnm_get_height(img);

    int nb_neighbours = 0;
    float sum = 0;
    for(int x = i - SIZE_NEIGHBOURHOOD / 2; x < i + SIZE_NEIGHBOURHOOD / 2; x++){
        for(int y = j - SIZE_NEIGHBOURHOOD / 2; y < j + SIZE_NEIGHBOURHOOD / 2; y++){
            if(x >= 0 && x < cols &&
               y >= 0 && y < rows){
                   unsigned short value = pnm_get_component(img, y, x, PnmRed);
                   nb_neighbours++;
                   sum += value;
            }
        }
    }

    if(nb_neighbours == 0)
        return 0;

    return sum / nb_neighbours;
}

int is_too_far(unsigned short px, unsigned short mean){
    unsigned short tmp = fabs(px - mean);

    return tmp > TOLERANCE;
}

pnm filter(pnm ims){
    int cols = pnm_get_width(ims);
    int rows = pnm_get_height(ims);

    pnm res = pnm_new(cols, rows, PnmRawPpm);

    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            float mean = get_mean_neighbourhood(ims, i, j);
            unsigned short px = pnm_get_component(ims, j, i, PnmRed);
            unsigned short value;
            if(is_too_far(px, mean)){
                value = 255;
            }
            else{
                value = mean;
            }
            for(int c = 0; c < 3; c++){
                pnm_set_component(res, j, i, c, value);
            }
        }
    }

    return res;
}

void rotate(int x_center, int y_center, float angle, pnm ims, pnm imd){
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    float cos_t = cosf(deg2rad(angle));
    float sin_t = sinf(deg2rad(angle));


    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float x = cos_t * (j-x_center) - sin_t * (i-y_center) + x_center;
            float y = sin_t * (j-x_center) + cos_t * (i-y_center) + y_center;
            for (int c = 0; c < 3; c++) {
                pnm_set_component(imd, i, j, c, bilinear_interpolation(x, y, ims, c));
            }
        }
    }
}

void process(char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    // int cols = pnm_get_width(ims);
    // int rows = pnm_get_height(ims);
    // pnm imd = pnm_new(imd_name, cols, rows, PnmRawPpm);
    pnm imd;
    imd = filter(ims);
    // pnm_free(ims);
    // ims = imd;
    imd = thresholding(180, ims);
    struct coord circle = find_circle(imd, 2);
    printf("circle : %d, %d\n", circle.i, circle.j);
    (void) circle;
    pnm_save(imd, PnmRawPpm, imd_name);
    pnm_free(ims);
    pnm_free(imd);

}

void usage(char* s) {
    fprintf(stderr,"%s <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 2
int main(int argc, char* argv[]) {
    if(argc != PARAM+1)
    usage(argv[0]);

    process(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
