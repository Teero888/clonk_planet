/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A piece of a DirectDraw surface */

const int C4FCT_None = 0,

          C4FCT_Left = 1, C4FCT_Right = 2, C4FCT_Top = 4, C4FCT_Bottom = 8,
          C4FCT_Center = 16,

          C4FCT_Alignment = C4FCT_Left | C4FCT_Right | C4FCT_Top |
                            C4FCT_Bottom | C4FCT_Center,

          C4FCT_Half = 32, C4FCT_Double = 64, C4FCT_Triple = 128;

class C4Facet {
public:
  C4Facet();

public:
  SURFACE Surface;
  int X, Y, Wdt, Hgt;

public:
  void Default();
  void Set(CSurface &rSfc);
  void Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt);
  void Wipe();
  void Expand(int iLeft = 0, int iRight = 0, int iTop = 0, int iBottom = 0);
  void DrawTile(SURFACE sfcTarget, int iX, int iY, int iWdt, int iHgt);
  void Draw(HWND hWnd, int iTx, int iTy, int iTWdt, int iTHgt,
            BOOL fAspect = TRUE, int iPhaseX = 0, int iPhaseY = 0);
  void DrawEnergyLevel(int iLevel, int iRange, int iColor = 45);
  void DrawX(SURFACE sfcTarget, int iX, int iY, int iWdt, int iHgt,
             int iPhaseX = 0, int iPhaseY = 0);
  void DrawValue(C4Facet &cgo, int iValue, int iPhaseX = 0, int iPhaseY = 0,
                 int iAlign = C4FCT_Center);
  void DrawValue2(C4Facet &cgo, int iValue1, int iValue2, int iPhaseX = 0,
                  int iPhaseY = 0, int iAlign = C4FCT_Center);
  void Draw(C4Facet &cgo, BOOL fAspect = TRUE, int iPhaseX = 0,
            int iPhaseY = 0);
  void Draw(SURFACE sfcTarget, int iX, int iY, int iPhaseX = 0,
            int iPhaseY = 0);
  void DrawSectionSelect(C4Facet &cgo, int iSelection, int iMaxSelect);
  C4Facet GetSection(int iSection);
  C4Facet TruncateSection(int iAlign = C4FCT_Left, BOOL fHalf = FALSE);
  C4Facet Truncate(int iAlign, int iSize);
  int GetSectionCount();
};
