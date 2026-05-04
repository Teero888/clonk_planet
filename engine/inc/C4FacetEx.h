/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A facet that can hold its own surface and also target coordinates */

const int C4FCT_Full = -1, C4FCT_Height = -2, C4FCT_Width = -3;

class C4FacetEx : public C4Facet {
public:
  ~C4FacetEx();
  C4FacetEx();
  C4FacetEx(const C4FacetEx &rhs) {
    Default();
    Set(rhs.Surface, rhs.X, rhs.Y, rhs.Wdt, rhs.Hgt, rhs.TargetX, rhs.TargetY);
  }

public:
  int TargetX, TargetY;

protected:
  C4Surface Face;

public:
  void Default();
  void Clear();
  void Set(SURFACE nsfc, int nx, int ny, int nwdt, int nhgt, int ntx = 0, int nty = 0);
  void SetPalette(BYTE *bpPalette, BOOL fAdapt);
  void DrawBolt(int iX1, int iY1, int iX2, int iY2, BYTE bCol);
  void DrawLine(int iX1, int iY1, int iX2, int iY2, BYTE bCol1, BYTE bCol2);
  BOOL Create(int iWdt, int iHgt, int iWdt2 = C4FCT_Full, int iHgt2 = C4FCT_Full);
  BOOL Load(C4Group &hGroup, const char *szName, int iWdt = C4FCT_Full, int iHgt = C4FCT_Full);
  BOOL Load(BYTE *bpBitmap, int iWdt = C4FCT_Full, int iHgt = C4FCT_Full);
  BOOL Save(C4Group &hGroup, const char *szName);
  BYTE *GetPalette();
  C4FacetEx GetSection(int iSection);
  C4FacetEx GetPhase(int iPhaseX = 0, int iPhaseY = 0);

public:
  BOOL ShiftColor(int iShift, int iStart = 0, int iEnd = 255);
  C4FacetEx operator=(const C4Facet &rhs) {
    Clear();
    Default();
    Set(rhs.Surface, rhs.X, rhs.Y, rhs.Wdt, rhs.Hgt);
    return *this;
  }
  C4FacetEx operator=(const C4FacetEx &rhs) {
    Clear();
    Default();
    Set(rhs.Surface, rhs.X, rhs.Y, rhs.Wdt, rhs.Hgt, rhs.TargetX, rhs.TargetY);
    return *this;
  }
};