#include <stdlib.h>
#include <stdio.h>
#include <pnm.h>
#include <math.h>

#include <fft.h>


void process(char *ims_name, char *imd_name){
    (void) ims_name;
    (void) imd_name;
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
