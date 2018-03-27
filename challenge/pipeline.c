#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>
#include <domain.h>

#include <fft.h>

#define PI 3.14159265359
#define SIZE_NEIGHBOURHOOD 7
#define TOLERANCE_DIFF_PIXELS_FILTER 60
#define TOLERANCE_RATIO_CIRCLE_NOISE 0.90
#define MAX_RADIUS 5
#define MAX_POSSIBLES 512

struct coord {
    int x;
    int y;
};

void search_circles(pnm img, struct coord final_points[4]);

struct coord find_circle(pnm img){
    (void) img;
    struct coord c;
    c.x = 0;
    c.y = 0;
    return c;
}

float deg2rad(float deg) {
    return deg * PI / 180.0;
}

float rad2deg(float rad) {
    return rad * 180 / PI;
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

    return tmp > TOLERANCE_DIFF_PIXELS_FILTER;
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

int nb_pixels_ok(unsigned short *px, int a, int b, int len, unsigned short correct_value){
    int res = 0;

    for(int j = a; j < b; j++){
        if(j >= 0 && j < len){
            if(px[j] == correct_value){
                res++;
            }
        }
    }

    return res;
}

int px_is_possible(unsigned short *px, int i, int r, int len){
    int tmp = nb_pixels_ok(px, i - r, i + r, len, 255);//Middle of the circle should be white
    if((float)tmp < TOLERANCE_RATIO_CIRCLE_NOISE * (float)r){
        return 0;
    }

    tmp = nb_pixels_ok(px, i - 3 * r, i - r, len, 0);//Left side of the circle should be black
    if((float)tmp < TOLERANCE_RATIO_CIRCLE_NOISE * (float)r){
        return 0;
    }

    tmp = nb_pixels_ok(px, i + r, i + 3 * r, len, 0);//Right side of the circle should be black
    if((float)tmp < TOLERANCE_RATIO_CIRCLE_NOISE * (float)r){
        return 0;
    }

    tmp = nb_pixels_ok(px, i - 4 * r, i - 3 * r, len, 255);//Left of the circle should be white
    if((float)tmp < TOLERANCE_RATIO_CIRCLE_NOISE *(float) r){
        return 0;
    }

    tmp = nb_pixels_ok(px, i + 3 * r, i + 4 * r, len, 255);//Right of the circle should be white
    if((float)tmp < TOLERANCE_RATIO_CIRCLE_NOISE * (float)r){
        return 0;
    }

    return 1;
}

struct coord mean_heap(struct coord *heap, int nb_elements){

    struct coord res;
    res.x = 0;
    res.y = 0;

    if(nb_elements == 0){
        return res;
    }

    for(int i = 0; i < nb_elements; i++){
        res.x += heap[i].x;
        res.y += heap[i].y;
    }

    res.x /= nb_elements;
    res.y /= nb_elements;

    return res;
}

int is_neighbour(struct coord *heap, int nb_elements, struct coord px){
    struct coord mean = mean_heap(heap, nb_elements);

    return sqrt(pow(mean.x - px.x, 2) + pow(mean.y - px.y, 2)) < 2 * MAX_RADIUS;
}

void extract_circles(struct coord *possible_px, int nb_possibles, struct coord out[4]){
    struct coord heap[4][MAX_POSSIBLES];
    int nb_elements[4] = {0};
    int sorted = 0;

    for(int i = 0; i < nb_possibles; i++){
        sorted = 0;
        for(int j = 0; j < 4 && !sorted; j++){
            if(nb_elements[j] != 0){
                if(is_neighbour(heap[j], nb_elements[j], possible_px[i])){
                    if(nb_elements[j] < MAX_POSSIBLES){
                        heap[j][nb_elements[j]] = possible_px[i];
                        nb_elements[j]++;
                        sorted = 1;
                    }
                }
            }
        }
        if(!sorted){
            for(int j = 0; j < 4 && !sorted; j++){
                if(nb_elements[j] == 0){
                    if(nb_elements[j] < MAX_POSSIBLES){
                        heap[j][nb_elements[j]].x = possible_px[i].x;
                        heap[j][nb_elements[j]].y = possible_px[i].y;

                        nb_elements[j]++;
                        sorted = 1;
                    }
                }
            }
        }
    }

    for(int i = 0; i < 4; i++){
        out[i] = mean_heap(heap[i], nb_elements[i]);
    }
}

void search_circles(pnm img, struct coord final_points[4]){
    int cols = pnm_get_width(img);
    int rows = pnm_get_height(img);
    int current_radius = 5;

    unsigned short *buf = malloc(cols * rows * sizeof(short));
    struct coord possible_px[MAX_POSSIBLES];
    int nb_possibles = 0;

    if(!buf){
        fprintf(stderr, "Error malloc (%dx%d sized img)\n", cols, rows);
        exit(EXIT_FAILURE);
    }

    for(int radius = current_radius; radius < MAX_RADIUS + 1; radius++){
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                buf[j] = pnm_get_component(img, j, i, PnmRed);
            }
            for(int j = 0; j < rows; j++){
                if(px_is_possible(buf, j, radius, rows)){
                    if(nb_possibles < MAX_POSSIBLES){
                        possible_px[nb_possibles].x = i;
                        possible_px[nb_possibles].y = j;
                        nb_possibles++;
                    }
                }
            }
        }
    }

    extract_circles(possible_px, nb_possibles, final_points);

    for(int i = 0; i < 4; i++){
        printf("Circle %d : (%d %d)\n", i, final_points[i].x, final_points[i].y);
    }
}

float distance(struct coord p1, struct coord p2){
    return(sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)));
}

float compute_angle(struct coord centers[4]){
    float max_dist = 0;
    int index_max_dist = 0;
    int index_target = 0;

    for(int i = 1; i < 4; i++){//finds diag
        float tmp = distance(centers[i], centers[0]);
        if(tmp > max_dist){
            max_dist = tmp;
            index_max_dist = i;
        }
    }

    max_dist = 0;

    for(int i = 1; i < 4; i++){//finds longer side
        if(i != index_max_dist){
            float tmp = distance(centers[i], centers[0]);
            if(tmp > max_dist){
                max_dist = tmp;
                index_target = i;
            }
        }
    }

    float a = fabs(centers[0].x - centers[index_target].x);
    float b = fabs(centers[0].y - centers[index_target].y);

    if(b == 0)
        return 0;

    return rad2deg(atanf(a/b));
}
void process(char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    int cols = pnm_get_width(ims);
    int rows = pnm_get_height(ims);
    // pnm imd = pnm_new(imd_name, cols, rows, PnmRawPpm);
    pnm imd;
    imd = filter(ims);
    // pnm_free(ims);
    // ims = imd;
    imd = thresholding(180, imd);
    //struct coord circle = find_circle(imd);
    //(void) circle;
    struct coord centers[4];
    search_circles(imd, centers);
    float angle = compute_angle(centers);//degrees
    rotate(cols/2, rows/2, angle, ims, imd);
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
