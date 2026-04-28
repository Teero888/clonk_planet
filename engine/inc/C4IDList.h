/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* At static list of C4IDs */

const int C4IDListSize = 150;

#pragma pack(push, 1)

class C4IDList {
public:
  C4IDList();
  ~C4IDList();

protected:
  int Size;
  C4ID id[C4IDListSize];
  int Count[C4IDListSize];

public:
  // General
  void Default();
  void Clear();
  BOOL IsClear();
  // Access by direct index
  C4ID GetID(int index, int *ipCount = NULL);
  int GetCount(int index);
  BOOL SetCount(int index, int iCount);
  // Access by ID
  int GetIDCount(C4ID c_id, int iZeroDefVal = 0);
  BOOL SetIDCount(C4ID c_id, int iCount, BOOL fAddNewID = FALSE);
  BOOL IncreaseIDCount(C4ID c_id, BOOL fAddNewID = TRUE);
  BOOL DecreaseIDCount(C4ID c_id, BOOL fRemoveEmptyID = TRUE);
  int GetNumberOfIDs();
  // Access by category-sorted index
  C4ID GetID(C4DefList &rDefs, DWORD dwCategory, int index, int *ipCount = NULL);
  int GetCount(C4DefList &rDefs, DWORD dwCategory, int index);
  BOOL SetCount(C4DefList &rDefs, DWORD dwCategory, int index, int iCount);
  int GetNumberOfIDs(C4DefList &rDefs, DWORD dwCategory);
  // IDList merge
  BOOL Add(C4IDList &rList);
  // Aux
  BOOL Consolidate();
  BOOL ConsolidateValids(C4DefList &rDefs);
  BOOL ConsolidateCounts();
  void SortByCategory(C4DefList &rDefs);
  void SortByValue(C4DefList &rDefs);
  int GetValue(C4DefList &rDefs);
  void Load(C4DefList &rDefs, DWORD dwCategory);
  // Item operation
  BOOL DeleteItem(int iIndex);
  BOOL SwapItems(int iIndex1, int iIndex2);
  // Compilation
  BOOL Read(const char *szSource, int iDefValue = 0);
  BOOL Write(char *szTarget, BOOL fValues = TRUE);
  // Graphics
  void Draw(C4Facet &cgo, int iSelection, C4DefList &rDefs, DWORD dwCategory, BOOL fCounts = TRUE, int iAlign = 0);

public:
  bool operator==(const C4IDList &rhs) { return MemEqual((BYTE *)this, (BYTE *)&rhs, sizeof(C4IDList)); }
};

#pragma pack(pop)
