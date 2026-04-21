/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper class to DirectDraw */

const int MaxResX = 1024, MaxResY = 768;

const int CBlack=0,CGray1=1,CGray2=2,CGray3=3,CGray4=4,CGray5=5,CWhite=6,
					CDRed=7,CDGreen=8,CDBlue=9,CRed=10,CGreen=11,CLBlue=12,CYellow=13,CBlue=14;

class CStdDDraw  
	{
	public:
		CStdDDraw();
		~CStdDDraw();
	public:
		SURFACE lpPrimary;
		SURFACE lpBack;
		int ClipX1,ClipX2,ClipY1,ClipY2;
		int StoreClipX1,StoreClipX2,StoreClipY1,StoreClipY2;
	protected:
		LPDIRECTDRAW        lpDirectDraw;
		LPDIRECTDRAWPALETTE	lpPalette;
		LPDIRECTDRAWCLIPPER	lpClipper;
		LPDIRECTDRAWCLIPPER	lpWindowClipper;
		BOOL fBackAttached;
		BOOL fPageLock;
		CStdFont Font;
		BYTE *Pattern;
		int PatWdt,PatHgt,PatWdtMod,PatHgtMod;
		BOOL PatQuickMod;
	public:
		// General
		BOOL Init(HWND hWnd, BOOL Fullscreen, int iResX, int iResY, BOOL fUsePageLock);
		void Clear();
		void Default();
		BOOL PageFlip();
		// Palette		
		BOOL SetPrimaryPalette(BYTE *pBuf);
		BOOL SetPrimaryPaletteQuad(BYTE *pBuf);
		BOOL AttachPrimaryPalette(SURFACE sfcSurface);
		// Clipper
		BOOL GetPrimaryClipper(int &rX1, int &rY1, int &rX2, int &rY2);
		BOOL SetPrimaryClipper(int iX1, int iY1, int iX2, int iY2);
		BOOL SubPrimaryClipper(int iX1, int iY1, int iX2, int iY2);
		BOOL RestorePrimaryClipper();
		BOOL NoPrimaryClipper();
		BOOL ApplyPrimaryClipper(SURFACE sfcSurface);
		BOOL DetachPrimaryClipper(SURFACE sfcSurface);
		// Surface
		BYTE *LockSurface(SURFACE sfcSurface, int &lPitch, int *lpSfcWdt=0, int *lpSfcHgt=0);
		void UnLockSurface(SURFACE sfcSurface);
		BOOL GetSurfaceSize(SURFACE sfcSurface, int &iWdt, int &iHgt);
		HDC GetSurfaceDC(SURFACE sfcSurface);
		BOOL WipeSurface(SURFACE sfcSurface);
		SURFACE DuplicateSurface(SURFACE sfcSurface);
		void DestroySurface(SURFACE sfcSurface);
		SURFACE CreateSurface(int iWdt, int iHgt);
		void SurfaceShiftColor(SURFACE sfcSfc, int iShift);
		void SurfaceShiftColorRange(SURFACE sfcSfc, int iRngLo, int iRngHi, int iShift);
		void SurfaceAllowColor(SURFACE sfcSfc, BYTE iRngLo, BYTE iRngHi, BOOL fAllowZero=FALSE);
		// Blit
		BOOL BlitFast(SURFACE sfcSource, int fx, int fy, 
									SURFACE sfcTarget, int tx, int ty, int wdt, int hgt);
		BOOL Blit(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt, 
							SURFACE sfcTarget, int tx, int ty, int twdt, int thgt, 
							BOOL fSrcColKey=FALSE);
		BOOL BlitRotate(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt,
										SURFACE sfcTarget, int tx, int ty, int twdt, int thgt,
										int iAngle);
		BOOL BlitRotateOriginal(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt,
										SURFACE sfcTarget, int tx, int ty, int twdt, int thgt,
										int iAngle);
		BOOL BlitSurface(SURFACE sfcSurface, SURFACE sfcTarget, int tx, int ty);
		BOOL BlitSurfaceTile(SURFACE sfcSurface, SURFACE sfcTarget, int iToX, int iToY, int iToWdt, int iToHgt, int iOffsetX=0, int iOffsetY=0, BOOL fSrcColKey=FALSE);
		BOOL BlitSurface2Window(SURFACE sfcSource, int fX, int fY, int fWdt, int fHgt, HWND hWnd, int tX, int tY, int tWdt, int tHgt);
		// Text
		BOOL InitFont(const char *szFontName, int iSize);	
		BOOL TextExtent(const char *szString, int &rWdt, int &rHgt);
		int TextHeight(const char *szText=NULL);
		int TextWidth(const char *szText=NULL);
		BOOL TextOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol=FWhite, int iBCol=FBlack, BYTE byForm=ALeft);
		BOOL StringOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol=FWhite, int iBCol=FBlack, BYTE byForm=ALeft);
		// Drawing
		BOOL SetPixel(SURFACE sfcDest, int tx, int ty, BYTE col);
		BYTE GetPixel(SURFACE sfcSource, int fx, int fy);
		void DrawBox(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE col);
		void DrawBoxColorTable(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE *bypColorTable);
		void DrawCircle(SURFACE sfcDest, int x, int y, int r, BYTE col);
		void DrawHorizontalLine(SURFACE sfcDest, int x1, int x2, int y, BYTE col);
		void DrawVerticalLine(SURFACE sfcDest, int x, int y1, int y2, BYTE col);
		void DrawFrame(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE col);
		void DrawInline(SURFACE sfcDest, int iX1, int iY1, int iX2, int iY2, BYTE byCol, BYTE byOnCol, BYTE byAdjacentCol);
		BOOL DrawLine(SURFACE sfcTarget, int x1, int y1, int x2, int y2, BYTE byCol);
		BOOL DrawPolygon(SURFACE sfcTarget, int iNum, int *ipVtx, int iCol);
		// Pattern
		BOOL DefinePattern(SURFACE sfcSource);
		void NoPattern();

	protected:
		BOOL CreatePrimaryPalette(SURFACE sfcAttachTo);
		BOOL CreatePrimaryClipper();
		BOOL CreatePrimarySurfaces(BOOL fFlipAttach);
		BOOL Error(const char *szMsg);
		BOOL SetDisplayMode(int iResX, int iResY, int iColorDepth);
		BOOL SetCooperativeLevel(HWND hWnd, DWORD dwLevel);
		BOOL CreateDirectDraw();
		int SfcCall(HRESULT ddrval);
		BOOL SurfaceSetColorKey(SURFACE sfcSurface, BYTE byCol);		
	};


// Global DDraw access pointer
extern CStdDDraw *lpDDraw;
