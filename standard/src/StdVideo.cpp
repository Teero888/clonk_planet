/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some functions to help with saving AVI files using Video for Windows */

#include <Windows.h>
#include <MMSystem.h>
#include <VfW.h>

#include <Standard.h>
#include <StdVideo.h>

BOOL AVIOpenOutput(const char *szFilename,
									 PAVIFILE *ppAviFile,
									 PAVISTREAM *ppAviStream,
									 int iWidth, int iHeight)
	{

	// Create avi file
	if ( AVIFileOpen(
					ppAviFile,
					szFilename,
					OF_CREATE | OF_WRITE,
					NULL) != 0) 
		{
		return FALSE;
		}

	// Create stream
	AVISTREAMINFO avi_info;
	RECT frame; frame.left=0; frame.top=0; frame.right=iWidth; frame.bottom=iHeight;
	avi_info.fccType= streamtypeVIDEO;
	avi_info.fccHandler= mmioFOURCC('M','S','V','C');
	avi_info.dwFlags= 0;
	avi_info.dwCaps= 0;
	avi_info.wPriority= 0;
	avi_info.wLanguage= 0;
	avi_info.dwScale= 1;
	avi_info.dwRate= 35;
	avi_info.dwStart= 0;
	avi_info.dwLength= 10; // ??
	avi_info.dwInitialFrames= 0;
	avi_info.dwSuggestedBufferSize= 0;
	avi_info.dwQuality= -1;
	avi_info.dwSampleSize= 0;
	avi_info.rcFrame= frame;
	avi_info.dwEditCount= 0;
	avi_info.dwFormatChangeCount= 0;
	SCopy("MyRecording",avi_info.szName);

	if ( AVIFileCreateStream(
					*ppAviFile,
					ppAviStream,
					&avi_info) != 0) 
		{
		return FALSE;
		}

	return TRUE;
	}


BOOL AVICloseOutput(PAVIFILE *ppAviFile,
 									  PAVISTREAM *ppAviStream)
	{
	if (ppAviStream && *ppAviStream)
		{ AVIStreamRelease(*ppAviStream); *ppAviStream=NULL; }	
	if (ppAviFile && *ppAviFile)
		{ AVIFileRelease(*ppAviFile); *ppAviFile=NULL; }
	return TRUE;
	}


BOOL AVIPutFrame(PAVISTREAM pAviStream,
								 long lFrame,
								 void *lpInfo, long lInfoSize,
								 void *lpData, long lDataSize)
	{
	long lBytesWritten=0,lSamplesWritten=0;

	AVIStreamSetFormat(
		pAviStream,
		lFrame,
		lpInfo,
		lInfoSize	
		);

	if (AVIStreamWrite(
					pAviStream,
					lFrame,
					1,
					lpData, 
					lDataSize,
					AVIIF_KEYFRAME,
					&lSamplesWritten,
					&lBytesWritten) != 0) return FALSE;

	return TRUE;
	}


BOOL AVIOpenGrab(const char *szFilename,
								 PAVISTREAM *ppAviStream,
								 PGETFRAME *ppGetFrame,
								 int &rAviLength, int &rFrameWdt, int &rFrameHgt,
								 int &rFrameBitsPerPixel, int &rFramePitch)
	{

	// Open avi stream
  if ( AVIStreamOpenFromFile( 
					ppAviStream,
					szFilename,
					streamtypeVIDEO,
					0,
					OF_READ,
					NULL) != 0) return FALSE;

	// Get stream info
	AVISTREAMINFO avi_info;
	AVIStreamInfo(*ppAviStream,&avi_info,sizeof(avi_info));
	rAviLength=avi_info.dwLength;

	// Open get frame
	if (!(*ppGetFrame = AVIStreamGetFrameOpen(*ppAviStream,NULL))) return FALSE;

	// Get sample frame
	void *pframe;
	if (!(pframe = AVIStreamGetFrame(*ppGetFrame,0))) return FALSE;

	// Assign sample bmp info
	BITMAPINFOHEADER *sample = (BITMAPINFOHEADER*) pframe;
	rFrameWdt = sample->biWidth;
	rFrameHgt = sample->biHeight;
	rFrameBitsPerPixel = sample->biBitCount;
	rFramePitch = DWordAligned(rFrameWdt*rFrameBitsPerPixel/8);

	return TRUE;
	}

void AVICloseGrab(PAVISTREAM *ppAviStream,
									PGETFRAME *ppGetFrame)
	{
	if (ppGetFrame && *ppGetFrame) 
		{ AVIStreamGetFrameClose(*ppGetFrame); *ppGetFrame=NULL; }
	if (ppAviStream && *ppAviStream)
		{ AVIStreamRelease(*ppAviStream); *ppAviStream=NULL; }
	}
