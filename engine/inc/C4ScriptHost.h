/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles script file components (calls, inheritance, function maps) */

const int C4MaxGlobal = C4MaxVariable;

const int C4SCR_MaxIDLen = 100, C4SCR_MaxDesc = 256;

class C4ScriptFnRef {
public:
  char Name[C4SCR_MaxIDLen + 1];
  char Desc[C4SCR_MaxDesc + 1];
  char Condition[C4SCR_MaxIDLen + 1];
  const char *Code;
  int Access;
  C4ID idImage;
  C4ScriptFnRef *Next;
};

class C4ScriptHost : public C4ComponentHost {
public:
  C4ScriptHost();
  ~C4ScriptHost();

protected:
  C4ScriptFnRef *FunctionTable;

public:
  int Counter;
  BOOL Go;
  intptr_t Global[C4MaxGlobal];
  C4ID idDef;
  char *Script;

public:
  void Default();
  void Clear();
  void Close();
  void ClearPointers(C4Object *pObj);
  void MakeFunctionTable();
  BOOL Load(const char *szName, C4Group &hGroup, const char *szFilename, const char *szLanguage = NULL, C4ID idDefinition = C4ID_None);
  BOOL DenumerateVariablePointers();
  BOOL EnumerateVariablePointers();
  BOOL Execute();
  BOOL GetFunctionDeclaration(int iIndex, char *sTarget, char *sQualifier = NULL, const char **ppCode = NULL, char *sDesc = NULL, BOOL fRegular = TRUE, C4ID *pidImage = NULL, char *sCondition = NULL);
  const char *GetControlDesc(const char *szFunctionFormat, int iCom, C4ID *pidImage = NULL);
  int GetControlFlag(const char *szFunctionFormat);
  int ResolveIncludes(C4DefList &rDefs);
  intptr_t ObjectCall(C4Thread *pCaller, C4Object *pObj, const char *szFunction, intptr_t par0 = 0, intptr_t par1 = 0, intptr_t par2 = 0, intptr_t par3 = 0, intptr_t par4 = 0, intptr_t par5 = 0, intptr_t par6 = 0, intptr_t par7 = 0,
                  intptr_t par8 = 0, intptr_t par9 = 0);
  intptr_t Call(C4Thread *pCaller, const char *szFunction, intptr_t par0 = 0, intptr_t par1 = 0, intptr_t par2 = 0, intptr_t par3 = 0, intptr_t par4 = 0, intptr_t par5 = 0, intptr_t par6 = 0, intptr_t par7 = 0, intptr_t par8 = 0,
            intptr_t par9 = 0);
  C4ScriptFnRef *GetFunctionRef(const char *szFunction);
  C4ScriptFnRef *GetFunctionRef(int iFunction);

protected:
  void ClearFunctionTable();
  void AddFunctionTable(C4ScriptFnRef *pFn);
  void SetError(const char *szMessage);
  void MakeScript();
  intptr_t FunctionCall(C4Thread *pCaller, const char *szFunction, C4Object *pObj, intptr_t iPar0, intptr_t iPar1, intptr_t iPar2, intptr_t iPar3, intptr_t iPar4, intptr_t iPar5, intptr_t iPar6, intptr_t iPar7, intptr_t iPar8, intptr_t iPar9);
  int QualifierAccess(const char *szQualifier);
  BOOL ScanFunctionDesc(const char *szDesc, char *sDesc, C4ID *pidImage, char *sCondition);
};
