#define STB_TRUETYPE_IMPLEMENTATION
#include "standard/inc/external/stb_truetype.h"
#include <stdio.h>
#include <stdlib.h>

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

    int iSize = 10;
    float scale = stbtt_ScaleForMappingEmToPixels(&font, iSize * 96.0f / 72.0f);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

    printf("Scale: %f\n", scale);
    printf("Total height (ascent-descent)*scale: %f\n", (ascent - descent) * scale);
    
    int width, height, xoff, yoff;
    unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, 'A', &width, &height, &xoff, &yoff);
    printf("Bitmap height: %d\n", height);
    stbtt_FreeBitmap(bitmap, NULL);

    return 0;
}
