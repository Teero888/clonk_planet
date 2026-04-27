/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A simple class for reading and writing files */

#ifndef INC_CStdFile
#define INC_CStdFile

#include <Standard.h>

#if defined(_ZLIB_H) || defined(ZLIB_H)
#ifndef GZFILE_DEFINED
typedef struct gzFile_s *gzFile;
#define GZFILE_DEFINED
#endif
#else
#ifndef GZFILE_DEFINED
typedef void* gzFile;
#define GZFILE_DEFINED
#endif
#endif

const int CStdFileBufSize = 4096;

class CStdFile
	{
	public:
		CStdFile();
		~CStdFile();
	public:
		char Name[_MAX_PATH+1];
		int Status;
	protected:
		FILE *hFile;
		gzFile hgzFile;
		int ModeWrite;
		BYTE Buffer[CStdFileBufSize];
		int BufferPtr,BufferLoad;
	public:
		BOOL Open(const char *szFilename, BOOL fCompressed=FALSE);
		BOOL Create(const char *szFilename, BOOL fCompressed=FALSE);
		BOOL Close();
		BOOL Read(void *pBuffer, int iSize, BOOL fPacked=FALSE, int *ipActual=NULL);
		BOOL Write(void *pBuffer, int iSize, BOOL fPacked=FALSE, int *ipFSize=NULL);
		BOOL WriteString(const char *szStr);
		BOOL Rewind();
		BOOL Advance(int iOffset, BOOL fPacked=FALSE);
		BOOL Save(const char *szFilename, BYTE *pBuffer, int iSize, BOOL fCompressed=FALSE);
		BOOL Load(const char *szFilename, BYTE **ppBuffer, int *ipSize, int iExtra=0, BOOL fCompressed=FALSE);
        BOOL IsCompressed() { return hgzFile != NULL; }
	protected:
		void ClearBuffer();
		int LoadBuffer();
		BOOL SaveBuffer();
	};

int UncompressedFileSize(const char *szFilename);

#endif
