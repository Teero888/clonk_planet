/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* At static list of C4IDs */

#include <C4Include.h>

C4IDList::C4IDList() {
  Size = C4IDListSize;
  Default();
}

C4IDList::~C4IDList() {}

void C4IDList::Clear() {
  for (int cnt = 0; cnt < Size; cnt++) {
    id[cnt] = C4ID_None;
    Count[cnt] = 0;
  }
}

BOOL C4IDList::IsClear() {
  for (int cnt = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None)
      return FALSE;
  return TRUE;
}

C4ID C4IDList::GetID(int index, int *ipCount) {
  if (!Inside(index, 0, Size - 1))
    return C4ID_None;
  if (ipCount)
    *ipCount = Count[index];
  return id[index];
}

int C4IDList::GetCount(int index) {
  if (!Inside(index, 0, Size - 1))
    return 0;
  return Count[index];
}

BOOL C4IDList::SetCount(int index, int iCount) {
  if (!Inside(index, 0, Size - 1))
    return FALSE;
  Count[index] = iCount;
  return TRUE;
}

int C4IDList::GetIDCount(C4ID c_id, int iZeroDefVal) {
  int cnt;
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] == c_id) {
      if (Count[cnt] == 0)
        return iZeroDefVal;
      return Count[cnt];
    }
  return 0;
}

BOOL C4IDList::SetIDCount(C4ID c_id, int iCount, BOOL fAddNewID) {
  int cnt;
  // ID exists
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] == c_id) {
      Count[cnt] = iCount;
      return TRUE;
    }
  // Add new id
  if (fAddNewID)
    for (cnt = 0; cnt < Size; cnt++)
      if (id[cnt] == C4ID_None) {
        id[cnt] = c_id;
        Count[cnt] = iCount;
        return TRUE;
      }
  return FALSE;
}

int C4IDList::GetNumberOfIDs() {
  int cnt, iTotal;
  for (cnt = 0, iTotal = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None)
      iTotal++;
  return iTotal;
}

BOOL C4IDList::IncreaseIDCount(C4ID c_id, BOOL fAddNewID) {
  int cnt;
  // Check for existing count to increase
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] == c_id) {
      Count[cnt]++;
      return TRUE;
    }
  // ID not in list and no add desired
  if (!fAddNewID)
    return TRUE;
  // Add new id
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] == C4ID_None) {
      id[cnt] = c_id;
      Count[cnt] = 1;
      return TRUE;
    }
  // ID not in list and no add possible
  return FALSE;
}

BOOL C4IDList::DecreaseIDCount(C4ID c_id, BOOL fRemoveEmptyID) {
  int cnt;
  BOOL fOkay;
  // Find id to decrease
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] == c_id) {
      fOkay = FALSE;
      if (Count[cnt] > 0) {
        Count[cnt]--;
        fOkay = TRUE;
      }
      if (Count[cnt] == 0)
        if (fRemoveEmptyID) {
          id[cnt] = C4ID_None;
          Consolidate();
        }
      return fOkay;
    }
  // ID not in list
  return FALSE;
}

// Access by category-sorted index

C4ID C4IDList::GetID(C4DefList &rDefs, DWORD dwCategory, int index, int *ipCount) {
  int cnt, cindex = -1;
  C4Def *cDef;
  if (ipCount)
    *ipCount = 0;
  for (cnt = 0; cnt < Size; cnt++)
    if ((dwCategory == C4D_All) || ((cDef = rDefs.ID2Def(id[cnt])) && (cDef->Category & dwCategory))) {
      cindex++;
      if (cindex == index) {
        if (ipCount)
          *ipCount = Count[cnt];
        return id[cnt];
      }
    }
  return C4ID_None;
}

int C4IDList::GetCount(C4DefList &rDefs, DWORD dwCategory, int index) {
  int cnt, cindex = -1;
  C4Def *cDef;
  for (cnt = 0; cnt < Size; cnt++)
    if ((dwCategory == C4D_All) || ((cDef = rDefs.ID2Def(id[cnt])) && (cDef->Category & dwCategory))) {
      cindex++;
      if (cindex == index)
        return Count[cnt];
    }
  return 0;
}

BOOL C4IDList::SetCount(C4DefList &rDefs, DWORD dwCategory, int index, int iCount) {
  int cnt, cindex = -1;
  C4Def *cDef;
  for (cnt = 0; cnt < Size; cnt++)
    if ((dwCategory == C4D_All) || ((cDef = rDefs.ID2Def(id[cnt])) && (cDef->Category & dwCategory))) {
      cindex++;
      if (cindex == index) {
        Count[cnt] = iCount;
        return TRUE;
      }
    }
  return FALSE;
}

int C4IDList::GetNumberOfIDs(C4DefList &rDefs, DWORD dwCategory) {
  int cnt, idnum = 0;
  C4Def *cDef;
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None)
      if ((dwCategory == C4D_All) || ((cDef = rDefs.ID2Def(id[cnt])) && (cDef->Category & dwCategory)))
        idnum++;
  return idnum;
}

// IDList merge

BOOL C4IDList::Add(C4IDList &rList) {
  for (int cnt = 0; cnt < Size; cnt++)
    if (rList.id[cnt] != C4ID_None)
      for (int cnt2 = 0; cnt2 < rList.Count[cnt]; cnt2++)
        if (!IncreaseIDCount(rList.id[cnt]))
          return FALSE;
  return TRUE;
}

// Removes all empty id gaps from the list.

BOOL C4IDList::Consolidate() {
  int cnt, cnt2;
  BOOL fCons = FALSE;
  for (cnt = 0; cnt < Size - 1; cnt++)
    if (id[cnt] == C4ID_None)                   // This one is empty
      for (cnt2 = cnt + 1; cnt2 < Size; cnt2++) // Search rest of list
        if (id[cnt2] != C4ID_None)              // id found: consolidate
        {
          id[cnt] = id[cnt2];
          Count[cnt] = Count[cnt2];
          id[cnt2] = C4ID_None;
          Count[cnt2] = 0;
          fCons = TRUE;
          break;
        }
  return fCons;
}

BOOL C4IDList::ConsolidateValids(C4DefList &rDefs) {
  int cnt;
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None)
      if (!rDefs.ID2Def(id[cnt]))
        id[cnt] = C4ID_None;
  return Consolidate();
}

void C4IDList::SortByCategory(C4DefList &rDefs) {
  BOOL fBubble;
  int cnt;
  C4Def *cdef1, *cdef2;
  C4ID t_id;
  int t_count;
  do {
    fBubble = FALSE;
    for (cnt = 0; cnt + 1 < Size; cnt++)
      if ((id[cnt] != C4ID_None) && (id[cnt + 1] != C4ID_None))
        if ((cdef1 = rDefs.ID2Def(id[cnt])) && (cdef2 = rDefs.ID2Def(id[cnt + 1])))
          if ((cdef1->Category & C4D_SortLimit) < (cdef2->Category & C4D_SortLimit)) {
            t_id = id[cnt + 1];
            t_count = Count[cnt + 1];
            id[cnt + 1] = id[cnt];
            Count[cnt + 1] = Count[cnt];
            id[cnt] = t_id;
            Count[cnt] = t_count;
            fBubble = TRUE;
          }
  } while (fBubble);
}

void C4IDList::SortByValue(C4DefList &rDefs) {
  BOOL fBubble;
  int cnt;
  C4Def *cdef1, *cdef2;
  C4ID t_id;
  int t_count;
  do {
    fBubble = FALSE;
    for (cnt = 0; cnt + 1 < Size; cnt++)
      if ((id[cnt] != C4ID_None) && (id[cnt + 1] != C4ID_None))
        if ((cdef1 = rDefs.ID2Def(id[cnt])) && (cdef2 = rDefs.ID2Def(id[cnt + 1])))
          if (cdef1->Value > cdef2->Value) {
            t_id = id[cnt + 1];
            t_count = Count[cnt + 1];
            id[cnt + 1] = id[cnt];
            Count[cnt + 1] = Count[cnt];
            id[cnt] = t_id;
            Count[cnt] = t_count;
            fBubble = TRUE;
          }
  } while (fBubble);
}

int C4IDList::GetValue(C4DefList &rDefs) {
  int cnt, value = 0;
  C4Def *cdef;
  for (cnt = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None)
      if (cdef = rDefs.ID2Def(id[cnt]))
        value += cdef->Value * Count[cnt];
  return value;
}

void C4IDList::Load(C4DefList &rDefs, DWORD dwCategory) {
  C4Def *cdef;
  Clear();
  for (int cnt = 0; (cnt < Size) && (cdef = rDefs.GetDef(cnt, dwCategory)); cnt++)
    id[cnt] = cdef->id;
}

BOOL C4IDList::Write(char *szTarget, int fValues) {
  char buf[50], buf2[5];
  if (!szTarget)
    return FALSE;
  szTarget[0] = 0;
  for (int cnt = 0; cnt < Size; cnt++)
    if (id[cnt] != C4ID_None) {
      GetC4IdText(id[cnt], buf2);
      if (fValues)
        sprintf(buf, "%s=%d;", buf2, Count[cnt]);
      else
        sprintf(buf, "%s;", buf2);
      SAppend(buf, szTarget);
    }
  return TRUE;
}

BOOL C4IDList::Read(const char *szSource, int iDefValue) {
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

    if (SLen(buf) == 4)
      if (!SetIDCount(C4Id(buf), value, TRUE))
        return FALSE;
  }
  return TRUE;
}

void C4IDList::Draw(C4Facet &cgo, int iSelection, C4DefList &rDefs, DWORD dwCategory, BOOL fCounts, int iAlign) {
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - - - - - - -

  int sections = cgo.GetSectionCount();
  int idnum = GetNumberOfIDs(rDefs, dwCategory);
  int firstid = BoundBy(iSelection - sections / 2, 0, Max(idnum - sections, 0));
  int idcount;
  C4ID c_id;
  C4Facet cgo2;
  char buf[10];
  for (int cnt = 0; (cnt < sections) && (c_id = GetID(rDefs, dwCategory, firstid + cnt, &idcount)); cnt++) {
    cgo2 = cgo.TruncateSection(iAlign);
    rDefs.Draw(c_id, cgo2, (firstid + cnt == iSelection), 0);
    sprintf(buf, "%dx", idcount);
    if (fCounts)
      Engine.DDraw.TextOut(buf, cgo2.Surface, cgo2.X + cgo2.Wdt - 1, cgo2.Y + cgo2.Hgt - 1 - Engine.DDraw.TextHeight(), FWhite, FBlack, ARight);
  }

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - -
}

void C4IDList::Default() {
  for (int cnt = 0; cnt < Size; cnt++) {
    id[cnt] = C4ID_None;
    Count[cnt] = 0;
  }
}

BOOL C4IDList::ConsolidateCounts() {
  int cnt;
  for (cnt = 0; cnt < Size; cnt++)
    if (!Count[cnt])
      id[cnt] = C4ID_None;
  return Consolidate();
}

BOOL C4IDList::SwapItems(int iIndex1, int iIndex2) {
  // Invalid index
  if (!Inside(iIndex1, 0, Size - 1))
    return FALSE;
  if (!Inside(iIndex2, 0, Size - 1))
    return FALSE;
  // Swap id & count
  C4ID idTemp = id[iIndex2];
  int iTemp = Count[iIndex2];
  id[iIndex2] = id[iIndex1];
  Count[iIndex2] = Count[iIndex1];
  id[iIndex1] = idTemp;
  Count[iIndex1] = iTemp;
  // Done
  return TRUE;
}

// Clear index entry and shift all entries above down by one.

BOOL C4IDList::DeleteItem(int iIndex) {
  // Invalid index
  if (!Inside(iIndex, 0, Size - 1))
    return FALSE;
  // Clear entry id & count
  id[iIndex] = C4ID_None;
  Count[iIndex] = 0;
  // Shift down all entries above
  for (; iIndex + 1 < Size; iIndex++) {
    id[iIndex] = id[iIndex + 1];
    Count[iIndex] = Count[iIndex + 1];
  }
  // Done
  return TRUE;
}
