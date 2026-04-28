/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Move liquids in the landscape using individual transport spots */

const int C4MassMoverChunk = 10000;

class C4MassMoverSet;

class C4MassMover {
  friend C4MassMoverSet;

protected:
  int Mat, x, y;

protected:
  void Cease();
  BOOL Execute();
  BOOL Init(int tx, int ty);
  BOOL Corrosion(int dx, int dy);
};

class C4MassMoverSet {
public:
  C4MassMoverSet();
  ~C4MassMoverSet();

public:
  int Count;
  int CreatePtr;

protected:
  C4MassMover Set[C4MassMoverChunk];

public:
  void Copy(C4MassMoverSet &rSet);
  void Synchronize();
  void Default();
  void Clear();
  void Draw();
  void Execute();
  BOOL Create(int x, int y, BOOL fExecute = FALSE);
  BOOL Load(C4Group &hGroup);
  BOOL Save(C4Group &hGroup);

protected:
  void Consolidate();
};
