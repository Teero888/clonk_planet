/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A very primitive piece of surface */

#include <Windows.h>
#include <DDraw.h>

#include <Standard.h>
#include <StdSurface.h>
#include <StdFacet.h>
#include <StdFont.h>
#include <StdDDraw.h>

CFacet::CFacet()
	{
	Default();
	}

CFacet::~CFacet()
	{
	Clear();
	}

void CFacet::Default()
	{
	Surface = NULL;
	X=Y=Wdt=Hgt=0;
	}

void CFacet::Clear()
	{
	}

void CFacet::Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt)
  { 
	Surface=nsfc; X=nx; Y=ny; Wdt=nwdt; Hgt=nhgt; 
	}

void CFacet::Draw(SURFACE sfcSurface, int iX, int iY, int iPhaseX, int iPhaseY)
	{
	lpDDraw->Blit(Surface, X+Wdt*iPhaseX, Y+Hgt*iPhaseY, Wdt,Hgt, 
								sfcSurface,iX,iY,Wdt,Hgt, TRUE);
	}
