/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Captures uncompressed AVI from game view */

class C4Video {
public:
  C4Video();
  ~C4Video();

protected:
  bool Active;
  PAVIFILE pAviFile;
  PAVISTREAM pAviStream;
  int AviFrame;
  double AspectRatio;
  int X, Y, Width, Height;
  BYTE *Buffer;
  int BufferSize;
  int InfoSize;
  bool Recording;
  SURFACE Surface;
  int ShowFlash;

public:
  void Draw();
  void Draw(C4FacetEx &cgo);
  void Resize(int iChange);
  bool Start(const char *szFilename);
  void Default();
  void Init(SURFACE sfcSource, int iWidth = 200, int iHeight = 150);
  void Clear();
  bool Start();
  bool Stop();
  bool Toggle();
  void Execute();
  void Reduce();
  void Enlarge();

protected:
  bool RecordFrame();
  bool AdjustPosition();
};
