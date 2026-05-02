#define STB_TRUETYPE_IMPLEMENTATION
#include "standard/inc/external/stb_truetype.h"
#include <stdio.h>
#include <stdlib.h>

void dump_char(stbtt_fontinfo* font, float scale, int ch, int threshold) {
    int width, height, xoff, yoff;
    unsigned char* bitmap = stbtt_GetCodepointBitmap(font, 0, scale, ch, &width, &height, &xoff, &yoff);
    printf("Threshold %d:\n", threshold);
    if (!bitmap) { printf("No bitmap\n\n"); return; }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (bitmap[y * width + x] > threshold) printf("#");
            else printf(".");
        }
        printf("\n");
    }
    stbtt_FreeBitmap(bitmap, NULL);
    printf("\n");
}

int main() {
    const char *fontPath = "/usr/share/fonts/TTF/Comic.TTF";
    FILE* f = fopen(fontPath, "rb");
    if (!f) return 1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* ttf_buffer = (unsigned char*)malloc(size);
    fread(ttf_buffer, 1, size, f);
    fclose(f);

    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

    float scale = stbtt_ScaleForPixelHeight(&font, 10 * 96.0f / 72.0f);
    printf("Scale: %f\n", scale);
    
    dump_char(&font, scale, 'A', 32);
    dump_char(&font, scale, 'A', 127);
    dump_char(&font, scale, 'A', 160);

    return 0;
}
