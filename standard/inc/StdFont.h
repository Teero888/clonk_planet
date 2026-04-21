/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Class providing a quick bitmap font in DirectDraw created using true type */

const int FNT_MaxChar = 256,
					FNT_MaxCol	= 10;

const int ALeft=0,ACenter=1,ARight=2;

const int FTrans=-1,FWhite=0,FBlack=1,FRed=3,FPlayer=2;

class CStdDDraw;

class CStdFont  
	{
	friend CStdDDraw;
	public:
		CStdFont();
		~CStdFont();
	protected:
		CSurface Surface;
		CFacet Character[FNT_MaxChar];
	protected:
		BOOL TextExtent(const char *szText, int &rWdt, int &rHgt);
		BOOL TextOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol=FWhite, int iBCol=FBlack, BYTE byForm=ALeft);
		BOOL StringOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol=FWhite, int iBCol=FBlack, BYTE byForm=ALeft);
		BOOL CharOut(BYTE chChar, SURFACE sfcSurface, int iX, int iY, int iColor);
		int GetTextHeight(const char *szText=NULL);
		int GetTextWidth(const char *szText);				
		BOOL Init(HDC hDC, const char *szFontname, int iSize);
		void Clear();
		void Default();
		HFONT CreateFont(HDC hdc, const char *szFacename, int iPtHeight, int iAttributes);
	};
