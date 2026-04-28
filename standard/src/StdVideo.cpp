
#include <C4Include.h>
#include <StdVideo.h>
BOOL AVIOpenOutput(const char *szFilename, void **ppFile, void **ppStream,
                   int iWidth, int iHeight) {
  return FALSE;
}
BOOL AVICloseOutput(void **ppFile, void **ppStream) { return FALSE; }
BOOL AVIPutFrame(void *pStream, long iFrame, void *pData, long iDataSize,
                 void *pBmi, long iBmiSize) {
  return FALSE;
}
