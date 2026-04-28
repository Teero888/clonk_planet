/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A static list of strings and integer values, i.e. for material amounts */

#include <C4Include.h>

C4NameList::C4NameList() { Clear(); }

void C4NameList::Clear() { ZeroMem(this, sizeof(C4NameList)); }

BOOL C4NameList::Set(const char *szName, int iCount) {
  int cnt;
  // Find existing name, set count
  for (cnt = 0; cnt < C4MaxNameList; cnt++)
    if (SEqual(Name[cnt], szName)) {
      Count[cnt] = iCount;
      return TRUE;
    }
  // Find empty spot, set name and count
  for (cnt = 0; cnt < C4MaxNameList; cnt++)
    if (Name[cnt][0] == 0) {
      SCopy(szName, Name[cnt], C4MaxName);
      Count[cnt] = iCount;
      return TRUE;
    }
  // No empty spots
  return FALSE;
}

BOOL C4NameList::Read(const char *szSource, int iDefValue) {
  char buf[50];
  if (!szSource)
    return FALSE;
  Clear();
  for (int cseg = 0; SCopySegment(szSource, cseg, buf, ';', 50); cseg++) {
    SClearFrontBack(buf);
    int value = iDefValue;
    if (SCharCount('=', buf)) {
      value = strtol(buf + SCharPos('=', buf) + 1, NULL, 10);
      buf[SCharPos('=', buf)] = 0;
      SClearFrontBack(buf);
    }
    if (!Set(buf, value))
      return FALSE;
  }
  return TRUE;
}

BOOL C4NameList::Write(char *szTarget, BOOL fValues) {
  char buf[50];
  if (!szTarget)
    return FALSE;
  szTarget[0] = 0;
  for (int cnt = 0; cnt < C4MaxNameList; cnt++)
    if (Name[cnt][0]) {
      if (fValues)
        sprintf(buf, "%s=%d; ", Name[cnt], Count[cnt]);
      else
        sprintf(buf, "%s; ", Name[cnt]);
      SAppend(buf, szTarget);
    }
  return TRUE;
}

BOOL C4NameList::Add(const char *szName, int iCount) {
  // Find empty spot, set name and count
  for (int cnt = 0; cnt < C4MaxNameList; cnt++)
    if (!Name[cnt][0]) {
      SCopy(szName, Name[cnt], C4MaxName);
      Count[cnt] = iCount;
      return TRUE;
    }
  // No empty spots
  return FALSE;
}

BOOL C4NameList::IsEmpty() {
  for (int cnt = 0; cnt < C4MaxNameList; cnt++)
    if (Name[cnt][0])
      return FALSE;
  return TRUE;
}
