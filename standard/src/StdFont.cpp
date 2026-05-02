/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Class providing a quick bitmap font in DirectDraw created using true type */

#include <Windows.h>
#include <StdIO.h>
#include <DDraw.h>

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

BOOL CStdFont::Init(HDC hdc, const char *szFontname, int iSize)
	{
	BYTE byColorIndex[FNT_MaxCol]={31,16,39,47,55,63,71,79,87,95};
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
	for (cnt=0; cnt<FNT_MaxCol; cnt++) Surface.SetPalette(byColorIndex[cnt],20*cnt+30,10*cnt,5*cnt);
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
		for (cnt=0; cnt<FNT_MaxChar; cnt++)
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
	for (szText; *szText; szText++)
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
	for (const char *cPos=szText; *cPos; *cPos++)
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
	for (int iResult=0; *szText; szText++) iResult=Max(iResult,Character[*((BYTE*)szText)].Hgt);
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
