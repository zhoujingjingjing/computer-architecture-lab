#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <m5op.h>
#include "ffos.h"

//==========================
//  The Main Function
//==========================
int
main(int argc, char* argv[]){
    int w, h, t;
    unsigned char * input_img;
    unsigned char * output_img;

    input_img = read_pgm(&w, &h, "input.pgm");
    printf("\nInput image width = %d, height = %d\n\n", w, h);

    output_img = malloc(w * h * sizeof(unsigned char));

    // ====================
    //  Processing Engine
    // ====================
    printf("Resetting stats\n");
    m5_reset_stats(0, 0);
    t = ffos_process_image_part((unsigned char *) input_img, w, h,
                                (unsigned char *) output_img);
    printf("Threshold = %d\n", t);
    if (argc > 1)
        write_pgm(output_img, w, h, "output.pgm");

    free(input_img);
    free(output_img);

    return 0;
}
