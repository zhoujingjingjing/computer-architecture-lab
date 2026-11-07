#include <stdio.h>
#include <stdlib.h>
#include "ffos.h"

void
erode(unsigned char * pbuf_erode, unsigned char * data, int w, int h){
    int i, j;
    //#pragma omp parallel for
    for(i = 0; i < h; i++) {
        //#pragma omp parallel for
        for(j = 0; j < w; j++) {
            // 5x5 rectangle
            if ((i < 2) || (i >= (h-2)) || (j < 2) || (j >= (w-2)) ) {
                pbuf_erode[i*w + j] = 0;
            } else {               
                pbuf_erode[i*w + j] = (   data[(i-2)*w+j-2] && data[(i-2)*w+j-1]
                                       && data[(i-2)*w+j  ] && data[(i-2)*w+j+1]
                                       && data[(i-2)*w+j+2] && data[(i-1)*w+j-2]
                                       && data[(i-1)*w+j-1] && data[(i-1)*w+j  ]
                                       && data[(i-1)*w+j+1] && data[(i-1)*w+j+2]
                                       && data[(i  )*w+j-2] && data[(i  )*w+j-1]
                                       && data[(i  )*w+j  ] && data[(i  )*w+j+1]
                                       && data[(i  )*w+j+2] && data[(i+1)*w+j-2]
                                       && data[(i+1)*w+j-1] && data[(i+1)*w+j  ]
                                       && data[(i+1)*w+j+1] && data[(i+1)*w+j+2]
                                       && data[(i+2)*w+j-2] && data[(i+2)*w+j-1]
                                       && data[(i+2)*w+j  ] && data[(i+2)*w+j+1]
                                       && data[(i+2)*w+j+2]) ? 255 : 0;
            }
        }
    }
}// end erode()
