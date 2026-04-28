/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic object list */

class C4ObjectLink
  {
  public:
    C4Object *Obj;
    C4ObjectLink *Prev,*Next;
  };

class C4ObjectList
  {
  public:
    C4ObjectList();
    ~C4ObjectList();
  public: 
    C4ObjectLink *First,*Last;
    int Mass;
		char *szEnumerated;
  public:
	  void SortByCategory();
	  void Default();
    void Clear();
	  void Sort();
	  void Enumerate();
	  void Denumerate();
	  void Copy(C4ObjectList &rList);
	  void Draw(C4FacetEx &cgo);
	  void DrawList(C4Facet &cgo, int iSelection=-1, DWORD dwCategory=C4D_All);
		void DrawIDList(C4Facet &cgo, int iSelection, C4DefList &rDefs, DWORD dwCategory, C4RegionList *pRegions=NULL, int iRegionCom=COM_None, BOOL fDrawOneCounts=TRUE);
	  void DrawSelectMark(C4FacetEx &cgo);
	  void CloseMenus();
	  void UpdateFaces();
	  void SyncClearance();
	  void ResetAudibility();
	  void UpdateTransferZones();
	  void SetOCF();
	  void GetIDList(C4IDList &rList, DWORD dwCategory=C4D_All);
	  void ClearInfo(C4ObjectInfo *pInfo);
	  void PutSolidMasks();
	  void RemoveSolidMasks(BOOL fCauseInstability=TRUE);
	  
    BOOL Add(C4Object *nObj, BOOL fSorted=TRUE);
    BOOL Remove(C4Object *pObj);
		BOOL Save(const char *szFilename, BOOL fSaveGame);
	  BOOL Save(C4Group &hGroup, BOOL fSaveGame);
	  BOOL AssignInfo();
	  BOOL ValidateOwners();
	  BOOL WriteNameList(char *szTarget, C4DefList &rDefs, DWORD dwCategory=C4D_All);
		BOOL IsClear();
		BOOL ReadEnumerated(const char *szSource);
		BOOL DenumerateRead();
		BOOL Write(char *szTarget);
	  
		int Load(C4Group &hGroup);
	  int ObjectNumber(C4Object *pObj);
	  int ClearPointers(C4Object *pObj);
    int ObjectCount(C4ID id=C4ID_None, DWORD dwCategory=C4D_All);
    int MassCount();
    int ListIDCount(DWORD dwCategory);

		C4Object* Denumerated(C4Object *pObj);
	  C4Object* Enumerated(C4Object *pObj);
	  C4Object* ObjectPointer(int iNumber);
    C4Object* GetObject(int Index=0);
    C4Object* Find(C4ID id, int iOwner=ANY_OWNER);
    C4Object* FindOther(C4ID id, int iOwner=ANY_OWNER);
	  
    C4ObjectLink* GetLink(C4Object *pObj);

    C4ID GetListID(DWORD dwCategory, int Index);

  protected:
	  void InsertLink(C4ObjectLink *pLink, C4ObjectLink *pAfter);
	  void RemoveLink(C4ObjectLink *pLnk);
  };
