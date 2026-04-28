/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper class to DirectDraw surfaces */

typedef LPDIRECTDRAWSURFACE2 SURFACE;

class CSurface  
	{
	public:
		CSurface();
		~CSurface();
	public:
		int Wdt,Hgt,Pitch;
		int ClipX,ClipY,ClipX2,ClipY2;
		BYTE *Bits;
		SURFACE Surface;
		BYTE Palette[768];
	protected:
		int Locked;
		BOOL Attached;
	public:
		BOOL Load(const char *szFilename);
		BOOL Load(BYTE *bpBitmap);
		void SetPalette(BYTE *bpPalette, BOOL fAdapt);
		BOOL SetPalette(int iColor, int iRed, int iGreen, int iBlue);
		BOOL Pix(int iX, int iY, int iCol);
		BOOL HLine(int iX, int iX2, int iY, int iCol);
		BOOL Polygon(int iNum, int *ipVtx, int iCol);
		BOOL Box(int iX, int iY, int iX2, int iY2, int iCol);
		BOOL Unlock();
		BOOL Lock();
		BOOL Create(int iWdt, int iHgt);
		BOOL Attach(SURFACE sfcSurface);
		void Clear();
		void Default();
		void Clip(int iX, int iY, int iX2, int iY2);
		void NoClip();
		HDC GetDC();
		void ReleaseDC(HDC hdc);
		BOOL Save(const char *szFilename);
		BOOL AttachPalette();
	protected:
		void MapBytes(BYTE *bpMap);
		BOOL ReadBytes(BYTE **lpbpData, void *bpTarget, int iSize);
	};

