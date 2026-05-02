#ifndef _WIN32
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#endif

BOOL CStdFont::Init(HDC hdc, const char *szFontname, int iSize)
	{
	BYTE byColorIndex[FNT_MaxCol]={31,16,39,47,55,63,71,79,87,95};
#ifdef _WIN32
	char szChar[2];
	
	// Create windows font
	HFONT hFont = CreateFont(hdc,szFontname,iSize,0);
	if (!hFont) return FALSE;
	SelectObject(hdc,hFont);
	
	// Get character & surface extent
	SIZE csize;
	int iSfcWdt=0,iFontHgt=0;
	for (int cnt=0; cnt<FNT_MaxChar; cnt++)
		{
		szChar[0]=cnt; szChar[1]=0;
		if (GetTextExtentPoint32(hdc,szChar,1,&csize)) 
			{ 
			Character[cnt].Wdt=csize.cx;
			iSfcWdt+=csize.cx+FNT_CharSpace; iFontHgt=Max(iFontHgt,csize.cy); 
			}
		}

	// Create font surface
	if (!Surface.Create(iSfcWdt,(iFontHgt+2)*FNT_MaxCol)) { DeleteObject(hFont); return FALSE; }
	// Set font index colors
	for (int cnt=0; cnt<FNT_MaxCol; cnt++) Surface.SetPalette(byColorIndex[cnt],20*cnt+30,10*cnt,5*cnt);
	Surface.AttachPalette();

	// Set & draw characters
	int cx=0,cy=0;
	HDC hdc2 = Surface.GetDC();
	if (!hdc2) { DeleteObject(hFont); return FALSE; }
	SelectObject(hdc2,hFont);
	SetTextAlign(hdc2,TA_LEFT); 
  SetBkMode(hdc2,TRANSPARENT);
	for (int cnt2=0; cnt2<FNT_MaxCol; cnt2++)
		{
		SetTextColor(hdc2,RGB(20*cnt2+30,10*cnt2,5*cnt2));
		for (int cnt=0; cnt<FNT_MaxChar; cnt++)
			{
			szChar[0]=cnt; szChar[1]=0;
			RECT rect; rect.left=cx; rect.top=cy; rect.right=cx+Character[cnt].Wdt; rect.bottom=cy+iFontHgt;
			BOOL fResult = ExtTextOut(hdc2,cx,cy,ETO_CLIPPED,&rect,szChar,1,NULL);
			Character[cnt].Set(Surface.Surface,cx,0,Character[cnt].Wdt,iFontHgt);
			cx+=Character[cnt].Wdt+FNT_CharSpace; 
			}
		cy+=iFontHgt+FNT_CharSpace; cx=0;
		}
	Surface.ReleaseDC(hdc2);

	// Delete windows font
  DeleteObject(hFont);
	
	// Success 
	return TRUE;
#else
    const char *fontPath = "/usr/share/fonts/TTF/Comic.TTF";
    FILE* f = fopen(fontPath, "rb");
    if (!f) {
        const char* fallbacks[] = {
            "/usr/share/fonts/truetype/msttcorefonts/comic.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            NULL
        };
        for (int i = 0; fallbacks[i]; i++) {
            f = fopen(fallbacks[i], "rb");
            if (f) break;
        }
    }
    if (!f) return FALSE;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* ttf_buffer = new unsigned char[size];
    fread(ttf_buffer, 1, size, f);
    fclose(f);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0))) {
        delete[] ttf_buffer;
        return FALSE;
    }

    float scale = stbtt_ScaleForPixelHeight(&font, iSize * 96.0f / 72.0f);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    int iFontHgt = (ascent - descent) * scale;
    if (iFontHgt == 0) iFontHgt = iSize;

    int iSfcWdt=0;
    for (int cnt=0; cnt<FNT_MaxChar; cnt++)
        {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, cnt, &advance, &lsb);
        Character[cnt].Wdt = advance * scale;
        if (Character[cnt].Wdt == 0) {
            stbtt_GetCodepointHMetrics(&font, 'x', &advance, &lsb);
            Character[cnt].Wdt = advance * scale;
            if (Character[cnt].Wdt == 0) Character[cnt].Wdt = iSize;
        }
        iSfcWdt += Character[cnt].Wdt + FNT_CharSpace; 
        }

    if (!Surface.Create(iSfcWdt,(iFontHgt+2)*FNT_MaxCol)) { delete[] ttf_buffer; return FALSE; }

    for (int cnt=0; cnt<FNT_MaxCol; cnt++) Surface.SetPalette(byColorIndex[cnt],20*cnt+30,10*cnt,5*cnt);
    Surface.AttachPalette();

    if (!Surface.Lock()) {
        delete[] ttf_buffer;
        return FALSE;
    }

    for (int y = 0; y < (iFontHgt+2)*FNT_MaxCol; y++) {
        memset(Surface.Bits + y * Surface.Pitch, 0, iSfcWdt);
    }

    int cx=0,cy=0;
    for (int cnt2=0; cnt2<FNT_MaxCol; cnt2++)
        {
        for (int cnt=0; cnt<FNT_MaxChar; cnt++)
            {
            int width, height, xoff, yoff;
            unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, cnt, &width, &height, &xoff, &yoff);
            if (bitmap) {
                int baseline = ascent * scale;
                for (int y = 0; y < height; y++) {
                    int dst_y = cy + baseline + yoff + y;
                    int dst_x = cx + xoff;
                    if (dst_y >= cy && dst_y < cy + iFontHgt && dst_x >= cx && dst_x < cx + Character[cnt].Wdt) {
                        for (int x = 0; x < width; x++) {
                            if (dst_x + x < cx + Character[cnt].Wdt && bitmap[y * width + x] > 127) {
                                Surface.Bits[dst_y * Surface.Pitch + dst_x + x] = byColorIndex[cnt2];
                            }
                        }
                    }
                }
                stbtt_FreeBitmap(bitmap, NULL);
            }
            Character[cnt].Set(Surface.Surface,cx,0,Character[cnt].Wdt,iFontHgt);
            cx+=Character[cnt].Wdt+FNT_CharSpace; 
            }
        cy+=iFontHgt+FNT_CharSpace; cx=0;
        }
    Surface.Unlock();
    delete[] ttf_buffer;
    return TRUE;
#endif
	}
