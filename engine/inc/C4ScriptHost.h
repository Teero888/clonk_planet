/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles script file components (calls, inheritance, function maps) */

const int C4MaxGlobal = C4MaxVariable;

const int C4SCR_MaxIDLen = 100,
					C4SCR_MaxDesc	 = 256;

class C4ScriptFnRef
	{
	public:
		char Name[C4SCR_MaxIDLen+1];
		char Desc[C4SCR_MaxDesc+1];
		char Condition[C4SCR_MaxIDLen+1];
		const char *Code;
		int Access;
		C4ID idImage;
		C4ScriptFnRef *Next;
	};

class C4ScriptHost: public C4ComponentHost  
	{
	public:
		C4ScriptHost();
		~C4ScriptHost();
	protected:
		C4ScriptFnRef *FunctionTable;
	public:
		int Counter;
		BOOL Go;
		int Global[C4MaxGlobal];
		C4ID idDef;
		char *Script;
	public:
		void Default();
		void Clear();
		void Close();
		void ClearPointers(C4Object *pObj);
		void MakeFunctionTable();
		BOOL Load(const char *szName, C4Group &hGroup, const char *szFilename, 
							const char *szLanguage=NULL, C4ID idDefinition=C4ID_None);
		BOOL DenumerateVariablePointers();
		BOOL EnumerateVariablePointers();
		BOOL Execute();
		BOOL GetFunctionDeclaration(int iIndex, char *sTarget, char *sQualifier=NULL, 
																const char **ppCode=NULL, char *sDesc=NULL, 
																BOOL fRegular=TRUE, C4ID *pidImage=NULL,
																char *sCondition=NULL);
		const char *GetControlDesc(const char *szFunctionFormat, int iCom, C4ID *pidImage=NULL);
		int GetControlFlag(const char *szFunctionFormat);
		int ResolveIncludes(C4DefList &rDefs);
		int ObjectCall(C4Thread *pCaller, C4Object *pObj, const char *szFunction,
             int par0=0, int par1=0, int par2=0, int par3=0, int par4=0,
						 int par5=0, int par6=0, int par7=0, int par8=0, int par9=0);
		int Call(C4Thread *pCaller, const char *szFunction,
             int par0=0, int par1=0, int par2=0, int par3=0, int par4=0,
						 int par5=0, int par6=0, int par7=0, int par8=0, int par9=0);
		C4ScriptFnRef* GetFunctionRef(const char *szFunction);
		C4ScriptFnRef* GetFunctionRef(int iFunction);
	protected:
		void ClearFunctionTable();
		void AddFunctionTable(C4ScriptFnRef *pFn);
		void SetError(const char *szMessage);
		void MakeScript();
		int FunctionCall(C4Thread *pCaller, const char *szFunction, C4Object *pObj, int iPar0, int iPar1, int iPar2, int iPar3, int iPar4, int iPar5, int iPar6, int iPar7, int iPar8, int iPar9);
		int QualifierAccess(const char *szQualifier);
		BOOL ScanFunctionDesc(const char *szDesc, char *sDesc, C4ID *pidImage, char *sCondition);
	};

