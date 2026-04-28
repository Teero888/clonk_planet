/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some useful wrappers to globals */

//=================================== ID2Def
//==============================================

inline C4Def *C4Id2Def(C4ID id) { return Game.Defs.ID2Def(id); }

//==================================== IFT
//===============================================

inline BYTE PixColIFT(BYTE pixc) {
  if (pixc >= GBM + IFT)
    return IFT;
  return 0;
}

//==================================== Density
//===========================================

inline int DensitySolid(int dens) { return (dens >= C4M_Solid); }

inline int DensitySemiSolid(int dens) { return (dens >= C4M_SemiSolid); }

inline int DensityLiquid(int dens) {
  return ((dens >= C4M_Liquid) && (dens < C4M_Solid));
}

//==================================== Materials
//=========================================

extern int MVehic, MTunnel, MWater, MSnow, MEarth, MGranite;

inline BOOL MatValid(int mat) { return Inside(mat, 0, Game.Material.Num - 1); }

inline int PixCol2Mat(BYTE pixc) {
  if (pixc < GBM)
    return MNone;
  pixc &= 63; // Substract GBM, ignore IFT
  if (pixc > Game.Material.Num * C4M_ColsPerMat - 1)
    return MNone;
  return pixc / C4M_ColsPerMat;
}

inline int PixColMatIndex(BYTE pixc) {
  if (pixc < GBM)
    return 0;
  pixc &= 63; // Substract GBM, ignore IFT
  if (pixc > Game.Material.Num * C4M_ColsPerMat - 1)
    return 0;
  return pixc % C4M_ColsPerMat;
}

inline BYTE Mat2PixCol(int mat) {
  if (!MatValid(mat))
    return 0;
  return (BYTE)(GBM + mat * C4M_ColsPerMat);
}

inline BYTE Mat2PixColRnd(int mat) {
  return (BYTE)(Mat2PixCol(mat) + 1 + (BYTE)Rnd3());
}

inline int MatDensity(int mat) {
  if (!MatValid(mat))
    return 0;
  return Game.Material.Map[mat].Density;
}

inline int MatDigFree(int mat) {
  if (!MatValid(mat))
    return 1;
  return Game.Material.Map[mat].DigFree;
}

#define GBackWdt Game.Landscape.Width
#define GBackHgt Game.Landscape.Height
#define GBackPix Game.Landscape.GetPix
#define SBackPix Game.Landscape.SetPix
#define ClearBackPix Game.Landscape.ClearPix

inline BYTE GBackIFT(int x, int y) { return PixColIFT(GBackPix(x, y)); }

inline int GBackMat(int x, int y) { return PixCol2Mat(GBackPix(x, y)); }

inline int GBackMatColIndex(int x, int y) {
  return PixColMatIndex(GBackPix(x, y));
}

inline int GBackDensity(int x, int y) { return MatDensity(GBackMat(x, y)); }

inline int GBackSolid(int x, int y) { return DensitySolid(GBackDensity(x, y)); }

inline int GBackSemiSolid(int x, int y) {
  return DensitySemiSolid(GBackDensity(x, y));
}

inline int GBackLiquid(int x, int y) {
  return DensityLiquid(GBackDensity(x, y));
}

//=============================== Sound
//==================================================

// Plays a sound effect from the bank.
//     iMode  0 Single Effect
//           +1 Increase Loop Level
//           -1 Decrease Loop Level

inline BOOL SoundEffect(const char *szSndName, int iMode = 0, int iVolume = 100,
                        C4Object *pObj = NULL) {
  if (pObj)
    iVolume = pObj->Audible * iVolume / 100;
  return Game.SoundSystem.PlayEffect(szSndName, iMode, iVolume);
}

inline BOOL SoundEffectPlayer(const char *szSndName, int iPlayer, int iMode = 0,
                              int iVolume = 100) {
  C4Player *pPlr;
  if (!(pPlr = Game.Players.Get(iPlayer)))
    return FALSE;
  if (!pPlr->LocalControl)
    return FALSE;
  return Game.SoundSystem.PlayEffect(szSndName, iMode, iVolume);
}

// Controls a continuous sound.
// If the sound is not playing, loops the sound at vol.
// If the sound is playing, sets the volume.
// If the volume is zero, stops the sound.

inline BOOL SoundLevel(const char *szSndName, int iVolume = 100) {
  return Game.SoundSystem.SoundLevel(szSndName, iVolume);
}

//=========================== Graphics Resource
//=========================================

#define GfxR (&(Game.GraphicsResource))

//=============================== Messages
//=============================================

inline void GameMsgObject(const char *szText, C4Object *pTarget,
                          int iFCol = FWhite) {
  Game.Messages.New(szText, pTarget, NO_OWNER, -1, -1, iFCol);
}

inline void GameMsgGlobal(const char *szText, int iFCol = FWhite) {
  Game.Messages.New(szText, NULL, ANY_OWNER, -1, -1, iFCol);
}

inline void GameMsgPlayer(const char *szText, int iPlayer, int iFCol = FWhite) {
  Game.Messages.New(szText, NULL, iPlayer, -1, -1, iFCol);
}

//============================== DirectDraw
//==============================================

inline void DestroySurface(SURFACE sfcSurface) {
  Engine.DDraw.DestroySurface(sfcSurface);
}

inline SURFACE DuplicateSurface(SURFACE sfcSurface) {
  return Engine.DDraw.DuplicateSurface(sfcSurface);
}

inline SURFACE CreateSurface(int iWdt, int iHgt) {
  return Engine.DDraw.CreateSurface(iWdt, iHgt);
}

inline BYTE *LockSurface(SURFACE sfcSurface, int &lPitch, int *lpSfcWdt = 0,
                         int *lpSfcHgt = 0) {
  return Engine.DDraw.LockSurface(sfcSurface, lPitch, lpSfcWdt, lpSfcHgt);
}

inline void UnLockSurface(SURFACE sfcSurface) {
  Engine.DDraw.UnLockSurface(sfcSurface);
}

inline BOOL GetSurfaceSize(SURFACE sfcSurface, int &iWdt, int &iHgt) {
  return Engine.DDraw.GetSurfaceSize(sfcSurface, iWdt, iHgt);
}

//===================================== Ticks
//==========================================

#define Tick2 Game.iTick2
#define Tick3 Game.iTick3
#define Tick5 Game.iTick5
#define Tick10 Game.iTick10
#define Tick35 Game.iTick35
#define Tick255 Game.iTick255
#define Tick500 Game.iTick500
#define Tick1000 Game.iTick1000

//================================== Players
//============================================

inline int ValidPlr(int plr) { return Game.Players.Valid(plr); }

inline int Hostile(int plr1, int plr2) {
  return Game.Players.Hostile(plr1, plr2);
}
