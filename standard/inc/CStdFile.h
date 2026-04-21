/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A handy wrapper class to gzio files */

const int CStdFileBufSize = 4096;

typedef void* gzFile;

class CStdFile
  {
  public:
    CStdFile();
    ~CStdFile();
  public:
    BOOL Status;
		char Name[_MAX_PATH+1];
  protected:    
    FILE *hFile; 
		gzFile hgzFile;
    BYTE Buffer[CStdFileBufSize];
    int BufferLoad,BufferPtr;
    BOOL ModeWrite;
  public:   
    BOOL Create(const char *szFileName, BOOL fCompressed=FALSE); 
    BOOL Open(const char *szFileName, BOOL fCompressed=FALSE); 
    BOOL Close();
    BOOL Read(void *pBuffer, int iSize, BOOL fPacked=FALSE, int *ipFSize=NULL);
    BOOL Write(void *pBuffer, int iSize, BOOL fPacked=FALSE, int *ipFSize=NULL);
		BOOL WriteString(const char *szStr);
    BOOL Rewind();
    BOOL Advance(int iOffset, BOOL fPacked=FALSE);
		// Single line commands
		BOOL Load(const char *szFileName, BYTE **lpbpBuf,
              int *ipSize=NULL, int iAppendZeros=0, 
							BOOL fCompressed=FALSE);
		BOOL Save(const char *szFileName, BYTE *bpBuf,
              int iSize, 
							BOOL fCompressed=FALSE);
  protected:
    void ClearBuffer();
    int LoadBuffer();
    BOOL SaveBuffer();
  };

int UncompressedFileSize(const char *szFileName);

