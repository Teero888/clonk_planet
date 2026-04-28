/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Textures used by the landscape */

#include <C4Include.h>

C4Texture::C4Texture() {
  Name[0] = 0;
  Surface = NULL;
  Next = NULL;
}

C4Texture::~C4Texture() {
#ifdef C4ENGINE
  if (Surface)
    DestroySurface(Surface);
#endif
}

C4TextureMap::C4TextureMap() { Default(); }

C4TextureMap::~C4TextureMap() { Clear(); }

BOOL C4TextureMap::AddEntry(BYTE byIndex, const char *szMaterialTexture) {
  C4TexMapEntry *pEntry;
  if (!(pEntry = new C4TexMapEntry))
    return FALSE;
  pEntry->Index = byIndex;
  SCopy(szMaterialTexture, pEntry->MaterialTexture, 2 * C4M_MaxName + 1);
  pEntry->Next = FirstEntry;
  FirstEntry = pEntry;
  return TRUE;
}

BOOL C4TextureMap::AddTexture(const char *szTexture, SURFACE sfcSurface) {
  C4Texture *pTexture;
  if (!(pTexture = new C4Texture))
    return FALSE;
  SCopy(szTexture, pTexture->Name, C4M_MaxName);
  pTexture->Surface = sfcSurface;
  pTexture->Next = FirstTexture;
  FirstTexture = pTexture;
  return TRUE;
}

void C4TextureMap::Clear() {
  C4TexMapEntry *centry, *next;
  for (centry = FirstEntry; centry; centry = next) {
    next = centry->Next;
    delete centry;
  }
  FirstEntry = NULL;
  C4Texture *ctex, *next2;
  for (ctex = FirstTexture; ctex; ctex = next2) {
    next2 = ctex->Next;
    delete ctex;
  }
  FirstTexture = NULL;
}

int C4TextureMap::LoadMap(C4Group &hGroup, const char *szEntryName) {
  BYTE *bpMap;
  char szLine[100 + 1];
  int iTextures = 0, cnt;
  int iIndex;
  // Load text file into memory
  if (!hGroup.LoadEntry(szEntryName, &bpMap, NULL, 1))
    return 0;
  // Scan text buffer lines
  for (cnt = 0; SCopySegment((char *)bpMap, cnt, szLine, 0x0A, 100); cnt++)
    if ((szLine[0] != '#') && (SCharCount('=', szLine) == 1)) {
      SReplaceChar(szLine, 0x0D, 0x00);
      if (Inside(iIndex = strtol(szLine, NULL, 10), 0, C4M_MaxTexIndex - 1))
        if (AddEntry(iIndex, szLine + SCharPos('=', szLine) + 1))
          iTextures++;
    }
  // Delete buffer, return entry count
  delete[] bpMap;
  return iTextures;
}

int C4TextureMap::LoadTextures(C4Group &hGroup) {
  int texnum = 0;

#ifdef C4ENGINE

  char texname[256 + 1];
  SURFACE ctex;
  int binlen;
  // Load all bitmap files from group
  hGroup.ResetSearch();
  while (hGroup.AccessNextEntry(C4CFN_BitmapFiles, &binlen, texname))
    if (ctex = GroupReadSurface(hGroup)) {
      Engine.DDraw.SurfaceAllowColor(ctex, 0, 2, TRUE);
      SReplaceChar(texname, '.', 0);
      if (AddTexture(texname, ctex))
        texnum++;
      else
        DestroySurface(ctex);
    }

#endif

  return texnum;
}

const char *C4TextureMap::GetMaterialTexture(BYTE byIndex) {
  C4TexMapEntry *pEntry;
  for (pEntry = FirstEntry; pEntry; pEntry = pEntry->Next)
    if (pEntry->Index == byIndex)
      return pEntry->MaterialTexture;
  return NULL;
}

BYTE C4TextureMap::GetIndex(const char *szMaterialTexture, BOOL fAddIfNotExist) {
  C4TexMapEntry *pEntry;
  BYTE byIndex;
  // Find existing
  for (pEntry = FirstEntry; pEntry; pEntry = pEntry->Next)
    if (SEqualNoCase(pEntry->MaterialTexture, szMaterialTexture))
      return pEntry->Index;
  // Add new entry
  if (fAddIfNotExist)
    for (byIndex = 1; byIndex < C4M_MaxTexIndex; byIndex++)
      if (!GetMaterialTexture(byIndex)) {
        if (AddEntry(byIndex, szMaterialTexture))
          return byIndex;
        return 0;
      }
  // Else, fail
  return 0;
}

SURFACE C4TextureMap::GetTexture(const char *szTexture) {
  C4Texture *pTexture;
  for (pTexture = FirstTexture; pTexture; pTexture = pTexture->Next)
    if (SEqualNoCase(pTexture->Name, szTexture))
      return pTexture->Surface;
  return NULL;
}

const char *C4TextureMap::GetTexture(int iIndex) {
  C4Texture *pTexture;
  int cindex;
  for (pTexture = FirstTexture, cindex = 0; pTexture; pTexture = pTexture->Next, cindex++)
    if (cindex == iIndex)
      return pTexture->Name;
  return NULL;
}

void C4TextureMap::Default() {
  FirstEntry = NULL;
  FirstTexture = NULL;
}

void C4TextureMap::StoreMapPalette(BYTE *bypPalette, C4MaterialMap &rMaterial) {
  // Zero palette
  ZeroMem(bypPalette, 256 * 3);
  // Sky color
  bypPalette[0] = 192;
  bypPalette[1] = 196;
  bypPalette[2] = 252;
  // Material colors by texture map entries
  C4TexMapEntry *pEntry;
  char szMaterial[C4M_MaxName + 1];
  int iMaterial;
  for (pEntry = FirstEntry; pEntry; pEntry = pEntry->Next) {
    // Find material
    SCopyUntil(pEntry->MaterialTexture, szMaterial, '-', C4M_MaxName);
    if ((iMaterial = rMaterial.Get(szMaterial)) != MNone) {
      bypPalette[3 * pEntry->Index + 0] = rMaterial.Map[iMaterial].Color[6];
      bypPalette[3 * pEntry->Index + 1] = rMaterial.Map[iMaterial].Color[7];
      bypPalette[3 * pEntry->Index + 2] = rMaterial.Map[iMaterial].Color[8];
      bypPalette[3 * (pEntry->Index + C4M_MaxTexIndex) + 0] = rMaterial.Map[iMaterial].Color[3];
      bypPalette[3 * (pEntry->Index + C4M_MaxTexIndex) + 1] = rMaterial.Map[iMaterial].Color[4];
      bypPalette[3 * (pEntry->Index + C4M_MaxTexIndex) + 2] = rMaterial.Map[iMaterial].Color[5];
    }
  }
}
