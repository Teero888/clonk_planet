/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Lots of file helpers */

#include <Windows.h>
#include <Stdio.h>
#include <Io.h>
#include <Direct.h>
#include <StdLib.h>
#include <Sys\Types.h>
#include <Sys\Stat.h>

#include <Standard.h>
#include <StdFile.h>

//---------------------------- Some system-dependent stuff --------------------------------------------------------------------------------------------------------

const char *GetWorkingDirectory()
  {
  static char buf[_MAX_PATH+1];
  return _getcwd(buf,_MAX_PATH);
  }

BOOL SetWorkingDirectory(const char *szPath)
  {
  return (_chdir(szPath)==0);
  }

//--------------------------------- Path & Filename --------------------------------------------------------------------------------------------------------

// Return pointer to position after last backslash.

char *GetFilename(const char *szPath) 
  {
	if (!szPath) return NULL;
	const char *pPos,*pFilename=szPath;
	for (pPos=szPath; *pPos; pPos++) if (*pPos==Backslash) pFilename = pPos+1;
	return (char*) pFilename;
  }

// Return pointer to last file extension.

char *GetExtension(const char *szFilename)
  {
  int pos, end;
  for (end=0; szFilename[end]; end++);
  pos = end;
  while ((pos>0) && (szFilename[pos-1] != '.') && (szFilename[pos-1] != Backslash)) pos--;
  if (szFilename[pos-1] == '.') return (char*) szFilename+pos;
  return (char*) szFilename+end;
  }

// Copy (extended) parent path (without backslash) to target buffer.

BOOL GetParentPath(const char *szFilename, char *szBuffer, int iBufferSize)
	{
	// Prepare filename
	SCopy(szFilename,szBuffer,iBufferSize);
	// Extend relative single filenames
	if (!SCharCount(Backslash,szFilename)) _fullpath(szBuffer,szFilename,iBufferSize);
	// Truncate path
	return TruncatePath(szBuffer);
	}	

// Truncate string before last backslash.

BOOL TruncatePath(char *szPath)
  {
  if (!szPath) return FALSE;
  int iBSPos;
  iBSPos=SCharLastPos(Backslash,szPath); if (iBSPos<0) return FALSE;
  szPath[iBSPos]=0;
  return TRUE;
  }

// Append terminating backslash if not present.

void AppendBackslash(char *szFilename)
  {
  int i=SLen(szFilename);
  if (i>0) if ((szFilename[i-1]==Backslash)) return;
  SAppendChar(Backslash,szFilename);
  }

// Remove terminating backslash if present.

void TruncateBackslash(char *szFilename)
	{
  int i=SLen(szFilename);
  if (i>0) if ((szFilename[i-1]==Backslash)) szFilename[i-1]=0;
	}

// Append extension if no extension.

void DefaultExtension(char *szFilename, const char *szExtension)
  {
  if (!(*GetExtension(szFilename)))
    { SAppend(".",szFilename); SAppend(szExtension,szFilename); }
  }

// Append or overwrite extension.

void EnforceExtension(char *szFilename, const char *szExtension)
  {
	char *ext = GetExtension(szFilename);
  if (ext[0])	{ SCopy(szExtension,ext); }
	else { SAppend(".",szFilename); SAppend(szExtension,szFilename); }
  }

// Enforce indexed extension until item does not exist. 

void MakeTempFilename(char *szFilename)
  {
  DefaultExtension(szFilename,"tmp");
  char *fn_ext=GetExtension(szFilename); 
  int cnum=-1;
  do
    {
    cnum++;
    sprintf(fn_ext,"%03d",cnum);
    }
  while (FileExists(szFilename) && (cnum<999));
  }

BOOL WildcardMatch(const char *szWildcard, const char *szFilename)
  {

  char tfname1[_MAX_FNAME+1],tfname2[_MAX_FNAME+1];
  char tfext1[10+1],tfext2[10+1];
    
  SCopy(GetFilename(szWildcard),tfname1,_MAX_FNAME);
  SCapitalize(tfname1);
  SCopy(GetExtension(tfname1),tfext1,10);
  *(GetExtension(tfname1))=0;
    
  SCopy(GetFilename(szFilename),tfname2,_MAX_FNAME);
  SCapitalize(tfname2);
  SCopy(GetExtension(tfname2),tfext2,10);
  *(GetExtension(tfname2))=0;
  
  if (SEqualUntil(tfname1,tfname2,'*') && SEqualUntil(tfext1,tfext2,'*')) return TRUE;
  
  return FALSE;
  }

//----------------------------------- Files --------------------------------------------------------------------------------------------------------

int FileAttributes(const char *szFilename)
  {
  struct _finddata_t fdt; int shnd;
  if (!szFilename) return 0;
  if ((shnd=_findfirst((char*)szFilename,&fdt))<0) return 0;
  _findclose(shnd);
  return fdt.attrib;
  }

BOOL FileExists(const char *szFilename, int *lpAttr)
  {
  struct _finddata_t fdt; int shnd;
  if ((shnd=_findfirst((char*)szFilename,&fdt))<0) return FALSE;
  _findclose(shnd);
  if (lpAttr) *lpAttr=fdt.attrib;
  return TRUE;
  }

int FileSize(const char *szFilename)
  {
  FILE *fhnd;
  int rval;
  if (!(fhnd=fopen(szFilename,"rb"))) return 0;
  rval=filelength(fileno(fhnd));
  fclose(fhnd);
  return rval;
  }

int FileTime(const char *szFilename)
  {
	struct _stat stStats;
	if (_stat(szFilename,&stStats)!=0) return 0;
	return stStats.st_mtime;
  }

BOOL EraseFile(const char *szFilename)
  {
	_chmod(szFilename,200);
	return (remove(szFilename)==0);
  }

BOOL EraseFiles(const char *szFilePath)
	{
  return ForEachFile(szFilePath,_A_ALL_FILES,&EraseFile);
	}

BOOL RenameFile(const char *szFilename, const char *szNewFileName)
  {
  if (rename(szFilename,szNewFileName)<0) return FALSE;
  return TRUE;
  }

//-------------------------------- Directories --------------------------------------------------------------------------------------------------------

BOOL CopyDirectory(const char *szSource, const char *szTarget)
  {
  // Source check
  if (!szSource || !szTarget) return FALSE;
  if (!(FileAttributes(szSource) & _A_SUBDIR)) return FALSE;

  // Do not process system navigation directories
  if (SEqual(GetFilename(szSource),".") 
   || SEqual(GetFilename(szSource),".."))
    return TRUE;

  // Overwrite target
  if (ItemExists(szTarget))
    if (!EraseItem(szTarget)) return FALSE;

  // Create target directory
  if (_mkdir(szTarget)!=0) return FALSE;

  // Copy contents to target directory
  char contents[_MAX_PATH+1];
  SCopy(szSource,contents); AppendBackslash(contents); 
  SAppend("*.*",contents);
  _finddata_t fdt; int hfdt;
  BOOL status=TRUE;
  if ( (hfdt=_findfirst(contents,&fdt)) > -1 )
    {
    do
      {
      char itemsource[_MAX_PATH+1],itemtarget[_MAX_PATH+1];
      SCopy(szSource,itemsource); AppendBackslash(itemsource); SAppend(fdt.name,itemsource);
      SCopy(szTarget,itemtarget); AppendBackslash(itemtarget); SAppend(fdt.name,itemtarget);
      if (!CopyItem(itemsource,itemtarget)) status=FALSE;
      }
    while (_findnext(hfdt,&fdt)==0);
    _findclose(hfdt);
    }
  
  return status;
  }

BOOL EraseDirectory(const char *szDirName)
  {
  // Do not process system navigation directories
  if (SEqual(GetFilename(szDirName),".") 
   || SEqual(GetFilename(szDirName),".."))
    return TRUE;
  // Get path to directory contents
  char szPath[_MAX_PATH+1];
  SCopy(szDirName,szPath); SAppend("\\*.*",szPath);
  // Erase subdirectories
  ForEachFile(szPath,_A_SUBDIR,&EraseDirectory);
  // Erase file contents
  ForEachFile(szPath,_A_ALL_FILES,&EraseFile);
  // Check working directory
  if (SEqual(szDirName,GetWorkingDirectory()))
    {
    // Will work only if szDirName is full path and correct case!
    SCopy(GetWorkingDirectory(),szPath);
    int lbacks=SCharLastPos(Backslash,szPath);
    if (lbacks>-1)
      { szPath[lbacks]=0; SetWorkingDirectory(szPath); }
    }
  // Remove directory
	_chmod(szDirName,200);
  return (_rmdir(szDirName)==0);
  }

//--------------------------------- Items --------------------------------------------------------------------------------------------------------

int ItemAttributes(const char *szItemName)
	{
	return FileAttributes(szItemName);
	}

BOOL ItemExists(const char *szItemName, int *lpAttr)
	{
	return FileExists(szItemName,lpAttr);
	}

BOOL RenameItem(const char *szItemName, const char *szNewItemName)
	{
	return RenameFile(szItemName,szNewItemName);
	}

BOOL EraseItem(const char *szItemName)
  {
  if (FileAttributes(szItemName) & _A_SUBDIR) return EraseDirectory(szItemName);
  return EraseFile(szItemName);
  }

BOOL CreateItem(const char *szItemname)
  {
	// Overwrite any old item
	EraseItem(szItemname);
	// Create dummy item
  FILE *fhnd;
  if (!(fhnd=fopen(szItemname,"wb"))) return FALSE;
  fclose(fhnd);
	// Success
	return TRUE;
  }

BOOL EraseItems(const char *szItemPath)
	{
  return ForEachFile(szItemPath,_A_ALL,&EraseItem);
	}

BOOL CopyItem(const char *szSource, const char *szTarget)
  {
	// Check for identical source and target
	if (ItemIdentical(szSource,szTarget)) return TRUE;
  // Overwrite target
  if (FileExists(szTarget)) 
		if (!EraseItem(szTarget)) 
			return FALSE;
  // Copy directory
  if (FileAttributes(szSource) & _A_SUBDIR) 
		return CopyDirectory(szSource,szTarget);
  // Copy file
  return CopyFile(szSource,szTarget,FALSE);
  }

BOOL MoveItem(const char *szSource, const char *szTarget)
	{
	if (!CopyItem(szSource,szTarget) || !EraseItem(szSource)) return FALSE;
	return TRUE;
	}

BOOL ItemIdentical(const char *szFilename1, const char *szFilename2)
	{
	char szFullFile1[_MAX_PATH+1],szFullFile2[_MAX_PATH+1];
	_fullpath(szFullFile1,szFilename1,_MAX_PATH);
	_fullpath(szFullFile2,szFilename2,_MAX_PATH);
	if (SEqualNoCase(szFullFile1,szFullFile2)) return TRUE;
	return FALSE;
	}

//------------------------- Multi File Processing --------------------------------------------------------------------------------------------------------

int ForEachFile(const char *szPath, int lAttrib, BOOL (*fnCallback)(const char *))
  {
  char szFilename[_MAX_PATH+1];
  int iFileCount = 0;
  struct _finddata_t fdt; int fdthnd;
  if (!szPath || !fnCallback) return 0;
  if ((fdthnd=_findfirst( (char*) szPath, &fdt))<0) return 0;
  do
    {
    if (fdt.attrib & lAttrib)
      {
      SCopy(szPath,szFilename);
      SCopy(fdt.name,GetFilename(szFilename)); 
      if ((*fnCallback)(szFilename)) iFileCount++;
      }
    }
  while (_findnext(fdthnd,&fdt)==0);
  _findclose(fdthnd);
  return iFileCount;
  }

//------------------------------- Text Files --------------------------------------------------------------------------------------------------------

// Sets file pointer to end of located string.

BOOL LocateInFile(FILE *file, char *index, BYTE wrap, BYTE lbeg)
  {
  int fchr,needok,idxcnt=0,loops=0;        
  BYTE exok=0,islbeg=1;                   
  if (!file || !index) return 0;
  needok=SLen(index);
  if (!file) return 0;
  do
    {
		// Get next char
    fchr=fgetc(file);
		// Check EOF/wrap
    if (fchr==EOF)
      if (wrap) { rewind(file); fchr=fgetc(file); loops++; if (loops>=2) exok=2; }
      else return 0;
		// Compare to search index
    if ((fchr==index[idxcnt]) && (!lbeg || islbeg))
      { idxcnt++; if (idxcnt==needok) exok=1; }
    else
      { idxcnt=0; islbeg=0; }
	// Check EOL
	if (fchr==0x0A) islbeg=1;
    }
  while (!exok);
  if (exok==1) return 1;
  return 0;
  }

// Reads the next line from the file. Returns zero if EOF has been encountered.

BOOL ReadFileLine(FILE *fhnd, char *tobuf, int maxlen) 
  {			  			       
  int cread;
  char inc;
  if (!fhnd || !tobuf) return 0;
  for (cread=0; cread<maxlen; cread++)
    {
    inc=fgetc(fhnd);
		if (inc==0x0D) // Binary text file line feed
			{ fgetc(fhnd); break; }
		if (inc==0x0A) break; // Text file line feed
    if (!inc || (inc==EOF)) break; // End of file
    *tobuf=inc; tobuf++;
    }
  *tobuf=0;
  if (inc==EOF) return 0;
  return 1;
  }

// Reads remaining line contents following string *info to target buffer.

BOOL ReadFileInfoLine(FILE *fhnd, char *info, char *tbuf, int maxlen, int wrap)
  {
  tbuf[0]=0;
  if (!LocateInFile(fhnd,info,wrap,1)) return 0;
  if (!ReadFileLine(fhnd,tbuf,maxlen)) return 0;
  return 1;
  }

void AdvanceFileLine(FILE *fhnd)
  {
  int cread,loops=0;
  if (!fhnd) return;
  do
    {
    cread=fgetc(fhnd);
    if (cread==EOF) { rewind(fhnd); loops++; }
    }
  while ((cread!=0x0A) && (loops<2));
  }

DWORD ReadFileUntil(FILE *fhnd, char *tbuf, char smark, int maxlen)
  {
  DWORD rcnt=0;
  int cread;
  if (!fhnd || !tbuf) return rcnt;
  while (maxlen>0)
    {
    cread=fgetc(fhnd);
    if ((cread==smark) || (cread==EOF)) break;
    *tbuf=cread; tbuf++;
    maxlen--;
    rcnt++;
    }
  *tbuf=0;
  return rcnt;
  }

BOOL WriteFileLine(FILE *hfile, const char *szLine)
  {
  if ( (fputs(szLine,hfile)<0) || (fputs("\n",hfile)<0) )  return FALSE;
	return TRUE;
	}
