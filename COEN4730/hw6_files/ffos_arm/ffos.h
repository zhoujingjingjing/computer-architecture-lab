#ifndef _FFOS_H
#define _FFOS_H


//**********************
//  Macro Definition
//**********************
#define OLED_WIDTH   36              // width of the OLED
#define OLED_HEIGHT  12              // height of the OLED

//**********************
//  Functions
//**********************
// otsu
int  otsu(unsigned char * data, int w, int h);      // floating-point version
int  otsu_i(unsigned char * data, int w, int h);    // integer version

// erosion
void erode(unsigned char * pbuf_erode, unsigned char * data, int w, int h);

// ffos pipeline (part)
int ffos_process_image_part(unsigned char * data_in, int w, int h,
                             unsigned char * data_out);

// image in/out
void write_pgm(unsigned char * data, int w, int h,
               const char * filename);
unsigned char * read_pgm(int *w, int *h, const char * filename);

#endif //_FFOS_H

