#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

void usage (char *s) {
    fprintf(stderr,"Usage: %s %s", s, "<ims1> <ims2> <ims3> <imd>\n");
    exit(EXIT_FAILURE);
}


void gray2color(pnm r, pnm g, pnm b, pnm img_dst) {
    unsigned short *channel_r = pnm_get_channel(r, NULL, 0);
    unsigned short *channel_g = pnm_get_channel(g, NULL, 0);
    unsigned short *channel_b = pnm_get_channel(b, NULL, 0);
    pnm_set_channel(img_dst, channel_r, 0);
    pnm_set_channel(img_dst, channel_g, 1);
    pnm_set_channel(img_dst, channel_b, 2);
    free(channel_r);
    free(channel_g);
    free(channel_b);
}


#define PARAM 4
int main(int argc, char *argv[]) {
    if (argc != PARAM+1) usage(argv[0]);
    pnm r = pnm_load(argv[1]);
    pnm g = pnm_load(argv[2]);
    pnm b = pnm_load(argv[3]);
    pnm img_dst = pnm_new(pnm_get_width(r), pnm_get_height(r), PnmRawPpm);

    gray2color(r, g, b, img_dst);

    pnm_save(img_dst, PnmRawPpm, argv[4]);
    pnm_free(r);
    pnm_free(g);
    pnm_free(b);
    pnm_free(img_dst);
    return EXIT_SUCCESS;
}
