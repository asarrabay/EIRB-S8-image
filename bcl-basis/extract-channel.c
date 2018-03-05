#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

void usage (char *s) {
    fprintf(stderr,"Usage: %s %s", s, "<num> <ims> <imd>\n");
    exit(EXIT_FAILURE);
}


void extract_channel(pnmChannel num, pnm img_src, pnm img_dst) {
    unsigned short *channel = pnm_get_channel(img_src, NULL, num);
    for (size_t k = 0; k < 3; k++) {
        pnm_set_channel(img_dst, channel, k);
    }
    free(channel);
}


#define PARAM 3
int main(int argc, char *argv[]) {
    if (argc != PARAM+1) usage(argv[0]);
    pnm img_src = pnm_load(argv[2]);
    pnm img_dst = pnm_new(pnm_get_width(img_src), pnm_get_height(img_src), PnmRawPpm);

    extract_channel(atoi(argv[1]), img_src, img_dst);

    pnm_save(img_dst, PnmRawPpm, argv[3]);
    pnm_free(img_src);
    pnm_free(img_dst);
    return EXIT_SUCCESS;
}
