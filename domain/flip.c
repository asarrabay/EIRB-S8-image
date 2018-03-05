#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>



pnm flip(char dir, pnm ims){
    int cols, rows;
    pnm imd;
    rows = pnm_get_height(ims);
    cols = pnm_get_width(ims);
    if (dir == 't') {
        imd = pnm_new(rows, cols, PnmRawPpm);
    } else {
        imd = pnm_new(cols, rows, PnmRawPpm);
    }


    unsigned short *data_s = pnm_get_image(ims);
    unsigned short *data_d = pnm_get_image(imd);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int new_i = i;
            int new_j = j;
            switch (dir) {
                case 'h':
                    new_j = cols - j - 1;
                break;
                case 'v':
                    new_i = rows - i - 1;
                break;
                case 't':
                    new_i = j;
                    new_j = i;
                break;
            }
            int p = pnm_offset(ims, i, j);
            int q = pnm_offset(imd, new_i, new_j);
            for (size_t k = 0; k < 3; k++) {
                data_d[q+k] = data_s[p+k];
            }
        }

    }
    return imd;
}

void process(char *dir, char* ims_name, char* imd_name){
    pnm imd = NULL;
    pnm ims = pnm_load(ims_name);
    while(*dir != '\0'){
        imd = flip(*dir, ims);
        pnm_free(ims);
        ims = imd;
        dir++;
    }
    pnm_save(imd, PnmRawPpm, imd_name);
    // pnm_free(ims);
    pnm_free(imd);
}

void usage(char* s) {
    fprintf(stderr,"%s <dir> <ims> <imd>\n",s);
    exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char* argv[]) {
    if(argc != PARAM+1)
    usage(argv[0]);

    process(argv[1], argv[2], argv[3]);

    return EXIT_SUCCESS;
}
