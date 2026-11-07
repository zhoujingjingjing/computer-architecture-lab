#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <m5op.h>
#include "ffos.h"
#include <assert.h>
#include <time.h>

//==========================
//  The Main Function
//==========================


int
main(int argc, char* argv[]){
    int w, h, t;
    unsigned char * input_img;
    unsigned char * output_img;


	time_t start_time, end_time;
	clock_t start_clock, end_clock;
	clock_t diff_clock;
	time_t diff_time;
	start_time = time(NULL);
	start_clock = clock();


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


	end_time  = time(NULL);
	end_clock = clock();
	diff_time = end_time - start_time;
	diff_clock = end_clock - start_clock;
	printf ("\n");
	printf ("cputime : start_clock = %lu units, end_clock = %lu units\n", start_clock, end_clock);
	printf ("cputime : processor time used = %.3f sec\n", (double)diff_clock/CLOCKS_PER_SEC);
	printf ("walltime : start_time = %lu sec, end_time = %lu sec\n", start_time, end_time);
	printf ("walltime : elapsed (wall clock) time = %lu sec\n", diff_time);
	printf ("\n");


    return 0;
}
