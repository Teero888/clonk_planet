/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A piece of a DirectDraw surface */

#include <C4Include.h>

void C4Facet::Default()
	{
	Set(NULL,0,0,0,0);
	}

void C4Facet::Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt)
  { 
	Surface=nsfc; X=nx; Y=ny; Wdt=nwdt; Hgt=nhgt; 
	}

C4Facet::C4Facet()
	{
	Default();
	}

int C4Facet::GetSectionCount()
	{
	if (Hgt==0) return 0;
	return Wdt/Hgt;
	}

C4Facet C4Facet::GetSection(int iSection)
	{
	C4Facet rval;
	rval.Set(Surface,X+Hgt*iSection,Y,Hgt,Hgt);
	return rval;
	}

void C4Facet::Draw(SURFACE sfcTarget, int iX, int iY, int iPhaseX, int iPhaseY)
	{
	if (!lpDDraw || !Surface || !sfcTarget || !Wdt || !Hgt) return;

	lpDDraw->Blit(Surface,
								X+Wdt*iPhaseX,Y+Hgt*iPhaseY,Wdt,Hgt,
								sfcTarget,
								iX,iY,Wdt,Hgt,TRUE);

	}

void C4Facet::Draw(C4Facet &cgo, BOOL fAspect, int iPhaseX, int iPhaseY)
	{
	// Valid parameter check
	if (!lpDDraw || !Surface || !cgo.Surface || !Wdt || !Hgt) return;
	// Drawing area
	C4Facet ccgo = cgo;
	// Adjust for fixed aspect ratio
	if (fAspect)
		{
		// By height
		if (100*cgo.Wdt/Wdt<100*cgo.Hgt/Hgt) 
			{ 
			ccgo.Hgt=Hgt*cgo.Wdt/Wdt;
			ccgo.Y+=(cgo.Hgt-ccgo.Hgt)/2; 
			}
		// By width
		else if (100*cgo.Hgt/Hgt<100*cgo.Wdt/Wdt)
			{ 
			ccgo.Wdt=Wdt*cgo.Hgt/Hgt;
			ccgo.X+=(cgo.Wdt-ccgo.Wdt)/2;
			}
		}
	// Blit
	lpDDraw->Blit(Surface,
								X+Wdt*iPhaseX,Y+Hgt*iPhaseY,Wdt,Hgt,
								ccgo.Surface,
								ccgo.X,ccgo.Y,ccgo.Wdt,ccgo.Hgt,
								TRUE);		
	}

C4Facet C4Facet::TruncateSection(int iAlign, BOOL fHalf)
	{
	C4Facet fctResult; fctResult.Set(Surface,0,0,0,0);
	// Consider old-style parameter
	if (fHalf) iAlign|=C4FCT_Half;
	// Calculate section size
	int iWdt=Wdt,iHgt=Hgt;		
	switch (iAlign & C4FCT_Alignment)
		{
		case C4FCT_Left: case C4FCT_Right: 
			iWdt=Hgt; 
			if (iAlign & C4FCT_Triple) iWdt*=3;
			if (iAlign & C4FCT_Double) iWdt*=2;
			if (iAlign & C4FCT_Half) iWdt/=2;	
			break;
		case C4FCT_Top: case C4FCT_Bottom: 
			iHgt=iWdt; 
			if (iAlign & C4FCT_Triple) iHgt*=3;
			if (iAlign & C4FCT_Double) iHgt*=2;
			if (iAlign & C4FCT_Half) iHgt/=2;
			break;
		}
	// Size safety
	if ((iWdt>Wdt) || (iHgt>Hgt)) return fctResult;
	// Truncate
	switch (iAlign & C4FCT_Alignment)
		{
		case C4FCT_Left: fctResult.Set(Surface,X,Y,iWdt,iHgt); X+=iWdt; Wdt-=iWdt;	break;
		case C4FCT_Right: fctResult.Set(Surface,X+Wdt-iWdt,Y,iWdt,iHgt); Wdt-=iWdt; break;
		case C4FCT_Top: fctResult.Set(Surface,X,Y,iWdt,iHgt); Y+=iHgt; Hgt-=iHgt; break;
		case C4FCT_Bottom: fctResult.Set(Surface,X,Y+Hgt-iHgt,iWdt,iHgt); Hgt-=iHgt; break;
		}
	// Done
	return fctResult;
	}

C4Facet C4Facet::Truncate(int iAlign, int iSize)
	{
	C4Facet fctResult; fctResult.Set(Surface,0,0,0,0);
	// Calculate section size
	int iWdt=Wdt,iHgt=Hgt;		
	switch (iAlign)
		{
		case C4FCT_Left: case C4FCT_Right: iWdt=iSize;	break;
		case C4FCT_Top: case C4FCT_Bottom: iHgt=iSize;	break;
		}
	// Size safety
	if ((iWdt>Wdt) || (iHgt>Hgt)) return fctResult;
	// Truncate
	switch (iAlign)
		{
		case C4FCT_Left: fctResult.Set(Surface,X,Y,iWdt,iHgt); X+=iWdt; Wdt-=iWdt;	break;
		case C4FCT_Right: fctResult.Set(Surface,X+Wdt-iWdt,Y,iWdt,iHgt); Wdt-=iWdt; break;
		case C4FCT_Top: fctResult.Set(Surface,X,Y,iWdt,iHgt); Y+=iHgt; Hgt-=iHgt; break;
		case C4FCT_Bottom: fctResult.Set(Surface,X,Y+Hgt-iHgt,iWdt,iHgt); Hgt-=iHgt; break;
		}
	// Done
	return fctResult;
	}

void C4Facet::DrawSectionSelect(C4Facet &cgo, int iSelection, int iMaxSelect)
	{
	int sections = cgo.GetSectionCount();
	int idnum = iMaxSelect;
	int firstid = BoundBy(iSelection-sections/2,0,Max(idnum-sections,0));  
	C4Facet cgo2;
	for (int cnt=0; (cnt<sections) && (firstid+cnt<idnum); cnt++)
		{
		cgo2 = cgo.GetSection(cnt);
		if (iSelection==firstid+cnt)
			lpDDraw->DrawBox(cgo2.Surface,cgo2.X,cgo2.Y,cgo2.X+cgo2.Wdt-1,cgo2.Y+cgo2.Hgt-1,CRed);
		Draw(cgo2,TRUE,firstid+cnt,0);
    }    	
	}

void C4Facet::DrawValue(C4Facet &cgo, int iValue, int iSectionX, int iSectionY, int iAlign)
	{
	if (!lpDDraw) return;
	char ostr[25]; sprintf(ostr,"%i",iValue);
	switch (iAlign)
		{
		case C4FCT_Center:
			Draw(cgo,TRUE,iSectionX,iSectionY);
			lpDDraw->TextOut(ostr,cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,FWhite,FBlack,ARight);
			break;
		case C4FCT_Right:
			lpDDraw->TextOut(ostr,cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y,FWhite,FBlack,ARight);
			cgo.Set(cgo.Surface,cgo.X+cgo.Wdt-1-lpDDraw->TextWidth(ostr)-2*cgo.Hgt,cgo.Y,2*cgo.Hgt,cgo.Hgt);
			Draw(cgo,TRUE,iSectionX,iSectionY);
			break;
		}
	}

void C4Facet::DrawValue2(C4Facet &cgo, int iValue1, int iValue2, int iSectionX, int iSectionY, int iAlign)
	{
	if (!lpDDraw) return;
	char ostr[25]; sprintf(ostr,"%i/%i",iValue1,iValue2);
	switch (iAlign)
		{
		case C4FCT_Center:
			Draw(cgo,TRUE,iSectionX,iSectionY);
			lpDDraw->TextOut(ostr,cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,FWhite,FBlack,ARight);
			break;
		case C4FCT_Right:
			lpDDraw->TextOut(ostr,cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y,FWhite,FBlack,ARight);
			cgo.Set(cgo.Surface,cgo.X+cgo.Wdt-1-lpDDraw->TextWidth(ostr)-2*cgo.Hgt,cgo.Y,2*cgo.Hgt,cgo.Hgt);
			Draw(cgo,TRUE,iSectionX,iSectionY);
			break;
		}
	}

void C4Facet::DrawX(SURFACE sfcTarget, int iX, int iY, int iWdt, int iHgt, int iSectionX, int iSectionY)
	{	
	if (!lpDDraw || !Surface || !sfcTarget || !Wdt || !Hgt) return;
	lpDDraw->Blit(Surface,
								X+Wdt*iSectionX,Y+Hgt*iSectionY,Wdt,Hgt,
								sfcTarget,
								iX,iY,iWdt,iHgt,
								TRUE);
	}

void C4Facet::DrawEnergyLevel(int iLevel, int iRange, int iColor)
	{
	if (!lpDDraw) return;
	iLevel = BoundBy(iLevel,0,iRange);
	lpDDraw->DrawBox(Surface,
				X,Y,
				X+Wdt-1,Y+Hgt-1,
				40);
	lpDDraw->DrawBox(Surface,
				X+1,Y+1+(Hgt-2)-(Hgt-2)*iLevel/Max(iRange,1),
				X+Wdt-2,Y+Hgt-2,
				iColor);
	}

void C4Facet::Set(CSurface &rSfc)
	{
	Set(rSfc.Surface,0,0,rSfc.Wdt,rSfc.Hgt);
	}

void C4Facet::Draw(HWND hWnd, int iTx, int iTy, int iTWdt, int iTHgt, BOOL fAspect, int iPhaseX, int iPhaseY)
	{
	if (!lpDDraw || !Surface || !hWnd || !Wdt || !Hgt) return;

	if (fAspect)
		{
		int iTx2=iTx,iTy2=iTy,iTWdt2=iTWdt,iTHgt2=iTHgt;
		// Adjust height aspect by width aspect
		if (100*iTWdt/Wdt<100*iTHgt/Hgt) 
			{ 
			iTHgt2=Hgt*iTWdt/Wdt;
			iTy2+=(iTHgt-iTHgt2)/2; 
			}
		// Adjust width aspect by height aspect
		else if (100*iTHgt/Hgt<100*iTWdt/Wdt)
			{ 
			iTWdt2=Wdt*iTHgt/Hgt;
			iTx2+=(iTWdt-iTWdt2)/2;
			}
		// Blit
		lpDDraw->BlitSurface2Window(Surface,
																X+Wdt*iPhaseX,Y+Hgt*iPhaseY,Wdt,Hgt,
																hWnd,
																iTx2,iTy2,iTWdt2,iTHgt2
																/*TRUE*/);
		}
	
	else
		{
		// Blit
		lpDDraw->BlitSurface2Window(Surface,
																X+Wdt*iPhaseX,Y+Hgt*iPhaseY,Wdt,Hgt,
																hWnd,
																iTx,iTy,iTWdt,iTHgt
																/*TRUE*/);
		}
	}

void C4Facet::DrawTile(SURFACE sfcTarget, int iX, int iY, int iWdt, int iHgt)
	{
	if (!lpDDraw || !Surface || !Wdt || !Hgt) return;
	// Blits whole source surface, not surface facet area
	lpDDraw->BlitSurfaceTile(Surface,sfcTarget,iX,iY,iWdt,iHgt,0,0,TRUE);
	}

void C4Facet::Expand(int iLeft, int iRight, int iTop, int iBottom)
	{
	X-=iLeft; Wdt+=iLeft;
	Wdt+=iRight;
	Y-=iTop; Hgt+=iTop;
	Hgt+=iBottom;
	}

void C4Facet::Wipe()
	{
	if (!lpDDraw || !Surface || !Wdt || !Hgt) return;
	lpDDraw->WipeSurface(Surface);
	}
