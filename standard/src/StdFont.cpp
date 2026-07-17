/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Class providing a quick bitmap font in DirectDraw created using true type */

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdio.h>
#ifdef _WIN32
#include <DDraw.h>
#endif

#include <Standard.h>
#include <StdSurface.h>
#include <StdFacet.h>
#include <StdFont.h>
#include <StdDDraw.h>

const int FNT_CharSpace = 0;

CStdFont::CStdFont()
	{
	Default();
	}

CStdFont::~CStdFont()
	{
	Clear();
	}

void CStdFont::Default()
	{
	Surface.Default();
	for (int cnt=0; cnt<FNT_MaxChar; cnt++)	Character[cnt].Default();
	}

void CStdFont::Clear()
	{
	Surface.Clear();
	for (int cnt=0; cnt<FNT_MaxChar; cnt++)	Character[cnt].Clear();
	}

#ifndef _WIN32
#include <ft2build.h>
#include FT_FREETYPE_H
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
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) return FALSE;

    FT_Face face;
    bool found = false;

    // If Comic Sans is requested, try local bundled font
    if (SEqualNoCase(szFontname, "Comic Sans MS")) {
        if (!FT_New_Face(ft, "./Comic.ttf", 0, &face)) found = true;
    }

    if (!found) {
        const char *fontPath = "/usr/share/fonts/TTF/Comic.TTF";
        const char* fallbacks[] = {
            "/usr/share/fonts/truetype/msttcorefonts/comic.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            NULL
        };
        if (FT_New_Face(ft, fontPath, 0, &face)) {
            for (int i = 0; fallbacks[i]; i++) {
                if (!FT_New_Face(ft, fallbacks[i], 0, &face)) {
                    found = true;
                    break;
                }
            }
        } else {
            found = true;
        }
    }

    if (!found) { FT_Done_FreeType(ft); return FALSE; }

    // GDI with positive lfHeight = iSize matches FT_Set_Pixel_Sizes(0, iSize)
    // Actually, GDI's tmHeight = tmAscent + tmDescent.
    // FT_Set_Pixel_Sizes sets the EM square size? No, it sets the nominal height.
    FT_Set_Pixel_Sizes(face, 0, iSize);

    int iFontHgt = face->size->metrics.height >> 6;
    if (iFontHgt == 0) iFontHgt = iSize;

    int iSfcWdt=0;
    for (int cnt=0; cnt<FNT_MaxChar; cnt++)
        {
        // Use MONO loading to get hinted, sharp pixels
        if (FT_Load_Char(face, cnt, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO)) {
            Character[cnt].Wdt = iSize / 2; // Fallback
        } else {
            Character[cnt].Wdt = face->glyph->advance.x >> 6;
        }
        iSfcWdt += Character[cnt].Wdt + FNT_CharSpace; 
        }

    if (!Surface.Create(iSfcWdt,(iFontHgt+2)*FNT_MaxCol)) { FT_Done_Face(face); FT_Done_FreeType(ft); return FALSE; }

    for (int cnt=0; cnt<FNT_MaxCol; cnt++) Surface.SetPalette(byColorIndex[cnt],20*cnt+30,10*cnt,5*cnt);
    Surface.AttachPalette();

    if (!Surface.Lock()) {
        FT_Done_Face(face); FT_Done_FreeType(ft);
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
            if (!FT_Load_Char(face, cnt, FT_LOAD_RENDER | FT_LOAD_TARGET_MONO)) {
                FT_Bitmap& bitmap = face->glyph->bitmap;
                int baseline = face->size->metrics.ascender >> 6;
                
                for (int y = 0; y < bitmap.rows; y++) {
                    for (int x = 0; x < bitmap.width; x++) {
                        // Extract bit from MONO bitmap
                        unsigned char byte = bitmap.buffer[y * bitmap.pitch + (x >> 3)];
                        if (byte & (128 >> (x & 7))) {
                            int dst_y = cy + baseline - face->glyph->bitmap_top + y;
                            int dst_x = cx + face->glyph->bitmap_left + x;
                            if (dst_y >= cy && dst_y < cy + iFontHgt && dst_x >= cx && dst_x < cx + Character[cnt].Wdt) {
                                Surface.Bits[dst_y * Surface.Pitch + dst_x] = byColorIndex[cnt2];
                            }
                        }
                    }
                }
            }
            Character[cnt].Set(Surface.Surface,cx,0,Character[cnt].Wdt,iFontHgt);
            cx+=Character[cnt].Wdt+FNT_CharSpace; 
            }
        cy+=iFontHgt+FNT_CharSpace; cx=0;
        }
    Surface.Unlock();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return TRUE;
#endif
	}

HFONT CStdFont::CreateFont(HDC hdc, const char *szFacename, int iPtHeight, int iAttributes)
	{
	HFONT hFont;
	LOGFONT lf;
	SaveDC(hdc);
	SetGraphicsMode(hdc, GM_ADVANCED);
	ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
	SetViewportOrgEx(hdc, 0, 0, NULL);
	SetWindowOrgEx(hdc, 0, 0, NULL);
	lf.lfHeight         = iPtHeight;
	lf.lfWidth          = 0 ;
	lf.lfEscapement     = 0 ;
	lf.lfOrientation    = 0 ;
	lf.lfWeight         = 0 ;
	lf.lfItalic         = 0 ;
	lf.lfUnderline      = 0 ;
	lf.lfStrikeOut      = 0 ;
	lf.lfCharSet        = 0 ;
	lf.lfOutPrecision   = 0 ;
	lf.lfClipPrecision  = 0 ;
	lf.lfQuality        = 0 ;
	lf.lfPitchAndFamily = 0 ;
	strcpy(lf.lfFaceName, szFacename);
	hFont = CreateFontIndirect (&lf);
	RestoreDC(hdc, -1);
	return hFont ;
	}

BOOL CStdFont::CharOut(BYTE chChar, SURFACE sfcSurface, int iX, int iY, int iColor)
	{
	if (iColor!=FTrans)	Character[chChar].Draw(sfcSurface,iX,iY,0,BoundBy(iColor,0,FNT_MaxCol-1));
	return TRUE;
	}

BOOL CStdFont::StringOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm)
	{
	if (!szText) return FALSE;
	// Alignment
	switch (byForm)
		{
		case ACenter: iTx-=GetTextWidth(szText)/2; break;
		case ARight:  iTx-=GetTextWidth(szText);	 break;
		}
	// Draw string
	for (; *szText; szText++)
		{
		CharOut(*((BYTE*)szText),sfcDest,iTx+1,iTy+1,iBCol);
		CharOut(*((BYTE*)szText),sfcDest,iTx,iTy,iFCol);
		iTx+=Character[*((BYTE*)szText)].Wdt;
		}
	return TRUE;
	}

int CStdFont::GetTextWidth(const char *szText)
	{
	if (!szText) return FALSE;
	int iLineWidth=0,iMaxLineWidth=0;
	for (const char *cPos=szText; *cPos; cPos++)
		{
		// Line break
		if (*cPos == '|') iLineWidth=0;
		// Add line width
		else iLineWidth += Character[*((BYTE*)cPos)].Wdt;
		// Get max line
		if (iLineWidth>iMaxLineWidth) iMaxLineWidth=iLineWidth;
		}
	return iMaxLineWidth;
	}

int CStdFont::GetTextHeight(const char *szText)
	{
	if (!szText) return Character[0].Hgt;
	int iLines = 1 + SCharCount('|',szText);
	int iResult=0;
	for (; *szText; szText++) iResult=Max(iResult,Character[*((BYTE*)szText)].Hgt);
	iResult*=iLines; 
	return iResult;
	}

BOOL CStdFont::TextOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm)
	{
	static char szLinebuf[2500+1];
	for (int cnt=0; SCopySegment(szText,cnt,szLinebuf,'|',2500); cnt++,iTy+=GetTextHeight())
		StringOut(szLinebuf,sfcDest,iTx,iTy,iFCol,iBCol,byForm);
	return TRUE;
	}

BOOL CStdFont::TextExtent(const char *szText, int &rWdt, int &rHgt)
	{
	rWdt = GetTextWidth(szText); rHgt = GetTextHeight(szText);
	return TRUE;
	}
