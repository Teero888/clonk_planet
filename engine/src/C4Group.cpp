/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles group files */

#include <C4Include.h>

#include <utime.h>

//------------------------------ File Sort Lists -------------------------------------------

const char *C4CFN_FLS[] = {
    C4CFN_Mouse, C4FLS_Mouse,       C4CFN_Keyboard, C4FLS_Keyboard,        C4CFN_Easy,   C4FLS_Easy,          C4CFN_Material, C4FLS_Material,    C4CFN_Graphics, C4FLS_Graphics, C4CFN_DefFiles,
    C4FLS_Def,   C4CFN_PlayerFiles, C4FLS_Player,   C4CFN_ObjectInfoFiles, C4FLS_Object, C4CFN_ScenarioFiles, C4FLS_Scenario, C4CFN_FolderFiles, C4FLS_Folder,   NULL,           NULL};

//---------------------------- Global C4Group_Functions -------------------------------------------

char C4Group_Maker[C4GroupMaxMaker + 1] = "";
char C4Group_Passwords[CFG_MaxString + 1] = "";
char C4Group_TempPath[_MAX_PATH + 1] = "";
char C4Group_Ignore[_MAX_PATH + 1] = ".;..;cvs";
const char **C4Group_SortList = NULL;
BOOL (*C4Group_ProcessCallback)(const char *, int) = NULL;

void C4Group_SetProcessCallback(BOOL (*fnCallback)(const char *, int)) { C4Group_ProcessCallback = fnCallback; }

void C4Group_SetSortList(const char **ppSortList) { C4Group_SortList = ppSortList; }

void C4Group_SetMaker(const char *szMaker) {
  if (!szMaker)
    C4Group_Maker[0] = 0;
  else
    SCopy(szMaker, C4Group_Maker, C4GroupMaxMaker);
}

void C4Group_SetPasswords(const char *szPassword) {
  if (!szPassword)
    C4Group_Passwords[0] = 0;
  else
    SCopy(szPassword, C4Group_Passwords, CFG_MaxString);
}

void C4Group_SetTempPath(const char *szPath) {
  if (!szPath || !szPath[0])
    C4Group_TempPath[0] = 0;
  else {
    SCopy(szPath, C4Group_TempPath, _MAX_PATH);
    AppendBackslash(C4Group_TempPath);
  }
}

const char *C4Group_GetTempPath() { return C4Group_TempPath; }

BOOL C4Group_IsGroup(const char *szFilename) {
  C4Group hGroup;
  if (hGroup.Open(szFilename)) {
    hGroup.Close();
    return TRUE;
  }
  return FALSE;
}

int C4Group_GetCreation(const char *szFilename) {
  int iResult = -1;
  C4Group hGroup;
  if (hGroup.Open(szFilename)) {
    iResult = hGroup.GetCreation();
    hGroup.Close();
  }
  return iResult;
}

BOOL C4Group_SetOriginal(const char *szFilename, BOOL fOriginal) {
  C4Group hGroup;
  if (!hGroup.Open(szFilename))
    return FALSE;
  hGroup.MakeOriginal(fOriginal);
  if (!hGroup.Close())
    return FALSE;
  return TRUE;
}

BOOL C4Group_CopyItem(const char *szSource, const char *szTarget1) {
  // Parameter check
  char szTarget[_MAX_PATH + 1];
  SCopy(szTarget1, szTarget, _MAX_PATH);
  if (!szSource || !szTarget || !szSource[0] || !szTarget[0]) {
    return FALSE;
  }

  // Backslash terminator indicates target is a path only (append filename)
  if (szTarget[SLen(szTarget) - 1] == '\\')
    SAppend(GetFilename(szSource), szTarget);

  // Check for identical source and target
  if (ItemIdentical(szSource, szTarget))
    return TRUE;

  // Source and target are simple items
  if (ItemExists(szSource) && CreateItem(szTarget)) {
    if (!CopyItem(szSource, szTarget)) {
        return FALSE;
    }
    return TRUE;
  }

  // Source & target
  C4Group hSourceParent, hTargetParent;
  char szSourceParentPath[_MAX_PATH + 1], szTargetParentPath[_MAX_PATH + 1];
  GetParentPath(szSource, szSourceParentPath);
  GetParentPath(szTarget, szTargetParentPath);

  // Temp filename
  char szTempFilename[_MAX_PATH + 1];
  SCopy(C4Group_TempPath, szTempFilename);
  SAppend(GetFilename(szSource), szTempFilename);
  MakeTempFilename(szTempFilename);

  // Extract source to temp file
  if (!hSourceParent.Open(szSourceParentPath)) { return FALSE; }
  if (!hSourceParent.Extract(GetFilename(szSource), szTempFilename)) { return FALSE; }
  if (!hSourceParent.Close()) { return FALSE; }

  // Move temp file to target
  if (!hTargetParent.Open(szTargetParentPath)) { EraseItem(szTempFilename); return FALSE; }
  if (!hTargetParent.Move(szTempFilename, GetFilename(szTarget))) { EraseItem(szTempFilename); return FALSE; }
  if (!hTargetParent.Close()) { EraseItem(szTempFilename); return FALSE; }

  return TRUE;
}

BOOL C4Group_MoveItem(const char *szSource, const char *szTarget1) {
  // Parameter check
  char szTarget[_MAX_PATH + 1];
  SCopy(szTarget1, szTarget, _MAX_PATH);
  if (!szSource || !szTarget || !szSource[0] || !szTarget[0])
    return FALSE;

  // Backslash terminator indicates target is a path only (append filename)
  if (szTarget[SLen(szTarget) - 1] == '\\')
    SAppend(GetFilename(szSource), szTarget);

  // Check for identical source and target
  if (ItemIdentical(szSource, szTarget))
    return TRUE;

  // Source and target are simple items
  if (ItemExists(szSource) && CreateItem(szTarget))
    return MoveItem(szSource, szTarget);

  // Source & target
  C4Group hSourceParent, hTargetParent;
  char szSourceParentPath[_MAX_PATH + 1], szTargetParentPath[_MAX_PATH + 1];
  GetParentPath(szSource, szSourceParentPath);
  GetParentPath(szTarget, szTargetParentPath);

  // Temp filename
  char szTempFilename[_MAX_PATH + 1];
  SCopy(C4Group_TempPath, szTempFilename);
  SAppend(GetFilename(szSource), szTempFilename);
  MakeTempFilename(szTempFilename);

  // Extract source to temp file
  if (!hSourceParent.Open(szSourceParentPath) || !hSourceParent.Extract(GetFilename(szSource), szTempFilename) || !hSourceParent.Close())
    return FALSE;

  // Move temp file to target
  if (!hTargetParent.Open(szTargetParentPath) || !hTargetParent.Move(szTempFilename, GetFilename(szSource)) || !hTargetParent.Close()) {
    EraseItem(szTempFilename);
    return FALSE;
  }

  // Delete original file
  if (!hSourceParent.Open(szSourceParentPath) || !hSourceParent.DeleteEntry(GetFilename(szSource)) || !hSourceParent.Close())
    return FALSE;

  return TRUE;
}

BOOL C4Group_PackDirectory(const char *szFilename) {

  // Check file type
  if (!(FileAttributes(szFilename) & _A_SUBDIR))
    return FALSE;

  // Ignore
  if (SIsModule(C4Group_Ignore, GetFilename(szFilename)))
    return TRUE;

  // Turn subfolders to groups
  char szContents[_MAX_PATH + 1];
  SCopy(szFilename, szContents);
  SAppend("/*", szContents);
  ForEachFile(szContents, _A_SUBDIR, C4Group_PackDirectory);

  // Process message
  if (C4Group_ProcessCallback)
    C4Group_ProcessCallback(szFilename, 0);

  // Create group file
  C4Group hGroup;
  char szTempFilename[_MAX_PATH + 1];
  if (C4Group_TempPath[0]) {
    SCopy(C4Group_TempPath, szTempFilename);
    SAppend(GetFilename(szFilename), szTempFilename);
  } else
    SCopy(szFilename, szTempFilename);
  MakeTempFilename(szTempFilename);
  if (!hGroup.Open(szTempFilename, TRUE))
    return FALSE;

  // Add folder contents to group
  SCopy(szFilename, szContents);
  SAppend("/*", szContents);
  if (!hGroup.Add(szContents)) {
    hGroup.Close();
    return FALSE;
  }

  // Close group
  hGroup.SortByList(C4Group_SortList, szFilename);
  if (!hGroup.Close())
    return FALSE;

  // Delete folder
  if (!EraseDirectory(szFilename))
    return FALSE;

  // Name group file
  if (!RenameFile(szTempFilename, szFilename))
    return FALSE;

  return TRUE;
}

BOOL C4Group_UnpackDirectory(const char *szFilename) {
  // Already unpacked: success
  if (FileAttributes(szFilename) & _A_SUBDIR)
    return TRUE;

  // Not a real file: unpack parent directory first
  char szParentFilename[_MAX_PATH + 1];
  if (!FileExists(szFilename))
    if (GetParentPath(szFilename, szParentFilename, _MAX_PATH))
      if (!C4Group_UnpackDirectory(szParentFilename))
        return FALSE;

  // Open group
  C4Group hGroup;
  if (!hGroup.Open(szFilename))
    return FALSE;

  // Process message
  if (C4Group_ProcessCallback)
    C4Group_ProcessCallback(szFilename, 0);

  // Create target directory
  char szFoldername[_MAX_PATH + 1];
  SCopy(szFilename, szFoldername, _MAX_PATH);
  MakeTempFilename(szFoldername);
  if (!CreateDirectory(szFoldername, NULL)) {
    hGroup.Close();
    return FALSE;
  }

  // Extract files to folder
  if (!hGroup.Extract("*.*", szFoldername)) {
    hGroup.Close();
    return FALSE;
  }

  // Close group
  hGroup.Close();

  // Delete group file
  if (!EraseItem(szFilename))
    return FALSE;

  // Rename target directory
  if (!RenameFile(szFoldername, szFilename))
    return FALSE;

  // Success
  return TRUE;
}

BOOL C4Group_ExplodeDirectory(const char *szFilename) {
  // Ignore
  if (SIsModule(C4Group_Ignore, GetFilename(szFilename)))
    return TRUE;

  // Unpack this directory
  if (!C4Group_UnpackDirectory(szFilename))
    return FALSE;

  // Explode all children
  char szContents[_MAX_PATH + 1];
  SCopy(szFilename, szContents);
  SAppend("/*", szContents);
  ForEachFile(szContents, _A_ALL, C4Group_ExplodeDirectory);

  // Success
  return TRUE;
}

void MemScramble(BYTE *bypBuffer, int iSize) {
  int cnt;
  for (cnt = 0; cnt + 2 < iSize; cnt += 3) {
    bypBuffer[cnt] ^= 237;
    bypBuffer[cnt + 1] ^= 237;
    bypBuffer[cnt + 2] ^= 237;
    BYTE b = bypBuffer[cnt];
    bypBuffer[cnt] = bypBuffer[cnt + 2];
    bypBuffer[cnt + 2] = b;
  }
}

//------------------------------- Bitmap Helper -------------------------------------------

void DeleteBitmap(HBITMAP hbmp) { DeleteObject(hbmp); }

//---------------------------------- C4Group ---------------------------------------------

C4GroupHeader::C4GroupHeader() { ZeroMem(this, sizeof(C4GroupHeader)); }

void C4GroupHeader::Init() {
  SCopy(C4GroupFileID, id);
  Ver1 = C4GroupFileVer1;
  Ver2 = C4GroupFileVer2;
  Entries = 0;
  SCopy("New C4Group", Maker, C4GroupMaxMaker);
  Password[0] = 0;
}

C4GroupEntry::C4GroupEntry() { ZeroMem(this, sizeof(C4GroupEntry)); }

C4GroupEntry::~C4GroupEntry() {
  if (HoldBuffer)
    if (bpMemBuf)
      delete[] bpMemBuf;
}

void C4GroupEntry::Set(_finddata_t &Fdt, const char *szPath) {
  ZeroMem(this, sizeof(C4GroupEntry));
  SCopy(Fdt.name, FileName, _MAX_FNAME);
  Size = Fdt.size;
  Time = Fdt.time_create;
  SCopy(szPath, DiskPath);
  AppendBackslash(DiskPath);
  SAppend(FileName, DiskPath);
  Status = C4GRES_OnDisk;
  Packed = FALSE;
  ChildGroup = FALSE; // FileGroupCheck(DiskPath);
  // Notice folder entries are not checked for ChildGroup status.
  // This would cause extreme performance loss and be good for
  // use in entry list display only.
}

C4Group::C4Group() {
  Init();
  StdOutput = FALSE;
  fnProcessCallback = NULL;
  MadeOriginal = FALSE;
}

void C4Group::Init() {
  // General
  Status = GRPF_Inactive;
  FileName[0] = 0;
  // Child status
  Mother = NULL;
  ExclusiveChild = FALSE;
  // File only
  FilePtr = 0;
  EntryOffset = 0;
  Modified = FALSE;
  Head.Init();
  FirstEntry = NULL;
  SearchPtr = NULL;
  // Folder only
  hFdt = -1;
  // Error status
  SCopy("No Error", ErrorString, C4GroupMaxError);
}

C4Group::~C4Group() { Clear(); }

BOOL C4Group::Error(const char *szStatus) {
  SCopy(szStatus, ErrorString, C4GroupMaxError);
  return FALSE;
}

const char *C4Group::GetError() { return ErrorString; }

void C4Group::SetStdOutput(BOOL fStatus) { StdOutput = fStatus; }

BOOL C4Group::Open(const char *szGroupName, BOOL fCreate) {
  if (!szGroupName)
    return Error("Open: Null filename");
  if (!szGroupName[0])
    return Error("Open: Empty filename");

  char szFixedName[_MAX_PATH + 1];
  SCopy(szGroupName, szFixedName, _MAX_PATH);
  // Normalize slashes for Linux
  for (int i = 0; szFixedName[i]; i++)
    if (szFixedName[i] == '\\')
      szFixedName[i] = '/';

  // Real reference
  int attrib;
  if (FileExists(szFixedName, &attrib)) {
    // Init
    Init();
    // Open group or folder
    return OpenReal(szFixedName);
  }

  // If requested, try creating a new group file
  if (fCreate) {
    CStdFile temp;
    if (temp.Create(szGroupName, FALSE)) {
      // Temporary file has been created
      temp.Close();
      // Init
      Init();
      Status = GRPF_File;
      SCopy(szGroupName, FileName, _MAX_FNAME);
      return TRUE;
    }
  }

  // While not a real reference (child group), trace back to mother group or
  // folder. Open mother and child in exclusive mode.
  char szRealGroup[_MAX_FNAME];
  SCopy(szGroupName, szRealGroup, _MAX_FNAME);
  do {
    if (!TruncatePath(szRealGroup)) {
      return Error("Open: File not found");
    }
  } while (!FileExists(szRealGroup));

  // Open mother and child in exclusive mode
  C4Group *pMother;
  if (!(pMother = new C4Group))
    return Error("Open: mem");
  pMother->SetStdOutput(StdOutput);
  if (!pMother->Open(szRealGroup)) {
    Clear();
    return Error("Open: Cannot open mother");
  }
  const char *szEntryName = szGroupName + SLen(szRealGroup);
  if (szEntryName[0] == '/' || szEntryName[0] == '\\')
    szEntryName++;
  if (!OpenAsChild(pMother, szEntryName, TRUE)) {
    Clear();
    return Error("Open:: Cannot open as child");
  }

  // Success
  return true;
}

BOOL C4Group::OpenReal(const char *szFilename) {
  // Get original filename
  if (!szFilename)
    return FALSE;
  SCopy(szFilename, FileName, _MAX_FNAME);
  struct _finddata_t fdt;
  intptr_t shnd;
  if ((shnd = _findfirst((char *)szFilename, &fdt)) < 0) {
    return FALSE;
  }
  _findclose(shnd);
  SCopy(GetFilename(fdt.name), GetFilename(FileName), _MAX_FNAME);

  // Folder
  if (FileAttributes(FileName) & _A_SUBDIR) {
    // Ignore
    if (SIsModule(C4Group_Ignore, GetFilename(szFilename)))
      return FALSE;
    // OpenReal: Simply set status and return
    Status = GRPF_Folder;
    SCopy("Open directory", Head.Maker, C4GroupMaxMaker);
    ResetSearch();
    // Success
    return TRUE;
  }

  // File: Try reading header and entries
  if (OpenRealGrpFile()) {
    Status = GRPF_File;
    ResetSearch();
    return TRUE;
  }

  return Error("OpenReal: Not a valid group");
}

BOOL C4Group::OpenRealGrpFile() {
  int cnt, file_entries;
  C4GroupEntryCore corebuf;

  // Open StdFile
  if (!StdFile.Open(FileName, TRUE))
    return Error("OpenRealGrpFile: Cannot open standard file");

  // Read header
  if (!StdFile.Read((BYTE *)&Head, sizeof(C4GroupHeader)))
    return Error("OpenRealGrpFile: Error reading header");
  MemScramble((BYTE *)&Head, sizeof(C4GroupHeader));
  EntryOffset += sizeof(C4GroupHeader);

  // Check Header
  if (!SEqual(Head.id, C4GroupFileID) || (Head.Ver1 != C4GroupFileVer1) || (Head.Ver2 > C4GroupFileVer2)) {
    return Error("OpenRealGrpFile: Invalid header");
  }

  // Read Entries
  file_entries = Head.Entries;
  Head.Entries = 0; // Reset, will be recounted by AddEntry
  for (cnt = 0; cnt < file_entries; cnt++) {
    if (!StdFile.Read((BYTE *)&corebuf, sizeof(C4GroupEntryCore)))
      return Error("OpenRealGrpFile: Error reading entries");
    EntryOffset += sizeof(C4GroupEntryCore);
    if (!AddEntry(C4GRES_InGroup, corebuf.ChildGroup, corebuf.FileName, corebuf.Size, corebuf.Time))
      return Error("OpenRealGrpFile: Cannot add entry");
  }

  return TRUE;
}

BOOL C4Group::AddEntry(int status, BOOL childgroup, const char *fname, long size, time_t time, const char *entryname, BYTE *membuf, BOOL fDeleteOnDisk, BOOL fHoldBuffer) {

  // Folder: add file to folder immediately
  if (Status == GRPF_Folder) {

    // Close open StdFile
    StdFile.Close();

    // Get path to target folder file
    char tfname[_MAX_FNAME];
    SCopy(FileName, tfname);
    AppendBackslash(tfname);
    if (entryname)
      SAppend(entryname, tfname);
    else
      SAppend(GetFilename(fname), tfname);

    switch (status) {

    case C4GRES_OnDisk: // Copy/move file to folder
      return (CopyItem(fname, tfname) && (!fDeleteOnDisk || EraseItem(fname)));

    case C4GRES_InMemory: // Save buffer to file in folder
      CStdFile hFile;
      BOOL fOkay;
      if (hFile.Create(tfname, childgroup))
        fOkay = hFile.Write(membuf, size);
      hFile.Close();

      if (fHoldBuffer)
        delete[] membuf;

      return fOkay;

      // InGrp & Deleted ignored
    }

    return Error("Add to folder: Invalid request");
  }

  // Group file: add to virtual entry list

  C4GroupEntry *nentry, *lentry, *centry;

  // Delete existing entries of same name
  centry = GetEntry(GetFilename(entryname ? entryname : fname));
  if (centry) {
    centry->Status = C4GRES_Deleted;
    Head.Entries--;
  }

  // Allocate memory for new entry
  if (!(nentry = new C4GroupEntry))
    return FALSE; //...theoretically, delete Hold buffer here

  // Find end of list
  for (lentry = FirstEntry; lentry && lentry->Next; lentry = lentry->Next)
    ;

  // Init entry core data
  if (entryname)
    SCopy(entryname, nentry->FileName, _MAX_FNAME);
  else
    SCopy(GetFilename(fname), nentry->FileName, _MAX_FNAME);
  nentry->Size = size;
  nentry->Time = time;
  nentry->ChildGroup = childgroup;
  nentry->Offset = 0;
  nentry->DeleteOnDisk = fDeleteOnDisk;
  nentry->HoldBuffer = fHoldBuffer;
  if (lentry)
    nentry->Offset = lentry->Offset + lentry->Size;

  // Init list entry data
  SCopy(fname, nentry->DiskPath, _MAX_FNAME);
  nentry->Status = status;
  nentry->bpMemBuf = membuf;
  nentry->Next = NULL;

  // Append entry to list
  if (lentry)
    lentry->Next = nentry;
  else
    FirstEntry = nentry;

  // Increase virtual file count of group
  Head.Entries++;

  return TRUE;
}

C4GroupEntry *C4Group::GetEntry(const char *szName) {
  if (Status == GRPF_Folder)
    return NULL;
  C4GroupEntry *centry;
  for (centry = FirstEntry; centry; centry = centry->Next)
    if (centry->Status != C4GRES_Deleted)
      if (WildcardMatch(szName, centry->FileName))
        return centry;
  return NULL;
}

BOOL C4Group::Close() {
  C4GroupEntry *centry;
  C4GroupEntryCore *save_core;
  BOOL fRewrite = FALSE;
  int cscore;
  char szTempFileName[_MAX_FNAME + 1], szGrpFileName[_MAX_FNAME + 1];

  if (Status == GRPF_Inactive)
    return FALSE;

  // Folder: just close
  if (Status == GRPF_Folder) {
    CloseExclusiveMother();
    Clear();
    return TRUE;
  }

  // Rewrite check
  for (centry = FirstEntry; centry; centry = centry->Next)
    if (centry->Status != C4GRES_InGroup)
      fRewrite = TRUE;
  if (Modified)
    fRewrite = TRUE;
  if (Head.Entries == 0)
    fRewrite = TRUE;

  // No rewrite: just close
  if (!fRewrite) {
    CloseExclusiveMother();
    Clear();
    return TRUE;
  }

  if (StdOutput)
    printf("Writing group file...\n");

  // Set new version
  Head.Ver1 = C4GroupFileVer1;
  Head.Ver2 = C4GroupFileVer2;

  // Creation stamp
  Head.Creation = time(NULL);

  // Lose original on any save unless made in this session
  if (!MadeOriginal)
    Head.Original = 0;

  // Automatic maker
  if (C4Group_Maker[0])
    SCopy(C4Group_Maker, Head.Maker);

  // Automatic sort
  SortByList(C4Group_SortList);

  // Create temporary core list with new actual offsets to be saved
  save_core = new C4GroupEntryCore[Head.Entries];
  cscore = 0;
  for (centry = FirstEntry; centry; centry = centry->Next)
    if (centry->Status != C4GRES_Deleted) {
      save_core[cscore] = (C4GroupEntryCore)*centry;
      // Make actual offset
      save_core[cscore].Offset = 0;
      if (cscore > 0)
        save_core[cscore].Offset = save_core[cscore - 1].Offset + save_core[cscore - 1].Size;
      cscore++;
    }

  // Create target temp file (in system temp directory!)
  SCopy(FileName, szGrpFileName);
  SCopy(Config.AtTempPath(GetFilename(FileName)), szTempFileName);
  MakeTempFilename(szTempFileName);
  // (Temp file must not have the same name as the group.)
  if (SEqual(szTempFileName, szGrpFileName)) {
    SAppend(".tmp", szTempFileName); // Add a second temp extension
    MakeTempFilename(szTempFileName);
  }

  // Create the new (temp) group file
  CStdFile tfile;
  if (!tfile.Create(szTempFileName, TRUE)) {
    CloseExclusiveMother();
    Clear();
    delete[] save_core;
    return Error("Close: ...");
  }

  // Save header and core list
  C4GroupHeader headbuf = Head;
  MemScramble((BYTE *)&headbuf, sizeof(C4GroupHeader));
  if (!tfile.Write((BYTE *)&headbuf, sizeof(C4GroupHeader)) || !tfile.Write((BYTE *)save_core, Head.Entries * sizeof(C4GroupEntryCore))) {
    CloseExclusiveMother();
    Clear();
    tfile.Close();
    delete[] save_core;
    return Error("Close: ...");
  }
  delete[] save_core;

  // Save Entries to temp file
  int iTotalSize = 0, iSizeDone = 0;
  for (centry = FirstEntry; centry; centry = centry->Next)
    iTotalSize += centry->Size;
  for (centry = FirstEntry; centry; centry = centry->Next)
    if (AppendEntry2StdFile(centry, tfile)) {
      iSizeDone += centry->Size;
      if (iTotalSize && fnProcessCallback)
        fnProcessCallback(centry->FileName, 100 * iSizeDone / iTotalSize);
    } else {
      CloseExclusiveMother();
      Clear();
      tfile.Close();
      return FALSE;
    }
  tfile.Close();

  // Child: move temp file to mother
  if (Mother) {
    if (!Mother->Move(szTempFileName, GetFilename(FileName))) {
      CloseExclusiveMother();
      Clear();
      return Error("Close: Cannot move rewritten child temp file to mother");
    }
    StdFile.Close();
    CloseExclusiveMother();
    Clear();
    return TRUE;
  }

  // Clear (close file)
  Clear();

  // Delete old group file, rename new file
  if (FileExists(szGrpFileName))
    if (!EraseFile(szGrpFileName))
      return Error("Close: Cannot erase temp file");
  if (!RenameFile(szTempFileName, szGrpFileName))
    return Error("Close: Cannot rename group file");

  return TRUE;
}

void C4Group::Clear() {
  // Delete entries
  C4GroupEntry *next;
  while (FirstEntry) {
    next = FirstEntry->Next;
    delete FirstEntry;
    FirstEntry = next;
  }
  // Close std file
  StdFile.Close();
  // Delete mother
  if (Mother && ExclusiveChild) {
    delete Mother;
    Mother = NULL;
  }
  // Release fdt search
  if (hFdt > -1)
    _findclose(hFdt);
  // Reset
  Init();
}

BOOL C4Group::AppendEntry2StdFile(C4GroupEntry *centry, CStdFile &hTarget) {
  CStdFile hSource;
  long csize;
  BYTE fbuf;

  switch (centry->Status) {

  case C4GRES_InGroup: // Copy from group to std file
    if (!SetFilePtr(centry->Offset))
      return Error("AE2S: Cannot set file pointer");
    for (csize = centry->Size; csize > 0; csize--) {
      if (!Read(&fbuf, 1))
        return Error("AE2S: Cannot read entry from group file");
      if (!hTarget.Write(&fbuf, 1))
        return Error("AE2S: Cannot write to target file");
    }
    break;

  case C4GRES_OnDisk: // Copy/move from disk item to std file
    char szFileSource[_MAX_FNAME + 1];
    SCopy(centry->DiskPath, szFileSource);

    // Disk item is a directory
    if (FileAttributes(centry->DiskPath) & _A_SUBDIR)
      return Error("AE2S: Cannot add directory to group file");

    // Append disk source to target file
    if (!hSource.Open(szFileSource, centry->ChildGroup))
      return Error("AE2S: Cannot open on-disk file");
    for (csize = centry->Size; csize > 0; csize--) {
      if (!hSource.Read(&fbuf, 1)) {
        hSource.Close();
        return Error("AE2S: Cannot read on-disk file");
      }
      if (!hTarget.Write(&fbuf, 1)) {
        hSource.Close();
        return Error("AE2S: Cannot write to target file");
      }
    }
    hSource.Close();

    // Erase disk source if requested
    if (centry->DeleteOnDisk)
      EraseItem(szFileSource);

    break;

  case C4GRES_InMemory: // Copy from mem to std file
    if (!centry->bpMemBuf)
      return Error("AE2S: no buffer");
    if (!hTarget.Write(centry->bpMemBuf, centry->Size))
      return Error("AE2S: writing error");
    break;

  case C4GRES_Deleted: // Don't save
    break;

  default: // Unknown file status
    return Error("AE2S: Unknown file status");
  }

  return TRUE;
}

void C4Group::ResetSearch() {
  switch (Status) {
  case GRPF_Folder:
    if (hFdt > -1)
      _findclose(hFdt);
    static char szSearchPath[_MAX_FNAME + 1];
    SCopy(FileName, szSearchPath);
    AppendBackslash(szSearchPath);
    SAppend("*", szSearchPath);
    SearchPtr = NULL;
    if ((hFdt = _findfirst(szSearchPath, &Fdt)) > -1) {
      FolderSearchEntry.Set(Fdt, FileName);
      SearchPtr = &FolderSearchEntry;
    }
    break;
  case GRPF_File:
    SearchPtr = FirstEntry;
    break;
  }
}

C4GroupEntry *C4Group::GetNextFolderEntry() {
  if (hFdt < 0)
    return NULL;
  if (_findnext(hFdt, &Fdt) != 0)
    return NULL;
  FolderSearchEntry.Set(Fdt, FileName);
  return &FolderSearchEntry;
}

C4GroupEntry *C4Group::SearchNextEntry(const char *szName) {
  // Search by group type
  C4GroupEntry *pEntry;
  switch (Status) {
  case GRPF_File:
    for (pEntry = SearchPtr; pEntry; pEntry = pEntry->Next)
      if (pEntry->Status != C4GRES_Deleted)
        if (WildcardMatch(szName, pEntry->FileName)) {
          SearchPtr = pEntry->Next;
          return pEntry;
        }
    break;
  case GRPF_Folder:
    for (pEntry = SearchPtr; pEntry; pEntry = GetNextFolderEntry())
      if (WildcardMatch(szName, pEntry->FileName))
        if (!SIsModule(C4Group_Ignore, GetFilename(pEntry->FileName))) {
          LastFolderSearchEntry = (*pEntry);
          pEntry = &LastFolderSearchEntry;
          SearchPtr = GetNextFolderEntry();
          return pEntry;
        }
    break;
  }
  // No entry found: reset search pointer
  SearchPtr = NULL;
  return NULL;
}

BOOL C4Group::SetFilePtr(int iOffset) {
  if (Status == GRPF_Folder)
    return Error("SetFilePtr not implemented for Folders");

  // Rewind if necessary
  if (FilePtr > iOffset)
    if (!RewindFilePtr())
      return FALSE;

  // Advance to target pointer
  if (FilePtr < iOffset)
    if (!AdvanceFilePtr(iOffset - FilePtr))
      return FALSE;

  return TRUE;
}

BOOL C4Group::Advance(int iOffset) {
  BYTE buf;
  for (; iOffset > 0; iOffset--)
    if (!Read(&buf, 1))
      return FALSE;
  return TRUE;
}

BOOL C4Group::Read(void *pBuffer, int iSize) {

  switch (Status) {
  case GRPF_File:
    // Child group: read from mother group
    if (Mother) {
      if (!Mother->Read(pBuffer, iSize)) {
        RewindFilePtr();
        return Error("Read:");
      }
    }
    // Regular group: read from standard file
    else {
      if (!StdFile.Read(pBuffer, iSize)) {
        RewindFilePtr();
        return Error("Read:");
      }
    }
    FilePtr += iSize;
    break;
  case GRPF_Folder:
    if (!StdFile.Read(pBuffer, iSize))
      return Error("Read: Error reading from folder contents");
    break;
  }

  return TRUE;
}

BOOL C4Group::AdvanceFilePtr(int iOffset, C4Group *pByChild) {
  // Child group file: pass command to mother
  if ((Status == GRPF_File) && Mother) {

    // Ensure mother file ptr for it may have been moved by foreign access to
    // mother
    if (!Mother->EnsureChildFilePtr(this))
      return FALSE;

    if (!Mother->AdvanceFilePtr(iOffset, this))
      return FALSE;

  }
  // Regular group
  else if (Status == GRPF_File) {
    if (!StdFile.Advance(iOffset))
      return FALSE;
  }
  // Open folder
  else {
    if (!StdFile.Advance(iOffset))
      return FALSE;
  }

  // Advanced
  FilePtr += iOffset;

  return TRUE;
}

BOOL C4Group::RewindFilePtr() {

  // Child group file: pass command to mother
  if ((Status == GRPF_File) && Mother) {
    if (!Mother->SetFilePtr2Entry(FileName, this)) // Set to group file start
      return FALSE;
    if (!Mother->AdvanceFilePtr(EntryOffset, this)) // Advance data offset
      return FALSE;
  }
  // Regular group or open folder: rewind standard file
  else {
    if (!StdFile.Rewind()) // Set to group file start
      return FALSE;
    if (!StdFile.Advance(EntryOffset)) // Advance data offset
      return FALSE;
  }

  FilePtr = 0;

  return TRUE;
}

BOOL C4Group::View(const char *szFiles) {
  char oformat[100];
  C4GroupEntry *centry;
  int fcount = 0, bcount = 0; // Virtual counts
  int maxfnlen = 0;

  if (!StdOutput)
    return FALSE;

  // Display list
  ResetSearch();
  while (centry = SearchNextEntry(szFiles)) {
    fcount++;
    bcount += centry->Size;
    maxfnlen = Max(maxfnlen, SLen(centry->FileName));
  }
  sprintf(oformat, "%%%ds %%7ld Bytes %%02d.%%02d.%%02d %%02d:%%02d:%%02d%%s\n", maxfnlen);

  printf("Maker: %s  Creation: %i  %s\n\rVersion: %d.%d  Password: %s\n", GetMaker(), Head.Creation, GetOriginal() ? "Original" : "", Head.Ver1, Head.Ver2, GetPassword()[0] ? GetPassword() : "None");
  ResetSearch();
  while (centry = SearchNextEntry(szFiles)) {
    time_t entryTime = centry->Time;
    tm coretm = *localtime(&entryTime);
    printf(oformat, centry->FileName, centry->Size, coretm.tm_mday, coretm.tm_mon + 1, coretm.tm_year % 100, coretm.tm_hour, coretm.tm_min, coretm.tm_sec, centry->ChildGroup ? " (Group)" : "");
  }
  printf("%d Entries, %ld Bytes\n", fcount, bcount);

  return TRUE;
}

BOOL C4Group::Add(const char *szFiles) {
  BOOL fMove = FALSE;

  if (StdOutput)
    printf("%s...\n", fMove ? "Moving" : "Adding");

  // Add files & directories
  char szFileName[_MAX_FNAME + 1];
  int iFileCount = 0;
  long lAttrib = _A_ALL;
  struct _finddata_t fdt;
  intptr_t fdthnd;

  // Process segmented path & search wildcards
  for (int cseg = 0; SCopySegment(szFiles, cseg, szFileName, ';'); cseg++)
    if ((fdthnd = _findfirst((char *)szFileName, &fdt)) >= 0) {
      do {
        if (fdt.attrib & lAttrib) {
          // Compose item path
          SCopy(szFiles, szFileName);
          SCopy(fdt.name, GetFilename(szFileName));
          // File count
          iFileCount++;
          // Process output & callback
          if (StdOutput)
            printf("%s\n", GetFilename(szFileName));
          if (fnProcessCallback)
            fnProcessCallback(GetFilename(szFileName), 0); // cbytes/tbytes
                                                           // AddEntryOnDisk
          AddEntryOnDisk(szFileName, NULL, fMove);
        }
      } while (_findnext(fdthnd, &fdt) == 0);
      _findclose(fdthnd);
    }

  if (StdOutput)
    printf("%d file(s) %s.\n", iFileCount, fMove ? "moved" : "added");

  return TRUE;
}

BOOL C4Group::Move(const char *szFiles) {
  BOOL fMove = TRUE;

  if (StdOutput)
    printf("%s...\n", fMove ? "Moving" : "Adding");

  // Add files & directories
  char szFileName[_MAX_FNAME + 1];
  int iFileCount = 0;
  long lAttrib = _A_ALL;
  struct _finddata_t fdt;
  intptr_t fdthnd;

  // Process segmented path & search wildcards
  for (int cseg = 0; SCopySegment(szFiles, cseg, szFileName, ';'); cseg++)
    if ((fdthnd = _findfirst((char *)szFileName, &fdt)) >= 0) {
      do {
        if (fdt.attrib & lAttrib) {
          // Compose item path
          SCopy(szFiles, szFileName);
          SCopy(fdt.name, GetFilename(szFileName));
          // File count
          iFileCount++;
          // Process output & callback
          if (StdOutput)
            printf("%s\n", GetFilename(szFileName));
          if (fnProcessCallback)
            fnProcessCallback(GetFilename(szFileName), 0); // cbytes/tbytes
                                                           // AddEntryOnDisk
          AddEntryOnDisk(szFileName, NULL, fMove);
        }
      } while (_findnext(fdthnd, &fdt) == 0);
      _findclose(fdthnd);
    }

  if (StdOutput)
    printf("%d file(s) %s.\n", iFileCount, fMove ? "moved" : "added");

  return TRUE;
}

BOOL C4Group::AddEntryOnDisk(const char *szFilename, const char *szAddAs, BOOL fMove) {

  // Do not process yourself
  if (ItemIdentical(szFilename, FileName))
    return TRUE;

  // File is a directory: copy to temp path, pack, and add packed file
  if (FileAttributes(szFilename) & _A_SUBDIR) {
    // Ignore
    if (SIsModule(C4Group_Ignore, GetFilename(szFilename)))
      return TRUE;
    // Temp filename
    char szTempFilename[_MAX_PATH + 1];
    if (C4Group_TempPath[0]) {
      SCopy(C4Group_TempPath, szTempFilename);
      SAppend(GetFilename(szFilename), szTempFilename);
    } else
      SCopy(szFilename, szTempFilename);
    MakeTempFilename(szTempFilename);
    // Copy or move item to temp file (moved items might be killed if later
    // process fails)
    if (fMove) {
      if (!MoveItem(szFilename, szTempFilename))
        return Error("AddEntryOnDisk: Move failure");
    } else {
      if (!CopyItem(szFilename, szTempFilename))
        return Error("AddEntryOnDisk: Copy failure");
    }
    // Pack temp file
    if (!C4Group_PackDirectory(szTempFilename))
      return Error("AddEntryOnDisk: Pack directory failure");
    // Add temp file
    if (!szAddAs)
      szAddAs = GetFilename(szFilename);
    szFilename = szTempFilename;
    fMove = TRUE;
  }

  // Determine size
  BOOL fIsGroup = C4Group_IsGroup(szFilename);
  int iSize = fIsGroup ? UncompressedFileSize(szFilename) : FileSize(szFilename);

  // AddEntry
  return AddEntry(C4GRES_OnDisk, fIsGroup, szFilename, iSize, FileTime(szFilename), szAddAs, NULL, fMove);
}

BOOL C4Group::Add(const char *szFile, const char *szAddAs) {
  BOOL fMove = FALSE;

  if (StdOutput)
    printf("%s %s as %s...\n", fMove ? "Moving" : "Adding", GetFilename(szFile), szAddAs);

  return AddEntryOnDisk(szFile, szAddAs, fMove);
}

BOOL C4Group::Move(const char *szFile, const char *szAddAs) {
  BOOL fMove = TRUE;

  if (StdOutput)
    printf("%s %s as %s...\n", fMove ? "Moving" : "Adding", GetFilename(szFile), szAddAs);

  return AddEntryOnDisk(szFile, szAddAs, fMove);
}

BOOL C4Group::Delete(const char *szFiles) {
  int fcount;
  C4GroupEntry *tentry;

  // Segmented file specs
  if (SCharCount(';', szFiles)) {
    BOOL success = TRUE;
    char filespec[_MAX_FNAME + 1];
    for (int cseg = 0; SCopySegment(szFiles, cseg, filespec, ';', _MAX_FNAME); cseg++)
      if (!Delete(filespec))
        success = FALSE;
    return success;
  }

  // Delete all matching Entries
  ResetSearch();
  fcount = 0;
  while ((tentry = SearchNextEntry(szFiles))) {
    // StdOutput
    if (StdOutput)
      printf("%s\n", tentry->FileName);
    if (!DeleteEntry(tentry->FileName))
      return Error("Delete: Could not delete entry");
    fcount++;
  }

  // StdOutput
  if (StdOutput)
    printf("%d file(s) deleted.\n", fcount);

  return TRUE;
}

BOOL C4Group::DeleteEntry(const char *szFilename) {
  switch (Status) {
  case GRPF_File:
    // Get entry
    C4GroupEntry *pEntry;
    if (!(pEntry = GetEntry(szFilename)))
      return FALSE;
    // Delete moved source files
    if (pEntry->Status == C4GRES_OnDisk)
      if (pEntry->DeleteOnDisk)
        EraseItem(pEntry->DiskPath);
    // (moved buffers are deleted by ~C4GroupEntry)
    // Delete status and update virtual file count
    pEntry->Status = C4GRES_Deleted;
    Head.Entries--;
    break;
  case GRPF_Folder:
    StdFile.Close();
    char szPath[_MAX_FNAME + 1];
    sprintf(szPath, "%s/%s", FileName, szFilename);
    if (!EraseItem(szPath))
      return FALSE;
    break;
  }
  return TRUE;
}

BOOL C4Group::Rename(const char *szFile, const char *szNewName) {

  if (StdOutput)
    printf("Renaming %s to %s...\n", szFile, szNewName);

  switch (Status) {
  case GRPF_File:
    // Get entry
    C4GroupEntry *pEntry;
    if (!(pEntry = GetEntry(szFile)))
      return Error("Rename: File not found");
    // Check double name
    if (GetEntry(szNewName))
      return Error("Rename: File exists already");
    // Rename
    SCopy(szNewName, pEntry->FileName, _MAX_FNAME);
    Modified = TRUE;
    break;
  case GRPF_Folder:
    StdFile.Close();
    char path[_MAX_FNAME + 1];
    SCopy(FileName, path);
    AppendBackslash(path);
    SAppend(szFile, path);
    char path2[_MAX_FNAME + 1];
    SCopy(FileName, path2);
    AppendBackslash(path2);
    SAppend(szNewName, path2);
    if (!RenameFile(path, path2))
      return Error("Rename: Failure");
    break;
  }

  return TRUE;
}

BOOL C4Group::Extract(const char *szFiles, const char *szExtractTo) {

  // StdOutput
  if (StdOutput) {
    printf("Extracting");
    if (szExtractTo)
      printf(" to %s", szExtractTo);
    printf("...\n");
  }

  int fcount = 0;
  int cbytes, tbytes;
  C4GroupEntry *tentry;

  cbytes = 0;
  tbytes = EntrySize();

  // Search all entries
  ResetSearch();
  while (tentry = SearchNextEntry(szFiles)) {
    // Process data & output
    if (StdOutput)
      printf("%s\n", tentry->FileName);
    cbytes += tentry->Size;
    if (fnProcessCallback)
      fnProcessCallback(tentry->FileName, 100 * cbytes / Max(tbytes, 1));

    // Extract
    if (!ExtractEntry(tentry->FileName, szExtractTo))
      return Error("Extract: Could not extract entry");

    fcount++;
  }

  if (StdOutput)
    printf("%d file(s) extracted.\n", fcount);

  return TRUE;
}

BOOL C4Group::ExtractEntry(const char *szFilename, const char *szExtractTo) {
  CStdFile tfile;
  CStdFile hDummy;
  char szTempFName[_MAX_FNAME + 1], szTargetFName[_MAX_FNAME + 1];

  // Target file name
  if (szExtractTo) {
    SCopy(szExtractTo, szTargetFName);
    if (FileAttributes(szTargetFName) & _A_SUBDIR) {
      AppendBackslash(szTargetFName);
      SAppend(szFilename, szTargetFName);
    }
  } else
    SCopy(szFilename, szTargetFName);

  // Extract
  switch (Status) {
  case GRPF_File: // Copy entry to target
    // Get entry
    C4GroupEntry *pEntry;
    if (!(pEntry = GetEntry(szFilename)))
      return FALSE;
    // Create dummy file to reserve target file name
    hDummy.Save(szTargetFName, (BYTE *)"Dummy", 5);
    // Make temp target file name
    SCopy(szTargetFName, szTempFName);
    MakeTempFilename(szTempFName);
    // Create temp target file
    if (!tfile.Create(szTempFName, pEntry->ChildGroup)) // Use ChildGroup flag so only subgroups are compressed
      return Error("Extract: Cannot create target file");
    // Write entry file to temp target file
    if (!AppendEntry2StdFile(pEntry, tfile)) {
      // Failure: close and erase temp target file
      tfile.Close();
      EraseItem(szTempFName);
      // Also erase reservation target file
      EraseItem(szTargetFName);
      // Failure
      return FALSE;
    }
    // Close target file
    tfile.Close();
    // Make temp file to original file
    if (FileExists(szTargetFName))
      if (!EraseFile(szTargetFName))
        return Error("Extract: Cannot erase temporary file");
    if (!RenameFile(szTempFName, szTargetFName))
      return Error("Extract: Cannot rename temporary file");
    // Set output file time
    _utimbuf tftime;
    tftime.actime = pEntry->Time;
    tftime.modtime = pEntry->Time;
    _utime(szTargetFName, &tftime);
    break;
  case GRPF_Folder: // Copy item from folder to target
    char szPath[_MAX_FNAME + 1];
    sprintf(szPath, "%s/%s", FileName, szFilename);
    if (!CopyItem(szPath, szTargetFName))
      return Error("ExtractEntry: Cannot copy item");
    break;
  }

  return TRUE;
}

BOOL C4Group::OpenAsChild(C4Group *pMother, const char *szEntryName, BOOL fExclusive) {

  if (!pMother)
    return Error("OpenAsChild: No mother specified");

  if (SCharCount('*', szEntryName))
    return Error("OpenAsChild: No wildcards allowed");

  // Open nested child group check: If szEntryName is a reference to
  // a nested group, open the first mother, then open the child
  // in exclusive mode

  if (SCharCount('\\', szEntryName)) {
    char mothername[_MAX_FNAME + 1];
    SCopyUntil(szEntryName, mothername, '\\', _MAX_FNAME);

    C4Group *pMother2;
    if (!(pMother2 = new C4Group))
      return Error("OpenAsChild: mem");
    pMother2->SetStdOutput(StdOutput);
    if (!pMother2->OpenAsChild(pMother, mothername, TRUE))
      return Error("OpenAsChild: Cannot open mother");
    return OpenAsChild(pMother2, szEntryName + SLen(mothername) + 1, TRUE);
  }

  // Init
  SCopy(szEntryName, FileName, _MAX_FNAME);
  Mother = pMother;
  ExclusiveChild = fExclusive;

  // Folder: Simply set status and return
  char path[_MAX_FNAME + 1];
  SCopy(GetFullName(), path);
  if (FileAttributes(path) & _A_SUBDIR) {
    SCopy(path, FileName);
    SCopy("Open directory", Head.Maker, C4GroupMaxMaker);
    Status = GRPF_Folder;
    ResetSearch();
    return TRUE;
  }

  // File
  Status = GRPF_File;

  // Get original entry name
  C4GroupEntry *centry;
  if (centry = Mother->GetEntry(FileName))
    SCopy(centry->FileName, FileName);

  // Access entry in mother group
  if (!Mother->AccessEntry(FileName))
    return Error("OpenAsChild: Entry not in mother group");

  // Read header
  if (!Mother->Read(&Head, sizeof(C4GroupHeader)))
    return Error("OpenAsChild: Entry reading error");
  MemScramble((BYTE *)&Head, sizeof(C4GroupHeader));
  EntryOffset += sizeof(C4GroupHeader);

  // Check Header
  if (!SEqual(Head.id, C4GroupFileID) || (Head.Ver1 != C4GroupFileVer1) || (Head.Ver2 > C4GroupFileVer2))
    return Error("OpenAsChild: Invalid Header");

  // Read Entries
  C4GroupEntryCore corebuf;
  int file_entries = Head.Entries;
  Head.Entries = 0; // Reset, will be recounted by AddEntry
  for (int cnt = 0; cnt < file_entries; cnt++) {
    if (!Mother->Read(&corebuf, sizeof(C4GroupEntryCore)))
      return Error("OpenAsChild: Entry reading error");
    EntryOffset += sizeof(C4GroupEntryCore);
    if (!AddEntry(C4GRES_InGroup, corebuf.ChildGroup, corebuf.FileName, corebuf.Size, corebuf.Time))
      return Error("OpenAsChild: Insufficient memory");
  }

  ResetSearch();

  return TRUE;
}

BOOL C4Group::AccessEntry(const char *szWildCard, int *iSize, char *sFileName, BOOL *fChild) {
  char fname[_MAX_FNAME + 1];
  if (!FindEntry(szWildCard, fname, iSize, fChild))
    return FALSE;
  if (!SetFilePtr2Entry(fname))
    return FALSE;
  if (sFileName)
    SCopy(fname, sFileName);
  return TRUE;
}

BOOL C4Group::AccessNextEntry(const char *szWildCard, int *iSize, char *sFileName, BOOL *fChild) {
  char fname[_MAX_FNAME + 1];
  if (!FindNextEntry(szWildCard, fname, iSize, fChild))
    return FALSE;
  if (!SetFilePtr2Entry(fname))
    return FALSE;
  if (sFileName)
    SCopy(fname, sFileName);
  return TRUE;
}

BOOL C4Group::SetFilePtr2Entry(const char *szName, C4Group *pByChild) {
  switch (Status) {

  case GRPF_File:
    C4GroupEntry *centry;
    if (!(centry = GetEntry(szName)))
      return FALSE;
    if (centry->Status != C4GRES_InGroup)
      return FALSE;
    return SetFilePtr(centry->Offset);

  case GRPF_Folder:
    StdFile.Close();
    char path[_MAX_FNAME + 1];
    SCopy(FileName, path);
    AppendBackslash(path);
    SAppend(szName, path);
    BOOL childgroup = C4Group_IsGroup(path);
    return StdFile.Open(path, childgroup);
  }
  return FALSE;
}

BOOL C4Group::FindEntry(const char *szWildCard, char *sFileName, int *iSize, BOOL *fChild) {
  ResetSearch();
  return FindNextEntry(szWildCard, sFileName, iSize, fChild);
}

BOOL C4Group::FindNextEntry(const char *szWildCard, char *sFileName, int *iSize, BOOL *fChild, BOOL fStartAtFilename) {
  C4GroupEntry *centry;
  if (!szWildCard)
    return FALSE;

  // Reset search to specified position
  if (fStartAtFilename)
    FindEntry(sFileName);

  if (!(centry = SearchNextEntry(szWildCard)))
    return FALSE;
  if (sFileName)
    SCopy(centry->FileName, sFileName);
  if (iSize)
    *iSize = centry->Size;
  if (fChild)
    *fChild = centry->ChildGroup;
  return TRUE;
}

BOOL C4Group::Add(const char *szName, void *pBuffer, int iSize, BOOL fChild, BOOL fHoldBuffer) {
  return AddEntry(C4GRES_InMemory, fChild, szName, iSize, time(NULL), szName, (BYTE *)pBuffer, FALSE, fHoldBuffer);
}

HBITMAP C4Group::SubReadDDB(HDC hdc, int sx, int sy, int swdt, int shgt, int twdt, int thgt, BOOL transcol) {
  HBITMAP hbmp;
  BITMAPFILEHEADER fhead;
  BITMAPINFO *pbmi = (BITMAPINFO *)new BYTE[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
  long bfoffs;
  BYTE fbuf;
  BYTE *bmpbits;
  const BOOL f256Only = TRUE; // Format accept flag

  // Read and check file header
  if (!Read(&fhead, sizeof(fhead))) {
    delete pbmi;
    return NULL;
  }
  if (fhead.bfType != *((WORD *)"BM")) {
    delete pbmi;
    return NULL;
  }

  // Read and check bitmap info header
  if (!Read(&(pbmi->bmiHeader), sizeof(BITMAPINFOHEADER))) {
    delete pbmi;
    return NULL;
  }
  if (f256Only)
    if ((pbmi->bmiHeader.biBitCount != 8) || (pbmi->bmiHeader.biCompression != 0)) {
      delete pbmi;
      return NULL;
    }
  if (!pbmi->bmiHeader.biSizeImage) {
    delete pbmi;
    return NULL;
  }

  // Read colors
  if (!Read(pbmi->bmiColors, 256 * sizeof(RGBQUAD))) {
    delete pbmi;
    return NULL;
  }

  if (transcol) {
    pbmi->bmiColors[0].rgbRed = 0xFF;
    pbmi->bmiColors[0].rgbBlue = 0xFF;
    pbmi->bmiColors[0].rgbGreen = 0x00;
  }

  // Read offset to pixels
  for (bfoffs = fhead.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER) - 256 * sizeof(RGBQUAD); bfoffs > 0; bfoffs--)
    if (!Read(&fbuf, 1)) {
      delete pbmi;
      return NULL;
    }

  // Read the pixels
  int iBufferSize = pbmi->bmiHeader.biHeight * DWordAligned(pbmi->bmiHeader.biWidth);
  if (!(bmpbits = new BYTE[iBufferSize])) {
    delete pbmi;
    return NULL;
  }
  if (!Read(bmpbits, iBufferSize)) {
    delete[] bmpbits;
    delete pbmi;
    return NULL;
  }

  // Cut bitmap to section if desired
  if ((sx > -1) && (sy > -1) && (swdt > -1) && (shgt > -1)) {
    if (twdt == -1)
      twdt = swdt;
    if (thgt == -1)
      thgt = shgt;

    if (sx + swdt > pbmi->bmiHeader.biWidth) {
      delete pbmi;
      return NULL;
    }
    if (sy + shgt > pbmi->bmiHeader.biHeight) {
      delete pbmi;
      return NULL;
    }

    int tbufwdt = twdt;
    DWordAlign(tbufwdt);
    int tbufhgt = thgt;
    int sbufwdt = pbmi->bmiHeader.biWidth;
    DWordAlign(sbufwdt);
    int sbufhgt = pbmi->bmiHeader.biHeight;
    BYTE *tbuf;
    if (tbuf = new BYTE[tbufhgt * tbufwdt]) {
      ZeroMem(tbuf, tbufhgt * tbufwdt);
      BufferBlitAspect(bmpbits, sbufwdt, sbufhgt, sx, sy, swdt, shgt, tbuf, tbufwdt, tbufhgt, 0, 0, twdt, thgt);
      delete[] bmpbits;
      bmpbits = tbuf;
      pbmi->bmiHeader.biSizeImage = tbufwdt * tbufhgt;
      pbmi->bmiHeader.biWidth = twdt;
      pbmi->bmiHeader.biHeight = thgt;
    }
  }

  // Create the bitmap
  hbmp = (HBITMAP)CreateDIBitmap(hdc, (BITMAPINFOHEADER *)&(pbmi->bmiHeader), CBM_INIT, bmpbits, (BITMAPINFO *)pbmi, DIB_RGB_COLORS);

  delete[] bmpbits;
  delete pbmi;

  return hbmp;
}

BOOL C4Group::ReadDDB(HBITMAP *lphBitmap, HDC hdc) {
  BOOL fOwnHDC = FALSE;
  if (!lphBitmap)
    return FALSE;
  // Check DC
  if (!hdc) {
    hdc = (HDC)GetDC(NULL);
    fOwnHDC = TRUE;
  }
  // Read bitmap
  *lphBitmap = SubReadDDB(hdc);
  // Release DC
  if (fOwnHDC)
    ReleaseDC(NULL, hdc);
  if (!(*lphBitmap))
    return FALSE;
  return TRUE;
}

BOOL C4Group::ReadDDBSection(HBITMAP *lphBitmap, HDC hdc, int iSecX, int iSecY, int iSecWdt, int iSecHgt, int iImgWdt, int iImgHgt, BOOL fTransCol) {
  BOOL fOwnHDC = FALSE;
  // Init & argument check
  if (!lphBitmap)
    return FALSE;
  // Check DC
  if (!hdc) {
    hdc = (HDC)GetDC(NULL);
    fOwnHDC = TRUE;
  }
  // Read bitmap
  *lphBitmap = SubReadDDB(hdc, iSecX, iSecY, iSecWdt, iSecHgt, iImgWdt, iImgHgt, fTransCol);
  // Release DC
  if (fOwnHDC)
    ReleaseDC(NULL, hdc);
  if (!(*lphBitmap))
    return FALSE;
  return TRUE;
}

const char *C4Group::GetName() { return FileName; }

int C4Group::EntryCount(const char *szWildCard) {
  int fcount;
  C4GroupEntry *tentry;
  // All files if no wildcard
  if (!szWildCard)
    szWildCard = "*";
  // Match wildcard
  ResetSearch();
  fcount = 0;
  while (tentry = SearchNextEntry(szWildCard))
    fcount++;
  return fcount;
}

int C4Group::EntrySize(const char *szWildCard) {
  int fsize;
  C4GroupEntry *tentry;
  // All files if no wildcard
  if (!szWildCard)
    szWildCard = "*";
  // Match wildcard
  ResetSearch();
  fsize = 0;
  while (tentry = SearchNextEntry(szWildCard))
    fsize += tentry->Size;
  return fsize;
}

int C4Group::EntryTime(const char *szFilename) {
  int iTime = 0;
  switch (Status) {
  case GRPF_File:
    C4GroupEntry *pEntry;
    pEntry = GetEntry(szFilename);
    if (pEntry)
      iTime = pEntry->Time;
    break;
  case GRPF_Folder:
    char szPath[_MAX_FNAME + 1];
    sprintf(szPath, "%s/%s", FileName, szFilename);
    iTime = FileTime(szPath);
    break;
  }
  return iTime;
}

int C4Group::LoadEntry(const char *szEntryName, BYTE **lpbpBuf, int *ipSize, int iAppendZeros) {
  int size;

  // Access entry, allocate buffer, read data
  (*lpbpBuf) = NULL;
  if (ipSize)
    *ipSize = 0;
  if (!AccessEntry(szEntryName, &size))
    return Error("LoadEntry: Not found");
  if (!((*lpbpBuf) = new BYTE[size + iAppendZeros]))
    return Error("LoadEntry: Insufficient memory");
  if (!Read(*lpbpBuf, size)) {
    delete[] (*lpbpBuf);
    return Error("LoadEntry: Reading error");
  }

  if (ipSize)
    *ipSize = size;

  if (iAppendZeros)
    ZeroMem((*lpbpBuf) + size, iAppendZeros);

  return TRUE;
}

void C4Group::SetMaker(const char *szMaker) {
  if (!SEqual(szMaker, Head.Maker))
    Modified = TRUE;
  SCopy(szMaker, Head.Maker, C4GroupMaxMaker);
}

void C4Group::SetPassword(const char *szPassword) {
  if (!SEqual(szPassword, Head.Password))
    Modified = TRUE;
  SCopy(szPassword, Head.Password, C4GroupMaxPassword);
}

const char *C4Group::GetMaker() { return Head.Maker; }

const char *C4Group::GetPassword() { return Head.Password; }

int C4Group::GetVersion() { return Head.Ver1 * 10 + Head.Ver2; }

void C4Group::SetProcessCallback(BOOL (*fnCallback)(const char *, int)) { fnProcessCallback = fnCallback; }

int SortRank(const char *szElement, const char *szSortList) {
  int cnt;
  char csegment[_MAX_FNAME + 1];

  for (cnt = 0; SCopySegment(szSortList, cnt, csegment, '|', _MAX_FNAME); cnt++)
    if (WildcardMatch(csegment, szElement))
      return (SCharCount('|', szSortList) + 1) - cnt;

  return 0;
}

BOOL C4Group::Sort(const char *szSortList) {
  BOOL fBubble;
  C4GroupEntry *centry, *prev, *next, *nextnext;

  if (!szSortList || !szSortList[0])
    return FALSE;

  do {
    fBubble = FALSE;

    for (prev = NULL, centry = FirstEntry; centry; prev = centry, centry = next)
      if (next = centry->Next)
        if (SortRank(centry->FileName, szSortList) < SortRank(next->FileName, szSortList)) {
          nextnext = next->Next;
          if (prev)
            prev->Next = next;
          else
            FirstEntry = next;
          next->Next = centry;
          centry->Next = nextnext;
          next = nextnext;

          fBubble = TRUE;
          Modified = TRUE;
        }

  } while (fBubble);

  return TRUE;
}

C4Group *C4Group::GetMother() { return Mother; }

BOOL C4Group::LoadIcon(const char *szEntryname, HICON *lphIcon) {
  if (!szEntryname || !lphIcon)
    return FALSE;

  *lphIcon = NULL;

  BYTE *bpBuf;
  int iSize;
  if (!LoadEntry(szEntryname, &bpBuf, &iSize))
    return FALSE;

  *lphIcon = CreateIconFromResource(bpBuf, iSize, TRUE, 0x00030000);

  delete[] bpBuf;

  if (*lphIcon)
    return TRUE;
  return FALSE;
}

int C4Group::GetStatus() { return Status; }

BOOL C4Group::CloseExclusiveMother() {
  if (Mother && ExclusiveChild) {
    Mother->Close();
    delete Mother;
    Mother = NULL;
    return TRUE;
  }
  return FALSE;
}

int C4Group::GetCreation() { return Head.Creation; }

BOOL C4Group::SortByList(const char **ppSortList, const char *szFilename) {
  // No sort list specified
  if (!ppSortList)
    return FALSE;
  // No group name specified, use own
  if (!szFilename)
    szFilename = FileName;
  // Find matching filename entry in sort list
  const char **ppListEntry;
  for (ppListEntry = ppSortList; *ppListEntry; ppListEntry += 2)
    if (WildcardMatch(*ppListEntry, szFilename))
      break;
  // Sort by sort list entry
  if (*ppListEntry && *(ppListEntry + 1))
    Sort(*(ppListEntry + 1));
  // Success
  return TRUE;
}

void C4Group::ProcessOut(const char *szMessage, int iProcess) {
  if (fnProcessCallback)
    fnProcessCallback(szMessage, iProcess);
  if (C4Group_ProcessCallback)
    C4Group_ProcessCallback(szMessage, iProcess);
}

BOOL C4Group::EnsureChildFilePtr(C4Group *pChild) {

  // Group files assumed to be always valid (cannot ensure ptr for group childs
  // currently...)
  if (Status == GRPF_File)
    return TRUE;

  // Open standard file is not the child file			...or StdFile
  // ptr does not match pChild->FilePtr
  char szChildPath[_MAX_PATH + 1];
  sprintf(szChildPath, "%s/%s", FileName, GetFilename(pChild->FileName));
  if (!ItemIdentical(StdFile.Name, szChildPath)) {
    // Reopen correct child stdfile
    if (!SetFilePtr2Entry(GetFilename(pChild->FileName)))
      return FALSE;
    // Advance to child's old file ptr
    if (!AdvanceFilePtr(pChild->FilePtr))
      return FALSE;
  }

  // Looks okay
  return TRUE;
}

const char *C4Group::GetFullName() {
  static char szPath[_MAX_PATH + 1];
  szPath[0] = 0;

  for (C4Group *pGroup = this; pGroup; pGroup = pGroup->Mother) {
    if (szPath[0])
      SInsert(szPath, "\\");
    SInsert(szPath, pGroup->FileName);

    if (pGroup->Status == GRPF_Folder)
      break; // Folder is assumed to have full path
  }

  return szPath;
}

void C4Group::MakeOriginal(BOOL fOriginal) {
  Modified = TRUE;
  if (fOriginal) {
    Head.Original = 1234567;
    MadeOriginal = TRUE;
  } else {
    Head.Original = 0;
    MadeOriginal = FALSE;
  }
}

BOOL C4Group::GetOriginal() { return (Head.Original == 1234567); }

BOOL C4Group::Add(const char *szEntryname, C4Group &hSource) {
  BYTE *bpBuf;
  int iSize;
  // Load entry from source group to buffer
  if (!hSource.LoadEntry(szEntryname, &bpBuf, &iSize))
    return FALSE;
  // Add entry (hold buffer)
  if (!Add(szEntryname, bpBuf, iSize, FALSE, TRUE)) {
    delete[] bpBuf;
    return FALSE;
  }
  // Success
  return TRUE;
}
