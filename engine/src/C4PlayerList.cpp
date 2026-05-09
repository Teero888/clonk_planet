/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic list to hold runtime player data */

#include "C4Include.h"

C4PlayerList::C4PlayerList() { Default(); }

C4PlayerList::~C4PlayerList() { Clear(); }

void C4PlayerList::Default() { First = NULL; }

void C4PlayerList::Clear() {
  C4Player *pPlr;
  while (pPlr = First) {
    First = pPlr->Next;
    delete pPlr;
  }
  First = NULL;
}

void C4PlayerList::Execute() {
  C4Player *pPlr;
  // Execute
  for (pPlr = First; pPlr; pPlr = pPlr->Next)
    pPlr->Execute();
  // Check retirement
  for (pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Eliminated && !pPlr->RetireDelay) {
      Retire(pPlr);
      break;
    }
}

void C4PlayerList::ClearPointers(C4Object *pObj) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    pPlr->ClearPointers(pObj);
}

BOOL C4PlayerList::Valid(int iPlayer) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Number == iPlayer)
      return TRUE;
  return FALSE;
}

BOOL C4PlayerList::Hostile(int iPlayer1, int iPlayer2) {
  C4Player *pPlr1 = Get(iPlayer1);
  C4Player *pPlr2 = Get(iPlayer2);
  if (!pPlr1 || !pPlr2)
    return FALSE;
  if (pPlr1->Number == pPlr2->Number)
    return FALSE;
  if (pPlr1->Hostility.GetIDCount(pPlr2->Number + 1) || pPlr2->Hostility.GetIDCount(pPlr1->Number + 1))
    return TRUE;
  return FALSE;
}

BOOL C4PlayerList::PositionTaken(int iPosition) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Position == iPosition)
      return TRUE;
  return FALSE;
}

BOOL C4PlayerList::ColorTaken(int iColor) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Color == iColor)
      return TRUE;
  return FALSE;
}

BOOL C4PlayerList::ControlTaken(int iControl) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Control == iControl)
      if (pPlr->LocalControl)
        return TRUE;
  return FALSE;
}

C4Player *C4PlayerList::Get(int iNumber) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Number == iNumber)
      return pPlr;
  return NULL;
}

int C4PlayerList::GetIndex(C4Player *tPlr) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next, cindex++)
    if (pPlr == tPlr)
      return cindex;
  return -1;
}

C4Player *C4PlayerList::GetByIndex(int iIndex) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next, cindex++)
    if (cindex == iIndex)
      return pPlr;
  return NULL;
}

C4Player *C4PlayerList::GetLocalByKbdSet(int iKbdSet) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->LocalControl)
      if (pPlr->Control == iKbdSet)
        return pPlr;
  return NULL;
}

int C4PlayerList::GetCount() {
  int iCount = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    iCount++;
  return iCount;
}

int C4PlayerList::GetFreeNumber() {
  int iNumber = -1;
  BOOL fFree;
  do {
    iNumber++;
    fFree = TRUE;
    for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
      if (pPlr->Number == iNumber)
        fFree = FALSE;
  } while (!fFree);
  return iNumber;
}

BOOL C4PlayerList::Remove(int iPlayer) { return Remove(Get(iPlayer)); }

BOOL C4PlayerList::Remove(C4Player *pPlr) {
  if (!pPlr)
    return FALSE;

  // Remove eliminated crew
  pPlr->RemoveCrewObjects();

  // Clear object info pointers
  pPlr->CrewInfoList.DetachFromObjects();

  // Clear viewports
  Game.GraphicsSystem.CloseViewport(pPlr->Number);
  // Check fullscreen viewports
  FullScreen.ViewportCheck();

  // Remove player
  C4Player *pPrev;
  for (pPrev = First; pPrev; pPrev = pPrev->Next)
    if (pPrev->Next == pPlr)
      break;
  if (pPrev)
    pPrev->Next = pPlr->Next;
  else
    First = pPlr->Next;
  delete pPlr;

  // Validate object owners
  Game.Objects.ValidateOwners();

  // Update console
  Console.UpdateMenus();

  return TRUE;
}

C4Player *C4PlayerList::Join(const char *szFilename, BOOL fScenarioInit, int iAtClient, const char *szAtClientName) {

  // Log
  sprintf(OSTR, LoadResStr(IDS_PRC_JOINPLR), szFilename);
  Log(OSTR);

  // Too many players in fullscreen
  if (Application.Fullscreen)
    if (GetCount() + 1 > Game.C4S.Head.MaxPlayer) {
      sprintf(OSTR, LoadResStr(IDS_PRC_TOOMANYPLRS), Game.C4S.Head.MaxPlayer);
      Log(OSTR);
      return NULL;
    }

  // Check duplicate file usage
  if (FileInUse(szFilename)) {
    Log(LoadResStr(IDS_PRC_PLRFILEINUSE));
    return NULL;
  }

  // Create
  C4Player *pPlr = new C4Player;

  // Append to player list
  C4Player *pLast;
  for (pLast = First; pLast && pLast->Next; pLast = pLast->Next)
    ;
  if (pLast)
    pLast->Next = pPlr;
  else
    First = pPlr;

  // Init
  if (!pPlr->Init(GetFreeNumber(), iAtClient, szAtClientName, szFilename, fScenarioInit)) {
    Remove(pPlr);
    Log(LoadResStr(IDS_PRC_JOINFAIL));
    return NULL;
  }

  // Done
  return pPlr;
}

void SetClientPrefix(char *szFilename, const char *szClient) {
  char szTemp[1024 + 1];
  // Compose prefix
  char szPrefix[1024 + 1];
  SCopy(szClient, szPrefix);
  SAppendChar('-', szPrefix);
  // Prefix already set?
  SCopy(GetFilename(szFilename), szTemp, SLen(szPrefix));
  if (SEqualNoCase(szTemp, szPrefix))
    return;
  // Insert prefix
  SCopy(GetFilename(szFilename), szTemp);
  SCopy(szPrefix, GetFilename(szFilename));
  SAppend(szTemp, szFilename);
}

BOOL C4PlayerList::Save(C4Group &hGroup, BOOL fNetwork) {
  char szAddFilename[_MAX_PATH + 1];
  // Save to external player files and add to group
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next) {
    // Save to original file
    if (!pPlr->Save()) {
      return FALSE;
    }
    // Set add filename
    SCopy(GetFilename(pPlr->Filename), szAddFilename);
    // Network: client prefix to player filename
    if (fNetwork)
      SetClientPrefix(szAddFilename, pPlr->AtClientName);
    // Add to group
    if (!hGroup.Add(pPlr->Filename, szAddFilename)) {
      return FALSE;
    }
  }
  return TRUE;
}
BOOL C4PlayerList::Save() {
  // Save to external player files
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (!pPlr->Save()) {
      return FALSE;
    }
  return TRUE;
}

BOOL C4PlayerList::Evaluate() {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    pPlr->Evaluate();
  return TRUE;
}

BOOL C4PlayerList::Retire(C4Player *pPlr) {
  if (!pPlr)
    return FALSE;

  pPlr->Evaluate();
  pPlr->Save();
  Remove(pPlr);

  return TRUE;
}

int C4PlayerList::AverageValueGain() {
  int iResult = 0;
  if (First) {
    for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
      iResult += Max(pPlr->ValueGain, 0);
    iResult /= GetCount();
  }
  return iResult;
}

C4Player *C4PlayerList::GetByName(const char *szName, int iExcluding) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (SEqual(pPlr->Name, szName))
      if (pPlr->Number != iExcluding)
        return pPlr;
  return NULL;
}

BOOL C4PlayerList::FileInUse(const char *szFilename) {
  // Check original player files
  C4Player *cPlr;
  for (cPlr = First; cPlr; cPlr = cPlr->Next)
    if (ItemIdentical(cPlr->Filename, szFilename))
      return TRUE;
  // Compare to any network path player files with prefix (hack)
  if (Game.Network.Active) {
    char szWithPrefix[_MAX_PATH + 1];
    SCopy(GetFilename(szFilename), szWithPrefix);
    SetClientPrefix(szWithPrefix, Game.Network.LocalName);
    for (cPlr = First; cPlr; cPlr = cPlr->Next)
      if (SEqualNoCase(GetFilename(cPlr->Filename), szWithPrefix))
        return TRUE;
  }
  // Not in use
  return FALSE;
}

C4Player *C4PlayerList::GetLocalByIndex(int iIndex) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->LocalControl) {
      if (cindex == iIndex)
        return pPlr;
      cindex++;
    }
  return NULL;
}

BOOL C4PlayerList::RemoveAtClient(int iClient) {
  C4Player *pPlr;
  // Get players
  while (pPlr = GetAtClient(iClient)) {
    // Log
    sprintf(OSTR, LoadResStr(IDS_PRC_REMOVEPLR), pPlr->Name);
    Log(OSTR);
    // Remove
    Remove(pPlr);
  }
  return TRUE;
}

BOOL C4PlayerList::RemoveAtClient(const char *szName) {
  C4Player *pPlr;
  // Get players
  while (pPlr = GetAtClient(szName)) {
    // Log
    sprintf(OSTR, LoadResStr(IDS_PRC_REMOVEPLR), pPlr->Name);
    Log(OSTR);
    // Remove
    Remove(pPlr);
  }
  return TRUE;
}

C4Player *C4PlayerList::GetAtClient(int iClient, int iIndex) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->AtClient == iClient) {
      if (cindex == iIndex)
        return pPlr;
      cindex++;
    }
  return NULL;
}

C4Player *C4PlayerList::GetAtClient(const char *szName, int iIndex) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (SEqualNoCase(pPlr->AtClientName, szName)) {
      if (cindex == iIndex)
        return pPlr;
      cindex++;
    }
  return NULL;
}

BOOL C4PlayerList::RemoveAtRemoteClient() {
  C4Player *pPlr;
  // Get players
  while (pPlr = GetAtRemoteClient()) {
    // Log
    sprintf(OSTR, LoadResStr(IDS_PRC_REMOVEPLR), pPlr->Name);
    Log(OSTR);
    // Remove
    Remove(pPlr);
  }
  return TRUE;
}

C4Player *C4PlayerList::GetAtRemoteClient(int iIndex) {
  int cindex = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->AtClient != Game.Network.GetClientNumber()) {
      if (cindex == iIndex)
        return pPlr;
      cindex++;
    }
  return NULL;
}

void C4PlayerList::EnumeratePointers() {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    pPlr->EnumeratePointers();
}

void C4PlayerList::DenumeratePointers() {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    pPlr->DenumeratePointers();
}

int C4PlayerList::ControlTakenBy(int iControl) {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->Control == iControl)
      if (pPlr->LocalControl)
        return pPlr->Number;
  return NO_OWNER;
}

BOOL C4PlayerList::MouseControlTaken() {
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (pPlr->MouseControl)
      if (pPlr->LocalControl)
        return TRUE;
  return FALSE;
}

int C4PlayerList::GetCountNotEliminated() {
  int iCount = 0;
  for (C4Player *pPlr = First; pPlr; pPlr = pPlr->Next)
    if (!pPlr->Eliminated)
      iCount++;
  return iCount;
}
