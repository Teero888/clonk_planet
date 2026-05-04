
#include <C4Include.h>
#include <StdVideo.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

struct StdVideoInfo {
  char szBaseName[256];
  int iWidth, iHeight;
  int iFrameCount;
};

BOOL AVIOpenOutput(const char *szFilename, PAVIFILE *ppAviFile, PAVISTREAM *ppAviStream, int iWidth, int iHeight) {
  if (!ppAviStream)
    return FALSE;
  StdVideoInfo *pInfo = new StdVideoInfo();
  SCopy(szFilename, pInfo->szBaseName);
  pInfo->iWidth = iWidth;
  pInfo->iHeight = iHeight;
  pInfo->iFrameCount = 0;

  // Create directory for frames
  char szDir[512];
  sprintf(szDir, "%s_frames", szFilename);
  mkdir(szDir, 0755);

  *ppAviStream = (PAVISTREAM)pInfo;
  return TRUE;
}

BOOL AVICloseOutput(PAVIFILE *ppAviFile, PAVISTREAM *ppAviStream) {
  if (!ppAviStream || !*ppAviStream)
    return FALSE;
  StdVideoInfo *pInfo = (StdVideoInfo *)*ppAviStream;
  delete pInfo;
  *ppAviStream = NULL;
  return TRUE;
}

BOOL AVIPutFrame(PAVISTREAM pAviStream, long lFrame, void *lpInfo, long lInfoSize, void *lpData, long lDataSize) {
  if (!pAviStream)
    return FALSE;
  StdVideoInfo *pInfo = (StdVideoInfo *)pAviStream;

  char szFilename[512];
  sprintf(szFilename, "%s_frames/frame_%05d.bmp", pInfo->szBaseName, pInfo->iFrameCount++);

  FILE *f = fopen(szFilename, "wb");
  if (!f)
    return FALSE;

  BITMAPFILEHEADER bfh;
  memset(&bfh, 0, sizeof(bfh));
  bfh.bfType = 0x4D42;
  bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + lInfoSize;
  bfh.bfSize = bfh.bfOffBits + lDataSize;

  fwrite(&bfh, sizeof(bfh), 1, f);
  fwrite(lpInfo, lInfoSize, 1, f);
  fwrite(lpData, lDataSize, 1, f);
  fclose(f);

  return TRUE;
}

BOOL AVIOpenGrab(const char *szFilename, PAVISTREAM *ppAviStream, PGETFRAME *ppGetFrame, int &rAviLength, int &rFrameWdt, int &rFrameHgt, int &rFrameBitsPerPixel, int &rFramePitch) {
  return FALSE;
}

void AVICloseGrab(PAVISTREAM *ppAviStream, PGETFRAME *ppGetFrame) {}
