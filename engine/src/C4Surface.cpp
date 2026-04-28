/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Extension to CSurface that handles bitmaps in C4Group files */

#include <C4Include.h>

BOOL C4Surface::Load(C4Group &hGroup, const char *szFilename) {
  if (!hGroup.AccessEntry(szFilename))
    return FALSE;
  if (!Read(hGroup))
    return FALSE;
  return TRUE;
}

BOOL C4Surface::Read(C4Group &hGroup) {
  int cnt, lcnt, iLineRest;
  BYTE fbuf[4];

  // Read and check bitmap info
  CBitmap256Info BitmapInfo;
  if (!hGroup.Read(&BitmapInfo, sizeof(BitmapInfo)))
    return FALSE;
  if (!BitmapInfo.Valid())
    return FALSE;

  // Read offset
  if (!hGroup.Advance(BitmapInfo.FileBitsOffset()))
    return FALSE;

  // Create and lock surface
  if (!Create(BitmapInfo.Info.biWidth, BitmapInfo.Info.biHeight))
    return FALSE;
  if (!Lock()) {
    Clear();
    return FALSE;
  }

  // Copy palette
  for (cnt = 0; cnt < 256; cnt++) {
    Palette[cnt * 3 + 0] = BitmapInfo.Colors[cnt].rgbRed;
    Palette[cnt * 3 + 1] = BitmapInfo.Colors[cnt].rgbGreen;
    Palette[cnt * 3 + 2] = BitmapInfo.Colors[cnt].rgbBlue;
  }

  // Read lines
  iLineRest = DWordAligned(BitmapInfo.Info.biWidth) - BitmapInfo.Info.biWidth;
  for (lcnt = Hgt - 1; lcnt >= 0; lcnt--) {
    if (!hGroup.Read(Bits + (Pitch * lcnt), Wdt)) {
      Clear();
      return FALSE;
    }
    if (iLineRest > 0)
      if (!hGroup.Read(fbuf, iLineRest)) {
        Clear();
        return FALSE;
      }
  }

  Unlock();

  return TRUE;
}

BOOL C4Surface::Save(C4Group &hGroup, const char *szFilename) {
  // Using temporary file at C4Group temp path
  char szTemp[_MAX_PATH + 1];
  SCopy(C4Group_GetTempPath(), szTemp);
  SAppend(GetFilename(szFilename), szTemp);
  MakeTempFilename(szTemp);
  // Save to temporary file
  if (!CSurface::Save(szTemp))
    return FALSE;
  // Move temp file to group
  if (!hGroup.Move(szTemp, GetFilename(szFilename)))
    return FALSE;
  // Success
  return TRUE;
}
