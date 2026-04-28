/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Small member of the landscape class to handle the sky background */

class C4Sky {
public:
  C4Sky();
  ~C4Sky();

public:
  SURFACE Surface;
  int Width, Height;
  BYTE Palette[256 * 3];

public:
  BOOL Init();
  BOOL Save(C4Group &hGroup);
  void Clear();
  void SetColor(int iIndex, int iRed, int iGreen, int iBlue);
  void SetFadePalette(int *ipColors);

protected:
  SURFACE CreateColorFadeSky();
};