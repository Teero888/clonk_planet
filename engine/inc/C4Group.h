/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles group files */

const int C4GroupFileVer1=1, C4GroupFileVer2=2;  

const int C4GroupMaxMaker    = 30,
          C4GroupMaxPassword = 30,
					C4GroupMaxError		 = 100;

#define C4GroupFileID "RedWolf Design GrpFolder"

void C4Group_SetMaker(const char *szMaker);
void C4Group_SetPasswords(const char *szPassword);
void C4Group_SetTempPath(const char *szPath);
const char* C4Group_GetTempPath();
void C4Group_SetSortList(const char **ppSortList);
void C4Group_SetProcessCallback(BOOL (*fnCallback)(const char *, int));
BOOL C4Group_IsGroup(const char *szFilename);
BOOL C4Group_CopyItem(const char *szSource, const char *szTarget);
BOOL C4Group_MoveItem(const char *szSource, const char *szTarget);
BOOL C4Group_PackDirectory(const char *szFilename);
BOOL C4Group_UnpackDirectory(const char *szFilename);
BOOL C4Group_ExplodeDirectory(const char *szFilename);
int C4Group_GetCreation(const char *szFilename);
BOOL C4Group_SetOriginal(const char *szFilename, BOOL fOriginal);

void DeleteBitmap(HBITMAP hbmp);

extern const char *C4CFN_FLS[];

#pragma pack(push, 1)
class C4GroupHeader 
  { 
	public:
		C4GroupHeader();
  public:
    char id[24+1]; 
    int32_t Ver1,Ver2;
    int32_t Entries;  
    char Maker[C4GroupMaxMaker+1];
    char Password[C4GroupMaxPassword+1];
		int32_t Creation,Original;
    BYTE fbuf[92];
  public:
    void Init();
  };

class C4GroupEntryCore  
  { 
  public:
    char FileName[_MAX_FNAME+1];
    int32_t Packed,ChildGroup;
    int32_t Size,EntrySize,Offset;
    int32_t Time;
    BYTE fbuf[30];
  };
#pragma pack(pop)

const int C4GRES_InGroup  = 0,	
					C4GRES_OnDisk		= 1,
					C4GRES_InMemory = 2,
					C4GRES_Deleted	= 3;

class C4GroupEntry: public C4GroupEntryCore
  { 
  public:
    C4GroupEntry();
		~C4GroupEntry();
  public:
    char DiskPath[_MAX_PATH+1];
    int Status;
		BOOL DeleteOnDisk;
		BOOL HoldBuffer;
    BYTE *bpMemBuf;
    C4GroupEntry *Next;
  public:
    void Set(_finddata_t &Fdt, const char *szPath);
  };

const int GRPF_Inactive=0,
          GRPF_File=1, 
          GRPF_Folder=2;

class C4Group
  {
  public:
    C4Group();
    ~C4Group();

  protected:
    int Status;
    char FileName[_MAX_PATH+1];	
    // Parent status
    C4Group *Mother;
    BOOL ExclusiveChild;
    // File & Folder
    C4GroupEntry *SearchPtr;
    CStdFile StdFile;
    // File only
    int FilePtr;
    int EntryOffset;
    BOOL Modified;
    C4GroupHeader Head;
    C4GroupEntry *FirstEntry;
    // Folder only
    struct _finddata_t Fdt; 
    long hFdt;
    C4GroupEntry FolderSearchEntry;
    C4GroupEntry LastFolderSearchEntry;

    BOOL StdOutput;
		BOOL (*fnProcessCallback)(const char *, int);
    char ErrorString[C4GroupMaxError+1];
		BOOL MadeOriginal;

  public:

    BOOL Open(const char *szGroupName, BOOL fCreate=FALSE);
    BOOL Close();
    BOOL OpenAsChild(C4Group *pMother, const char *szEntryName,
                     BOOL fExclusive=FALSE);
    BOOL Add(const char *szFiles);
    BOOL Add(const char *szFile, const char *szAddAs);
    BOOL Add(const char *szName, void *pBuffer, int iSize, 
						 BOOL fChild=FALSE, BOOL fHoldBuffer=FALSE);
	  BOOL Add(const char *szEntryname, C4Group &hSource);
    BOOL Move(const char *szFiles);
    BOOL Move(const char *szFile, const char *szAddAs);
    BOOL Extract(const char *szFiles, const char *szExtractTo=NULL);
    BOOL ExtractEntry(const char *szFilename, const char *szExtractTo=NULL);   
    BOOL Delete(const char *szFiles);
    BOOL DeleteEntry(const char *szFilename);
    BOOL Rename(const char *szFile, const char *szNewName);
    BOOL Sort(const char *szSortList);
	  BOOL SortByList(const char **ppSortList, const char *szFilename=NULL);
    BOOL View(const char *szFiles);
		BOOL GetOriginal();
	  BOOL LoadIcon(const char *szEntryname, HICON *lphIcon);
    BOOL AccessEntry(const char *szWildCard, 
                     int *iSize=NULL, char *sFileName=NULL,
										 BOOL *fChild=NULL);
    BOOL AccessNextEntry(const char *szWildCard, 
                     int *iSize=NULL, char *sFileName=NULL,
										 BOOL *fChild=NULL);
    BOOL LoadEntry(const char *szEntryName, BYTE **lpbpBuf, 
                   int *ipSize=NULL, int iAppendZeros=0);
    BOOL FindEntry(const char *szWildCard, 
                   char *sFileName=NULL, 
									 int *iSize=NULL,
									 BOOL *fChild=NULL);
    BOOL FindNextEntry(const char *szWildCard, 
                       char *sFileName=NULL, 
											 int *iSize=NULL,
											 BOOL *fChild=NULL,
											 BOOL fStartAtFilename=FALSE);
    BOOL Read(void *pBuffer, int iSize);
		BOOL Advance(int iOffset);
    BOOL ReadDDB(HBITMAP *lphBitmap, HDC hdc = NULL);
    BOOL ReadDDBSection(HBITMAP *lphBitmap, HDC hdc,
                        int iSecX, int iSecY, int iSecWdt, int iSecHgt,
                        int iImgWdt=-1, int iImgHgt=-1,
                        BOOL fTransCol = FALSE);
		void SetMaker(const char *szMaker);
		void SetPassword(const char *szPassword);
    void SetStdOutput(BOOL fStatus);
		void SetProcessCallback(BOOL (*fnCallback)(const char *, int));
		void MakeOriginal(BOOL fOriginal);
    void ResetSearch();
    const char *GetError();
    const char *GetMaker();
    const char *GetPassword();
    const char *GetName();
	  const char *GetFullName();
    int EntryCount(const char *szWildCard=NULL);
    int EntrySize(const char *szWildCard=NULL);
		int EntryTime(const char *szFilename);
		int GetVersion();
	  int GetCreation();
	  int GetStatus();
		C4Group *GetMother();

  protected:
    void Init();
    void Clear();
	  void ProcessOut(const char *szMessage, int iProcess=0);
	  BOOL EnsureChildFilePtr(C4Group *pChild);
	  BOOL CloseExclusiveMother();
    BOOL Error(const char *szStatus);
    BOOL OpenReal(const char *szGroupName);
    BOOL OpenRealGrpFile();
    BOOL SetFilePtr(int iOffset);
    BOOL RewindFilePtr();
    BOOL AdvanceFilePtr(int iOffset, C4Group *pByChild=NULL);
    BOOL AddEntry(int status, BOOL childgroup,
                  const char *fname, long size, time_t time,
                  const char *entryname=NULL,
                  BYTE *membuf=NULL, BOOL fDeleteOnDisk=FALSE,
									BOOL fHoldBuffer=FALSE);
    BOOL AddEntryOnDisk(const char *szFilename, const char *szAddAs=NULL, BOOL fMove=FALSE);
    BOOL SetFilePtr2Entry(const char *szName, C4Group *pByChild=NULL);
    BOOL AppendEntry2StdFile(C4GroupEntry *centry, CStdFile &stdfile);
    C4GroupEntry *GetEntry(const char *szName);
    C4GroupEntry *SearchNextEntry(const char *szName);    
    C4GroupEntry *GetNextFolderEntry();
    HBITMAP SubReadDDB(HDC hdc, 
                       int sx=-1, int sy=-1, int swdt=-1, int shgt=-1,
                       int twdt=-1, int thgt=-1,
                       BOOL transcol=FALSE);

  };


