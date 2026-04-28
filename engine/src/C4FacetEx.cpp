/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A facet that can hold its own surface and also target coordinates */

#include <C4Include.h>

void C4FacetEx::Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt, int ntx, int nty) {
  C4Facet::Set(nsfc, nx, ny, nwdt, nhgt);
  TargetX = ntx;
  TargetY = nty;
}

void C4FacetEx::DrawLine(int iX1, int iY1, int iX2, int iY2, BYTE bCol1, BYTE bCol2) {
  if (!lpDDraw || !Surface || !Wdt || !Hgt)
    return;
  // Scroll position
  iX1 -= TargetX;
  iY1 -= TargetY;
  iX2 -= TargetX;
  iY2 -= TargetY;
  // Facet bounds (no cut sections drawn)
  if (!Inside(iX1, 0, Wdt - 1) && !Inside(iX2, 0, Wdt - 1))
    return;
  if (!Inside(iY1, 0, Hgt - 1) && !Inside(iY2, 0, Hgt - 1))
    return;
  // Draw line
  lpDDraw->DrawLine(Surface, X + iX1, Y + iY1, X + iX2, Y + iY2, bCol1);
  lpDDraw->SetPixel(Surface, X + iX1, Y + iY1, bCol2);
}

void C4FacetEx::DrawBolt(int iX1, int iY1, int iX2, int iY2, BYTE bCol) {
  if (!lpDDraw || !Surface || !Wdt || !Hgt)
    return;
  // Scroll position
  iX1 -= TargetX;
  iY1 -= TargetY;
  iX2 -= TargetX;
  iY2 -= TargetY;
  // Facet bounds
  if (!Inside(iX1, 0, Wdt - 1) && !Inside(iX2, 0, Wdt - 1))
    return;
  if (!Inside(iY1, 0, Hgt - 1) && !Inside(iY2, 0, Hgt - 1))
    return;
  // Draw bolt
  int pvtx[2 * 4];
  pvtx[0] = X + iX1;
  pvtx[1] = Y + iY1;
  pvtx[2] = X + iX2;
  pvtx[3] = Y + iY2;
#ifdef C4ENGINE
  pvtx[4] = X + iX2 + SafeRandom(3) - 1;
  pvtx[5] = Y + iY2 + SafeRandom(3) - 1;
  pvtx[6] = X + iX1 + SafeRandom(3) - 1;
  pvtx[7] = Y + iY1 + SafeRandom(3) - 1;
#else
  pvtx[4] = X + iX2 + X % 3 - 1;
  pvtx[5] = Y + iY2 + X % 3 - 1;
  pvtx[6] = X + iX1 + Y % 3 - 1;
  pvtx[7] = Y + iY1 + Y % 3 - 1;
#endif
  // Draw in surface
  lpDDraw->DrawPolygon(Surface, 4, pvtx, bCol);
}

C4FacetEx::C4FacetEx() { Default(); }

C4FacetEx::~C4FacetEx() { Clear(); }

void C4FacetEx::Default() {
  C4Facet::Default();
  Face.Default();
  TargetX = TargetY = 0;
}

void C4FacetEx::Clear() {
  Face.Clear();
  Surface = NULL;
}

BOOL C4FacetEx::Create(int iWdt, int iHgt, int iWdt2, int iHgt2) {
  Clear();
  // Create surface
  if (!Face.Create(iWdt, iHgt))
    return FALSE;
  // Set facet
  if (iWdt2 == C4FCT_Full)
    iWdt2 = Face.Wdt;
  if (iWdt2 == C4FCT_Height)
    iWdt2 = Face.Hgt;
  if (iWdt2 == C4FCT_Width)
    iWdt2 = Face.Wdt;
  if (iHgt2 == C4FCT_Full)
    iHgt2 = Face.Hgt;
  if (iHgt2 == C4FCT_Height)
    iHgt2 = Face.Hgt;
  if (iHgt2 == C4FCT_Width)
    iHgt2 = Face.Wdt;
  Set(Face.Surface, 0, 0, iWdt2, iHgt2, 0, 0);
  return TRUE;
}

BOOL C4FacetEx::Save(C4Group &hGroup, const char *szName) {
  // Empty
  if (!Wdt || !Hgt)
    return FALSE;
  // Full surface
  if ((Wdt == Face.Wdt) && (Hgt == Face.Hgt)) {
    if (!Face.Save(hGroup, szName))
      return FALSE;
  }
  // Surface section
  else {
    C4Surface sfcFacet;
    if (!sfcFacet.Create(Wdt, Hgt))
      return FALSE;
    sfcFacet.SetPalette(GetPalette(), FALSE);
    Draw(sfcFacet.Surface, 0, 0);
    if (!sfcFacet.Save(hGroup, szName))
      return FALSE;
  }
  // Success
  return TRUE;
}

BOOL C4FacetEx::Load(C4Group &hGroup, const char *szName, int iWdt, int iHgt) {
  Clear();
  // Entry name
  char szFilename[_MAX_FNAME + 1];
  SCopy(szName, szFilename, _MAX_FNAME);
  DefaultExtension(szFilename, "bmp");
  // Load surface
  if (!Face.Load(hGroup, szFilename))
    return FALSE;
  // Set facet
  if (iWdt == C4FCT_Full)
    iWdt = Face.Wdt;
  if (iWdt == C4FCT_Height)
    iWdt = Face.Hgt;
  if (iWdt == C4FCT_Width)
    iWdt = Face.Wdt;
  if (iHgt == C4FCT_Full)
    iHgt = Face.Hgt;
  if (iHgt == C4FCT_Height)
    iHgt = Face.Hgt;
  if (iHgt == C4FCT_Width)
    iHgt = Face.Wdt;
  Set(Face.Surface, 0, 0, iWdt, iHgt, 0, 0);
  return TRUE;
}

C4FacetEx C4FacetEx::GetSection(int iSection) {
  C4FacetEx fctResult;
  fctResult.Set(Surface, X + Hgt * iSection, Y, Hgt, Hgt, 0, 0);
  return fctResult;
}

C4FacetEx C4FacetEx::GetPhase(int iPhaseX, int iPhaseY) {
  C4FacetEx fctResult;
  fctResult.Set(Surface, X + Wdt * iPhaseX, Y + Hgt * iPhaseY, Wdt, Hgt, 0, 0);
  return fctResult;
}

BYTE *C4FacetEx::GetPalette() { return Face.Palette; }

BOOL C4FacetEx::Load(BYTE *bpBitmap, int iWdt, int iHgt) {
  Clear();
  // Load surface
  if (!Face.CSurface::Load(bpBitmap))
    return FALSE;
  // Set facet
  if (iWdt == C4FCT_Full)
    iWdt = Face.Wdt;
  if (iWdt == C4FCT_Height)
    iWdt = Face.Hgt;
  if (iWdt == C4FCT_Width)
    iWdt = Face.Wdt;
  if (iHgt == C4FCT_Full)
    iHgt = Face.Hgt;
  if (iHgt == C4FCT_Height)
    iHgt = Face.Hgt;
  if (iHgt == C4FCT_Width)
    iHgt = Face.Wdt;
  Set(Face.Surface, 0, 0, iWdt, iHgt, 0, 0);
  return TRUE;
}

void C4FacetEx::SetPalette(BYTE *bpPalette, BOOL fAdapt) { Face.SetPalette(bpPalette, fAdapt); }

BOOL C4FacetEx::ShiftColor(int iShift, int iStart, int iEnd) {
  if (!lpDDraw || !Surface || !Wdt || !Hgt)
    return FALSE;
  BYTE *bpBuf, *bpPix;
  int cx, cy, iPitch;
  if (!(bpBuf = lpDDraw->LockSurface(Face.Surface, iPitch)))
    return FALSE;
  for (cy = Y; cy < Y + Hgt; cy++) {
    bpPix = bpBuf + iPitch * cy + X;
    for (cx = X; cx < X + Wdt; cx++, bpPix++)
      if (Inside(*bpPix, iStart, iEnd))
        *bpPix += iShift;
  }
  lpDDraw->UnLockSurface(Face.Surface);
  return TRUE;
}