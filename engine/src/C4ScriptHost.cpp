/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles script file components (calls, inheritance, function maps) */

#include <C4Include.h>

C4ScriptHost::C4ScriptHost()
	{
	Default();
	}

C4ScriptHost::~C4ScriptHost()
	{
	Clear();
	}

void C4ScriptHost::Default()
	{
	C4ComponentHost::Default();
	Script=NULL;
	Counter=0;
	Go=FALSE;
	FunctionTable=NULL;
	for (int cnt=0; cnt<C4MaxGlobal; cnt++) Global[cnt]=0;
	idDef=C4ID_None;
	}

void C4ScriptHost::Clear()
	{
	// Clear compiled script
	if (Script) delete [] Script; Script=NULL;
	// Clear funtion table
	C4ScriptFnRef *pFn,*pNext;
	for (pFn = FunctionTable; pFn; pFn=pNext)	{ pNext=pFn->Next; delete pFn; }
	FunctionTable=NULL;
	// Clear base class
	C4ComponentHost::Clear();
	}

BOOL C4ScriptHost::Execute()
	{
	if (!Script) return FALSE;
#ifdef C4ENGINE
	char buffer[500];
	if (Go && !Tick10)
		{
		sprintf(buffer,PSF_Script,Counter++);
		return Call(NULL,buffer);
		}
#endif
	return FALSE;
	}

long C4ScriptHost::Call(C4Thread *pCaller, const char *szFunction,
											 long par0, long par1, long par2, long par3, long par4,
											 long par5, long par6, long par7, long par8, long par9)
  {
	if (!Script || !szFunction) return FALSE;
	return FunctionCall(pCaller,szFunction,NULL,par0,par1,par2,par3,par4,par5,par6,par7,par8,par9);
	}

long C4ScriptHost::ObjectCall(C4Thread *pCaller, C4Object *pObj, const char *szFunction,
											 long par0, long par1, long par2, long par3, long par4,
											 long par5, long par6, long par7, long par8, long par9)
  {
  if (!Script || !pObj || !szFunction) return FALSE;
  return FunctionCall(pCaller,szFunction,pObj,par0,par1,par2,par3,par4,par5,par6,par7,par8,par9);
  }

BOOL C4ScriptHost::GetFunctionDeclaration(int iIndex, char *sTarget, char *sQualifier, 
																					const char **ppCode, char *sDesc, 
																					BOOL fRegular,C4ID *pidImage,char *sCondition)
	{
	if (!Script || !sTarget) return FALSE;
	int iLabel=-1; 
	const char *cpPos=Script,*cpScan;
	const char *cpColon,*cpLabel;
	char szDesc[C4SCR_MaxDesc+1];

	// Search all declarations by colon
	while (cpColon = SSearch(cpPos,":"))
		{
		cpColon--;
		
		// Trace back to label
		for (cpLabel=cpColon; (cpLabel-1>=cpPos) && IsIdentifier(*(cpLabel-1)); cpLabel--);
		// Isolated colon: ignore
		if (cpLabel==cpColon)
			{ cpPos=cpColon+1; continue; } 		
		// Colon within string literal: ignore
		if (SCharCount('"',Script,cpColon) % 2)
			{ cpPos=cpColon+1; continue; }
		// Colon within brackets: ignore
		if (SCharCount('[',Script,cpColon) > SCharCount(']',Script,cpColon))
			// May be screwed by uneven brackets within string literal or wild brackets...!
			{ cpPos=cpColon+1; continue; } 

		// Regular functions only: ignore Script_ & Frame_
		if (fRegular)
			if (SEqual2(cpLabel,"Script") || SEqual2(cpLabel,"Frame"))
				{ cpPos=cpColon+1; continue; }
		// Label okay
		iLabel++;
		
		// Indexed label found
		if (iLabel==iIndex) 
			{ 
			// Copy label
			SCopyUntil(cpLabel,sTarget,':'); 
			// Copy qualifier
			SCopyPrecedingIdentifier(Script,cpLabel,sQualifier,25);
			// Scan function desc
			if (sDesc) sDesc[0]=0;
			if (pidImage) *pidImage=C4ID_None;
			cpScan=cpColon+1; cpScan=SAdvanceSpace(cpScan);
			if (*cpScan=='[')
				{
				SCopyEnclosed(cpScan,'[',']',szDesc,C4SCR_MaxDesc);
				ScanFunctionDesc(szDesc,sDesc,pidImage,sCondition);
				cpScan=SAdvancePast(cpScan,']');
				}
			// Store code position
			if (ppCode) *ppCode = cpScan;
			// Success
			return TRUE; 
			}

		// Advance to next label
		cpPos=cpColon+1;
		}

	return FALSE;
	}
	
void C4ScriptHost::ClearPointers(C4Object *pObj)
	{
	// Global variable pointers
	for (int cnt=0; cnt<C4MaxVariable; cnt++)
		if ( Global[cnt] == (long)pObj )
			Global[cnt]=0;
	}

BOOL C4ScriptHost::EnumerateVariablePointers()
	{
#ifdef C4ENGINE
	int cnt,iPtrNum;
	for (cnt=0; cnt<C4MaxVariable; cnt++)
		if (iPtrNum = Game.Objects.ObjectNumber((C4Object*)Global[cnt]))
			Global[cnt] = 1000000000 + iPtrNum;
#endif
	return TRUE;
	}

BOOL C4ScriptHost::DenumerateVariablePointers()
	{
#ifdef C4ENGINE
	for (int cnt=0; cnt<C4MaxVariable; cnt++)
		if (Inside(Global[cnt],1000000000,1001000000))
			Global[cnt] = (long) Game.Objects.ObjectPointer(Global[cnt]-1000000000);
#endif
	return TRUE;
	}

BOOL C4ScriptHost::Load(const char *szName, C4Group &hGroup, const char *szFilename, 
												const char *szLanguage, C4ID idDefinition)
	{
	// Base load
	if (!C4ComponentHost::Load(szName,hGroup,szFilename,szLanguage)) return FALSE;
	// Set definition id
	idDef = idDefinition;
	// Make executable script
	MakeScript();
	// Success
	return TRUE;
	}

void C4ScriptHost::MakeScript()
	{
	// Clear any old
	if (Script) delete [] Script; Script=NULL;
	// Create copy
	if (!Data || !Data[0]) return;
	Script = new char [SLen(Data)+1];
	SCopy(Data,Script);
	// Remove comments
	SRemoveComments(Script);
	// Create function table
	MakeFunctionTable();
	}

void C4ScriptHost::Close()
	{
	// Base close
	C4ComponentHost::Close();
	// Make executable script
	MakeScript();
	// Update console
#ifdef C4ENGINE
	Console.UpdateInputCtrl();
#endif
	}

int C4ScriptHost::GetControlFlag(const char *szFunctionFormat)
	{
	int iResult = 0;

	if (!Script) return 0;

	// Scan for com defined control functions
#ifdef C4ENGINE
	int iCom;
	char szFunction[256+1];
	for (iCom=0; iCom<ComOrderNum; iCom++)
		{
		sprintf( szFunction, szFunctionFormat, ComName( ComOrder(iCom) ) );
		if (SSearchFunction(Script,szFunction))	iResult |= ( 1 << iCom );
		}
#endif

	return iResult;
	}

void C4ScriptHost::MakeFunctionTable()
	{
	// Clear old function table
	ClearFunctionTable();
	// Scan for functions and add to function table
	char szFunction[C4SCR_MaxIDLen+1],szQualifier[C4SCR_MaxIDLen+1];
	char szDesc[C4SCR_MaxDesc+1];
	char szCondition[C4SCR_MaxIDLen+1];
	const char *cpCode;
	C4ID idImage;
	for (int cnt=0; GetFunctionDeclaration(cnt,szFunction,szQualifier,&cpCode,szDesc,FALSE,&idImage,szCondition); cnt++)
		{
		C4ScriptFnRef *pFn = new C4ScriptFnRef;
		SCopy(szFunction,pFn->Name,C4SCR_MaxIDLen);
		SCopy(szDesc,pFn->Desc,C4SCR_MaxDesc);
		SCopy(szCondition,pFn->Condition,C4SCR_MaxIDLen);
		pFn->Code = cpCode;
		pFn->Access = QualifierAccess(szQualifier);
		if (idImage==C4ID_None) idImage=idDef;
		pFn->idImage = idImage;
		AddFunctionTable(pFn);
		}
	}

int C4ScriptHost::QualifierAccess(const char *szQualifier)
	{
	if (SEqual(szQualifier,C4SCR_Public)) return C4SCR_Access_Public;
	if (SEqual(szQualifier,C4SCR_Protected)) return C4SCR_Access_Protected;
	if (SEqual(szQualifier,C4SCR_Private)) return C4SCR_Access_Private;
	return C4SCR_Access_Public;
	}

C4ScriptFnRef* C4ScriptHost::GetFunctionRef(const char *szFunction)
	{
	for (C4ScriptFnRef *pFn=FunctionTable; pFn; pFn=pFn->Next)
		if (SEqual(pFn->Name,szFunction))
			return pFn;
	return NULL;
	}

C4ScriptFnRef* C4ScriptHost::GetFunctionRef(int iFunction)
	{
	for (C4ScriptFnRef *pFn=FunctionTable; pFn; pFn=pFn->Next,iFunction--)
		if (iFunction==0)
			return pFn;
	return NULL;
	}

long C4ScriptHost::FunctionCall(C4Thread *pCaller, const char *szFunction, C4Object *pObj, long iPar0, long iPar1, long iPar2, long iPar3, long iPar4, long iPar5, long iPar6, long iPar7, long iPar8, long iPar9)
	{

#ifdef C4ENGINE

	char szError[500+1];

	// Failsafe call
	BOOL fFailsafe = FALSE;	if (szFunction[0]=='~') { fFailsafe=TRUE; szFunction++; }

	// Get function reference from table
	C4ScriptFnRef *pFn = GetFunctionRef(szFunction);

	// Undefined function
	if (!pFn)
		{
		// Failsafe
		if (fFailsafe) return 0;
		// Error
		sprintf(szError,"Undefined function '%s'",szFunction); 
		if (pCaller) pCaller->SetError(szError); else Console.Out(szError);
		return FALSE;
		}

	// Object call: check access
	if (pObj)
		switch (pFn->Access)
			{
			// Access by same object or engine only
			case C4SCR_Access_Protected: 
				if (pCaller && pCaller->cObj && pCaller->cObj!=pObj)
					{
					sprintf(szError,"Cannot access protected member '%s' in %s",szFunction,pObj->Def->Name);
					if (pCaller) pCaller->SetError(szError); else Console.Out(szError);
					return FALSE;
					}
				break;
			// Access by same object only
			case C4SCR_Access_Private: 
				if (!(pCaller && (pCaller->cObj==pObj)))
					{
					sprintf(szError,"Cannot access private member '%s' in %s",szFunction,pObj->Def->Name);
					if (pCaller) pCaller->SetError(szError); else Console.Out(szError);
					return FALSE;
					}
				break;
			}

	// Call code	
	C4Thread cthr;
  return cthr.Execute(pCaller,Script,szFunction,pFn->Code,
											pObj,iPar0,iPar1,iPar2,iPar3,iPar4,iPar5,iPar6,iPar7,iPar8,iPar9);

#else

	return 0;

#endif

	}

void C4ScriptHost::SetError(const char *szMessage)
	{

	}

const char *C4ScriptHost::GetControlDesc(const char *szFunctionFormat, int iCom, C4ID *pidImage)
	{
#ifdef C4ENGINE
	// Compose script function
	char szFunction[256+1]; 
	sprintf(szFunction,szFunctionFormat,ComName(iCom));
	// Remove failsafe indicator
	if (szFunction[0]=='~') SCopy(szFunction+1,szFunction);
	// Find function reference
	C4ScriptFnRef *pFn = GetFunctionRef(szFunction);
	// Get image id
	if (pidImage) { *pidImage=idDef; if (pFn) *pidImage=pFn->idImage; }
	// Return function desc
	if (pFn && pFn->Desc[0]) return pFn->Desc;
#endif
	// No function
	return NULL;
	}

BOOL C4ScriptHost::ScanFunctionDesc(const char *szDesc, char *sDesc, C4ID *pidImage,
																		char *sCondition)
	{
	// Default zero values
	if (sDesc) sDesc[0]=0;
	if (pidImage) *pidImage=C4ID_None;
	if (sCondition) sCondition[0]=0;
	// Scan fields
	char szField[C4SCR_MaxDesc+1];
	for (int iField=0; SCopySegment(szDesc,iField,szField,'|',C4SCR_MaxDesc); iField++)
		{
		// Image field
		if (SEqual2(szField,"Image="))
			{
			// Scan image id
			if (pidImage) *pidImage = C4Id(szField+6);
			// Field done
			continue;
			}
		// Condition field
		if (SEqual2(szField,"Condition="))
			{
			// Scan condition
			if (sCondition) SCopy(szField+10,sCondition);
			// Field done
			continue;
			}
		// Desc field
		if (sDesc) SCopy(szField,sDesc);
		// Field done
		continue;
		}
	// Done
	return TRUE;
	}

void C4ScriptHost::AddFunctionTable(C4ScriptFnRef *pFunction)
	{
	// Remove (overload) any same name function
	C4ScriptFnRef *pPrev=NULL,*pNext;
	for (C4ScriptFnRef *pFn=FunctionTable; pFn; pFn=pNext)
		{
		pNext=pFn->Next;
		if (SEqual(pFn->Name,pFunction->Name))
			{
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		if (Config.Graphics.VerboseObjectLoading>=2)
			{ sprintf(OSTR,LoadResStr(IDS_PRC_FUNCTIONOVERLOAD),pFn->Name,C4IdText(idDef));	Log(OSTR); }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			if (pPrev) pPrev->Next=pNext; else FunctionTable=pNext;
			delete pFn;
			}
		else
			pPrev=pFn;
		}
	// Add new ref to end of function list
	for (pPrev=FunctionTable; pPrev && pPrev->Next; pPrev=pPrev->Next);
	if (pPrev) pPrev->Next=pFunction;
	else FunctionTable = pFunction;
	pFunction->Next = NULL;
	}

int C4ScriptHost::ResolveIncludes(C4DefList &rDefs)
	{
	int iInsertions=0;
	const char *cPos;
	char szID[4+1];
	C4ID idInclude;
	int iIncludeStart,iIncludeEnd;
	C4Def *pDef;
	char *cpScript;
	while (cPos=SSearch(Script,"#include"))
		{
		// Include starting position
		iIncludeStart=(cPos-Script)-SLen("#include");
		// Get include id
		cPos=SAdvanceSpace(cPos);
		SCopyIdentifier(cPos,szID,4);
		idInclude=C4Id(szID);
		// Include end position
		iIncludeEnd=(cPos-Script)+SLen(szID);
		// Cannot include self
		if (idInclude==idDef)
			{
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			sprintf(OSTR,LoadResStr(IDS_PRC_NOINCLUDESELF),C4IdText(idDef)); Log(OSTR);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
			break;
			}
		// Get def to include
		if (!(pDef=rDefs.ID2Def(idInclude)))
			{
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			sprintf(OSTR,LoadResStr(IDS_PRC_UNDEFINEDINCLUDE),C4IdText(idDef),szID); Log(OSTR);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
			break;
			}
		// Allocate new script buffer
		cpScript = new char [ SLen(Script) + SLen(pDef->Script.Script) + 1 ];
		// Compose new script
		SCopy(Script,cpScript,iIncludeStart);
		SAppend(pDef->Script.Script,cpScript);
		SAppend(Script+iIncludeEnd,cpScript);
		// Replace old script
		delete [] Script;
		Script = cpScript;
		// Count insertion
		iInsertions++;
		}

	return iInsertions;
	}

void C4ScriptHost::ClearFunctionTable()
	{
	C4ScriptFnRef *pFn,*pNext;
	for (pFn = FunctionTable; pFn; pFn=pNext)	{ pNext=pFn->Next; delete pFn; }
	FunctionTable=NULL;
	}
