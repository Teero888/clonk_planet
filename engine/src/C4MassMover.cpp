/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Move liquids in the landscape using individual transport spots */

#include <C4Include.h>

// Note: creation optimized using advancing CreatePtr, so sequential
// creation does not keep rescanning the complete set for a free
// slot. (This had caused extreme delays.) This had the effect that
// MMs created by another MM being executed were oftenly executed
// within the same frame repetitiously leading to long distance mass
// movement in no-time. To avoid this, set execution is done in
// opposite direction. We now have have smoothly running masses in
// a mathematical triangular shape with no delays! Since masses are
// running slower and smoother, overall MM counts are much lower,
// hardly ever exceeding 1000.
// October 1997

C4MassMoverSet::C4MassMoverSet() { Default(); }

C4MassMoverSet::~C4MassMoverSet() { Clear(); }

void C4MassMoverSet::Clear() {}

void C4MassMoverSet::Execute() {
  int cnt, speed = 2;
  C4MassMover *cmm;
  // Init counts
  Count = 0;
  // Execute & count
  for (; speed > 0; speed--)
    for (cnt = 0, cmm = &(Set[C4MassMoverChunk - 1]); cnt < C4MassMoverChunk; cnt++, cmm--)
      if (cmm->Mat != MNone) {
        Count++;
        cmm->Execute();
      }
}

BOOL C4MassMoverSet::Create(int x, int y, BOOL fExecute) {
  int cptr = CreatePtr;
  do {
    cptr++;
    if (cptr >= C4MassMoverChunk)
      cptr = 0;
    if (Set[cptr].Mat == MNone) {
      if (!Set[cptr].Init(x, y))
        return FALSE;
      CreatePtr = cptr;
      if (fExecute)
        Set[cptr].Execute();
      return TRUE;
    }
  } while (cptr != CreatePtr);
  return FALSE;
}

void C4MassMoverSet::Draw() {
  /*int cnt;
  for (cnt=0; cnt<C4MassMoverChunk; cnt++)
    if (Set[cnt].Mat!=MNone)*/
}

BOOL C4MassMover::Init(int tx, int ty) {
  // Out of bounds check
  if (!Inside(tx, 0, GBackWdt - 1) || !Inside(ty, 0, GBackHgt - 1))
    return FALSE;
  // Check mat
  Mat = GBackMat(tx, ty);
  x = tx;
  y = ty;
  return (Mat != MNone);
}

void C4MassMover::Cease() { Mat = MNone; }

BOOL C4MassMover::Execute() {
  int tx, ty;
  C4Material *pMat = &(Game.Material.Map[Mat]);

  // Lost target material
  if (GBackMat(x, y) != Mat) {
    Cease();
    return FALSE;
  }

  // Check for transfer target space
  tx = x;
  ty = y;
  if (!Game.Landscape.FindMatPath(tx, ty, +1, pMat->Density, pMat->MaxSlide)) {
    // Contact material reaction check: corrosion/evaporation
    if (Game.Material.Map[Mat].Corrosive || Game.Material.Map[Mat].Incindiary) {
      if (Corrosion(+0, +1))
        return TRUE;
      if (Corrosion(-1, +0))
        return TRUE;
      if (Corrosion(+1, +0))
        return TRUE;
    }

    // Contact material reaction check: incineration
    if (Game.Material.Map[Mat].Incindiary) {
      if (Game.Landscape.Incinerate(+0, +1))
        return TRUE;
      if (Game.Landscape.Incinerate(-1, +0))
        return TRUE;
      if (Game.Landscape.Incinerate(+1, +0))
        return TRUE;
    }

    // Contact material reaction check: might want to evaporate here...

    // No space, die
    Cease();
    return FALSE;
  }

  // Check target space mat for InMatConvert
  if (pMat->InMatConvert != MNone)
    if (pMat->InMatConvert == GBackMat(tx, ty)) {
      // Conversion-transfer to PXS
      Game.PXS.Create(Game.Landscape.ExtractMaterial(x, y), tx, ty);
      return TRUE;
    }

  // Transfer mass
  SBackPix(tx, ty, Mat2PixColRnd(Game.Landscape.ExtractMaterial(x, y)) + GBackIFT(tx, ty));

  // Create new mover at target
  Game.MassMover.Create(tx, ty, !Rnd3());

  return TRUE;
}

BOOL C4MassMover::Corrosion(int dx, int dy) {
  int tmat = GBackMat(x + dx, y + dy);
  if (MatValid(tmat))
    if ((Game.Material.Map[Mat].Incindiary && Game.Material.Map[tmat].Extinguisher) || ((Random(100) < Game.Material.Map[Mat].Corrosive) && (Random(100) < Game.Material.Map[tmat].Corrode))) {
      Game.Landscape.ExtractMaterial(x, y);
      ClearBackPix(x + dx, y + dy);
      if (!Random(5))
        Smoke(x, y, 3 + Random(3));
      return TRUE;
    }
  return FALSE;
}

void C4MassMoverSet::Default() {
  int cnt;
  for (cnt = 0; cnt < C4MassMoverChunk; cnt++)
    Set[cnt].Mat = MNone;
  Count = 0;
  CreatePtr = 0;
}

BOOL C4MassMoverSet::Save(C4Group &hGroup) {
  int cnt;
  // Consolidate
  Consolidate();
  // Recount
  Count = 0;
  for (cnt = 0; cnt < C4MassMoverChunk; cnt++)
    if (Set[cnt].Mat != MNone)
      Count++;
  // All empty: delete component
  if (!Count) {
    hGroup.Delete(C4CFN_MassMover);
    return TRUE;
  }
  // Save set
  if (!hGroup.Add(C4CFN_MassMover, Set, Count * sizeof(C4MassMover)))
    return FALSE;
  // Success
  return TRUE;
}

BOOL C4MassMoverSet::Load(C4Group &hGroup) {
  int iBinSize, iMoverSize = sizeof(C4MassMover);
  if (!hGroup.AccessEntry(C4CFN_MassMover, &iBinSize))
    return FALSE;
  if ((iBinSize % iMoverSize) != 0)
    return FALSE;
  Count = iBinSize / iMoverSize;
  if (!hGroup.Read(Set, iBinSize))
    return FALSE;
  return TRUE;
}

void C4MassMoverSet::Consolidate() {
  // Consolidate set
  int iSpot, iPtr, iConsolidated;
  for (iSpot = -1, iPtr = 0, iConsolidated = 0; iPtr < C4MassMoverChunk; iPtr++) {
    // Empty: set new spot if needed
    if (Set[iPtr].Mat == MNone) {
      if (iSpot == -1)
        iSpot = iPtr;
    }
    // Full: move down to empty spot if possible
    else if (iSpot != -1) {
      // Move to spot
      Set[iSpot] = Set[iPtr];
      Set[iPtr].Mat = MNone;
      iConsolidated++;
      // Advance empty spot (as far as ptr)
      for (; iSpot < iPtr; iSpot++)
        if (Set[iSpot].Mat == MNone)
          break;
      // No empty spot below ptr
      if (iSpot == iPtr)
        iSpot = -1;
    }
  }
  // Reset create ptr
  CreatePtr = 0;
}

void C4MassMoverSet::Synchronize() { Consolidate(); }

void C4MassMoverSet::Copy(C4MassMoverSet &rSet) {
  Clear();
  Count = rSet.Count;
  CreatePtr = rSet.CreatePtr;
  for (int cnt = 0; cnt < C4MassMoverChunk; cnt++)
    Set[cnt] = rSet.Set[cnt];
}
