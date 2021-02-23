#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./stb_image_write.h"

typedef unsigned char byte;
typedef struct {
    byte r; 
    byte g; 
    byte b;
} rgb;


//rotate/flip a quadrant appropriately
void rot(uint32_t n, uint32_t *x, uint32_t *y, uint32_t rx, uint32_t ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }
 
        //Swap x and y
        uint32_t t  = *x;
        *x = *y;
        *y = t;
    }
}

//convert (x,y) to d
uint32_t xy2d (uint32_t n, uint32_t x, uint32_t y) {
    uint32_t rx, ry, s, d=0;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(s, &x, &y, rx, ry);
    }
    return d;
}
 
//convert d to (x,y)
void d2xy(uint32_t n, uint32_t d, uint32_t *x, uint32_t *y) {
    uint32_t rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}

#define IMG_SIZE 16384
#define BUF_SIZE 65536
int main(int argc, char** argv){

    rgb empty_rgb[256]={{128,64,32}};
    // Nucleotide position
    uint32_t p = 0;

    // Bufs for genome chunk an image
    rgb buf[BUF_SIZE];
    rgb png[BUF_SIZE];
    // Output tile name 
    char filename[256];
    // Size of readed chunk
    size_t s;
    
    for(int k=1; k<argc; k++) {

        printf("read file %s\n", argv[k]);
        FILE* f = fopen(argv[k], "r");
        if(!f) continue;
        
        // Iterate through chunks
        while((s = fread(buf, 1,  BUF_SIZE*3, f)) > 0) {
            printf("read %u\n", s);
            // x,y of pixel
            uint32_t x, y, tx, ty, px, py;
            // hilbert pos to x, y
            d2xy(IMG_SIZE, p, &x, &y);

            // tile num
            tx = x/16;
            ty = y/16;
            sprintf(filename, "%s/10-%u-%u.png", argv[2], tx, ty);
            for(uint32_t i=0; i<BUF_SIZE; i+=256) {
                d2xy(IMG_SIZE, p, &x, &y);

                // pixel pos
                px = (x%16)*16;
                py = (y%16)*16;
                // printf("%u %u %u\n", p, x, y);

                uint32_t j = px+py*256;
                // copy png line by line
                for(int l = 0; l<16; l++) {
                    rgb* c = (i<s) ? (&buf[i+l*16]) : (&empty_rgb[l*16]);
                    memcpy(&png[j+l*256], c, 16*3);
                }
                p++;
            }
            // break;
            stbi_write_png(filename, 256, 256, 3, png, 256*3);
        };
        fclose(f);
        
        // Start file from an new 256x256 block
        // p = ((p>>16)+1)<<16;
        
    }
}

