/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Another C4Group bitmap-to-surface loader and saver */

#include <C4Include.h>

SURFACE GroupReadSurface(C4Group &hGroup, BYTE *bpPalette) {
  SURFACE tsurf;
  int cnt, lcnt, ladd, pitch;
  BYTE fbuf[4];
  BYTE *timgbuf;

  CBitmap256Info BitmapInfo;

  // Read and check bitmap info
  if (!hGroup.Read(&BitmapInfo, sizeof(BitmapInfo)))
    return NULL;
  if (!BitmapInfo.Valid())
    return NULL;

  // Copy colors to target palette
  if (bpPalette)
    for (cnt = 0; cnt < 256; cnt++) {
      bpPalette[cnt * 3 + 0] = BitmapInfo.Colors[cnt].rgbRed;
      bpPalette[cnt * 3 + 1] = BitmapInfo.Colors[cnt].rgbGreen;
      bpPalette[cnt * 3 + 2] = BitmapInfo.Colors[cnt].rgbBlue;
    }

  // Read offset
  if (!hGroup.Advance(BitmapInfo.FileBitsOffset()))
    return NULL;

  // Image line data in file is extended to be multiple of 4
  ladd = 0;
  if (BitmapInfo.Info.biWidth % 4 != 0)
    ladd = 4 - BitmapInfo.Info.biWidth % 4;

  // Create and lock sfcSurface
  if (!(tsurf = CreateSurface(BitmapInfo.Info.biWidth, BitmapInfo.Info.biHeight)))
    return NULL;
  if (!(timgbuf = LockSurface(tsurf, pitch))) {
    DestroySurface(tsurf);
    return NULL;
  }

  for (lcnt = BitmapInfo.Info.biHeight - 1; lcnt >= 0; lcnt--) {
    // Read lines
    if (!hGroup.Read(timgbuf + (pitch * lcnt), BitmapInfo.Info.biWidth)) {
      UnLockSurface(tsurf);
      DestroySurface(tsurf);
      return NULL;
    }

    if (ladd > 0)
      if (!hGroup.Read(fbuf, ladd)) {
        UnLockSurface(tsurf);
        DestroySurface(tsurf);
        return NULL;
      }
  }

  UnLockSurface(tsurf);

  return tsurf;
}

BOOL SaveSurface(const char *szFilename, SURFACE sfcSurface, BYTE *bpPalette) {
  BITMAPINFOHEADER bmpinfo;
  RGBQUAD rgbquad;
  BITMAPFILEHEADER bmphead;

  int cnt, lcnt, ladd, pitch;
  int imgwdt, imghgt;
  BYTE fbuf[4];
  BYTE *timgbuf;
  CStdFile hFile;

  // Lock the sfcSurface
  if (!(timgbuf = LockSurface(sfcSurface, pitch, &imgwdt, &imghgt)))
    return FALSE;

  // Image line data in file is extended to be multiple of 4
  ladd = 0;
  if (imgwdt % 4 != 0)
    ladd = 4 - imgwdt % 4;

  // Set bitmap info
  ZeroMem((BYTE *)&bmpinfo, sizeof(BITMAPINFOHEADER));
  bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
  bmpinfo.biWidth = imgwdt;
  bmpinfo.biHeight = imghgt;
  bmpinfo.biPlanes = 1;
  bmpinfo.biBitCount = 8;
  bmpinfo.biCompression = 0;
  bmpinfo.biSizeImage = imgwdt * imghgt;
  bmpinfo.biClrUsed = bmpinfo.biClrImportant = 256;

  // Set header
  ZeroMem((BYTE *)&bmphead, sizeof(BITMAPFILEHEADER));
  bmphead.bfType = *((WORD *)"BM");
  bmphead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) + (imgwdt + ladd) * imghgt;
  bmphead.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);

  if (!hFile.Create(szFilename, FALSE)) {
    UnLockSurface(sfcSurface);
    return FALSE;
  }

  hFile.Write(&bmphead, sizeof(bmphead));
  hFile.Write(&bmpinfo, sizeof(bmpinfo));

  for (cnt = 0; cnt < 256; cnt++) {
    rgbquad.rgbRed = bpPalette[cnt * 3 + 0];
    rgbquad.rgbGreen = bpPalette[cnt * 3 + 1];
    rgbquad.rgbBlue = bpPalette[cnt * 3 + 2];
    hFile.Write(&rgbquad, sizeof(rgbquad));
  }

  for (lcnt = imghgt - 1; lcnt >= 0; lcnt--) {
    hFile.Write(timgbuf + (pitch * lcnt), imgwdt);
    if (ladd > 0)
      hFile.Write(fbuf, ladd);
  }

  UnLockSurface(sfcSurface);
  hFile.Close();

  return TRUE;
}