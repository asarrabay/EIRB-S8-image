#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

void usage (char *s) {
    fprintf(stderr,"Usage: %s %s", s, "<ims> <imd>\n");
    exit(EXIT_FAILURE);
}


void color2mean(pnm s, pnm t) {
    int c = pnm_get_width(s);
    int l = pnm_get_height(s);
    unsigned short *data_src = pnm_get_image(s);
    unsigned short *data_dst = pnm_get_image(t);
    for (int i = 0; i < l*c; i++) {
        unsigned int avg = 0;
        avg += *data_src++;
        avg += *data_src++;
        avg += *data_src++;
        avg /= 3;
        *data_dst++ = (unsigned short) avg;
        *data_dst++ = (unsigned short) avg;
        *data_dst++ = (unsigned short) avg;
    }
}


#define PARAM 2
int main(int argc, char *argv[]) {
    if (argc != PARAM+1) usage(argv[0]);
    pnm img_src = pnm_load(argv[1]);
    pnm img_dst = pnm_new(pnm_get_width(img_src), pnm_get_height(img_src), PnmRawPpm);

    color2mean(img_src, img_dst);

    pnm_save(img_dst, PnmRawPpm, argv[2]);

    pnm_free(img_src);
    pnm_free(img_dst);

    return EXIT_SUCCESS;
}
