#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ffos.h"

static void 
binarization(unsigned char * bin_img, unsigned char * img,
             int threshold, int w, int h){

    int i, j;
    // Example use of OpenMP
    //#pragma omp parallel for
    for(i = 0; i < h; i++){
        for(j = 0; j < w; j++){
            if ( img[i*w+j] >= threshold){
                bin_img[i*w+j] = 255;
            }else{
                bin_img[i*w+j] = 0;
            }
        } // end j
    }// end i
}



//==========================
//  Process Function
//==========================
int
ffos_process_image_part (unsigned char * data_in, int w, int h,
                         unsigned char * data_out){
    // temporary data buffer
    unsigned char * pbuf  = malloc(w * h * sizeof(unsigned char));    
    
    // =============================
    //  Step 1: Calculate the best
    //  threshold for binarization
    // =============================
    int threshold = otsu(data_in, w, h);      // use floating-point version
    
    // for debug
#ifdef DBG_ALL
	  printf("otsu threshold: %d\n\n", threshold);    
#endif

    // ========================
    //  Step 2: Binarization
    // ========================
    binarization(pbuf, data_in, threshold, w, h);
	  
	  // for debug
#ifdef DBG_ALL
    printf("write the binary.pgm file. \n");
    write_pgm(pbuf, w, h, "binary.pgm");
#endif
    
    // ========================
    //  Step 3: Erosion 5x5
    // ========================
    // erosion on the binary image
    // data_out: output buffer, pbuf: input buffer
    erode(data_out, pbuf, w, h);
    
    // for debug
#ifdef DBG_ALL
    printf("write the erosion.pgm file. \n");
    write_pgm(data_out, w, h, "erosion.pgm");
#endif
    
    
    // free space
    free(pbuf);    
    return threshold;
} // end process_image()


