/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Textures used by the landscape */

class C4Texture {
  friend C4TextureMap;

public:
  C4Texture();
  ~C4Texture();

protected:
  char Name[C4M_MaxName + 1];
  SURFACE Surface;
  C4Texture *Next;
};

class C4TexMapEntry {
  friend C4TextureMap;

protected:
  BYTE Index; // Range 0-127, Bit 8 means IFT ("in-front-of-tunnel")
  char MaterialTexture[C4M_MaxDefName + 1]; // "Material-Texture"
  C4TexMapEntry *Next;
};

class C4TextureMap {
public:
  C4TextureMap();
  ~C4TextureMap();

protected:
  C4TexMapEntry *FirstEntry;
  C4Texture *FirstTexture;

public:
  void Default();
  void Clear();
  void StoreMapPalette(BYTE *bypPalette, C4MaterialMap &rMaterials);
  int LoadMap(C4Group &hGroup, const char *szEntryName);
  int LoadTextures(C4Group &hGroup);
  const char *GetTexture(int iIndex);
  const char *GetMaterialTexture(BYTE byIndex);
  BYTE GetIndex(const char *szMaterialTexture, BOOL fAddIfNotExist = TRUE);
  SURFACE GetTexture(const char *szTexture);

protected:
  BOOL AddEntry(BYTE byIndex, const char *szMaterialTexture);
  BOOL AddTexture(const char *szTexture, SURFACE sfcSurface);
};
