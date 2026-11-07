#include "ffos.h"
#include <stdio.h>
#include <stdlib.h>

/**
 *  otsu algorithm to find threshold of binarization
 *  @param data input image
 *  @param w    the width of input image
 *  @param h    the height of input image
 *  @return     the threshold of binarization
 */
int
otsu(unsigned char * data, int w, int h){
    int i, j, k;  
    int     T  = 0;        // threshold for binarization
    double mG  = 0.0;      // global mean
    double max = 0.0;      // max sigma value

    double pi[256];       // histogram
    double P1[256];       // cumulative histogram
    double m[256];        // cumulative mean
    double sigmaB[256];  
  
    // initialization
    for (i = 0; i < 256; i++){
        pi[i]     = 0.0;
        m[i]      = 0.0;
        P1[i]     = 0.0;
        sigmaB[i] = 0.0;
    }
  
    /* build histogram */
    for(i = 0; i < h; i++){
        for(j = 0; j < w; j++){
            pi[ data[i*w+j] ] += 1.0;
        }// end j
    } // end i
 
    /* normalize */
    for(i = 0; i < 256; i++){    
        pi[i] /= (w * h);
    }// end i
    
    /* calculate each probability */
    for(k = 255; k > -1; k--){
        for(i = 0; i<= k; i++){
            // cumulative probability up till class k
            P1[k] = P1[k] + pi[i];
            // cumulative mean up till class k
            m[k] = m[k] + (i*pi[i]);    
        }// end i    
        mG = m[255];                  // global mean
        
        // calculate sigma
        if( (P1[k] < 1E-6) || (P1[k] > (1.0 - 1E-6)) ){
            sigmaB[k] = 0;
        }else{
            sigmaB[k] = ( (mG*P1[k]-m[k]) * (mG*P1[k]-m[k]) )\
                / (P1[k] * (1-P1[k]) );
        }
    }// end k

    /* maximum of sigmaB */
    for(i = 0; i < 256; i++){
        if(sigmaB[i] > max){
            max = sigmaB[i];
            T = i;
        }
    }// end i

    return T;
}// otsu()

/**
 *  otsu algorithm to find threshold of binarization integer version
 *  @param data input image
 *  @param w    the width of input image
 *  @param h    the height of input image
 *  @return     the threshold of binarization
 */
int
otsu_i(unsigned char * data, int w, int h){
    int i, j, k;  
    int T   = 0;        // threshold for binarization
    unsigned long long int max = 0;      // max sigma value
    unsigned int sum = 0;
    unsigned int a_k = 0;
    unsigned int sum_k = 0;
    
    unsigned int pi[256];       // histogram
    unsigned long long int sigmaB[256];  
  
    // initialization
    for (i = 0; i < 256; i++){
        pi[i]     = 0;
        sigmaB[i] = 0;
    }
  
    /* build histogram */
    for(i = 0; i < h; i++){
        for(j = 0; j < w; j++){
            pi[ data[i*w+j] ] ++;
        }// end j
    } // end i

     for( i = 0; i < 256; i++ ){
         sum = sum + i*pi[i];
     }
/* calculate each probability */
    int s  = (w * h);
    for(k = 0; k <256; k++){
        a_k = a_k + pi[k];          // cumulative probability up till class k
        sum_k = sum_k + (k*pi[k]);  // cumulative mean up till class k

        // calculate sigma
        if(((s - a_k) != 0 ) && ( a_k != 0)){
            unsigned int temp_1 = (sum_k << 11) / a_k;
            temp_1 = (s * temp_1) >> 11;
            unsigned int temp_2 = (sum - sum_k) << 11;
            temp_2 = temp_2 / (s - a_k);
            temp_2 = (a_k * temp_2) >> 11;
            sigmaB[k] = ( (double)( sum - temp_1 ) )  * ( (double) ( temp_2 - sum_k ) );
        }else{
            sigmaB[k] = 0;
        }// ((s - a_k) != 0 ) && ( a_k != 0)
    }// end k
    
    
    /* maximum of sigmaB */
    for(i = 0; i < 256; i++){
        if(sigmaB[i] > max){
            max = sigmaB[i];
            T = i;
        }
    }// end i

    return T;
}// otsu_i()

