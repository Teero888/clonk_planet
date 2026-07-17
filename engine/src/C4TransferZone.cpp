/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Special regions to extend the pathfinder */

#include <C4Include.h>

C4TransferZone::C4TransferZone() { Default(); }

C4TransferZone::~C4TransferZone() { Clear(); }

void C4TransferZone::Default() {
  Object = NULL;
  X = Y = Wdt = Hgt = 0;
  Next = NULL;
  Used = FALSE;
}

void C4TransferZone::Clear() {}

C4TransferZones::C4TransferZones() { Default(); }

C4TransferZones::~C4TransferZones() { Clear(); }

void C4TransferZones::Default() { First = NULL; }

void C4TransferZones::Clear() {
  C4TransferZone *pZone, *pNext;
  for (pZone = First; pZone; pZone = pNext) {
    pNext = pZone->Next;
    delete pZone;
  }
  First = NULL;
}

void C4TransferZones::ClearPointers(C4Object *pObj) {
  // Clear object pointers
  for (C4TransferZone *pZone = First; pZone; pZone = pZone->Next)
    if (pZone->Object == pObj)
      pZone->Object = NULL;
  // Remove cleared zones immediately
  RemoveNullZones();
}

BOOL C4TransferZones::Set(int iX, int iY, int iWdt, int iHgt, C4Object *pObj) {
  C4TransferZone *pZone;
  // Empty zone: clear existing object zones
  if (!iWdt || !iHgt) {
    ClearPointers(pObj);
    return TRUE;
  }
  // Update existing zone
  if (pZone = Find(pObj)) {
    pZone->X = iX;
    pZone->Y = iY;
    pZone->Wdt = iWdt;
    pZone->Hgt = iHgt;
  }
  // Allocate and add new zone
  else
    Add(iX, iY, iWdt, iHgt, pObj);
  // Success
  return TRUE;
}

BOOL C4TransferZones::Add(int iX, int iY, int iWdt, int iHgt, C4Object *pObj) {
  C4TransferZone *pZone;
  // Allocate and add new zone
  pZone = new C4TransferZone;
  pZone->X = iX;
  pZone->Y = iY;
  pZone->Wdt = iWdt;
  pZone->Hgt = iHgt;
  pZone->Object = pObj;
  pZone->Next = First;
  First = pZone;
  // Success
  return TRUE;
}

void C4TransferZones::Synchronize() {
  Clear();
  if (Game.FrameCounter > 0)
    Game.Objects.UpdateTransferZones();
}

C4TransferZone *C4TransferZones::Find(int iX, int iY) {
  for (C4TransferZone *pZone = First; pZone; pZone = pZone->Next)
    if (Inside(iX - pZone->X, 0, pZone->Wdt - 1))
      if (Inside(iY - pZone->Y, 0, pZone->Hgt - 1))
        return pZone;
  return NULL;
}

void C4TransferZones::Draw(C4FacetEx &cgo) {
  for (C4TransferZone *pZone = First; pZone; pZone = pZone->Next)
    pZone->Draw(cgo);
}

void C4TransferZone::Draw(C4FacetEx &cgo, BOOL fHighlight) {
  if (Used)
    fHighlight = TRUE;
  lpDDraw->DrawFrame(cgo.Surface, cgo.X + X - cgo.TargetX, cgo.Y + Y - cgo.TargetY, cgo.X + X - cgo.TargetX + Wdt - 1, cgo.Y + Y - cgo.TargetY + Hgt - 1, fHighlight ? CGreen : CRed);
}

BOOL C4TransferZone::At(int iX, int iY) { return (Inside(iX - X, 0, Wdt - 1) && Inside(iY - Y, 0, Hgt - 1)); }

int C4TransferZones::RemoveNullZones() {
  int iResult = 0;
  C4TransferZone *pZone, *pNext, *pPrev = NULL;
  for (pZone = First; pZone; pZone = pNext) {
    pNext = pZone->Next;
    if (!pZone->Object) {
      delete pZone;
      if (pPrev)
        pPrev->Next = pNext;
      else
        First = pNext;
      iResult++;
    } else
      pPrev = pZone;
  }
  return iResult;
}

void AdjustMoveToTarget(int &rX, int &rY, BOOL fFreeMove, int iShapeHgt);

BOOL C4TransferZone::GetEntryPoint(int &rX, int &rY, int iToX, int iToY) {
  // Target inside zone: move outside horizontally
  if (Inside(iToX - X, 0, Wdt - 1) && Inside(iToY - Y, 0, Hgt - 1))
    if (iToX < X + Wdt / 2)
      iToX = X - 1;
    else
      iToX = X + Wdt;
  // Get closest adjacent point
  rX = BoundBy(iToX, X - 1, X + Wdt);
  rY = BoundBy(iToY, Y - 1, Y + Hgt);
  // Search around zone for free
  int iX1 = rX, iY1 = rY, iX2 = rX, iY2 = rY;
  int iXIncr1 = 0, iYIncr1 = -1, iXIncr2 = 0, iYIncr2 = +1;
  int cnt;
  for (cnt = 0; cnt < 2 * Wdt + 2 * Hgt; cnt++) {
    // Found free
    if (!GBackSolid(iX1, iY1)) {
      rX = iX1;
      rY = iY1;
      break;
    }
    if (!GBackSolid(iX2, iY2)) {
      rX = iX2;
      rY = iY2;
      break;
    }
    // Advance
    iX1 += iXIncr1;
    iY1 += iYIncr1;
    iX2 += iXIncr2;
    iY2 += iYIncr2;
    // Corners
    if (iY1 < Y - 1) {
      iY1 = Y - 1;
      iXIncr1 = +1;
      iYIncr1 = 0;
    }
    if (iX1 > X + Wdt) {
      iX1 = X + Wdt;
      iXIncr1 = 0;
      iYIncr1 = +1;
    }
    if (iY1 > Y + Hgt) {
      iY1 = Y + Hgt;
      iXIncr1 = -1;
      iYIncr1 = 0;
    }
    if (iX1 < X - 1) {
      iX1 = X - 1;
      iXIncr1 = 0;
      iYIncr1 = -1;
    }
    if (iY2 < Y - 1) {
      iY2 = Y - 1;
      iXIncr2 = -1;
      iYIncr2 = 0;
    }
    if (iX2 > X + Wdt) {
      iX2 = X + Wdt;
      iXIncr2 = 0;
      iYIncr2 = -1;
    }
    if (iY2 > Y + Hgt) {
      iY2 = Y + Hgt;
      iXIncr2 = +1;
      iYIncr2 = 0;
    }
    if (iX2 < X - 1) {
      iX2 = X - 1;
      iXIncr2 = 0;
      iYIncr2 = +1;
    }
  }
  // No free found
  if (cnt >= 2 * Wdt + 2 * Hgt)
    return FALSE;
  // Vertical walk-to adjust (only if at the side of zone)
  if (!Inside(rX - X, 0, Wdt - 1))
    AdjustMoveToTarget(rX, rY, FALSE, 20);
  // Success
  return TRUE;
}

void C4TransferZones::ClearUsed() {
  for (C4TransferZone *pZone = First; pZone; pZone = pZone->Next)
    pZone->Used = FALSE;
}

C4TransferZone *C4TransferZones::Find(C4Object *pObj) {
  for (C4TransferZone *pZone = First; pZone; pZone = pZone->Next)
    if (pZone->Object == pObj)
      return pZone;
  return NULL;
}
