/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Bitmap handling routines */

#include <Windows.h>
#include <StdIO.h>

#include <Standard.h>
#include <StdFile.h>
#include <CStdFile.h>
#include <StdBitmap.h>

//================================= CStdBitmapHead =======================================

CStdBitmapHead::CStdBitmapHead()
	{
	Clear();
	}

void CStdBitmapHead::Clear()
	{
	ZeroMem((BYTE*)this,sizeof(CStdBitmapHead));
	Set(0);
	}

void CStdBitmapHead::Set(int iBitOffset)
	{
	bfType=*((WORD*)"BM");
	bfSize=sizeof(CStdBitmapHead);
	bfReserved1=bfReserved2=0;
	bfOffBits=iBitOffset;
	}

BOOL CStdBitmapHead::Valid()
	{
  if (bfType != *((WORD*)"BM") ) return FALSE;
	return TRUE;
	}

//============================= CStdBitmapInfo =========================================

CStdBitmapInfo::CStdBitmapInfo()
	{
	Clear();
	}

void CStdBitmapInfo::Clear()
	{
	ZeroMem((BYTE*)this,sizeof(CStdBitmapInfo));
	}

void CStdBitmapInfo::Set(int iWdt, int iHgt, int iBitsPerPixel)
	{
	Clear();
	biSize=sizeof(CStdBitmapInfo);
	biPlanes=1;
	biWidth=iWdt;
	biHeight=iHgt;
	biBitCount=iBitsPerPixel;
	biCompression=BI_RGB;
	biSizeImage=DWordAligned(iWdt*iBitsPerPixel/8)*iHgt;
	}

int CStdBitmapInfo::Pitch()
	{
	if (!biHeight) return 0;
	return biSizeImage/Abs(biHeight);
	}

//=============================== CStdBitmap ===========================================

CStdBitmap::CStdBitmap()
	{
	Bits=NULL;
	Clear();
	}

CStdBitmap::~CStdBitmap()
	{
	Clear();
	}

void CStdBitmap::Clear()
	{
	Head.Clear();
	Info.Clear();
	ZeroMem((BYTE*)Colors,sizeof(RGBQUAD)*256);
	if (Bits) delete [] Bits; Bits=NULL;
	}

BOOL CStdBitmap::Create(int iWdt, int iHgt, int iBitsPerPixel)
	{
	// Init head
	int coloroffset=0; if (iBitsPerPixel==8) coloroffset=256*sizeof(RGBQUAD);
	Head.Set(sizeof(Head)+sizeof(Info)+coloroffset);
	// Init info
	Info.Set(iWdt,iHgt,iBitsPerPixel);
	// Allocate bits
	if (!(Bits=new BYTE [Info.biSizeImage])) return FALSE;
	ZeroMem(Bits,Info.biSizeImage);
	return TRUE;
	}

BOOL CStdBitmap::Save(const char *szFileName)
	{
	CStdFile hFile;
	if (!Bits) return FALSE;
	int savesize=sizeof(Head)+sizeof(Info);
  if (Info.biBitCount==8) savesize+=256*sizeof(RGBQUAD);
	if (!hFile.Create(szFileName,FALSE)
	 || !hFile.Write(this,savesize)
	 || !hFile.Write(Bits,Info.biSizeImage)
	 || !hFile.Close()) 
		return FALSE;
	return TRUE;
	}

BOOL CStdBitmap::Enlarge(int iWdt, int iHgt)
	{
	if (!Bits) return FALSE;
	iWdt=Max(iWdt,Info.biWidth);
	iHgt=Max(iHgt,Abs(Info.biHeight));
	if (!((iWdt>Info.biWidth) || (iHgt>Abs(Info.biHeight)))) return FALSE;
	int nPitch = DWordAligned(iWdt*Info.biBitCount/8);
	int nSize = nPitch*iHgt;
	BYTE *nBits;
	if (!(nBits = new BYTE [nSize])) return FALSE;
	ZeroMem(nBits,nSize);
	iHgt*=Sign(Info.biHeight);
	StdBlit(Bits,Info.Pitch(),Info.biHeight,
					0,0,Info.biWidth,Info.biHeight,
					nBits,nPitch,iHgt,
					0,0,Info.biWidth,Info.biHeight,
					Info.biBitCount/8);
	Info.biWidth=iWdt;
	Info.biHeight=iHgt;
	Info.biSizeImage=nSize;
	delete [] Bits;
	Bits=nBits;
	return TRUE;
	}
