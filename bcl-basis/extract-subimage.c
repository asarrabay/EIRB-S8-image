#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

void usage (char *s) {
    fprintf(stderr,"Usage: %s %s", s, "<i> <j> <rows> <cols> <ims> <imd>\n");
    exit(EXIT_FAILURE);
}

void extract(int start_i, int start_j, int r, int c, pnm img_src, pnm img_dst) {
    unsigned short *data_src = pnm_get_image(img_src);
    unsigned short *data_dst = pnm_get_image(img_dst);
    unsigned short *s;
    unsigned short *t;

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            s = data_src + pnm_offset(img_src, i + start_i, j + start_j);
            t = data_dst + pnm_offset(img_dst, i, j);
            *t++ = *s++;
            *t++ = *s++;
            *t = *s;
        }
    }

}

#define PARAM 6
int main(int argc, char *argv[]) {
    if (argc != PARAM+1) usage(argv[0]);
    int r = atoi(argv[3]);
    int c = atoi(argv[4]);
    pnm img_src = pnm_load(argv[5]);
    pnm img_dst = pnm_new(c, r, PnmRawPpm);

    extract(atoi(argv[1]), atoi(argv[2]), r, c, img_src, img_dst);

    pnm_save(img_dst, PnmRawPpm, argv[6]);
    pnm_free(img_src);
    pnm_free(img_dst);

    return EXIT_SUCCESS;
}
