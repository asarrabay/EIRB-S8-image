#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>
#include <domain.h>

#include <fft.h>

#define PI 3.14159265359
#define GAP_RATIO 1.5
#define SIZE_NEIGHBOURHOOD 7
#define TOLERANCE_DIFF_PIXELS_FILTER 60
#define TOLERANCE_RATIO_CIRCLE_NOISE 0.90
#define MAX_RADIUS 5
#define MAX_POSSIBLES 4192
#define MAX_HEAP 8

struct coord {
    int i;
    int j;
};
void search_circles(pnm img, struct coord final_points[4]);

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
    res.i = 0;
    res.j = 0;

    if(nb_elements == 0){
        return res;
    }

    for(int i = 0; i < nb_elements; i++){
        res.i += heap[i].i;
        res.j += heap[i].j;
    }

    res.i /= nb_elements;
    res.j /= nb_elements;

    return res;
}

int is_neighbour(struct coord *heap, int nb_elements, struct coord px){
    struct coord mean = mean_heap(heap, nb_elements);

    return sqrt(pow(mean.i - px.i, 2) + pow(mean.j - px.j, 2)) < 2 * MAX_RADIUS;
}

void extract_circles(struct coord *possible_px, int nb_possibles, struct coord out[4]){
    struct coord heap[MAX_HEAP][MAX_POSSIBLES];
    int nb_elements[MAX_HEAP] = {0};
    int heap_order[MAX_HEAP] = {0};
    int sorted = 0;

    for(int i = 0; i < nb_possibles; i++){//Creating heaps
        sorted = 0;
        for(int j = 0; j < MAX_HEAP && !sorted; j++){
            if(nb_elements[j] != 0){
                if(is_neighbour(heap[j], nb_elements[j], possible_px[i])){//if the pixel is part of the heap
                    if(nb_elements[j] < MAX_POSSIBLES){
                        heap[j][nb_elements[j]] = possible_px[i];
                        nb_elements[j]++;
                        sorted = 1;
                    }
                }
            }
        }
        if(!sorted){
            for(int j = 0; j < MAX_HEAP && !sorted; j++){//Otherwise, we put the pixel in another heap
                if(nb_elements[j] == 0){
                    if(nb_elements[j] < MAX_POSSIBLES){
                        heap[j][nb_elements[j]].i = possible_px[i].i;
                        heap[j][nb_elements[j]].j = possible_px[i].j;

                        nb_elements[j]++;
                        sorted = 1;
                    }
                }
            }
        }
    }

    for(int i = 0; i < MAX_HEAP; i++){
        heap_order[i] = i;
    }

    for(int j = 0; j < MAX_HEAP; j++){//sort heaps to only consider the most detected ones
        for(int i = j; i < MAX_HEAP; i++){
            if(nb_elements[heap_order[j]] < nb_elements[heap_order[i]]){
                int tmp = heap_order[j];
                heap_order[j] = heap_order[i];
                heap_order[i] = tmp;
            }
        }
    }

    for(int i = 0; i < 4; i++){//Meaning of heaps
        out[i] = mean_heap(heap[heap_order[i]], nb_elements[heap_order[i]]);
    }
}

void search_circles(pnm img, struct coord final_points[4]){
    int cols = pnm_get_width(img);
    int rows = pnm_get_height(img);
    int current_radius = 5;

    unsigned short *buf = malloc(cols * rows * sizeof(short));
    struct coord possible_px[MAX_POSSIBLES];//Finds the potential centers of 4 circles.
    int nb_possibles = 0;

    if(!buf){
        fprintf(stderr, "Error malloc (%dx%d sized img)\n", cols, rows);
        exit(EXIT_FAILURE);
    }

    for(int radius = current_radius; radius < MAX_RADIUS + 1; radius++){
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                buf[j] = pnm_get_component(img, j, i, PnmRed);//Create the buffer of an entire row.
            }
            for(int j = 0; j < rows; j++){
                if(px_is_possible(buf, j, radius, rows)){
                    if(nb_possibles < MAX_POSSIBLES){
                        possible_px[nb_possibles].i = i;
                        possible_px[nb_possibles].j = j;
                        nb_possibles++;
                    }
                }
            }
        }
    }

    //TODO : make the same test px_is_possible on cols, then eliminate pixels that are not possible on both dimension (works fine without it)

    printf("Found %d possible points\n", nb_possibles);

    extract_circles(possible_px, nb_possibles, final_points);//Create 4 points from all possible_px (mean of each heap)

    for(int i = 0; i < 4; i++){
        printf("Circle %d : (%d %d)\n", i, final_points[i].i, final_points[i].j);
    }
}

float distance(struct coord p1, struct coord p2){
    return(sqrt(pow(p1.i - p2.i, 2) + pow(p1.j - p2.j, 2)));
}

float compute_angle(struct coord centers[4]){
    int a = -1, b = -1, c = -1, d = -1;
    //a, b, c and d are the points of the square to readjust, a is top left point, c is bottom right one

    for(int i = 0; i < 4; i++){
        if(a == -1 || centers[a].j > centers[i].j)
          a = i;
    }

    for(int i = 0; i < 4; i++){
        if((b == -1 || centers[b].j > centers[i].j) && i != a)
          b = i;
    }

    if(centers[a].i > centers[b].j){
        int tmp = a;
        a = b;
        b = tmp;
    }

    for(int i = 0; i < 4; i++){
        if(c == -1 && i != a && i != b){
            c = i;
        }
    }

    for(int i = 0; i < 4; i++){
        if(d == -1 && i != a && i != b && i != c){
            d = i;
        }
    }

    if(centers[c].i < centers[d].j){
        int tmp = c;
        c = d;
        d = tmp;
    }

    float adx = centers[d].i - centers[a].i;
    float ady = centers[a].j - centers[d].j;
    float angle1 = 0;

    float bcx = centers[c].i - centers[b].i;
    float bcy = centers[b].j - centers[c].j;
    float angle2 = 0;

    if(ady != 0)
        angle1 = rad2deg(atanf(adx/ady));

    if(bcy != 0)
        angle2 = rad2deg(atanf(bcx/bcy));

    return (angle1 + angle2) / 2;//compute both angles then return the mean (could return an error if angles are too different)
}
void process(char *ims_name, char *imd_name){
    pnm ims = pnm_load(ims_name);
    int cols = pnm_get_width(ims);
    int rows = pnm_get_height(ims);
    // pnm imd = pnm_new(imd_name, cols, rows, PnmRawPpm);
    pnm imd;
    imd = filter(ims);//Reduces noise
    // pnm_free(ims);
    // ims = imd;
    imd = thresholding(180, imd);
    //struct coord circle = find_circle(imd);
    //(void) circle;
    // pnm_save(imd, PnmRawPpm, imd_name);
    // return;
    struct coord centers[4];
    search_circles(imd, centers);
    float angle = compute_angle(centers);//degrees
    printf("angle %f\n", angle);
    rotate(cols/2, rows/2, angle, ims, imd);//TODO : reshape imd so we don't lose any part of src img
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
