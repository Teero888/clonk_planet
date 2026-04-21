/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic list for crew member info */

class C4ObjectInfoList  
	{
	public:
		C4ObjectInfoList();
		~C4ObjectInfoList();
	protected:
		C4ObjectInfo *First;
	public:
		void Default();
		void Clear();
		void Evaluate();
		void DetachFromObjects();
		int Load(C4Group &hGroup);
		BOOL Add(C4ObjectInfo *pInfo);
		BOOL Save(C4Group &hGroup);
		C4ObjectInfo* New(C4ID n_id, C4DefList *pDefs, const char *cpNames);
		C4ObjectInfo* GetIdle(C4ID c_id, C4DefList &rDefs);
		C4ObjectInfo* GetIdle(const char *szByName);
	protected:
		void MakeValidName(char *sName);
		BOOL NameExists(const char *szName);
		C4ObjectInfo *GetLast();
		C4ObjectInfo *GetPrevious(C4ObjectInfo *pInfo);
	};
	