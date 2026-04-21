/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A handy wrapper class to gzio files */

#include <Windows.h>
#include <stdio.h>
#include <StdLib.h>

#include <Standard.h>
#include <StdFile.h>
#include <CStdFile.h>

#include <..\zlib\ZLib.h>

CStdFile::CStdFile()
  {
  Status=FALSE;
  hFile=NULL;
  hgzFile=NULL;
  ClearBuffer();
  ModeWrite=FALSE;
	Name[0]=0;
  }

CStdFile::~CStdFile()
  {
  Close();
  }

BOOL CStdFile::Create(const char *szFilename, BOOL fCompressed)
  {
	SCopy(szFilename,Name,_MAX_PATH);
	// Set modes
  ModeWrite=TRUE;	
	// Open standard file
	if (fCompressed)
		{	if (!(hgzFile=gzopen(Name,"wb1"))) return FALSE; }
	else
		{	if (!(hFile=fopen(Name,"wb"))) return FALSE; }
	// Reset buffer
  ClearBuffer();
	// Set status
  Status=TRUE;
  return TRUE;  
  }

BOOL CStdFile::Open(const char *szFilename, BOOL fCompressed)
  {
	SCopy(szFilename,Name,_MAX_PATH);
	// Set modes
  ModeWrite=FALSE;
	// Open standard file
	if (fCompressed)
		{ if (!(hgzFile=gzopen(Name,"rb"))) return FALSE; }
	else
		{ if (!(hFile=fopen(Name,"rb"))) return FALSE; }
	// Reset buffer
  ClearBuffer();
	// Set status
  Status=TRUE;
  return TRUE;  
  }

BOOL CStdFile::Close()
  {
  BOOL rval=TRUE;
  Status=FALSE;
	Name[0]=0;
  // Save buffer if in write mode
  if (ModeWrite && BufferLoad) if (!SaveBuffer()) rval=FALSE;
  // Close file(s)
  if (hgzFile) if (gzclose(hgzFile)!=Z_OK) rval=FALSE;
	if (hFile) if (fclose(hFile)!=0) rval=FALSE;
	hgzFile=NULL; hFile=NULL;
	return rval;  
  }

BOOL CStdFile::Read(void *pBuffer, int iSize, BOOL fPacked, int *ipFSize)
  {
  int transfer;
  if (!pBuffer) return FALSE;
  if (ModeWrite) return FALSE;
  BYTE *bypBuffer= (BYTE*) pBuffer;
  while (iSize>0)
    {
    // Valid data in the buffer: Transfer as much as possible
    if (BufferLoad>BufferPtr)
      {
      transfer=Min(BufferLoad-BufferPtr,iSize);
			MemCopy( Buffer+BufferPtr, bypBuffer, transfer );
      BufferPtr+=transfer;
      bypBuffer+=transfer;
      iSize-=transfer;
      }

    // Buffer empty: Load
    else if (!LoadBuffer()) return FALSE;
    }
  return TRUE;
  }

int CStdFile::LoadBuffer()
  {
	if (hFile) BufferLoad = fread(Buffer,1,CStdFileBufSize,hFile);
  if (hgzFile) BufferLoad = gzread(hgzFile, Buffer,CStdFileBufSize);
  BufferPtr=0;
  return BufferLoad;
  }

BOOL CStdFile::SaveBuffer()
  {
  int saved;
	if (hFile) saved=fwrite(Buffer,1,BufferLoad,hFile);
	if (hgzFile) saved=gzwrite(hgzFile,Buffer,BufferLoad);
	if (saved!=BufferLoad) return FALSE;
	BufferLoad=0;
	return TRUE;
  }

void CStdFile::ClearBuffer()
  {
  BufferLoad=BufferPtr=0;
  }

BOOL CStdFile::Write(void *pBuffer, int iSize, BOOL fPacked, int *ipFSize)
  {
  int transfer;
  if (!pBuffer) return FALSE;
  if (!ModeWrite) return FALSE;
  BYTE *bypBuffer= (BYTE*) pBuffer;
  while (iSize>0)
    {
    // Space in buffer: Transfer as much as possible
    if (BufferLoad<CStdFileBufSize)
      {
      transfer=Min(CStdFileBufSize-BufferLoad,iSize);
      memcpy(Buffer+BufferLoad,bypBuffer,transfer);
      BufferLoad+=transfer;
      bypBuffer+=transfer;
      iSize-=transfer;
      }
    // Buffer full: Save
    else if (!SaveBuffer()) return FALSE;
    }
  return TRUE;
  }

BOOL CStdFile::WriteString(const char *szStr)
	{
	BYTE nl[2]={0x0D,0x0A};
	if (!szStr) return FALSE;
	int size=SLen(szStr);
	if (!Write((void*)szStr,size)) return FALSE;
	if (!Write(nl,2)) return FALSE;
	return TRUE;
	}

BOOL CStdFile::Rewind()
  {
  if (ModeWrite) return FALSE;
  ClearBuffer();
	if (hFile) rewind(hFile);
	if (hgzFile)
		{
		if (gzclose(hgzFile)!=Z_OK) return FALSE;
		if (!(hgzFile=gzopen(Name,"rb"))) return FALSE;
		}
	return TRUE;
  }

BOOL CStdFile::Advance(int iOffset, BOOL fPacked)
  {
  BYTE buf;
  while (iOffset>0) 
    {
    if (!Read(&buf,1,fPacked)) return FALSE;
    iOffset--;
    }
  return TRUE;
  }

BOOL CStdFile::Save(const char *szFilename, BYTE *bpBuf,
										int iSize, BOOL fCompressed)
	{
	if (!bpBuf || (iSize<1)) return FALSE;
	if (!Create(szFilename,fCompressed)) return FALSE;
	if (!Write(bpBuf,iSize)) return FALSE;
	if (!Close()) return FALSE;
	return TRUE;
	}

BOOL CStdFile::Load(const char *szFilename, BYTE **lpbpBuf,
										int *ipSize, int iAppendZeros, 
										BOOL fCompressed)
	{
	int iSize = fCompressed ? UncompressedFileSize(szFilename) : FileSize(szFilename);
	if (!lpbpBuf || (iSize<1)) return FALSE;
	if (!Open(szFilename,fCompressed)) return FALSE;
	if (!(*lpbpBuf = new BYTE [iSize+iAppendZeros])) return FALSE;
	if (!Read(*lpbpBuf,iSize)) { delete [] *lpbpBuf; return FALSE; }
	if (iAppendZeros)	ZeroMem( (*lpbpBuf)+iSize, iAppendZeros );
	if (ipSize) *ipSize=iSize;
	Close();
	return TRUE;
	}

int UncompressedFileSize(const char *szFilename)
	{
	int rval=0;
	BYTE buf;
	gzFile hFile;
	if (!(hFile = gzopen(szFilename,"rb"))) return 0;
	while (gzread(hFile,&buf,1)>0) rval++;
	gzclose(hFile);
	return rval;
	}

