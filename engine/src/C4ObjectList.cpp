/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic object list */

#include <C4Include.h>

C4ObjectList::C4ObjectList() { Default(); }

C4ObjectList::~C4ObjectList() { Clear(); }

void C4ObjectList::Clear() {
  C4ObjectLink *cLnk, *nextLnk;
  for (cLnk = First; cLnk; cLnk = nextLnk) {
    nextLnk = cLnk->Next;
    delete cLnk;
  }
  First = Last = NULL;
  if (szEnumerated)
    delete[] szEnumerated;
  szEnumerated = NULL;
}

const int MaxTempListID = 500;
C4ID TempListID[MaxTempListID];

C4ID C4ObjectList::GetListID(DWORD dwCategory, int Index) {
  int clid;
  C4ObjectLink *clnk;
  C4Def *cdef;

  // Create a temporary list of all id's and counts
  for (clid = 0; clid < MaxTempListID; clid++)
    TempListID[clid] = C4ID_None;
  for (clnk = First; clnk && clnk->Obj; clnk = clnk->Next)
    if (clnk->Obj->Status)
      if ((dwCategory == C4D_All) || ((cdef = C4Id2Def(clnk->Obj->Def->id)) &&
                                      (cdef->Category & dwCategory)))
        for (clid = 0; clid < MaxTempListID; clid++) {
          // Already there
          if (TempListID[clid] == clnk->Obj->Def->id)
            break;
          // End of list, add id
          if (TempListID[clid] == C4ID_None) {
            TempListID[clid] = clnk->Obj->Def->id;
            break;
          }
        }

  // Returns indexed id
  if (Inside(Index, 0, MaxTempListID - 1))
    return TempListID[Index];

  return C4ID_None;
}

int C4ObjectList::ListIDCount(DWORD dwCategory) {
  int clid;
  C4ObjectLink *clnk;
  C4Def *cdef;

  // Create a temporary list of all id's and counts
  for (clid = 0; clid < MaxTempListID; clid++)
    TempListID[clid] = C4ID_None;
  for (clnk = First; clnk && clnk->Obj; clnk = clnk->Next)
    if (clnk->Obj->Status)
      if ((dwCategory == C4D_All) || ((cdef = C4Id2Def(clnk->Obj->Def->id)) &&
                                      (cdef->Category & dwCategory)))
        for (clid = 0; clid < MaxTempListID; clid++) {
          // Already there
          if (TempListID[clid] == clnk->Obj->Def->id)
            break;
          // End of list, add id
          if (TempListID[clid] == C4ID_None) {
            TempListID[clid] = clnk->Obj->Def->id;
            break;
          }
        }

  // Count different id's
  for (clid = 0; clid < MaxTempListID; clid++)
    if (TempListID[clid] == C4ID_None)
      return clid;

  return MaxTempListID;
}

BOOL C4ObjectList::Add(C4Object *nObj, BOOL fSorted) {
  C4ObjectLink *nLnk, *cLnk = NULL;
  if (!nObj || !nObj->Def || !nObj->Status)
    return FALSE;

  // Allocate new link
  if (!(nLnk = new C4ObjectLink))
    return FALSE;
  // Set link
  nLnk->Obj = nObj;

  // Find successor by matching category / id
  // Sort by matching category/id is necessary for inventory shifting.
  // It is not done for static back to allow multiobject outside structure.
  // Unsorted objects are ignored in comparison.
  if (fSorted)
    if (!cLnk)
      if (!(nObj->Category & C4D_StaticBack))
        for (cLnk = First; cLnk; cLnk = cLnk->Next)
          if (cLnk->Obj->Status && !cLnk->Obj->Unsorted)
            if ((cLnk->Obj->Category & C4D_SortLimit) ==
                (nObj->Category & C4D_SortLimit))
              if (cLnk->Obj->id == nObj->id)
                break;

  // Find successor by relative category
  if (fSorted)
    if (!cLnk)
      for (cLnk = First; cLnk; cLnk = cLnk->Next)
        if (cLnk->Obj->Status && !cLnk->Obj->Unsorted)
          if ((cLnk->Obj->Category & C4D_SortLimit) <=
              (nObj->Category & C4D_SortLimit))
            break;

  // Line object: enforce add to very end
  if (fSorted)
    if (nObj->Def->Line)
      cLnk = NULL;

  // Insert new link before successor
  if (cLnk) {
    nLnk->Next = cLnk;
    nLnk->Prev = cLnk->Prev;
    cLnk->Prev = nLnk;
    if (nLnk->Prev)
      nLnk->Prev->Next = nLnk;
    else
      First = nLnk;
  }
  // Else, add link to tail of list
  else {
    nLnk->Next = NULL;
    nLnk->Prev = Last;
    if (Last)
      Last->Next = nLnk;
    Last = nLnk;
    if (!First)
      First = nLnk;
  }

  // Add mass
  Mass += nObj->Mass;

  return TRUE;
}

BOOL C4ObjectList::Remove(C4Object *pObj) {
  C4ObjectLink *cLnk;

  // Find link
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj == pObj)
      break;
  if (!cLnk)
    return FALSE;

  // Remove link from list
  if (cLnk->Prev)
    cLnk->Prev->Next = cLnk->Next;
  else
    First = cLnk->Next;
  if (cLnk->Next)
    cLnk->Next->Prev = cLnk->Prev;
  else
    Last = cLnk->Prev;

  // Deallocate link
  delete cLnk;

  // Remove mass
  Mass -= pObj->Mass;
  if (Mass < 0)
    Mass = 0;

  return TRUE;
}

C4Object *C4ObjectList::Find(C4ID id, int owner) {
  C4ObjectLink *cLnk;
  // Find link and object
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      if (cLnk->Obj->Def->id == id)
        if ((owner == ANY_OWNER) || (cLnk->Obj->Owner == owner))
          return cLnk->Obj;
  return NULL;
}

C4Object *C4ObjectList::FindOther(C4ID id, int owner) {
  C4ObjectLink *cLnk;
  // Find link and object
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      if (cLnk->Obj->Def->id != id)
        if ((owner == ANY_OWNER) || (cLnk->Obj->Owner == owner))
          return cLnk->Obj;
  return NULL;
}

C4Object *C4ObjectList::GetObject(int Index) {
  int cIdx;
  C4ObjectLink *cLnk;
  // Find link and object
  for (cLnk = First, cIdx = 0; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status) {
      if (cIdx == Index)
        return cLnk->Obj;
      cIdx++;
    }
  return NULL;
}

C4ObjectLink *C4ObjectList::GetLink(C4Object *pObj) {
  if (!pObj)
    return NULL;
  C4ObjectLink *cLnk;
  // Find link
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj == pObj)
      return cLnk;
  return NULL;
}

int C4ObjectList::ObjectCount(C4ID id, DWORD dwCategory) {
  C4ObjectLink *cLnk;
  int iCount = 0;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      if ((id == C4ID_None) || (cLnk->Obj->Def->id == id))
        if ((dwCategory == C4D_All) || (cLnk->Obj->Category & dwCategory))
          iCount++;
  return iCount;
}

int C4ObjectList::MassCount() {
  C4ObjectLink *cLnk;
  int iMass = 0;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      iMass += cLnk->Obj->Mass;
  Mass = iMass;
  return iMass;
}

void C4ObjectList::DrawIDList(C4Facet &cgo, int iSelection, C4DefList &rDefs,
                              DWORD dwCategory, C4RegionList *pRegions,
                              int iRegionCom, BOOL fDrawOneCounts) {
  // Calculations & variables
  int iSections = cgo.GetSectionCount();
  int iItems = ListIDCount(dwCategory);
  int iFirstItem =
      BoundBy(iSelection - iSections / 2, 0, Max(iItems - iSections, 0));
  int cSec = 0;
  int cPos, iCount;
  C4ID c_id;
  C4Facet cgo2;
  C4Object *pFirstObj;
  char szCount[10];
  // Draw by list sorted ids
  for (cPos = 0; c_id = GetListID(dwCategory, cPos); cPos++)
    if (Inside(cPos, iFirstItem, iFirstItem + iSections - 1)) {
      // First object of this type
      pFirstObj = Find(c_id);
      // Count
      iCount = ObjectCount(c_id);
      // Section
      cgo2 = cgo.GetSection(cSec);
      // Draw by definition
      rDefs.Draw(c_id, cgo2, (cPos == iSelection), pFirstObj->Color);
      // Draw count
      sprintf(szCount, "%dx", iCount);
      if ((iCount != 1) || fDrawOneCounts)
        Engine.DDraw.TextOut(szCount, cgo2.Surface, cgo2.X + cgo2.Wdt - 1,
                             cgo2.Y + cgo2.Hgt - 1 - Engine.DDraw.TextHeight(),
                             FWhite, FBlack, ARight);
      // Region
      if (pRegions)
        pRegions->Add(cgo2.X, cgo2.Y, cgo2.Wdt, cgo2.Hgt, pFirstObj->GetName(),
                      iRegionCom, pFirstObj, COM_None, COM_None, pFirstObj->id);
      // Next section
      cSec++;
    }
}

int C4ObjectList::ClearPointers(C4Object *pObj) {
  int rval = 0;
  // Clear all primary list pointers
  while (Remove(pObj))
    rval++;
  // Clear all sub pointers
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    cobj->ClearPointers(pObj);
  return rval;
}

BOOL C4ObjectList::Save(C4Group &hGroup, BOOL fSaveGame) {
  // Save to temp file
  char szFilename[_MAX_PATH + 1];
  SCopy(Config.AtTempPath(C4CFN_ScenarioObjects), szFilename);
  if (!Save(szFilename, fSaveGame))
    return FALSE;
  // Move temp file to group
  hGroup.Move(szFilename); // check?
  // Success
  return TRUE;
}

void C4ObjectList::Draw(C4FacetEx &cgo) {
  C4ObjectLink *clnk;
  // Draw objects (base)
  for (clnk = Last; clnk; clnk = clnk->Prev)
    clnk->Obj->Draw(cgo);
  // Draw objects (top face)
  for (clnk = Last; clnk; clnk = clnk->Prev)
    clnk->Obj->DrawTopFace(cgo);
}

void C4ObjectList::Enumerate() {
  C4ObjectLink *cLnk;
  // Enumerate object pointers
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->EnumeratePointers();
}

int C4ObjectList::ObjectNumber(C4Object *pObj) {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj == pObj)
      return cLnk->Obj->Number;
  return 0;
}

void C4ObjectList::RemoveSolidMasks(BOOL fCauseInstability) {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      if (cLnk->Obj->MaskPut)
        cLnk->Obj->RemoveSolidMask(fCauseInstability);
}

void C4ObjectList::PutSolidMasks() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->PutSolidMask();
}

BOOL C4ObjectList::IsClear() { return (ObjectCount() == 0); }

BOOL C4ObjectList::ReadEnumerated(const char *szSource) {
  szEnumerated = new char[SLen(szSource) + 1];
  SCopy(szSource, szEnumerated);
  return TRUE;
}

BOOL C4ObjectList::DenumerateRead() {
  // Post-compile string of enumerated pointers
  char buf[50];
  if (!szEnumerated)
    return FALSE;
  for (int cseg = 0; SCopySegment(szEnumerated, cseg, buf, ';', 50); cseg++) {
    SClearFrontBack(buf);
    int iNumber = strtol(buf, NULL, 10);
    Add(Game.Objects.ObjectPointer(iNumber), FALSE); // Add to tail, unsorted
  }
  delete[] szEnumerated;
  szEnumerated = NULL;
  return TRUE;
}

BOOL C4ObjectList::Write(char *szTarget) {
  char ostr[25];
  szTarget[0] = 0;
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk && cLnk->Obj; cLnk = cLnk->Next)
    if (cLnk->Obj->Status) {
      sprintf(ostr, "%d;", cLnk->Obj->Number);
      SAppend(ostr, szTarget);
    }
  return TRUE;
}

#define C4CV_Section_Object "[Object]"

int C4ObjectList::Load(C4Group &hGroup) {
  BYTE *pSource;
  int iCount = 0;
  const char *cPos, *cStart;
  char *cEnd;
  C4Object *pObj;
  // Load data component
  if (!hGroup.LoadEntry(C4CFN_ScenarioObjects, &pSource, NULL, 1))
    return iCount;
  // Scan sections
  cPos = (char *)pSource;
  while (cPos = SSearch(cPos, C4CV_Section_Object)) {
    // Object section start
    cStart = cPos - SLen(C4CV_Section_Object);
    // Object section end
    cEnd = (char *)SSearch(cPos, C4CV_Section_Object);
    if (cEnd)
      cEnd -= SLen(C4CV_Section_Object);
    // Terminate end
    if (cEnd)
      *cEnd = 0x00;
    // Compile object section and add object
    pObj = new C4Object;
    if (pObj->Compile(cStart)) {
      // Add unsorted to head
      C4ObjectLink *nLnk = new C4ObjectLink;
      nLnk->Obj = pObj;
      if (!Last)
        Last = nLnk;
      if (First)
        First->Prev = nLnk;
      nLnk->Next = First;
      nLnk->Prev = NULL;
      First = nLnk;
      Mass += pObj->Mass;
      iCount++;
    } else {
      delete pObj;
    }
    // Unterminate end
    if (cEnd)
      *cEnd = '[';
  }
  // Done compiling
  delete[] pSource;
  // Denumerate pointers
  Denumerate();
  // Update faces
  UpdateFaces();
  // Update ocf
  SetOCF();
  // Update transfer zones
  UpdateTransferZones();
  // Done
  return iCount;
}

void C4ObjectList::Denumerate() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->DenumeratePointers();
}

C4Object *C4ObjectList::ObjectPointer(int iNumber) {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Number == iNumber)
      return cLnk->Obj;
  return NULL;
}

BOOL C4ObjectList::WriteNameList(char *szTarget, C4DefList &rDefs,
                                 DWORD dwCategory) {
  int cpos, idcount;
  C4ID c_id;
  C4Def *cdef;
  char ostr[100];
  for (cpos = 0; c_id = GetListID(dwCategory, cpos); cpos++)
    if (cdef = rDefs.ID2Def(c_id)) {
      idcount = ObjectCount(c_id);
      sprintf(ostr, "%dx %s", idcount, cdef->Name);
      if (cpos > 0)
        SAppend(", ", szTarget);
      SAppend(ostr, szTarget);
    }
  return TRUE;
}

BOOL C4ObjectList::ValidateOwners() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->ValidateOwner();
  return TRUE;
}

BOOL C4ObjectList::AssignInfo() {
  C4ObjectLink *cLnk;
  for (cLnk = Last; cLnk; cLnk = cLnk->Prev)
    if (cLnk->Obj->Status)
      cLnk->Obj->AssignInfo();
  return TRUE;
}

void C4ObjectList::ClearInfo(C4ObjectInfo *pInfo) {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->ClearInfo(pInfo);
}

C4Object *C4ObjectList::Enumerated(C4Object *pObj) {
  int iPtrNum;
  // If object is enumerated, convert to enumerated pointer
  if (iPtrNum = ObjectNumber(pObj))
    return (C4Object *)(C4EnumPointer1 + iPtrNum);
  // Failure, return original pointer
  return pObj;
}

C4Object *C4ObjectList::Denumerated(C4Object *pObj) {
  // If valid enumeration, convert to pointer
  if (Inside((long)pObj, C4EnumPointer1, C4EnumPointer2))
    return ObjectPointer((long)pObj - C4EnumPointer1);
  // Invalid, return original pointer
  return pObj;
}

void C4ObjectList::DrawList(C4Facet &cgo, int iSelection, DWORD dwCategory) {
  int iSections = cgo.GetSectionCount();
  int iObjects = ObjectCount(C4ID_None, dwCategory);
  int iFirstVisible =
      BoundBy(iSelection - iSections / 2, 0, Max(iObjects - iSections, 0));
  C4Facet cgo2;
  int iObj = 0, iSec = 0;
  C4ObjectLink *cLnk;
  C4Object *cObj;
  for (cLnk = First; cLnk && (cObj = cLnk->Obj); cLnk = cLnk->Next)
    if (cObj->Status && (cObj->Category && dwCategory)) {
      if (Inside(iObj, iFirstVisible, iFirstVisible + iSections - 1)) {
        cgo2 = cgo.GetSection(iSec++);
        cObj->DrawPicture(cgo2, (iObj == iSelection));
      }
      iObj++;
    }
}

void C4ObjectList::Sort() {
  C4ObjectLink *cLnk;
  BOOL fSorted;
  // Sort by id
  do {
    fSorted = TRUE;
    for (cLnk = First; cLnk && cLnk->Next; cLnk = cLnk->Next)
      if (cLnk->Obj->id > cLnk->Next->Obj->id) {
        RemoveLink(cLnk);
        InsertLink(cLnk, cLnk->Next);
        fSorted = FALSE;
        break;
      }
  } while (!fSorted);
}

void C4ObjectList::RemoveLink(C4ObjectLink *pLnk) {
  if (pLnk->Prev)
    pLnk->Prev->Next = pLnk->Next;
  else
    First = pLnk->Next;
  if (pLnk->Next)
    pLnk->Next->Prev = pLnk->Prev;
  else
    Last = pLnk->Prev;
}

void C4ObjectList::InsertLink(C4ObjectLink *pLnk, C4ObjectLink *pAfter) {
  // Insert after
  if (pAfter) {
    pLnk->Prev = pAfter;
    pLnk->Next = pAfter->Next;
    if (pAfter->Next)
      pAfter->Next->Prev = pLnk;
    else
      Last = pLnk;
    pAfter->Next = pLnk;
  }
  // Insert at head
  else {
    pLnk->Prev = NULL;
    pLnk->Next = First;
    if (First)
      First->Prev = pLnk;
    else
      Last = pLnk;
    First = pLnk;
  }
}

void C4ObjectList::SyncClearance() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj)
      cLnk->Obj->SyncClearance();
}

BOOL C4ObjectList::Save(const char *szFilename, BOOL fSaveGame) {
  // Create file
  CStdFile hTemp;
  if (!hTemp.Create(szFilename))
    return FALSE;

  // Enumerate
  Enumerate();

  // Static max big ol' buffer
  char *Buffer;
  int BufferSize;
  Buffer = new char[50000];
  C4Object *cobj;
  C4ObjectLink *clnk;

  // Decompile objects to buffer, add to file
  for (clnk = Last; clnk && (cobj = clnk->Obj);
       clnk = clnk->Prev) // Back to front
    if (cobj->Status)
      if (!cobj->Info || fSaveGame) // Info-objects in savegame only
      {
        Buffer[0] = 0;
        cobj->Decompile(&Buffer, &BufferSize);
        SAppend(LineFeed, Buffer);
        BufferSize = SLen(Buffer);
        hTemp.Write(Buffer, BufferSize);
      }

  delete[] Buffer;
  hTemp.Write((void *)EndOfFile, 1);

  // Denumerate
  Denumerate();

  // Close file
  hTemp.Close();

  // Success
  return TRUE;
}

void C4ObjectList::UpdateFaces() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->UpdateFace();
}

void C4ObjectList::DrawSelectMark(C4FacetEx &cgo) {
  C4ObjectLink *cLnk;
  for (cLnk = Last; cLnk; cLnk = cLnk->Prev)
    cLnk->Obj->DrawSelectMark(cgo);
}

void C4ObjectList::GetIDList(C4IDList &rList, DWORD dwCategory) {
  C4ObjectLink *clnk;
  C4Def *pDef;
  rList.Clear();
  for (clnk = First; clnk && clnk->Obj; clnk = clnk->Next)
    if (clnk->Obj->Status)
      if ((dwCategory == C4D_All) || ((pDef = C4Id2Def(clnk->Obj->Def->id)) &&
                                      (pDef->Category & dwCategory)))
        rList.IncreaseIDCount(clnk->Obj->Def->id);
}

void C4ObjectList::CloseMenus() {
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    cobj->CloseMenu();
}

void C4ObjectList::SetOCF() {
  C4ObjectLink *cLnk;
  for (cLnk = First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj->Status)
      cLnk->Obj->SetOCF();
}

void C4ObjectList::Copy(C4ObjectList &rList) {
  Clear();
  Default();
  C4ObjectLink *cLnk;
  for (cLnk = rList.First; cLnk; cLnk = cLnk->Next)
    Add(cLnk->Obj);
}

void C4ObjectList::Default() {
  First = Last = NULL;
  Mass = 0;
  szEnumerated = NULL;
}

void C4ObjectList::UpdateTransferZones() {
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    cobj->Call(PSF_UpdateTransferZone);
}

void C4ObjectList::ResetAudibility() {
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    cobj->Audible = 0;
}

void C4ObjectList::SortByCategory() {
  C4ObjectLink *cLnk;
  BOOL fSorted;
  // Sort by category
  do {
    fSorted = TRUE;
    for (cLnk = First; cLnk && cLnk->Next; cLnk = cLnk->Next)
      if ((cLnk->Obj->Category & C4D_SortLimit) <
          (cLnk->Next->Obj->Category & C4D_SortLimit)) {
        RemoveLink(cLnk);
        InsertLink(cLnk, cLnk->Next);
        fSorted = FALSE;
        break;
      }
  } while (!fSorted);
}
