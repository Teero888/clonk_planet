/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some functions to help with saving AVI files using Video for Windows */

BOOL AVIOpenGrab(const char *szFilename,
								 PAVISTREAM *ppAviStream,
								 PGETFRAME *ppGetFrame,
								 int &rAviLength, int &rFrameWdt, int &rFrameHgt,
								 int &rFrameBitsPerPixel, int &rFramePitch);

void AVICloseGrab(PAVISTREAM *ppAviStream,
									PGETFRAME *ppGetFrame);

BOOL AVIOpenOutput(const char *szFilename,
									 PAVIFILE *ppAviFile,
									 PAVISTREAM *ppAviStream,
									 int iWidth, int iHeight);

BOOL AVICloseOutput(PAVIFILE *ppAviFile,
 									  PAVISTREAM *ppAviStream);

BOOL AVIPutFrame(PAVISTREAM pAviStream,
								 long lFrame,
								 void *lpInfo, long lInfoSize,
								 void *lpData, long lDataSize);