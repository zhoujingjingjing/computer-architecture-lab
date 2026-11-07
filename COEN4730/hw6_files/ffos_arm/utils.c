#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

unsigned char *
read_pgm(int *w, int *h,const char * filename){
    unsigned char * data;
    FILE * in_file;
    char ch, type;
    int i;
    
    in_file = fopen(filename, "rb");
    if (! in_file){
        fprintf(stderr, "ERROR(0): Fail to open file %s\n", filename);
        exit(1);
    }
    /*determine pgm image type (only type three can be used)*/
    ch = getc(in_file);
    if(ch != 'P'){
        printf("ERROR(1): Not valid pgm/ppm file type\n");
        exit(1);
    }
    ch = getc(in_file);
    /*convert the one digit integer currently represented as a character to
      an integer(48 == '0')*/
    type = ch - 48;
    if(type != 5)
    {
        printf("ERROR(2): this file type (P%d) is not supported!\n", type);
        exit(1);
    }
    while(getc(in_file) != '\n');      /* skip to end of line*/
  
    while (getc(in_file) == '#'){       /* skip comment lines */
    
        while (getc(in_file) != '\n');
    }
    fseek(in_file, -1, SEEK_CUR);     /* backup one character*/

    i = fscanf(in_file,"%d", w);
    i = fscanf(in_file,"%d", h);
    i = fscanf(in_file,"%d", &i);  /* skipped here */
    while(getc(in_file) != '\n');
    data = malloc((*w)*(*h)*sizeof(unsigned char));
    
    i = fread(data, sizeof(unsigned char), (*w)*(*h), in_file);
    return data;
}


void
write_pgm(unsigned char * data, int w, int h,
               const char * filename){
    FILE * out_file;

    assert(w > 0);
    assert(h > 0);
    
    out_file = fopen(filename, "wb");
    if (! out_file){
        fprintf(stderr, "Fail to open file: %s\n", filename);
        exit(1);
    }
    
    fprintf(out_file, "P5\n");
    fprintf(out_file, "%d %d\n255\n", w, h);
    fwrite(data, sizeof(unsigned char), w * h, out_file);
    fclose(out_file);
}
