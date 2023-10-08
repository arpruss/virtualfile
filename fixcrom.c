#include <stdio.h>
#include <string.h>

void deoptimize_sprites_0x80(unsigned char* buf, unsigned char* tmp) {
    memset(tmp, 0, 0x80);
    for (int y = 0 ; y < 0x10 ; y++) {
        unsigned long dstData = ( buf[(y*8)+0] <<  0 |
                  buf[(y*8)+1] <<  8 |
                  buf[(y*8)+2] << 16 |
                  buf[(y*8)+3] << 24 );
        for (int x = 0 ; x < 8 ; x++) {
            tmp[(0x43 | y << 2)] |= (dstData >> x*4+3 & 1) << 7-x;
            tmp[(0x41 | y << 2)] |= (dstData >> x*4+2 & 1) << 7-x;
            tmp[(0x42 | y << 2)] |= (dstData >> x*4+1 & 1) << 7-x;
            tmp[(0x40 | y << 2)] |= (dstData >> x*4+0 & 1) << 7-x;
        }

        dstData = ( buf[(y*8)+4] <<  0 |
                  buf[(y*8)+5] <<  8 |
                  buf[(y*8)+6] << 16 |
                  buf[(y*8)+7] << 24 );
                  
        for (int x = 0 ; x < 8 ; x++) {
            tmp[(0x03 | y << 2)] |= (dstData >> x*4+3 & 1) << 7-x;
            tmp[(0x01 | y << 2)] |= (dstData >> x*4+2 & 1) << 7-x;
            tmp[(0x02 | y << 2)] |= (dstData >> x*4+1 & 1) << 7-x;
            tmp[(0x00 | y << 2)] |= (dstData >> x*4+0 & 1) << 7-x;
        }
    }
}

int main(int argc, char** argv) {
    char src[0x80];
    char dst[0x80];

    FILE* in = fopen(argv[1],"rb");
    FILE* out;
    if (argc < 3) {
        fdopen(dup(fileno(stdout)), "wb");
        out = stdout;
    }
    else 
        out = fopen(argv[2],"wb");
    
    while(1==fread(src, sizeof(src), 1, in)) {
        deoptimize_sprites_0x80(src, dst);
        fwrite(dst, sizeof(dst), 1, out);
    }
    
    fclose(out);
    fclose(in);
}

