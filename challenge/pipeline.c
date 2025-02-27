#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>

#define PI 3.14159265359
#define GAP_RATIO 1.5
#define RATIO_NEIGHBOURHOOD 0.01
#define TOLERANCE_DIFF_PIXELS_FILTER 60
#define TOLERANCE_RATIO_CIRCLE_NOISE 0.90
#define CONFIDENCE_RATIO 0.00005
#define MIN_RADIUS 5
#define MAX_RADIUS 30
#define MAX_POSSIBLES 4192
#define MAX_HEAP 8

struct coord {
    int i;
    int j;
};
void search_circles(pnm img, struct coord final_points[4]);

unsigned short bilinear_interpolation(float x, float y, pnm ims, int c) {
    int rows = pnm_get_height(ims);
    int cols = pnm_get_width(ims);
    int i = floor(y);
    int j = floor(x);
    float dx = x - j;
    float dy = y - i;
    float interpolation = 0;
    if (i >= 0 && j >= 0 && i < rows && j < cols) {
        interpolation = interpolation + (1-dx) * (1-dy) * pnm_get_component(ims, i, j, c);
    }
    if (i >= 0 && j+1 >= 0 && i < rows && j+1 < cols) {
        interpolation = interpolation + dx * (1-dy) * pnm_get_component(ims, i, j+1, c);
    }
    if (i+1 >= 0 && j >= 0 && i+1 < rows && j < cols) {
        interpolation = interpolation + (1-dx) * dy * pnm_get_component(ims, i+1, j, c);
    }
    if (i+1 >= 0 && j+1 >= 0 && i+1 < rows && j+1 < cols) {
        interpolation = interpolation + dx * dy * pnm_get_component(ims, i+1, j+1, c);
    }
    return (unsigned short)interpolation;
}

float deg2rad(float deg) {
    return deg * PI / 180.0;
}

float rad2deg(float rad) {
    return rad * 180 / PI;
}

pnm thresholding(unsigned short threshold, pnm ims){//set all pixels to white or black
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
    int size_n_c = RATIO_NEIGHBOURHOOD * cols;
    int size_n_r = RATIO_NEIGHBOURHOOD * rows;

    int nb_neighbours = 0;
    float sum = 0;
    for(int x = i - size_n_c / 2; x < i + size_n_c / 2; x++){
        for(int y = j - size_n_r / 2; y < j + size_n_r / 2; y++){
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

pnm filter(pnm ims){//Reduces noise
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

    unsigned short *buf_rows = malloc(rows * sizeof(short));
    unsigned short *buf_cols = malloc(cols * sizeof(short));
    struct coord possible_px[MAX_POSSIBLES];//Finds the potential centers of 4 circles.
    int nb_possibles = 0;

    if(!buf_rows || !buf_cols){
        fprintf(stderr, "Error malloc (%dx%d sized img)\n", cols, rows);
        exit(EXIT_FAILURE);
    }

    for(int radius = MIN_RADIUS; radius < MAX_RADIUS + 1 && (float)nb_possibles / (cols * rows) < CONFIDENCE_RATIO; radius++){
        nb_possibles = 0;
        printf("Testing radius %d ...\n", radius);
        for(int i = 0; i < cols; i++){
            for(int j = 0; j < rows; j++){
                buf_rows[j] = pnm_get_component(img, j, i, PnmRed);//Create the buffer of an entire row.
            }
            for(int j = 0; j < rows; j++){
                for(int c = 0; c < 8 * radius; c++){
                    int index_cols = i + c - 4 * radius;
                    if(index_cols >= 0 && index_cols < cols){
                        buf_cols[c] = pnm_get_component(img, j, index_cols, PnmRed);
                    }
                }
                if(px_is_possible(buf_rows, j, radius, rows) && px_is_possible(buf_cols, 4 * radius, radius, 8 * radius)){
                    if(nb_possibles < MAX_POSSIBLES){
                        possible_px[nb_possibles].i = i;
                        possible_px[nb_possibles].j = j;
                        nb_possibles++;
                    }
                }
            }
        }
        if((float)nb_possibles / (cols * rows) < CONFIDENCE_RATIO){
            printf("Failed\n");
        }
    }

    extract_circles(possible_px, nb_possibles, final_points);//Create 4 points from all possible_px (mean of each heap)

    printf("Centers found :\n");

    for(int i = 0; i < 4; i++){
        printf("(%d %d)\n", final_points[i].i, final_points[i].j);
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
    pnm imd;

    printf("Processing on %s :\n", ims_name);

    imd = filter(ims);//Reduces noise
    imd = thresholding(180, imd);

    struct coord centers[4];
    search_circles(imd, centers);
    float angle = compute_angle(centers);//degrees
    printf("Angle found : %f\n", angle);
    rotate(cols/2, rows/2, angle, ims, imd);
    pnm_save(imd, PnmRawPpm, imd_name);
    printf("Image saved on %s\n", imd_name);
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
