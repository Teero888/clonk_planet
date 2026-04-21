/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Auto-registering data structures */

const int CFG_MaxString	= 1024;

const int CFG_String		= 1,
					CFG_Integer		= 2,

					CFG_Company		= 10,
					CFG_Product		= 11,
					CFG_Section		= 12,
					CFG_End				= 0;

#define offsetof(s,m)	(size_t)&(((s *)0)->m)

class CStdConfigValue 
	{ 
	public:
    int Type;
		char *Name;
    int Offset;
    int Default;
  };

class CStdConfig  
	{
	public:
		CStdConfig();
		~CStdConfig();
	protected:
		void LoadDefault(CStdConfigValue *pCfgMap, void *vpData, const char *szOnlySection=NULL);
		BOOL Save(CStdConfigValue *pCfgMap, void *vpData);
		BOOL Load(CStdConfigValue *pCfgMap, void *vpData);
	};
