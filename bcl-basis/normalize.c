#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

void usage (char *s) {
    fprintf(stderr,"Usage: %s %s", s, "<min> <max> <ims> <imd>\n");
    exit(EXIT_FAILURE);
}


void normalize_channel(unsigned short min, unsigned short max, pnm src, pnm dst, pnmChannel num) {
    unsigned short max_I = 0;
    unsigned short min_I = 255;
    unsigned short *channel = pnm_get_channel(src, NULL, num);
    int size = pnm_get_width(src) * pnm_get_height(src);
    for (int i = 0; i < size; i++) {
        if (min_I > channel[i]) {
            min_I = channel[i];
        }
        if (max_I < channel[i]) {
            max_I = channel[i];
        }
    }
    for (int i = 0; i < size; i++) {
        channel[i] = ((float)(max - min)/(float)(max_I - min_I)) * channel[i] + ((float)(min * max_I - max * min_I)/(float)(max_I - min_I));
    }
    pnm_set_channel(dst, channel, num);
    free(channel);
}

void normalize(unsigned short min, unsigned short max, pnm src, pnm dst) {
    for (int i = 0; i < 3; i++) {
        normalize_channel(min, max, src, dst, i);
    }
}


#define PARAM 4
int main(int argc, char *argv[]) {
    if (argc != PARAM+1) usage(argv[0]);
    pnm img_src = pnm_load(argv[3]);
    pnm img_dst = pnm_new(pnm_get_width(img_src), pnm_get_height(img_src), PnmRawPpm);

    normalize(atoi(argv[1]), atoi(argv[2]), img_src, img_dst);

    pnm_save(img_dst, PnmRawPpm, argv[4]);

    pnm_free(img_src);
    pnm_free(img_dst);

    return EXIT_SUCCESS;
}
