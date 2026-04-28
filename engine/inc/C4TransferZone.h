/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Special regions to extend the pathfinder */

class C4TransferZones;

class C4TransferZone {
  friend C4TransferZones;

public:
  C4TransferZone();
  ~C4TransferZone();

public:
  C4Object *Object;
  int X, Y, Wdt, Hgt;
  BOOL Used;

protected:
  C4TransferZone *Next;

public:
  BOOL GetEntryPoint(int &rX, int &rY, int iToX, int iToY);
  void Draw(C4FacetEx &cgo, BOOL fHighlight = FALSE);
  void Default();
  void Clear();
  BOOL At(int iX, int iY);
};

class C4TransferZones {
public:
  C4TransferZones();
  ~C4TransferZones();

protected:
  int RemoveNullZones();
  C4TransferZone *First;

public:
  void Default();
  void Clear();
  void ClearUsed();
  void ClearPointers(C4Object *pObj);
  void Draw(C4FacetEx &cgo);
  void Synchronize();
  C4TransferZone *Find(C4Object *pObj);
  C4TransferZone *Find(int iX, int iY);
  BOOL Add(int iX, int iY, int iWdt, int iHgt, C4Object *pObj);
  BOOL Set(int iX, int iY, int iWdt, int iHgt, C4Object *pObj);
};
