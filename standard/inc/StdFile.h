/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Lots of file helpers */

#define Backslash '\\'
#define Wildcard '*'

/* File attribute constants for _findfirst() */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

const DWORD _A_ALL = _A_ARCH | _A_HIDDEN | _A_NORMAL | _A_RDONLY | _A_SUBDIR | _A_SYSTEM;
const DWORD _A_ALL_FILES = _A_ALL & ~_A_SUBDIR;

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

const char *GetWorkingDirectory();
char *GetFilename(const char *path);
char *GetExtension(const char *fname);
void DefaultExtension(char *szFileName, const char *szExtension);
void EnforceExtension(char *szFileName, const char *szExtension);
void AppendBackslash(char *szFileName);
void TruncateBackslash(char *szFilename);
void MakeTempFilename(char *szFileName);
BOOL WildcardMatch(const char *szFName1, const char *szFName2);
BOOL TruncatePath(char *szPath);
BOOL GetParentPath(const char *szFilename, char *szBuffer, int iBufferSize=_MAX_PATH);

int FileAttributes(const char *szFileName);
BOOL FileExists(const char *szFileName, int *lpAttr=NULL);
int FileSize(const char *fname);
int FileTime(const char *fname);
BOOL EraseFile(const char *szFileName);
BOOL EraseFiles(const char *szFilePath);
BOOL RenameFile(const char *szFileName, const char *szNewFileName);

BOOL CopyDirectory(const char *szSource, const char *szTarget);
BOOL EraseDirectory(const char *szDirName);

int ItemAttributes(const char *szItemName);
BOOL ItemIdentical(const char *szFilename1, const char *szFilename2);
BOOL ItemExists(const char *szItemName, int *lpAttr=NULL);
BOOL RenameItem(const char *szItemName, const char *szNewItemName);
BOOL EraseItem(const char *szItemName);
BOOL EraseItems(const char *szItemPath);
BOOL CopyItem(const char *szSource, const char *szTarget);
BOOL CreateItem(const char *szItemname);
BOOL MoveItem(const char *szSource, const char *szTarget);

int ForEachFile(const char *szPath, int lAttrib, BOOL (*fnCallback)(const char *));

BOOL LocateInFile(FILE *file, char *index, BYTE wrap=1, BYTE lbeg=0);
BOOL ReadFileLine(FILE *fhnd, char *tobuf, int maxlen);
BOOL ReadFileInfoLine(FILE *fhnd, char *info, char *tbuf, int maxlen=256, int wrap=1);
BOOL WriteFileLine(FILE *hfile, const char *szLine);
DWORD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen);
void AdvanceFileLine(FILE *fhnd);
