/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper class to DirectDraw */

#include <Windows.h>
#include <Stdio.h>
#include <Math.h>
#include <DDraw.h>
#include <Limits.h>

#include <Standard.h>
#include <StdSurface.h>
#include <StdFacet.h>
#include <StdFont.h>
#include <StdDDraw.h>

// Global access pointer
CStdDDraw *lpDDraw;

inline void SetRect(RECT &rect, int left, int top, int right, int bottom)
  {
  rect.left=left; rect.top=top; rect.bottom=bottom; rect.right=right;
  }

CStdDDraw::CStdDDraw()
	{
	Default();
	}

CStdDDraw::~CStdDDraw()
	{
	Clear();
	}

void CStdDDraw::Default()
	{
	lpPrimary=NULL;
	lpBack=NULL;
	lpDirectDraw=NULL;
	lpPalette=NULL;
	lpClipper=NULL;
	lpWindowClipper=NULL;
	fBackAttached=FALSE;
	fPageLock=FALSE;
	Font.Default();
	ClipX1=ClipY1=INT_MIN;
	ClipX2=ClipY2=INT_MAX;
	StoreClipX1=ClipX1; StoreClipY1=ClipY1; StoreClipX2=ClipX2; StoreClipY2=ClipY2;
	Pattern=NULL;
	PatQuickMod=FALSE;
	}

void CStdDDraw::Clear()
	{
	Font.Clear();
	NoPattern();
  if (lpDirectDraw)
    {
    if (lpPalette) { lpPalette->Release(); lpPalette=NULL; }
    if (lpClipper) { lpClipper->Release(); lpClipper=NULL; }
    if (lpWindowClipper) { lpWindowClipper->Release(); lpWindowClipper=NULL; }
    if (!fBackAttached) 
			if (lpBack) { lpBack->Release(); lpBack=NULL; }
    if (lpPrimary) { lpPrimary->Release(); lpPrimary=NULL; }
    lpDirectDraw->Release(); lpDirectDraw=NULL;
    }
	lpDDraw=NULL;
	}

BOOL CStdDDraw::PageFlip()
	{
  int iCall;
  while (!(iCall=SfcCall(lpPrimary->Flip(NULL,0))));
	return TRUE;
	}

SURFACE CStdDDraw::CreateSurface(int iWdt, int iHgt)
	{
	LPDIRECTDRAWSURFACE sfcSurface1;
  SURFACE sfcSurface2;
	// Pixel format	
	DDPIXELFORMAT	ddpf;
  memset( &ddpf, 0, sizeof(ddpf) );
	ddpf.dwSize = sizeof( ddpf );
	ddpf.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
	ddpf.dwRGBBitCount = 8;
	// Surface desc
  DDSURFACEDESC ddsd;
  memset( &ddsd, 0, sizeof(ddsd) );
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  ddsd.dwHeight = iHgt;
  ddsd.dwWidth = iWdt;
	ddsd.ddpfPixelFormat = ddpf;	
	// Create surface
  if (lpDirectDraw->CreateSurface(&ddsd,&sfcSurface1,NULL)!=DD_OK) 
		return NULL;
	// Get interface
  if (sfcSurface1->QueryInterface(IID_IDirectDrawSurface2,(void**)&sfcSurface2)!=DD_OK) 
		{ sfcSurface1->Release(); return NULL; }
	sfcSurface1->Release();
	// Set color key
  if (!SurfaceSetColorKey(sfcSurface2,0))
    { DestroySurface(sfcSurface2); return NULL; }
  return sfcSurface2; 
	}

void CStdDDraw::DestroySurface(SURFACE sfcSurface)
	{
	sfcSurface->Release();
	}

SURFACE CStdDDraw::DuplicateSurface(SURFACE sfcSurface)
	{
  SURFACE sfcNew;
  int iWdt,iHgt;
  if (!GetSurfaceSize(sfcSurface,iWdt,iHgt)) return NULL;
  if (!(sfcNew=CreateSurface(iWdt,iHgt))) return NULL;
  if (!BlitSurface(sfcSurface,sfcNew,0,0))
    { DestroySurface(sfcNew); return NULL; }
  return sfcNew;
	}

BOOL CStdDDraw::WipeSurface(SURFACE sfcSurface)
	{
  int iCall;
  DDBLTFX ddbltfx;
  ddbltfx.dwSize=sizeof(ddbltfx);
  ddbltfx.dwFillColor=0;
  while (!(iCall=SfcCall(sfcSurface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx))));
  if (iCall<0) return FALSE;
  return TRUE;
	}

BOOL CStdDDraw::GetSurfaceSize(SURFACE sfcSurface, int &iWdt, int &iHgt)
	{
	if (!sfcSurface) return FALSE;
  DDSURFACEDESC ddsd;
  int iCall;
  ddsd.dwSize=sizeof(ddsd);
  while (!(iCall=SfcCall(sfcSurface->GetSurfaceDesc(&ddsd))));
  if (iCall<0) return FALSE;
  iWdt=ddsd.dwWidth;
  iHgt=ddsd.dwHeight;
	return TRUE;
	}

BOOL CStdDDraw::SetPrimaryPalette(BYTE *pBuf)
	{
  int cnt;
  PALETTEENTRY newpal[256];
  // Set new palette entries
  for (cnt=0; cnt< 256; cnt++)
    {
    newpal[cnt].peRed   = pBuf[cnt*3+0];
    newpal[cnt].peGreen = pBuf[cnt*3+1];
    newpal[cnt].peBlue  = pBuf[cnt*3+2];
    newpal[cnt].peFlags = 0;
    }
  // Make changes to palette object
  lpPalette->SetEntries(0,0,256,newpal);
	return TRUE;
	}

BOOL CStdDDraw::SetPrimaryPaletteQuad(BYTE *pBuf)
	{
  int cnt;
  PALETTEENTRY newpal[256];
  // Set new palette entries
  for (cnt=0; cnt< 256; cnt++)
    {
    newpal[cnt].peRed   = pBuf[cnt*4+2];
    newpal[cnt].peGreen = pBuf[cnt*4+1];
    newpal[cnt].peBlue  = pBuf[cnt*4+0];
    newpal[cnt].peFlags = 0;
    }
  // Make changes to palette object
  lpPalette->SetEntries(0,0,256,newpal);
	return TRUE;
	}

BOOL CStdDDraw::SubPrimaryClipper(int iX1, int iY1, int iX2, int iY2)
	{
	// Store current primary clipper
	GetPrimaryClipper(StoreClipX1,StoreClipY1,StoreClipX2,StoreClipY2);
	// Set sub primary clipper
	SetPrimaryClipper(Max(iX1,StoreClipX1),Max(iY1,StoreClipY1),Min(iX2,StoreClipX2),Min(iY2,StoreClipY2));
	return TRUE;
	}

BOOL CStdDDraw::RestorePrimaryClipper()
	{
	// Restore primary clipper
	SetPrimaryClipper(StoreClipX1,StoreClipY1,StoreClipX2,StoreClipY2);
	return TRUE;
	}

BOOL CStdDDraw::SetPrimaryClipper(int iX1, int iY1, int iX2, int iY2)
	{
	// Set drawing surface clipper object
	struct RGNDATA1RECT { RGNDATAHEADER rdh; RECT rect;	};	
	RGNDATA1RECT rangedata;
  rangedata.rdh.dwSize=sizeof(RGNDATAHEADER);
  rangedata.rdh.iType=RDH_RECTANGLES;
  rangedata.rdh.nCount=1;
  rangedata.rdh.nRgnSize=sizeof(RECT);
	SetRect(rangedata.rdh.rcBound,iX1,iY1,iX2+1,iY2+1);
  SetRect(rangedata.rect,iX1,iY1,iX2+1,iY2+1);
  lpClipper->SetClipList( (RGNDATA*) &rangedata,0);
  lpBack->SetClipper(lpClipper);
	// Store drawing clip values
	ClipX1=iX1; ClipY1=iY1;	ClipX2=iX2; ClipY2=iY2;
	// Done	
	return TRUE;
	}

BOOL CStdDDraw::ApplyPrimaryClipper(SURFACE sfcSurface)
	{
	sfcSurface->SetClipper(lpClipper);
	return TRUE;
	}

BOOL CStdDDraw::DetachPrimaryClipper(SURFACE sfcSurface)
	{
	sfcSurface->SetClipper(NULL);
	return TRUE;
	}

BOOL CStdDDraw::NoPrimaryClipper()
	{
	// Remove clipper object from drawing surface
  lpBack->SetClipper(NULL);
	// Store drawing clip values
	ClipX1=ClipY1=INT_MIN;
	ClipX2=ClipY2=INT_MAX;
	// Done
	return TRUE;
	}

BYTE* CStdDDraw::LockSurface(SURFACE sfcSurface, int &lPitch, int *lpSfcWdt, int *lpSfcHgt)
	{
  DDSURFACEDESC DDSDesc;
  int iCall;
  // Init surface descriptor size
  DDSDesc.dwSize=sizeof(DDSDesc);
  // Surface lock loop
  while (!(iCall=SfcCall(sfcSurface->Lock(NULL,&DDSDesc,0,NULL))));
  if (iCall<0) return NULL;
  // Get surface size (assuming valid!)
  if (lpSfcWdt) *lpSfcWdt=DDSDesc.dwWidth;
  if (lpSfcHgt) *lpSfcHgt=DDSDesc.dwHeight;
  // Get Pitch
  lPitch=DDSDesc.dwWidth;
  if (DDSDesc.dwFlags & DDSD_PITCH) lPitch=DDSDesc.lPitch;
  // Return data pointer
  return (BYTE*) DDSDesc.lpSurface;
	}

void CStdDDraw::UnLockSurface(SURFACE sfcSurface)
	{
  sfcSurface->Unlock(NULL);
	}

BOOL CStdDDraw::BlitFast(SURFACE sfcSource, int fx, int fy, 
								SURFACE sfcTarget, int tx, int ty, int wdt, int hgt)
	{
  int iCall;
  RECT frect; SetRect(frect,fx,fy,fx+wdt,fy+hgt);
  while (!(iCall=SfcCall(sfcTarget->BltFast(tx,ty,sfcSource,&frect,DDBLTFAST_NOCOLORKEY))));
  if (iCall<0) return FALSE;
  return TRUE;
	}

BOOL CStdDDraw::Blit(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt, 
										 SURFACE sfcTarget, int tx, int ty, int twdt, int thgt, 
										 BOOL fSrcColKey)
	{
  int iCall;
  RECT frect,trect;
  SetRect(frect,fx,fy,fx+fwdt,fy+fhgt);
  SetRect(trect,tx,ty,tx+twdt,ty+thgt);
	if (fPageLock) 
		{
		sfcSource->PageLock(0);
		while (!(iCall=SfcCall(sfcTarget->Blt(&trect,sfcSource,&frect,fSrcColKey ? DDBLT_KEYSRC : 0,0))));	
		sfcSource->PageUnlock(0);
		}
	else
		{
		while (!(iCall=SfcCall(sfcTarget->Blt(&trect,sfcSource,&frect,fSrcColKey ? DDBLT_KEYSRC : 0,0))));
		}
	if (iCall<0) return FALSE;
  return TRUE;
	}

BOOL CStdDDraw::BlitRotate(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt,
								SURFACE sfcTarget, int tx, int ty, int twdt, int thgt,
								int iAngle)
	{
  // Object is first stretched to dest rect, then rotated at place.
  BYTE *bypSource,*bypDest;
	BYTE bPix;
  int lSrcPitch,lDestPitch,lDestSize,lDestWdt,lDestHgt;
  int xcnt,ycnt,fcx,fcy,tcx,tcy,cpcx,cpcy;
  BYTE *cpix;
  const double pi = 3.1415926535;
  int npcx,npcy;
  double mtx[4],dang;
  if (!fwdt || !fhgt || !twdt || !thgt) return FALSE;
  // Lock the surfaces
  if (!(bypSource=LockSurface(sfcSource,lSrcPitch))) 
    return FALSE;
  if (!(bypDest=LockSurface(sfcTarget,lDestPitch,&lDestWdt,&lDestHgt)))
    { UnLockSurface(sfcSource); return FALSE; }
	lDestSize=lDestPitch*lDestHgt;
	if (!lDestSize) return FALSE;
  // Rectangle centers
  fcx=fwdt/2; fcy=fhgt/2;
  tcx=twdt/2; tcy=thgt/2;
	// Adjust angle range
	while (iAngle<0) iAngle+=36000; while (iAngle>35999) iAngle-=36000;

	// Exact/free rotation
	switch (iAngle)
		{
		case 0:	
			for (ycnt=0; ycnt<thgt; ycnt++)
				if (Inside(cpcy=ty+tcy-thgt/2+ycnt,0,lDestHgt-1))
					for (xcnt=0; xcnt<twdt; xcnt++)
						if (Inside(cpcx=tx+tcx-twdt/2+xcnt,0,lDestWdt-1))
							if (bPix=bypSource[lSrcPitch*(ycnt*fhgt/thgt)+(xcnt*fwdt/twdt)])
								bypDest[lDestPitch*cpcy+cpcx]=bPix;
			break;

		case 9000:
			for (ycnt=0; ycnt<thgt; ycnt++)
				if (Inside(cpcx=ty+tcy+thgt/2-ycnt,0,lDestWdt-1))
					for (xcnt=0; xcnt<twdt; xcnt++)
						if (Inside(cpcy=tx+tcx-twdt/2+xcnt,0,lDestHgt-1))
							if (bPix=bypSource[lSrcPitch*(ycnt*fhgt/thgt)+(xcnt*fwdt/twdt)])
								bypDest[lDestPitch*cpcy+cpcx]=bPix;
			break;

		case 18000:
			for (ycnt=0; ycnt<thgt; ycnt++)
				if (Inside(cpcy=ty+tcy+thgt/2-ycnt,0,lDestHgt-1))
					for (xcnt=0; xcnt<twdt; xcnt++)
						if (Inside(cpcx=tx+tcx+twdt/2-xcnt,0,lDestWdt-1))
							if (bPix=bypSource[lSrcPitch*(ycnt*fhgt/thgt)+(xcnt*fwdt/twdt)])
								bypDest[lDestPitch*cpcy+cpcx]=bPix;
			break;

		case 27000:
			for (ycnt=0; ycnt<thgt; ycnt++)
				if (Inside(cpcx=ty+tcy-thgt/2+ycnt,0,lDestWdt-1))
					for (xcnt=0; xcnt<twdt; xcnt++)
						if (Inside(cpcy=tx+tcx+twdt/2-xcnt,0,lDestHgt-1))
							if (bPix=bypSource[lSrcPitch*(ycnt*fhgt/thgt)+(xcnt*fwdt/twdt)])
								bypDest[lDestPitch*cpcy+cpcx]=bPix;
			break;

		default:
			// Calculate rotation matrix
			dang=pi*iAngle/18000.0;
			mtx[0]=cos(dang); mtx[1]=-sin(dang);
			mtx[2]=sin(dang); mtx[3]= cos(dang);  
			// Blit source rect
			for (ycnt=0; ycnt<fhgt; ycnt++)
				{
				// Source line start
				cpix=bypSource+lSrcPitch*(fy+ycnt)+fx;
				for (xcnt=0; xcnt<fwdt; xcnt++,cpix++)
					{
					// Source line pixel not transparent
					if (*cpix)
						{
						// Current pixel coordinate as from source
						cpcx=xcnt-fcx; cpcy=ycnt-fcy;
						// Convert to coordinate as in dest
						cpcx=cpcx*twdt/fwdt; cpcy=cpcy*thgt/fhgt;
						// Rotate current pixel coordinate
						npcx= (int) ( mtx[0]*cpcx + mtx[1]*cpcy );
						npcy= (int) ( mtx[2]*cpcx + mtx[3]*cpcy );
						// Place in dest
						bypDest[BoundBy(lDestPitch*(ty+tcy+npcy)+tx+tcx+npcx,0,lDestSize-1)]=*cpix;
						bypDest[BoundBy(lDestPitch*(ty+tcy+npcy)+tx+tcx+npcx+1,0,lDestSize-1)]=*cpix;
						}
					}
				}  
			break;
		}

  // Unlock the surfaces
  UnLockSurface(sfcSource);
  UnLockSurface(sfcTarget);

  return TRUE;
	}

BOOL CStdDDraw::CreateDirectDraw()
	{
  if (DirectDrawCreate(NULL,&lpDirectDraw,NULL)!=DD_OK) return FALSE;
	return TRUE;
	}

BOOL CStdDDraw::SetCooperativeLevel(HWND hWnd, DWORD dwLevel)
	{
	if (lpDirectDraw->SetCooperativeLevel(hWnd,dwLevel)!=DD_OK) return FALSE;
	return TRUE;
	}

BOOL CStdDDraw::SetDisplayMode(int iResX, int iResY, int iColorDepth)
	{
	if (lpDirectDraw->SetDisplayMode(iResX,iResY,iColorDepth)!=DD_OK) return FALSE;
	return TRUE;
	}

BOOL CStdDDraw::Init(HWND hWnd, BOOL Fullscreen, int iResX, int iResY, BOOL fUsePageLock)
	{

	if (!CreateDirectDraw())
		return Error("CreateDirectDraw failure.");

	if (!SetCooperativeLevel(hWnd,Fullscreen ? DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN : DDSCL_NORMAL))
		return Error("SetCooperativeLevel failure.");

	if (Fullscreen)
		if (!SetDisplayMode(iResX,iResY,8))
			return Error("SetDislpayMode failure.");

	if (!CreatePrimarySurfaces(Fullscreen))
		return Error("CreatePrimarySurfaces failure.");

	if (!CreatePrimaryClipper())
		return Error("CreatePrimaryClipper failure.");

	if (!CreatePrimaryPalette(Fullscreen ? lpPrimary : lpBack))
		return Error("CreatePrimaryPalette failure.");

	fPageLock = fUsePageLock;

	// Global access pointer
	lpDDraw = this;

	return TRUE;
	}

#ifdef C4ENGINE

BOOL Log(const char *szMessage);

BOOL CStdDDraw::Error(const char *szMsg)
	{
	Log(szMsg); return FALSE;
	}

#else

BOOL CStdDDraw::Error(const char *szMsg)
	{
	return FALSE;
	}

#endif

BOOL CStdDDraw::CreatePrimarySurfaces(BOOL fFlipAttach)
	{
  DDSURFACEDESC       ddsd;
  DDSCAPS             ddscaps;

	ZeroMem(&ddsd,sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);

	// Complex primary with attached back for fullscreen flip
	if (fFlipAttach)
		{

		// Create primary surface
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
													DDSCAPS_FLIP |
													DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		LPDIRECTDRAWSURFACE sfcSurface1;
		if (lpDirectDraw->CreateSurface(&ddsd,&sfcSurface1,NULL)!=DD_OK) 
			return FALSE;
		if (sfcSurface1->QueryInterface(IID_IDirectDrawSurface2,(void**)&lpPrimary)!=DD_OK) 
			{ sfcSurface1->Release(); return FALSE; }
		sfcSurface1->Release();

		// Get back surface
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (lpPrimary->GetAttachedSurface(&ddscaps,&lpBack)!=DD_OK)
			return FALSE;
		fBackAttached=TRUE;

		WipeSurface(lpPrimary);
		WipeSurface(lpBack);

		return TRUE;
		}

	// Simple primary with system memory back for console mode
	else
		{
		
		// Create primary surface
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;  
		LPDIRECTDRAWSURFACE sfcSurface1;
		if (lpDirectDraw->CreateSurface(&ddsd,&sfcSurface1,NULL)!=DD_OK) 
			return FALSE;
		if (sfcSurface1->QueryInterface(IID_IDirectDrawSurface2,(void**)&lpPrimary)!=DD_OK) 
			{ sfcSurface1->Release(); return FALSE; }
		sfcSurface1->Release();

		// Secondary surface         
		if (!(lpBack=CreateSurface(MaxResX,MaxResY)))	return FALSE;
		fBackAttached=FALSE;

		return TRUE;
		}

	return FALSE;
	}

BOOL CStdDDraw::CreatePrimaryClipper()
	{
	if (lpDirectDraw->CreateClipper(0,&lpClipper,NULL)!=DD_OK) return FALSE;
	return TRUE;
	}

BOOL CStdDDraw::CreatePrimaryPalette(SURFACE sfcAttachTo)
	{
	PALETTEENTRY black[256]; 
	ZeroMem((BYTE*)black,sizeof(PALETTEENTRY)*256);

  if (lpDirectDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256,black,&lpPalette,NULL)!=DD_OK) 
		return FALSE;

  if (sfcAttachTo) sfcAttachTo->SetPalette(lpPalette);

	return TRUE;
	}

BOOL CStdDDraw::AttachPrimaryPalette(SURFACE sfcSurface)
	{
	if (sfcSurface->SetPalette(lpPalette)!=DD_OK) return FALSE;
	return TRUE;
	}

BOOL CStdDDraw::InitFont(const char *szFontName, int iSize)
  {
	BOOL fResult = TRUE;
	// Get dc
  HDC hdc;
  HRESULT ddrval;
  while (1)
    {
    ddrval=lpBack->GetDC(&hdc);
    if (ddrval==DD_OK) break;
    if (ddrval!=DDERR_WASSTILLDRAWING) return FALSE;
    }
  iSize=iSize*16/10;
	if (!Font.Init(hdc,szFontName,iSize)) fResult=FALSE;
	// Release dc
	lpBack->ReleaseDC(hdc);
	// Success
  return fResult;
  }

// SfcCall: Parameter return value of surface call
//          Returns  >0 Okay  0 Retry  <0 Failure

extern void OnSurfaceRestore();

int CStdDDraw::SfcCall(HRESULT ddrval)
  {
  if (ddrval==DD_OK) return 1; // Okay
  if (ddrval==DDERR_WASSTILLDRAWING) return 0; // Continue loop
  if (ddrval==DDERR_SURFACELOST)
    {
    OnSurfaceRestore();
    // Assuming only SPrimary/SBack can get lost.
    if (lpPrimary->Restore()==DD_OK) return 0;
    }
  return -1; // Failure
  }

BOOL CStdDDraw::SurfaceSetColorKey(SURFACE sfcSurface, BYTE byCol)
  {
  int iCall;
  DDCOLORKEY ddcolkey;
  ddcolkey.dwColorSpaceLowValue=ddcolkey.dwColorSpaceHighValue=byCol;
  while (!(iCall=SfcCall(sfcSurface->SetColorKey(DDCKEY_SRCBLT,&ddcolkey))));
  if (iCall<0) return FALSE;
  return TRUE;
  }

BOOL CStdDDraw::BlitSurface(SURFACE sfcSurface, SURFACE sfcTarget, int tx, int ty)
  {
  int iCall;
  RECT frect,trect;
  int wdt,hgt;
  if (!GetSurfaceSize(sfcSurface,wdt,hgt)) return FALSE;
  SetRect(frect,0,0,wdt,hgt);
  SetRect(trect,tx,ty,tx+wdt,ty+hgt);
  while (!(iCall=SfcCall(sfcTarget->Blt(&trect,sfcSurface,&frect,0,0))));
  if (iCall<0) return FALSE;
  return TRUE;
  }

BOOL CStdDDraw::BlitSurfaceTile(SURFACE sfcSurface, SURFACE sfcTarget, int iToX, int iToY, int iToWdt, int iToHgt, int iOffsetX, int iOffsetY, BOOL fSrcColKey)
  {
  int iSourceWdt,iSourceHgt,iX,iY,iBlitX,iBlitY,iBlitWdt,iBlitHgt;
	// Get source surface size
  if (!GetSurfaceSize(sfcSurface,iSourceWdt,iSourceHgt)) return FALSE;
	// Vertical blits
  for (iY=iToY+iOffsetY; iY<iToY+iToHgt-1; iY+=iSourceHgt)
		if (iY+iSourceHgt>=iToY)
			{
			// Vertical blit size
			iBlitY=Max(iToY-iY,0); iBlitHgt=Min(iSourceHgt,iToY+iToHgt-1-iY+1)-iBlitY;
			// Horizontal blits
			for (iX=iToX+iOffsetX; iX<iToX+iToWdt-1; iX+=iSourceWdt)
				if (iX+iSourceWdt>=iToX)
					{
					// Horizontal blit size
					iBlitX=Max(iToX-iX,0); iBlitWdt=Min(iSourceWdt,iToX+iToWdt-1-iX+1)-iBlitX;
					// Blit
					if (!Blit(sfcSurface,iBlitX,iBlitY,iBlitWdt,iBlitHgt,sfcTarget,iX+iBlitX,iY+iBlitY,iBlitWdt,iBlitHgt,fSrcColKey)) return FALSE;
					}
			}
  return TRUE;
  }

HDC CStdDDraw::GetSurfaceDC(SURFACE sfcSurface)
	{
  HDC hdc;
	int iCall;
  while (!(iCall=SfcCall(sfcSurface->GetDC(&hdc))));
  if (iCall<0) return NULL;
	return hdc;
	}

BOOL CStdDDraw::BlitSurface2Window(SURFACE sfcSource,
																	 int fX, int fY, int fWdt, int fHgt,
																	 HWND hWnd,
																	 int tX, int tY, int tWdt, int tHgt)
	{
	BOOL fOkay = FALSE;
	HDC hdcSource=GetSurfaceDC(sfcSource);
	HDC hdcTarget=GetDC(hWnd);
	if ( hdcSource && hdcTarget )
		{
		// Same size
		if ((fWdt==tWdt) && (fHgt=tHgt)) 
			{
			if (BitBlt( hdcTarget, tX,tY, tWdt,tHgt,
								  hdcSource, fX,fY, SRCCOPY ) ) fOkay=TRUE;
			}
		// Stretch
		else 
			{
			SetStretchBltMode(hdcTarget,STRETCH_DELETESCANS);	
			if (StretchBlt( hdcTarget, tX,tY, tWdt,tHgt,
										  hdcSource, fX,fY, fWdt,fHgt, SRCCOPY ) ) fOkay=TRUE;
			}
		}
	if (hdcSource) sfcSource->ReleaseDC(hdcSource);
	if (hdcTarget) ReleaseDC(hWnd,hdcTarget);
	return fOkay;
	}

BOOL CStdDDraw::TextExtent(const char *szString, int &rWdt, int &rHgt)
	{
	return Font.TextExtent(szString,rWdt,rHgt);
	}

BOOL CStdDDraw::TextOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm)
	{
	return Font.TextOut(szText,sfcDest,iTx,iTy,iFCol,iBCol,byForm);
	}

BOOL CStdDDraw::StringOut(const char *szText, SURFACE sfcDest, int iTx, int iTy, int iFCol, int iBCol, BYTE byForm)
	{
	return Font.StringOut(szText,sfcDest,iTx,iTy,iFCol,iBCol,byForm);
	}

int CStdDDraw::TextHeight(const char *szText)
	{
	return Font.GetTextHeight(szText);
	}

int CStdDDraw::TextWidth(const char *szText)
	{
	if (szText) return Font.GetTextWidth(szText);
	return Font.GetTextWidth("W");
	}

BOOL CStdDDraw::SetPixel(SURFACE sfcDest, int tx, int ty, BYTE col)
  {
  BYTE *bypBuf;
  int lPitch,lWdt,lHgt;
	if (!Inside(tx,ClipX1,ClipX2) || !Inside(ty,ClipY1,ClipY2)) return FALSE;
  if (!(bypBuf=LockSurface(sfcDest,lPitch,&lWdt,&lHgt))) return FALSE;
  if (!Inside(tx,0,lWdt-1) || !Inside(ty,0,lHgt-1))
    { UnLockSurface(sfcDest); return TRUE; }
  // No patterns...
  bypBuf[lPitch*ty+tx]=col;
  UnLockSurface(sfcDest);
  return TRUE;
  }

BYTE CStdDDraw::GetPixel(SURFACE sfcSource, int fx, int fy)
  {
  BYTE *bypBuf,byRVal;
  int lPitch, lWdt,lHgt;
  if (!(bypBuf=LockSurface(sfcSource,lPitch,&lWdt,&lHgt))) return 0;
  if (!Inside(fx,0,lWdt-1) || !Inside(fy,0,lHgt-1))
    { UnLockSurface(sfcSource); return 0; }
  byRVal=bypBuf[lPitch*fy+fx];
  UnLockSurface(sfcSource);
  return byRVal;
  } 

void CStdDDraw::DrawBox(SURFACE sfcDest, int iX1, int iY1, int iX2, int iY2, BYTE byCol)
  {
  BYTE *bypBuf;
  int iPitch,iSfcWdt,iSfcHgt,xcnt,ycnt;
  // Lock surface
  if (!(bypBuf=LockSurface(sfcDest,iPitch,&iSfcWdt,&iSfcHgt))) return;
  // Outside of surface/clip
  if ((iX2<Max(0,ClipX1)) || (iX1>Min(iSfcWdt-1,ClipX2)) 
	 || (iY2<Max(0,ClipY1)) || (iY1>Min(iSfcHgt-1,ClipY2))) 
    { UnLockSurface(sfcDest); return; }
  // Clip to surface/clip
  if (iX1<Max(0,ClipX1)) iX1=Max(0,ClipX1); if (iX2>Min(iSfcWdt-1,ClipX2)) iX2=Min(iSfcWdt-1,ClipX2); 
  if (iY1<Max(0,ClipY1)) iY1=Max(0,ClipY1); if (iY2>Min(iSfcHgt-1,ClipY2)) iY2=Min(iSfcHgt-1,ClipY2);
  // Advance to starting pixel
  bypBuf+=iPitch*iY1+iX1; 
  // Set lines
  if (!Pattern) // Solid color
    for (ycnt=iY2-iY1; ycnt>=0; ycnt--)
      {
      for (xcnt=iX2-iX1; xcnt>=0; xcnt--) { *bypBuf=byCol; bypBuf++; }
      bypBuf+=iPitch-(iX2-iX1+1);
      }
  else // Pattern
    {
		for (ycnt=0; ycnt<=iY2-iY1; ycnt++)
			{
			if (PatQuickMod)
				for (xcnt=0; xcnt<=iX2-iX1; xcnt++,bypBuf++) 
					*bypBuf= byCol + Pattern[ PatWdt * ((iY1+ycnt) & PatHgtMod) + ((iX1+xcnt) & PatWdtMod) ];
			else
				for (xcnt=0; xcnt<=iX2-iX1; xcnt++,bypBuf++) 
					*bypBuf= byCol + Pattern[ PatWdt * ((iY1+ycnt) % PatHgt) + ((iX1+xcnt) % PatWdt) ];
			bypBuf+=iPitch-(iX2-iX1+1);
			}
    }
  // Unlock surface
  UnLockSurface(sfcDest);
  }

void CStdDDraw::DrawBoxColorTable(SURFACE sfcDest, int iX1, int iY1, int iX2, int iY2, BYTE *bypColorTable)
  {
  BYTE *bypBuf;
  int iPitch,iSfcWdt,iSfcHgt,xcnt,ycnt;
  // Lock surface
  if (!(bypBuf=LockSurface(sfcDest,iPitch,&iSfcWdt,&iSfcHgt))) return;
  // Outside of surface/clip
  if ((iX2<Max(0,ClipX1)) || (iX1>Min(iSfcWdt-1,ClipX2)) 
	 || (iY2<Max(0,ClipY1)) || (iY1>Min(iSfcHgt-1,ClipY2))) 
    { UnLockSurface(sfcDest); return; }
  // Clip to surface/clip
  if (iX1<Max(0,ClipX1)) iX1=Max(0,ClipX1); if (iX2>Min(iSfcWdt-1,ClipX2)) iX2=Min(iSfcWdt-1,ClipX2); 
  if (iY1<Max(0,ClipY1)) iY1=Max(0,ClipY1); if (iY2>Min(iSfcHgt-1,ClipY2)) iY2=Min(iSfcHgt-1,ClipY2);
  // Advance to starting pixel
  bypBuf+=iPitch*iY1+iX1; 
  // Set lines
  for (ycnt=iY2-iY1; ycnt>=0; ycnt--)
    {
    for (xcnt=iX2-iX1; xcnt>=0; xcnt--) { *bypBuf=bypColorTable[*bypBuf]; bypBuf++; }
    bypBuf+=iPitch-(iX2-iX1+1);
    }
  // Unlock surface
  UnLockSurface(sfcDest);
  }

void CStdDDraw::DrawHorizontalLine(SURFACE sfcDest, int x1, int x2, int y, BYTE col)
  {
  BYTE *bypBuf,*bypPatLine;
  int lPitch,lWdt,lHgt,xcnt;
  // Lock surface
  if (!(bypBuf=LockSurface(sfcDest,lPitch,&lWdt,&lHgt))) return;
	// Fix coordinates
	if (x1>x2) Swap(x1,x2);
  // Determine clip
  int clpx1,clpx2,clpy1,clpy2;
  clpx1=Max(0,ClipX1); clpy1=Max(0,ClipY1);
  clpx2=Min(lWdt-1,ClipX2); clpy2=Min(lHgt-1,ClipY2);
  // Outside clip check
  if ((x2<clpx1) || (x1>clpx2) || (y<clpy1) || (y>clpy2)) 
    { UnLockSurface(sfcDest); return; }
  // Clip to clip
  if (x1<clpx1) x1=clpx1; if (x2>clpx2) x2=clpx2; 
  // Advance to starting pixel
  bypBuf+=lPitch*y+x1; 
  // Set line
  if (!Pattern) // Solid color
    for (xcnt=x2-x1; xcnt>=0; xcnt--) { *bypBuf=col; bypBuf++; }
  else // Pattern
    {
		if (PatQuickMod)
			{
	    bypPatLine=Pattern+PatWdt*(y & PatHgtMod);
			for (xcnt=x1; xcnt<=x2; xcnt++) 
				{ *bypBuf=col+bypPatLine[xcnt & PatWdtMod]; bypBuf++; }
			}
		else
			{
	    bypPatLine=Pattern+PatWdt*(y % PatHgt);
			for (xcnt=x1; xcnt<=x2; xcnt++) 
				{ *bypBuf=col+bypPatLine[xcnt % PatWdt]; bypBuf++; }
			}
    }
  // Unlock surface
  UnLockSurface(sfcDest);
  }

void CStdDDraw::DrawVerticalLine(SURFACE sfcDest, int x, int y1, int y2, BYTE col)
  {
  BYTE *bypBuf,*bypPatColumn;
  int lPitch,lWdt,lHgt,ycnt;
  // Lock surface
  if (!(bypBuf=LockSurface(sfcDest,lPitch,&lWdt,&lHgt))) return;
	// Fix coordinates
	if (y1>y2) Swap(y1,y2);
  // Determine clip
  int clpx1,clpx2,clpy1,clpy2;
  clpx1=Max(0,ClipX1); clpy1=Max(0,ClipY1);
  clpx2=Min(lWdt-1,ClipX2); clpy2=Min(lHgt-1,ClipY2);
  // Outside clip check
  if ((x<clpx1) || (x>clpx2) || (y2<clpy1) || (y1>clpy2)) 
    { UnLockSurface(sfcDest); return; }
  // Clip to clip
  if (y1<clpy1) y1=clpy1; if (y2>clpy2) y2=clpy2; 
  // Advance to starting pixel
  bypBuf+=lPitch*y1+x; 
  // Set line
  if (!Pattern) // Solid color
    for (ycnt=y1; ycnt<=y2; ycnt++) { *bypBuf=col; bypBuf+=lPitch; }
  else // Pattern
    {
		if (PatQuickMod)
			{
			bypPatColumn=Pattern+(x & PatWdtMod);
			for (ycnt=y1; ycnt<=y2; ycnt++) 
				{ *bypBuf=col+bypPatColumn[PatWdt*((y1+ycnt) & PatHgtMod)]; bypBuf+=lPitch; }
			}
		else
			{
			bypPatColumn=Pattern+(x % PatWdt);
			for (ycnt=y1; ycnt<=y2; ycnt++) 
				{ *bypBuf=col+bypPatColumn[PatWdt*((y1+ycnt) % PatHgt)]; bypBuf+=lPitch; }
			}
    }
  // Unlock surface
  UnLockSurface(sfcDest);
  }

void CStdDDraw::DrawFrame(SURFACE sfcDest, int x1, int y1, int x2, int y2, BYTE col)
  {
  DrawHorizontalLine(sfcDest,x1,x2,y1,col);
  DrawHorizontalLine(sfcDest,x1,x2,y2,col);
  DrawVerticalLine(sfcDest,x1,y1,y2,col);
  DrawVerticalLine(sfcDest,x2,y1,y2,col);
  }

// Globally locked surface variables - for DrawLine callback crap

BYTE *GLSBuffer=NULL;
int GLSWdt,GLSHgt,GLSPitch;

BYTE *LockSurfaceGlobal(SURFACE sfcTarget)
  {
  if (GLSBuffer) return NULL;
  GLSBuffer=lpDDraw->LockSurface(sfcTarget,GLSPitch,&GLSWdt,&GLSHgt);
  return GLSBuffer;
  }

BOOL UnLockSurfaceGlobal(SURFACE sfcTarget)
  {
  if (!GLSBuffer) return FALSE;
  lpDDraw->UnLockSurface(sfcTarget);
  GLSBuffer=NULL;
  return TRUE;
  } 

BOOL DLineSPix(int x, int y, int col)
  {
  if (!GLSBuffer) return FALSE;
  int clpx1,clpx2,clpy1,clpy2;
  clpx1=Max(0,lpDDraw->ClipX1); clpy1=Max(0,lpDDraw->ClipY1);
  clpx2=Min(GLSWdt-1,lpDDraw->ClipX2); clpy2=Min(GLSHgt-1,lpDDraw->ClipY2);
  if (!Inside(x,clpx1,clpx2) || !Inside(y,clpy1,clpy2)) return TRUE;
  GLSBuffer[GLSPitch*y+x]=col;
  return TRUE;
  }

BOOL CStdDDraw::DrawLine(SURFACE sfcTarget, int x1, int y1, int x2, int y2, BYTE byCol)
  {
  if (!LockSurfaceGlobal(sfcTarget)) return FALSE;
  ForLine(x1,y1,x2,y2,&DLineSPix,byCol);
  UnLockSurfaceGlobal(sfcTarget);
  return TRUE;
  }

void CStdDDraw::DrawCircle(SURFACE sfcDest, int x, int y, int r, BYTE col)
	{
  int ycnt,lwdt;
  for (ycnt=-r; ycnt<r; ycnt++)
    {
    lwdt= (int) sqrt(r*r-ycnt*ycnt);
		DrawHorizontalLine(sfcDest,x-lwdt,x+lwdt-1,y+ycnt,col);
    }
	}

BOOL CStdDDraw::DefinePattern(SURFACE sfcSource)
  {
  BYTE *sfcbuf;
  int sfcpitch,sfcwdt,sfchgt;
  int twdt,thgt,cy;
	// Safety
	if (!sfcSource) return FALSE;
  // Clear existing pattern
  NoPattern();
  // Determine width and height
  if (!(sfcbuf=LockSurface(sfcSource,sfcpitch,&sfcwdt,&sfchgt))) return FALSE;
	PatWdt=sfcwdt; PatHgt=sfchgt;
	// Check quick mod
  for (twdt=1; twdt<sfcwdt; twdt<<=1);
  for (thgt=1; thgt<sfchgt; thgt<<=1);
  if ((twdt==sfcwdt) && (thgt==sfchgt)) 
		{	PatQuickMod=TRUE; PatWdtMod=PatWdt-1; PatHgtMod=PatHgt-1; }
  // Allocate pattern memory and copy pattern
  if (!(Pattern=new BYTE [PatWdt*PatHgt])) 
    { UnLockSurface(sfcSource); return FALSE; }
  for (cy=0; cy<PatHgt; cy++)
    MemCopy(sfcbuf+sfcpitch*cy,Pattern+PatWdt*cy,PatWdt);
  // Done
  UnLockSurface(sfcSource);
  return TRUE;
  }

void CStdDDraw::NoPattern()
  {
  if (Pattern) delete [] Pattern;
  Pattern=NULL;
	PatQuickMod=FALSE;
  }

void CStdDDraw::SurfaceShiftColorRange(SURFACE sfcSfc, int iRngLo, int iRngHi, int iShift)
  {
  BYTE *sfcbuf,*lbuf;
  int xcnt,ycnt,pitch,wdt,hgt;
  if (!(sfcbuf=LockSurface(sfcSfc,pitch,&wdt,&hgt))) return;
  for (ycnt=0; ycnt<hgt; ycnt++)
    {
    lbuf=sfcbuf+pitch*ycnt;
    for (xcnt=0; xcnt<wdt; xcnt++,lbuf++)
      if (Inside(*lbuf,iRngLo,iRngHi)) *lbuf+=iShift; 
    }
  UnLockSurface(sfcSfc);
  }

void CStdDDraw::SurfaceShiftColor(SURFACE sfcSfc, int iShift)
  {
  SurfaceShiftColorRange(sfcSfc,0,255,iShift);
  }

void CStdDDraw::SurfaceAllowColor(SURFACE sfcSfc, BYTE iRngLo, BYTE iRngHi, BOOL fAllowZero)
  {
  BYTE *sfcbuf,*lbuf;
  int xcnt,ycnt,pitch,wdt,hgt;
	if (iRngHi<iRngLo) return;
  if (!(sfcbuf=LockSurface(sfcSfc,pitch,&wdt,&hgt))) return;
  for (ycnt=0; ycnt<hgt; ycnt++)
    {
    lbuf=sfcbuf+pitch*ycnt;
    for (xcnt=0; xcnt<wdt; xcnt++,lbuf++)
			if ((*lbuf!=0) || !fAllowZero)
				if ((*lbuf<iRngLo) || (*lbuf>iRngHi)) 
					*lbuf= iRngLo + *lbuf % (iRngHi-iRngLo+1);      
    }
  UnLockSurface(sfcSfc);
  }

/* Polygon drawing code extracted from ALLEGRO by Shawn Hargreaves */

struct CPolyEdge          // An edge for the polygon drawer
  {
  int y;                  // Current (starting at the top) y position
  int bottom;             // bottom y position of this edge
  int x;                  // Fixed point x position
  int dx;                 // Fixed point x gradient
  int w;                  // Width of line segment
  struct CPolyEdge *prev; // Doubly linked list
  struct CPolyEdge *next;
  };

#define POLYGON_FIX_SHIFT     16

static void fill_edge_structure(CPolyEdge *edge, int *i1, int *i2)
  {
  if (i2[1] < i1[1]) // Swap
    { int *t=i1; i1=i2; i2=t; }
  edge->y = i1[1];
  edge->bottom = i2[1] - 1;
  edge->dx = ((i2[0] - i1[0]) << POLYGON_FIX_SHIFT) / (i2[1] - i1[1]);
  edge->x = (i1[0] << POLYGON_FIX_SHIFT) + (1<<(POLYGON_FIX_SHIFT-1)) - 1;
  edge->prev = NULL;
  edge->next = NULL;
  if (edge->dx < 0)
    edge->x += Min(edge->dx+(1<<POLYGON_FIX_SHIFT), 0);
  edge->w = Max(Abs(edge->dx)-(1<<POLYGON_FIX_SHIFT), 0);
  }

static CPolyEdge *add_edge(CPolyEdge *list, CPolyEdge *edge, int sort_by_x)
  {
  CPolyEdge *pos = list;
  CPolyEdge *prev = NULL;
  if (sort_by_x) 
    {
    while ((pos) && (pos->x+pos->w/2 < edge->x+edge->w/2)) 
      { prev = pos; pos = pos->next; }
    }
  else 
    {
    while ((pos) && (pos->y < edge->y)) 
      { prev = pos; pos = pos->next; }
    }
  edge->next = pos;
  edge->prev = prev;
  if (pos) pos->prev = edge;
  if (prev) { prev->next = edge; return list; }
  else return edge;
  }

static CPolyEdge *remove_edge(CPolyEdge *list, CPolyEdge *edge)
  {
  if (edge->next) edge->next->prev = edge->prev;
  if (edge->prev) { edge->prev->next = edge->next; return list; }
  else return edge->next;
  }

// Global polygon quick buffer
const int QuickPolyBufSize = 20;
CPolyEdge QuickPolyBuf[QuickPolyBufSize];

BOOL CStdDDraw::DrawPolygon(SURFACE sfcTarget, int iNum, int *ipVtx, int iCol)
	{

  // Variables for polygon drawer
  int c,x1,x2,y;
  int top = INT_MAX;
  int bottom = INT_MIN;
  int *i1, *i2;
  CPolyEdge *edge, *next_edge, *edgebuf;
  CPolyEdge *active_edges = NULL;
  CPolyEdge *inactive_edges = NULL;
	BOOL use_qpb=FALSE;

  // Poly Buf
	if (iNum<=QuickPolyBufSize)
		{ edgebuf=QuickPolyBuf; use_qpb=TRUE; }
	else
		if (!(edgebuf = new CPolyEdge [iNum])) { return FALSE; }

  // Fill the edge table
  edge = edgebuf;
  i1 = ipVtx;
  i2 = ipVtx + (iNum-1) * 2;
  for (c=0; c<iNum; c++) 
    {
    if (i1[1] != i2[1]) 
      {
      fill_edge_structure(edge, i1, i2);
      if (edge->bottom >= edge->y) 
        {
        if (edge->y < top)  top = edge->y;
        if (edge->bottom > bottom) bottom = edge->bottom;
        inactive_edges = add_edge(inactive_edges, edge, FALSE);
        edge++;
        }
      }
    i2 = i1; i1 += 2;
    }

   // For each scanline in the polygon...
   for (c=top; c<=bottom; c++) 
     {
     // Check for newly active edges
     edge = inactive_edges;
     while ((edge) && (edge->y == c)) 
			 {
			 next_edge = edge->next;
			 inactive_edges = remove_edge(inactive_edges, edge);
			 active_edges = add_edge(active_edges, edge, TRUE);
			 edge = next_edge;
			 }

     // Draw horizontal line segments
     edge = active_edges;
     while ((edge) && (edge->next)) 
       {
       x1=edge->x>>POLYGON_FIX_SHIFT;
       x2=(edge->next->x+edge->next->w)>>POLYGON_FIX_SHIFT;
       y=c;
       DrawHorizontalLine(sfcTarget,x1,x2,y,iCol);     
       edge = edge->next->next;
       }

      // Update edges, sorting and removing dead ones
      edge = active_edges;
      while (edge) 
				{
				next_edge = edge->next;
				if (c >= edge->bottom) 
					{
					active_edges = remove_edge(active_edges, edge);
					}
				else 
					{
					edge->x += edge->dx;
					while ((edge->prev) && (edge->x+edge->w/2 < edge->prev->x+edge->prev->w/2)) 
						{
						if (edge->next) edge->next->prev = edge->prev;
						edge->prev->next = edge->next;
						edge->next = edge->prev;
						edge->prev = edge->prev->prev;
						edge->next->prev = edge;
						if (edge->prev) edge->prev->next = edge;
						else active_edges = edge;
					}
				}
			edge = next_edge;
      }
   }

	// Clear scratch memory
	if (!use_qpb) delete [] edgebuf;

	return TRUE;
	}

BOOL CStdDDraw::GetPrimaryClipper(int &rX1, int &rY1, int &rX2, int &rY2)
	{
	// Store drawing clip values
	rX1=ClipX1; rY1=ClipY1;	rX2=ClipX2; rY2=ClipY2;
	// Done	
	return TRUE;
	}

void CStdDDraw::DrawInline(SURFACE sfcDest, int iX1, int iY1, int iX2, int iY2, BYTE byCol, BYTE byOnCol, BYTE byAdjacentCol)
	{
  BYTE *bypBuf;
  int iPitch,iSfcWdt,iSfcHgt;
  // Lock surface
  if (!(bypBuf=LockSurface(sfcDest,iPitch,&iSfcWdt,&iSfcHgt))) return;
  // Outside of surface/clip
  if ((iX2<Max(0,ClipX1)) || (iX1>Min(iSfcWdt-1,ClipX2)) 
	 || (iY2<Max(0,ClipY1)) || (iY1>Min(iSfcHgt-1,ClipY2))) 
    { UnLockSurface(sfcDest); return; }
  // Clip to surface/clip
  if (iX1<Max(0,ClipX1)) iX1=Max(0,ClipX1); if (iX2>Min(iSfcWdt-1,ClipX2)) iX2=Min(iSfcWdt-1,ClipX2); 
  if (iY1<Max(0,ClipY1)) iY1=Max(0,ClipY1); if (iY2>Min(iSfcHgt-1,ClipY2)) iY2=Min(iSfcHgt-1,ClipY2);
  
	// Do pixels
	for (int cx=iX1; cx<=iX2; cx++)
		for (int cy=iY1; cy<=iY2; cy++)
			if (bypBuf[iPitch*cy+cx]==byOnCol)
				if ( (cx==iX1) || (bypBuf[iPitch*cy+cx-1]==byAdjacentCol)
					|| (cx==iX2) || (bypBuf[iPitch*cy+cx+1]==byAdjacentCol)
					|| (cy==iY1) || (bypBuf[iPitch*(cy-1)+cx]==byAdjacentCol)
					|| (cy==iY2) || (bypBuf[iPitch*(cy+1)+cx]==byAdjacentCol) )
						bypBuf[iPitch*cy+cx]=byCol;
  
	// Unlock surface
  UnLockSurface(sfcDest);
	}

BOOL CStdDDraw::BlitRotateOriginal(SURFACE sfcSource, int fx, int fy, int fwdt, int fhgt,
								SURFACE sfcTarget, int tx, int ty, int twdt, int thgt,
								int iAngle)
	{
  // Object is first stretched to dest rect, then rotated at place.
  BYTE *bypSource,*bypDest;
  int lSrcPitch,lDestPitch,lDestSize,lDestWdt,lDestHgt;
  int xcnt,ycnt,fcx,fcy,tcx,tcy,cpcx,cpcy;
  BYTE *cpix;
  const double pi = 3.1415926535;
  int npcx,npcy;
  double mtx[4],dang;

  if (!fwdt || !fhgt || !twdt || !thgt) return FALSE;

  // Lock the surfaces
  if (!(bypSource=LockSurface(sfcSource,lSrcPitch))) 
    return FALSE;
  if (!(bypDest=LockSurface(sfcTarget,lDestPitch,&lDestWdt,&lDestHgt)))
    { UnLockSurface(sfcSource); return FALSE; }
	lDestSize=lDestPitch*lDestHgt;
	if (!lDestSize) return FALSE;

  // Rectangle centers
  fcx=fwdt/2; fcy=fhgt/2;
  tcx=twdt/2; tcy=thgt/2;

	// Adjust angle range
	while (iAngle<0) iAngle+=36000; while (iAngle>35999) iAngle-=36000;

	// Calculate rotation matrix
	dang=pi*iAngle/18000.0;
	mtx[0]=cos(dang); mtx[1]=-sin(dang);
	mtx[2]=sin(dang); mtx[3]= cos(dang);  
	// Blit source rect
	for (ycnt=0; ycnt<fhgt; ycnt++)
		{
		// Source line start
		cpix=bypSource+lSrcPitch*(fy+ycnt)+fx;
		for (xcnt=0; xcnt<fwdt; xcnt++,cpix++)
			{
			// Source line pixel not transparent
			if (*cpix)
				{
				// Current pixel coordinate as from source
				cpcx=xcnt-fcx; cpcy=ycnt-fcy;
				// Convert to coordinate as in dest
				cpcx=cpcx*twdt/fwdt; cpcy=cpcy*thgt/fhgt;
				// Rotate current pixel coordinate
				npcx= (int) ( mtx[0]*cpcx + mtx[1]*cpcy );
				npcy= (int) ( mtx[2]*cpcx + mtx[3]*cpcy );
				// Place in dest
				bypDest[BoundBy(lDestPitch*(ty+tcy+npcy)+tx+tcx+npcx,0,lDestSize-1)]=*cpix;
				bypDest[BoundBy(lDestPitch*(ty+tcy+npcy)+tx+tcx+npcx+1,0,lDestSize-1)]=*cpix;
				}
			}
		}  

  // Unlock the surfaces
  UnLockSurface(sfcSource);
  UnLockSurface(sfcTarget);

  return TRUE;
	}

