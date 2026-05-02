#include <windows.h>
#include <stdio.h>

int main() {
    HDC hdc = CreateCompatibleDC(NULL);
    SetMapMode(hdc, MM_TEXT);
    HFONT hFont = CreateFontA(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Comic Sans MS");
    SelectObject(hdc, hFont);
    
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    printf("Height: %d, Ascent: %d, Descent: %d, Internal Leading: %d\n", tm.tmHeight, tm.tmAscent, tm.tmDescent, tm.tmInternalLeading);
    
    SIZE sz;
    GetTextExtentPoint32A(hdc, "A", 1, &sz);
    printf("A width: %d, height: %d\n", sz.cx, sz.cy);
    
    return 0;
}
