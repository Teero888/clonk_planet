/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper class to DirectDraw surfaces */

#include <Windows.h>
#include <MMSystem.h>
#include <ShellAPI.h>
#include <Math.h>
#include <Stdio.h>
#include <Io.h>
#include <StdLib.h>
#include <Time.h>
#include <DDraw.h>
#include <VfW.h>
#include <Limits.h>

#include <Standard.h>
#include <StdFile.h>
#include <CStdFile.h>
#include <StdRegistry.h>
#include <StdResStr.h>
#include <StdConfig.h>
#include <StdRandom.h>
#include <StdSurface.h>
#include <StdFacet.h>
#include <StdFont.h>
#include <StdDDraw.h>
#include <Bitmap256.h>

CSurface::CSurface()
	{
	Default();
	}

CSurface::~CSurface()
	{
	Clear();
	}

void CSurface::Default()
	{
	Wdt=Hgt=Pitch=0;
	ClipX=ClipY=ClipX2=ClipY2=0;
	Locked=0;
	Attached=FALSE;
	Bits=NULL;
	Surface=NULL;
	ZeroMem(Palette,768);
	}

void CSurface::Clear()
	{
	// Undo all locks
	while (Locked) Unlock();
	// Destroy surface if not attached
	if (!Attached) 
		if (Surface) 
			if (lpDDraw)
				lpDDraw->DestroySurface(Surface); 
	Surface=NULL;
	}

BOOL CSurface::Lock()
	{
	if (!Locked)
		if (!(Bits=lpDDraw->LockSurface(Surface,Pitch))) 
			return FALSE;
	Locked++;
	return TRUE;
	}

BOOL CSurface::Unlock()
	{
	if (!Locked) return FALSE;
	Locked--;
	if (!Locked)
		lpDDraw->UnLockSurface(Surface);
	return TRUE;
	}

BOOL CSurface::Box(int iX, int iY, int iX2, int iY2, int iCol)
	{
	if (!Lock()) return FALSE;
	for (int cy=iY; cy<=iY2; cy++) HLine(iX,iX2,cy,iCol);
	Unlock();
	return TRUE;
	}

// Polygon code extracted from ALLEGRO by Shawn Hargreaves

struct CPolyEdge						/* an edge for the polygon drawer */
  {
  int y;										/* current (starting at the top) y position */
  int bottom;								/* bottom y position of this edge */
  int x;										/* fixed point x position */
  int dx;										/* fixed point x gradient */
  int w;										/* width of line segment */
  struct CPolyEdge *prev;   /* doubly linked list */
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
CPolyEdge QuickPolyBuf2[QuickPolyBufSize];

BOOL CSurface::Polygon(int iNum, int *ipVtx, int iCol)
	{
	if (!Lock()) return FALSE;

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
		{ edgebuf=QuickPolyBuf2; use_qpb=TRUE; }
	else
		if (!(edgebuf = new CPolyEdge [iNum])) { Unlock(); return FALSE; }

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

   /* for each scanline in the polygon... */
   for (c=top; c<=bottom; c++) 
     {
     /* check for newly active edges */
     edge = inactive_edges;
     while ((edge) && (edge->y == c)) {
			 next_edge = edge->next;
			 inactive_edges = remove_edge(inactive_edges, edge);
			 active_edges = add_edge(active_edges, edge, TRUE);
			 edge = next_edge;
     }

     /* draw horizontal line segments */
     edge = active_edges;
     while ((edge) && (edge->next)) 
       {
       x1=edge->x>>POLYGON_FIX_SHIFT;
       x2=(edge->next->x+edge->next->w)>>POLYGON_FIX_SHIFT;
       y=c;

       HLine(x1,x2,y,iCol);
       
       edge = edge->next->next;
       }

      /* update edges, sorting and removing dead ones */
      edge = active_edges;
      while (edge) {
	 next_edge = edge->next;
	 if (c >= edge->bottom) {
	    active_edges = remove_edge(active_edges, edge);
	 }
	 else {
	    edge->x += edge->dx;
	    while ((edge->prev) && 
		   (edge->x+edge->w/2 < edge->prev->x+edge->prev->w/2)) {
	       if (edge->next)
		  edge->next->prev = edge->prev;
	       edge->prev->next = edge->next;
	       edge->next = edge->prev;
	       edge->prev = edge->prev->prev;
	       edge->next->prev = edge;
	       if (edge->prev)
		  edge->prev->next = edge;
	       else
		  active_edges = edge;
	    }
	 }
	 edge = next_edge;
      }
   }

	// Clear scratch memory
	if (!use_qpb) delete [] edgebuf;

	Unlock();
	return TRUE;
	}

void CSurface::NoClip()
	{
	ClipX=0; ClipY=0; ClipX2=Wdt-1; ClipY2=Hgt-1;
	}

void CSurface::Clip(int iX, int iY, int iX2, int iY2)
	{
	ClipX=BoundBy(iX,0,Wdt-1); ClipY=BoundBy(iY,0,Hgt-1); 
	ClipX2=BoundBy(iX2,0,Wdt-1); ClipY2=BoundBy(iY2,0,Hgt-1);
	}

BOOL CSurface::HLine(int iX, int iX2, int iY, int iCol)
	{
	if (!Lock()) return FALSE;
	for (int cx=iX; cx<=iX2; cx++) Pix(cx,iY,iCol);
	Unlock();
	return TRUE;
	}

BOOL CSurface::Pix(int iX, int iY, int iCol)
	{
	if ((iX<ClipX) || (iX>ClipX2) || (iY<ClipY) || (iY>ClipY2)) return TRUE;
	if (!Lock()) return FALSE;
	Bits[iY*Pitch+iX]=iCol;
	Unlock();
	return TRUE;
	}

BOOL CSurface::Create(int iWdt, int iHgt)
	{
	Clear(); Default();
	if (!(Surface=lpDDraw->CreateSurface(iWdt,iHgt))) return FALSE;
	Wdt=iWdt; Hgt=iHgt;
	return TRUE;
	}

// Attach this CSurface object to an existing SURFACE.
// Will not delete SURFACE on destruction.

BOOL CSurface::Attach(SURFACE sfcSurface)
	{
	Clear(); Default();
	Surface=sfcSurface;
	Attached=TRUE;
	if (!lpDDraw->GetSurfaceSize(Surface,Wdt,Hgt)) return FALSE;
	NoClip();
	return TRUE;
	}

HDC CSurface::GetDC()
	{
	return lpDDraw->GetSurfaceDC(Surface);
	}

void CSurface::ReleaseDC(HDC hdc)
	{
	Surface->ReleaseDC(hdc);
	}

BOOL CSurface::Save(const char *szFilename)
	{
	// Set bitmap info
	CBitmap256Info BitmapInfo;
	BitmapInfo.Set(Wdt,Hgt,Palette);

	// Lock 
	if (!Lock()) return FALSE;

	// Create file & write info
	CStdFile hFile; 
	
	if ( !hFile.Create(szFilename)
		|| !hFile.Write(&BitmapInfo,sizeof(BitmapInfo)) )
			{ Unlock(); return FALSE; }

	// Write lines
	char bpEmpty[4]; int iEmpty = DWordAligned(Wdt)-Wdt;
  for (int cnt=Hgt-1; cnt>=0; cnt--)
		{
    if (!hFile.Write(Bits+(Pitch*cnt),Wdt))
			{ Unlock(); return FALSE; }
		if (iEmpty)
			if (!hFile.Write(bpEmpty,iEmpty))
				{ Unlock(); return FALSE; }
    }

	// Close file
  hFile.Close();

	// Unlock
	Unlock();

	// Success
  return TRUE;
	}

BOOL CSurface::AttachPalette()
	{
	// Overrides primary palette
	if (!lpDDraw->SetPrimaryPalette(Palette)) return FALSE;
	if (!lpDDraw->AttachPrimaryPalette(Surface)) return FALSE;
	return TRUE;
	}

BOOL CSurface::Load(const char *szFilename)
	{
	int cnt,lcnt,iLineRest;
  BYTE fbuf[4];

	// Open file
	CStdFile hFile;
	if (!hFile.Open(szFilename)) return FALSE;

	// Read and check bitmap info
	CBitmap256Info BitmapInfo;
	if (!hFile.Read(&BitmapInfo,sizeof(BitmapInfo))) return FALSE;
	if (!BitmapInfo.Valid()) return FALSE;

  // Read offset
	if (!hFile.Advance(BitmapInfo.FileBitsOffset())) return FALSE;

  // Create and lock surface
	if (!Create(BitmapInfo.Info.biWidth,BitmapInfo.Info.biHeight)) return FALSE;
	if (!Lock()) { Clear(); return FALSE; }

	// Copy palette
	for (cnt=0; cnt<256; cnt++)
    {
    Palette[cnt*3+0]=BitmapInfo.Colors[cnt].rgbRed;
    Palette[cnt*3+1]=BitmapInfo.Colors[cnt].rgbGreen;
    Palette[cnt*3+2]=BitmapInfo.Colors[cnt].rgbBlue;
    }
    
	// Read lines
	iLineRest = DWordAligned(BitmapInfo.Info.biWidth) - BitmapInfo.Info.biWidth;
  for (lcnt=Hgt-1; lcnt>=0; lcnt--)
    {
    if (!hFile.Read(Bits+(Pitch*lcnt),Wdt))
      { Clear(); return FALSE; }
    if (iLineRest>0)
      if (!hFile.Read(fbuf,iLineRest))
				{ Clear(); return FALSE; }
    }

	hFile.Close();
  Unlock();

  return TRUE;
	}

BOOL CSurface::SetPalette(int iColor, int iRed, int iGreen, int iBlue)
	{
	Palette[iColor*3+0]=iRed;
	Palette[iColor*3+1]=iGreen;
	Palette[iColor*3+2]=iBlue;
	return TRUE;
	}

double ColorDistance(BYTE *bpRGB1, BYTE *bpRGB2)
	{
	return (double) (Abs(bpRGB1[0]-bpRGB2[0]) + Abs(bpRGB1[1]-bpRGB2[1]) + Abs(bpRGB1[2]-bpRGB2[2])) / 6.0;
	}

void CSurface::SetPalette(BYTE *bpPalette, BOOL fAdapt)
	{

	// Adapt bitmap
	if (fAdapt)
		{
		BYTE AdaptMap[256]; AdaptMap[0]=0;
		for (int iFromCol=1; iFromCol<256; iFromCol++) // Color zero transparent
			{
			int iToCol = 1;
			for (int cToCol=iToCol+1; cToCol<256; cToCol++)
				if ( ColorDistance(&Palette[iFromCol*3],&bpPalette[cToCol*3])
					 < ColorDistance(&Palette[iFromCol*3],&bpPalette[iToCol*3]) )
							iToCol = cToCol;

			AdaptMap[iFromCol] = iToCol;
			}

		MapBytes(AdaptMap);

		}

	// Set palette
	MemCopy(bpPalette,Palette,256*3);

	}

void CSurface::MapBytes(BYTE *bpMap)
	{
	if (!bpMap) return;
	if (!Lock()) return;
	for (int cnt=0; cnt<Pitch*Hgt; cnt++) Bits[cnt]=bpMap[Bits[cnt]];
	Unlock();
	}

BOOL CSurface::Load(BYTE *bpBitmap)
	{
	int cnt,lcnt,iLineRest;
  BYTE fbuf[4];

	// Open file
	if (!bpBitmap) return FALSE;

	// Read and check bitmap info
	CBitmap256Info BitmapInfo;
	if (!ReadBytes(&bpBitmap,&BitmapInfo,sizeof(BitmapInfo))) return FALSE;
	if (!BitmapInfo.Valid()) return FALSE;

  // Read offset
	bpBitmap += BitmapInfo.FileBitsOffset();

  // Create and lock surface
	if (!Create(BitmapInfo.Info.biWidth,BitmapInfo.Info.biHeight)) return FALSE;
	if (!Lock()) { Clear(); return FALSE; }

	// Copy palette
	for (cnt=0; cnt<256; cnt++)
    {
    Palette[cnt*3+0]=BitmapInfo.Colors[cnt].rgbRed;
    Palette[cnt*3+1]=BitmapInfo.Colors[cnt].rgbGreen;
    Palette[cnt*3+2]=BitmapInfo.Colors[cnt].rgbBlue;
    }
    
	// Read lines
	iLineRest = DWordAligned(BitmapInfo.Info.biWidth) - BitmapInfo.Info.biWidth;
  for (lcnt=Hgt-1; lcnt>=0; lcnt--)
    {
    if (!ReadBytes(&bpBitmap,Bits+(Pitch*lcnt),Wdt))
      { Clear(); return FALSE; }
    if (iLineRest>0)
      if (!ReadBytes(&bpBitmap,fbuf,iLineRest))
				{ Clear(); return FALSE; }
    }

  Unlock();

  return TRUE;
	}

BOOL CSurface::ReadBytes(BYTE **lpbpData, void *bpTarget, int iSize)
	{
	if (!lpbpData || !(*lpbpData) || !bpTarget) return FALSE;
	MemCopy(*lpbpData,bpTarget,iSize);
	(*lpbpData)+=iSize;
	return TRUE;
	}