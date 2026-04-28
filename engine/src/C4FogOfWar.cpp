/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A visibility map for each player */

#include <C4Include.h>

const int C4FOW_Resolution = 100, C4FOW_Border = 10, C4FOW_View_RangeX = 500, C4FOW_View_RangeY = 300, C4FOW_Visibility_RangeX = C4FOW_View_RangeX / C4FOW_Resolution,
          C4FOW_Visibility_RangeY = C4FOW_View_RangeY / C4FOW_Resolution;

C4FogOfWar::C4FogOfWar() { Default(); }

C4FogOfWar::~C4FogOfWar() { Clear(); }

void C4FogOfWar::Default() {
  Width = Height = 0;
  Visibility = NULL;
}

void C4FogOfWar::Clear() {
  if (Visibility)
    delete[] Visibility;
  Visibility = NULL;
}

void C4FogOfWar::Init(int iLandscapeWidth, int iLandscapeHeight) {
  // Calculate size and allocate space
  Width = Max(iLandscapeWidth / C4FOW_Resolution + 1, 2 * C4FOW_Visibility_RangeX + 1);
  Height = Max(iLandscapeHeight / C4FOW_Resolution + 1, 2 * C4FOW_Visibility_RangeY + 1);
  Visibility = new BYTE[Width * Height];
  ClearVisibility();
}

void C4FogOfWar::AddVisibility(int iX, int iY) {
  // Mark visible view range
  if (!Visibility)
    return;
  iX = BoundBy(iX / C4FOW_Resolution, 0 + C4FOW_Visibility_RangeX, Width - 1 - C4FOW_Visibility_RangeX);
  iY = BoundBy(iY / C4FOW_Resolution, 0 + C4FOW_Visibility_RangeY, Height - 1 - C4FOW_Visibility_RangeY);
  for (int cy = iY - C4FOW_Visibility_RangeY; cy <= iY + C4FOW_Visibility_RangeY; cy++)
    for (int cx = iX - C4FOW_Visibility_RangeX; cx <= iX + C4FOW_Visibility_RangeX; cx++)
      Visibility[cy * Width + cx] = TRUE;
}

void C4FogOfWar::ClearVisibility() {
  if (!Visibility)
    return;
  ZeroMem(Visibility, Width * Height);
}

void C4FogOfWar::Draw(C4FacetEx &cgo) {
  if (!Visibility)
    return;
  int iVisX = cgo.TargetX / C4FOW_Resolution;
  int iVisY = cgo.TargetY / C4FOW_Resolution;
  int iVisWdt = cgo.Wdt / C4FOW_Resolution + 2;
  int iVisHgt = cgo.Hgt / C4FOW_Resolution + 2;
  for (int cy = 0; cy < iVisHgt; cy++)
    for (int cx = 0; cx < iVisWdt; cx++)
      if (Inside(iVisX + cx, 0, Width - 1) && Inside(iVisY + cy, 0, Height - 1))
        if (!Visibility[(iVisY + cy) * Width + (iVisX + cx)])
          GfxR->fctFogOfWar.Draw(cgo.Surface, cgo.X - cgo.TargetX + (iVisX + cx) * C4FOW_Resolution - C4FOW_Border, cgo.Y - cgo.TargetY + (iVisY + cy) * C4FOW_Resolution - C4FOW_Border);
}

BOOL C4FogOfWar::IsVisible(int iX, int iY) {
  if (!Visibility)
    return FALSE;
  iX /= C4FOW_Resolution;
  iY /= C4FOW_Resolution;
  if (!Inside(iX, 0, Width - 1) || !Inside(iY, 0, Height - 1))
    return FALSE;
  return Visibility[iY * Width + iX];
}
