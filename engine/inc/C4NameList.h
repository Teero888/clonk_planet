/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A static list of strings and integer values, i.e. for material amounts */

#pragma pack(push, 1)

class C4NameList
  {
  public:
    C4NameList();
  public:
    char Name[C4MaxNameList][C4MaxName+1];
    int  Count[C4MaxNameList];
  public:
    void Clear();
		BOOL Add(const char *szName, int iCount=0);
		BOOL Set(const char *szName, int iCount);
		BOOL Read(const char *szSource, int iDefValue=0);
		BOOL Write(char *szTarget, BOOL fValues=TRUE);
	public:
		BOOL IsEmpty();
		bool operator==(const C4NameList& rhs)
			{	return MemEqual((BYTE*)this,(BYTE*)&rhs,sizeof(C4NameList)); }
  };

#pragma pack(pop)
