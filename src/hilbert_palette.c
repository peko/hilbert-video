#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

    static int old_tx = -1;
    static int old_ty = -1;
    
    int width, height, resolution, wc, hc;
    if (sscanf (argv[1], "%i", &width) != 1) {
        fprintf(stderr, "width not an integer");
        exit(1);
    }
    if (sscanf (argv[2], "%i", &height) != 1) {
        fprintf(stderr, "height - not an integer");
        exit(1);
    }

    resolution = width*height;
    wc = 256/width;
    hc = 256/height;
    
    rgb buf[resolution];
    rgb png[BUF_SIZE];
    
    rgb empty_rgb[256]={{128,64,32}};
    uint32_t p = 0;


    // Output tile name 
    char filename[256];
    // Size of readed chunk
    size_t s;    

    FILE* f = fdopen(dup(fileno(stdin)), "rb");
    
    // Read whole frame
    while((s = fread(buf, 1,  resolution*3, f)) > 0) {
        
        // x,y of pixel
        uint32_t x, y, tx, ty, px, py;
        // hilbert pos to x, y
        d2xy(IMG_SIZE, p, &x, &y);

        // tile num
        tx = x/wc;
        ty = y/hc;
        
        // create tile
        if (old_tx!=tx || old_ty!=ty) {
            if(old_tx >= 0) {
                printf("write tile %d %d\n", old_tx, old_ty);
                sprintf(filename, "%s/10-%u-%u.png", argv[1], old_tx, old_ty);
                stbi_write_png(filename, 256, 256, 3, png, 256*3);
            }
            old_tx = tx;
            old_ty = ty;
            printf("new tile %d %d\n", old_tx, old_ty);
        }      

        // pixel pos
        px = x * width  % 256;
        py = y * height % 256;

        // copy to
        uint32_t j = px+py*256;
        
        // copy bufer line by line
        for(int h = 0; h<height; h++) {
            rgb* c = &buf[h*width]
            memcpy(&png[j+h*256], c, width*3);
        }
        
        // next frame
        p++;
    };
    fclose(f);
    
}

