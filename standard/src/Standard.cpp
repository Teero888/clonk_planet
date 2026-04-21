/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* All kinds of valuable helpers */

#include <Memory.h>
#include <String.h>
#include <Math.h>
#include <sys/timeb.h>

#include <Standard.h>

//------------------------------------- Basics ----------------------------------------

void Swap(int &int1, int &int2)
  {
  int temp;
  temp=int2; int2=int1; int1=temp;
  }

int Max(int val1, int val2)
  {
  if (val1>val2) return val1;
  return val2;
  }

int Min(int val1, int val2)
  {
  if (val1<val2) return val1;
  return val2;
  }
  
int Distance(int iX1, int iY1, int iX2, int iY2)
	{
	return (int) sqrt(Abs(iX1-iX2)*Abs(iX1-iX2)+Abs(iY1-iY2)*Abs(iY1-iY2));
	}

int Angle(int iX1, int iY1, int iX2, int iY2)
	{
	const double pi = 3.141592654;
	int iAngle = (int) ( 180.0 * atan2( Abs(iY1-iY2), Abs(iX1-iX2) ) / pi );
	if (iX2>iX1 )
		{	if (iY2<iY1) iAngle = 90-iAngle; else iAngle = 90+iAngle;	}
	else
		{ if (iY2<iY1) iAngle = 270+iAngle;	else iAngle = 270-iAngle;	}
	return iAngle;
	}

void Toggle(BYTE &tbyte)
  {
  tbyte=!tbyte;
  }

void Toggle(BOOL &rBool)
  {
  rBool=!rBool;
  }

void FillMem(void *lpMem, DWORD dwSize, BYTE bValue)
  {
	memset(lpMem,bValue,dwSize);
  }

void ZeroMem(void *lpMem, DWORD dwSize)
  {
	FillMem(lpMem,dwSize,0);
  }

bool MemEqual(void *lpMem1, void *lpMem2, DWORD dwSize)
  {
	return !memcmp(lpMem1,lpMem2,dwSize);
  }

void MemCopy(void *lpMem1, void *lpMem2, DWORD dwSize)
  {
	memmove(lpMem2,lpMem1,dwSize);
	}

bool PathMove(int &rX, int &rY, int iTargetX, int iTargetY, int iSteps, int &rDelta)
	{
  int iDeltaX,iDeltaY,aincr,bincr,iDirX,iDirY,x,y;
	// No steps
	if (iSteps<=0) return false;
	// Y based
  if (Abs(iTargetX-rX)<Abs(iTargetY-rY))
    {
    iDirX=(iTargetX>rX) ? +1 : -1; iDirY=(iTargetY>rY) ? +1 : -1;
    iDeltaY=Abs(iTargetY-rY); iDeltaX=Abs(iTargetX-rX);   
		if (!rDelta) rDelta=2*iDeltaX-iDeltaY; 
		aincr=2*(iDeltaX-iDeltaY); bincr=2*iDeltaX; x=rX; y=rY;
		while (y!=iTargetY)
      {			
      if (rDelta>=0) { x+=iDirX; rDelta+=aincr; } else rDelta+=bincr;
			y+=iDirY;
			iSteps--; if (iSteps==0) { rX=x; rY=y; return true; }
      }
    }
	// X based
  else
    {
    iDirY=(iTargetY>rY) ? +1 : -1; iDirX=(iTargetX>rX) ? +1 : -1;
		iDeltaX=Abs(iTargetX-rX); iDeltaY=Abs(iTargetY-rY);
		if (!rDelta) rDelta=2*iDeltaY-iDeltaX; 
		aincr=2*(iDeltaY-iDeltaX); bincr=2*iDeltaY; x=rX; y=rY;
		while (x!=iTargetX)
      {
      if (rDelta>=0)	{ y+=iDirY; rDelta+=aincr; } else rDelta+=bincr;
			x+=iDirX;
			iSteps--; if (iSteps==0) { rX=x; rY=y; return true; }
      }
    }
	// Target reached (step overshoot)
	rX=x; rY=y;
  return false;
	}

bool ForLine(int x1, int y1, int x2, int y2, 
             BOOL (*fnCallback)(int, int, int), int iPar,
						 int *lastx, int *lasty)
  {
  int d,dx,dy,aincr,bincr,xincr,yincr,x,y;
  if (Abs(x2-x1)<Abs(y2-y1))
    {
    if (y1>y2) { Swap(x1,x2); Swap(y1,y2); }
    xincr=(x2>x1) ? 1 : -1;
    dy=y2-y1; dx=Abs(x2-x1);
    d=2*dx-dy; aincr=2*(dx-dy); bincr=2*dx; x=x1; y=y1;
    if (!fnCallback(x,y,iPar)) 
			{ if (lastx) *lastx=x; if (lasty) *lasty=y;
				return false; }
    for (y=y1+1; y<=y2; ++y)
      {
      if (d>=0) { x+=xincr; d+=aincr; }
      else d+=bincr;
      if (!fnCallback(x,y,iPar))
				{ if (lastx) *lastx=x; if (lasty) *lasty=y;
					return false; }
      }
    }
  else
    {
    if (x1>x2) { Swap(x1,x2); Swap(y1,y2); }
    yincr=(y2>y1) ? 1 : -1;
    dx=x2-x1; dy=Abs(y2-y1);
    d=2*dy-dx; aincr=2*(dy-dx); bincr=2*dy; x=x1; y=y1;
    if (!fnCallback(x,y,iPar))
			{ if (lastx) *lastx=x; if (lasty) *lasty=y;
				return false; }
    for (x=x1+1; x<=x2; ++x)
      {
      if (d>=0)	{ y+=yincr; d+=aincr; }
      else d+=bincr;
      if (!fnCallback(x,y,iPar))
				{ if (lastx) *lastx=x; if (lasty) *lasty=y;
					return false; }
      }
    }
  return true;
  }

//--------------------------------- Characters ------------------------------------------

char CharCapital(char cChar)
	{
  if (Inside(cChar,'a','z')) return (cChar-32);
  if (cChar=='ä') return 'Ä';
  if (cChar=='ö') return 'Ö';
  if (cChar=='ü') return 'Ü';
	return cChar;
	}

bool IsIdentifier(char cChar)
	{
  if (Inside(cChar,'A','Z')) return true;
  if (Inside(cChar,'a','z')) return true;
  if (Inside(cChar,'0','9')) return true;
  if (cChar=='_') return true; 
  if (cChar=='~') return true; 
  if (cChar=='+') return true;
  if (cChar=='-') return true;
	return false;
	}

bool IsWhiteSpace(char cChar)
	{
  if (cChar==' ') return true; 
  if (cChar==0x09) return true; // Tab
  if (cChar==0x0D) return true; // Line feed
  if (cChar==0x0A) return true; // Line feed
	return false;
	}

bool IsUpperCase(char cChar)
	{
  if (Inside(cChar,'A','Z')) return true;
  if (cChar=='Ä') return true;
  if (cChar=='Ö') return true;
  if (cChar=='Ü') return true;
	return false;
	}

bool IsLowerCase(char cChar)
	{
  if (Inside(cChar,'a','z')) return true;
  if (cChar=='ä') return true;
  if (cChar=='ö') return true;
  if (cChar=='ü') return true;
	return false;
	}

//------------------------------- Strings ------------------------------------------------

int SLen(const char *sptr)
  {
  int slen=0;
  if (!sptr) return 0;
  while (*sptr) { slen++; sptr++; }
  return slen;
  }

void SCopyL(const char *szSource, char *sTarget, int iMaxL)
  {
  if (!sTarget) return; *sTarget=0; if (!szSource) return;
  while (*szSource && (iMaxL>0))
    { *sTarget=*szSource; iMaxL--; szSource++; sTarget++; }    
  *sTarget=0;
  }

void SCopy(const char *szSource, char *sTarget, int iMaxL)
  {
	if (iMaxL==-1)
		{
		if (!sTarget) return; *sTarget=0; if (!szSource) return;
		strcpy(sTarget,szSource);
		}
	else
		SCopyL(szSource,sTarget,iMaxL);
  }

void SCopyUntil(const char *szSource, char *sTarget, char cUntil, int iMaxL)
  {
  if (!sTarget) return; *sTarget=0; if (!szSource) return;
  while ( *szSource && (*szSource!=cUntil) && (iMaxL!=0) )
    { *sTarget=*szSource; szSource++; sTarget++; iMaxL--; }
  *sTarget=0;
  }

int SCompare(const char *szStr1, const char *szStr2)
  {
  if (!szStr1 || !szStr2) return false;
	return strcmp(szStr1,szStr2);
  }

bool SEqual(const char *szStr1, const char *szStr2)
  {
  if (!szStr1 || !szStr2) return false;
	return !strcmp(szStr1,szStr2);
  }

bool SEqualUntil(const char *szStr1, const char *szStr2, char cWild)
  {
  if (!szStr1 || !szStr2) return false;
  while (*szStr1 || *szStr2)
    {
    if ((*szStr1==cWild) || (*szStr2==cWild)) return true;
    if (*szStr1!=*szStr2) return false;
    szStr1++; szStr2++;
    }
  return true;
  }

// Beginning of string 1 needs to match string 2.

bool SEqual2(const char *szStr1, const char *szStr2)
  { 
  if (!szStr1 || !szStr2) return false;
  while (*szStr1 && *szStr2)
    if (*szStr1++ != *szStr2++) return false;
  if (*szStr2) return false; // Str1 is shorter
  return true;
  }

bool SEqualNoCase(const char *szStr1, const char *szStr2, int iLen)
  {
  if (!szStr1 || !szStr2) return false;
	if (iLen==0) return true;
  while (*szStr1 && *szStr2)
		{
		if ( CharCapital(*szStr1++) != CharCapital(*szStr2++)) return false;
		if (iLen>0) { iLen--; if (iLen==0) return true; }
		}
  if (*szStr1 || *szStr2) return false;
  return true;
  }

int SCharPos(char cTarget, const char *szInStr, int iIndex)
  {
  const char *cpos;
  int ccpos;
  if (!szInStr) return -1;
  for (cpos=szInStr,ccpos=0; *cpos; cpos++,ccpos++)
    if (*cpos==cTarget) 
			if (iIndex==0) return ccpos;
			else iIndex--;
  return -1;
  }

int SCharLastPos(char cTarget, const char *szInStr)
  {
  const char *cpos;
  int ccpos,lcpos;
  if (!szInStr) return -1;
  for (cpos=szInStr,ccpos=0,lcpos=-1; *cpos; cpos++,ccpos++)
    if (*cpos==cTarget) lcpos=ccpos;
  return lcpos;
  }

void SAppend(const char *szSource, char *szTarget)
  {
  SCopy(szSource,szTarget+SLen(szTarget));  
  }

void SAppendChar(char cChar, char *szStr)
	{
	if (!szStr) return;
	for (char *cPos=szStr; *cPos; cPos++);
	*cPos=cChar; *(cPos+1)=0;
	}

bool SCopySegment(const char *szString, int iSegment, char *sTarget, 
                  char cSeparator, int iMaxL)
  {
	// Advance to indexed segment
  while (iSegment>0)
    {
    if (SCharPos(cSeparator,szString)==-1) { sTarget[0]=0; return false; } // No more segments
    szString += SCharPos(cSeparator,szString)+1;
    iSegment--;
    }
	// Copy segment contents
  SCopyUntil(szString,sTarget,cSeparator,iMaxL);
  return true;
  }

bool SCopyNamedSegment(const char *szString, const char *szName, char *sTarget, 
		                   char cSeparator, char cNameSeparator, int iMaxL)
  {
	// Advance to named segment
  while (!( SEqual2(szString,szName) && (szString[SLen(szName)]==cNameSeparator) ))
    {
    if (SCharPos(cSeparator,szString)==-1) { sTarget[0]=0; return false; } // No more segments
    szString += SCharPos(cSeparator,szString)+1;
    }
	// Copy segment contents
  SCopyUntil(szString+SLen(szName)+1,sTarget,cSeparator,iMaxL);
  return true;
  }

int SCharCount(char cTarget, const char *szInStr, const char *cpUntil)
  {
  int iResult=0;
	// Scan string
  while (*szInStr)
    {
		// End position reached (end character is not included)
		if (szInStr==cpUntil) return iResult;
		// Character found
    if (*szInStr==cTarget) iResult++;
		// Advance
    szInStr++;
    }
	// Done
  return iResult;
  }

int SCharCountEx(const char *szString, const char *szCharList)
	{
	int iResult = 0;
	while ( *szCharList )
		{
		iResult += SCharCount( *szCharList, szString );
		szCharList++;
		}
	return iResult;
	}

void SReplaceChar(char *str, char fc, char tc)
  {
  while (str && *str)
    { if (*str==fc) *str=tc; str++; }  
  }

void SCapitalize(char *str)
  {
  while (str && *str)
    {
		*str=CharCapital(*str);
    str++;
    }
  }

const char *SSearchIdentifier(const char *szString, const char *szIndex)
	{
	// Does not check whether szIndex itself is an identifier.
	// Just checks for space in front and back.
  const char *cscr;
  int indexlen,match=0;
	bool frontok=true;
  if (!szString || !szIndex) return NULL;
  indexlen=SLen(szIndex);
  for (cscr=szString; cscr && *cscr; cscr++)
    {
		// Match length    
		if (*cscr==szIndex[match]) match++;
    else match=0;
    // String is matched, front and back ok?
		if (match>=indexlen)
			if (frontok)
				if (!IsIdentifier(*(cscr+1)))
					return cscr+1;
    // Currently no match, check for frontok
		if (match==0) 
			if (IsIdentifier(*cscr)) frontok=false;
			else frontok=true;    
		}
  return NULL;
	}

const char *SSearch(const char *szString, const char *szIndex)
  {
  const char *cscr;
  int indexlen,match=0;
  if (!szString || !szIndex) return NULL;
  indexlen=SLen(szIndex);
  for (cscr=szString; cscr && *cscr; cscr++)
    {
    if (*cscr==szIndex[match]) match++;
    else match=0;
    if (match>=indexlen) return cscr+1;
    }
  return NULL;
  }

const char *SSearchNoCase(const char *szString, const char *szIndex)
  {
  const char *cscr;
  int indexlen,match=0;
  if (!szString || !szIndex) return NULL;
  indexlen=SLen(szIndex);
  for (cscr=szString; cscr && *cscr; cscr++)
    {
    if (CharCapital(*cscr)==CharCapital(szIndex[match])) match++;
    else match=0;
    if (match>=indexlen) return cscr+1;
    }
  return NULL;
  }

void SWordWrap(char *szText, char cSpace, char cSepa, int iMaxLine)
  {
  if (!szText) return;  
  // Scan string
  char *cPos,*cpLastSpace=NULL;
  int iLineRun=0;
  for (cPos=szText; *cPos; cPos++)
    {
		// Store last space
    if (*cPos==cSpace) cpLastSpace=cPos;
		// Separator encountered: reset line run
		if (*cPos==cSepa) iLineRun=0;
		// Need a break, insert at last space
    if (iLineRun>=iMaxLine)
      if (cpLastSpace)
        { *cpLastSpace=cSepa; iLineRun=cPos - cpLastSpace; }
		// Line run
    iLineRun++;
    }
  }

const char *SAdvanceSpace(const char *szSPos)
  {
  if (!szSPos) return NULL;
  while (IsWhiteSpace(*szSPos)) szSPos++;
  return szSPos;
  }

const char *SRewindSpace(const char *szSPos, const char *pBegin)
  {
  if (!szSPos || !pBegin) return NULL;
  while (IsWhiteSpace(*szSPos))
		{
		szSPos--;
		if (szSPos<pBegin) return NULL;
		}
  return szSPos;
  }

const char *SAdvancePast(const char *szSPos, char cPast)
  {
  if (!szSPos) return NULL;
  while (*szSPos)
    { 
    if (*szSPos==cPast) { szSPos++; break; }
    szSPos++; 
    }
  return szSPos;
  }

void SCopyIdentifier(const char *szSource, char *sTarget, int iMaxL)
  {
  if (!szSource || !sTarget) return;
  while (IsIdentifier(*szSource))
		{
		if (iMaxL==1) { *sTarget++ = *szSource++; break; }
		iMaxL--;
		*sTarget++ = *szSource++;
		}
  *sTarget=0;
  }

int SClearFrontBack(char *szString, char cClear)
	{
	int cleared=0;
	char *cpos;
	if (!szString) return 0;
	for (cpos=szString; *cpos && (*cpos==cClear); cpos++,cleared++);
	if (cpos!=szString) SCopy(cpos,szString);
	for (cpos=szString+SLen(szString)-1; (cpos>szString) && (*cpos==cClear); cpos--,cleared++)
		*cpos=0x00;
  return cleared;
	}

int SLenUntil(const char *szStr, char cUntil)
  {
  int slen;
  for (slen=0; *szStr && (*szStr!=cUntil); slen++,szStr++);
  return slen;
  }

void SNewSegment(char *szStr, const char *szSepa)
	{
	if (szStr[0]) SAppend(szSepa,szStr);
	}

char *SGetFilename(const char *szFilepath)
  {
  int iPos;
  for (iPos=0; szFilepath[iPos]; iPos++);
  while ((iPos>0) && (szFilepath[iPos-1]!='\\')) iPos--;
  return (char*) szFilepath+iPos;
  }

void SRemoveComments(char *szScript)
	{
	const char *pScriptCont;
	if (!szScript) return;
	while (*szScript)
		{
		// Advance to next slash
		while (*szScript && (*szScript!='/')) szScript++;
		if (!(*szScript)) return; // No more comments
		// Line comment
		if (szScript[1]=='/')
			{
			if (pScriptCont = SSearch(szScript+2,LineFeed))
				SCopy(pScriptCont-SLen(LineFeed),szScript);
			else
				szScript[0]=0;
			}
		// Block comment
		else if (szScript[1]=='*')
			{
			if (pScriptCont = SSearch(szScript+2,"*/"))
				SCopy(pScriptCont,szScript);
			else
				szScript[0]=0;
			}
		// No comment
		else
			{
			szScript++;
			}
		}
	}

bool SCopyPrecedingIdentifier(const char *pBegin, const char *pIdentifier, char *sTarget, int iSize)
	{
	// Safety
	if (!pIdentifier || !sTarget || !pBegin) return false;
	// Empty default
	sTarget[0]=0;
	// Identifier is at begin
	if (!(pIdentifier>pBegin)) return false;
	// Rewind space
	const char *cPos;
	if (!(cPos = SRewindSpace(pIdentifier-1,pBegin))) return false;
	// Rewind to beginning of identifier
  while ((cPos>pBegin) && IsIdentifier(cPos[-1])) cPos--;
	// Copy identifier
	SCopyIdentifier(cPos,sTarget,iSize);
	// Success
	return true;
	}

const char *SSearchFunction(const char *szString, const char *szIndex)
	{
	// Safety
	if (!szString || !szIndex) return NULL;
	// Ignore failsafe
	if (szIndex[0]=='~') szIndex++;
	// Buffer to append colon
	char szDeclaration[256+2];
	SCopy(szIndex,szDeclaration,256); SAppendChar(':',szDeclaration);
	// Search identifier
	return SSearchIdentifier(szString,szDeclaration);
	}

void SInsert(char *szString, const char *szInsert, int iPosition)
	{
	// Safety
	if (!szString || !szInsert || !szInsert[0]) return;
	// Move up string remainder
	MemCopy( szString+iPosition, szString+iPosition+SLen(szInsert), SLen(szString+iPosition)+1 );
	// Copy insertion
	MemCopy( (void*) szInsert, szString+iPosition, SLen(szInsert) );
	}

void SDelete(char *szString, int iLen, int iPosition)
	{
	// Safety
	if (!szString) return;
	// Move down string remainder
	MemCopy( szString+iPosition+iLen, szString+iPosition, SLen(szString+iPosition+iLen)+1 );
	}

bool SCopyEnclosed(const char *szSource, char cOpen, char cClose, char *sTarget, int iSize)
	{
	int iPos,iLen;
	if (!szSource || !sTarget) return false;
	if ((iPos = SCharPos(cOpen,szSource)) < 0) return false;
	if ((iLen = SCharPos(cClose,szSource+iPos+1)) < 0) return false;
	SCopy(szSource+iPos+1,sTarget,Min(iLen,iSize));
	return true;	
	}

bool SGetModule(const char *szList, int iIndex, char *sTarget, int iSize)
	{
	if (!szList || !sTarget) return false;
	if (!SCopySegment(szList,iIndex,sTarget,';',iSize)) return false;
	SClearFrontBack(sTarget);
	return true;
	}

bool SIsModule(const char *szList, const char *szString, int *ipIndex, bool fCase)
	{
	char szModule[1024+1];
	// Compare all modules
	for (int iMod=0; SGetModule(szList,iMod,szModule,1024); iMod++)
		if (fCase ? SEqual(szString,szModule) : SEqualNoCase(szString,szModule))
			{
			// Provide index if desired
			if (ipIndex) *ipIndex = iMod;
			// Found
			return true;
			}
	// Not found
	return false;
	}

bool SAddModule(char *szList, const char *szModule)
	{
	// Safety / no empties
	if (!szList || !szModule || !szModule[0]) return false;
	// Already a module?
	if (SIsModule(szList,szModule)) return false;
	// New segment, add string
	SNewSegment(szList);
	SAppend(szModule,szList);
	// Success
	return true;
	}

bool SAddModules(char *szList, const char *szModules)
	{
	// Safety / no empties
	if (!szList || !szModules || !szModules[0]) return false;
	// Add modules
	char szModule[1024+1]; // limited
	for (int cnt=0; SGetModule(szModules,cnt,szModule,1024); cnt++)
		SAddModule(szList,szModule);
	// Success
	return true;
	}

bool SRemoveModule(char *szList, const char *szModule, bool fCase)
	{
	int iMod,iPos,iLen;
	// Not a module
	if (!SIsModule(szList,szModule,&iMod,fCase)) return false;
	// Get module start
	iPos = 0;
	if (iMod > 0) iPos = SCharPos(';',szList,iMod-1)+1;
	// Get module length
	iLen = SCharPos(';',szList+iPos);
	if (iLen<0) iLen=SLen(szList); else iLen+=1;
	// Delete module
	SDelete(szList,iLen,iPos);
	// Success
	return true;
	}

int SModuleCount(const char *szList)
	{
	if (!szList) return 0;
	int iCount = 0;
	bool fNewModule = true;
	while (*szList)
		{
		switch (*szList)
			{
			case ' ': break;
			case ';': fNewModule=true; break;
			default: if (fNewModule) iCount++; fNewModule=false; break;
			}
		szList++;
		}
	return iCount;
	}

//--------------------------------- Blitting ---------------------------------------------

void BufferBlit(BYTE *bypSource, int iSourcePitch, 
                int iSrcBufHgt, // Positive: Bottom up
                int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
                BYTE *bypTarget, int iTargetPitch, 
                int iTrgBufHgt, // Positive: Bottom up
                int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt)
  {
  if (!bypSource || !bypTarget) return;
  if (!iTrgWdt || !iTrgHgt) return;
  int xcnt,ycnt,sline,tline,fy;
  for (ycnt=0; ycnt<iTrgHgt; ycnt++)
    {    
    fy = iSrcHgt * ycnt / iTrgHgt;
    if (iSrcBufHgt>0) sline = ( iSrcBufHgt - 1 - iSrcY - fy ) * iSourcePitch;
    else sline = ( iSrcY + fy ) * iSourcePitch;
    if (iTrgBufHgt>0) tline = ( iTrgBufHgt - 1 - iTrgY - ycnt ) * iTargetPitch;
    else tline = ( iTrgY + ycnt ) * iTargetPitch;
    for (xcnt=0; xcnt<iTrgWdt; xcnt++)
       bypTarget [ tline + iTrgX + xcnt ]
         = bypSource [ sline + iSrcX + iSrcWdt * xcnt / iTrgWdt ];
    }
  }

void BufferBlitAspect(BYTE *bypSource, int iSourcePitch, 
                int iSrcBufHgt, // Positive: Bottom up
                int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
                BYTE *bypTarget, int iTargetPitch, 
                int iTrgBufHgt, // Positive: Bottom up
                int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt)
  {
  int nhgt,nwdt;
  if (!iSrcWdt || !iSrcHgt) return;
  // Adjust height aspect by width aspect
  if (100*iTrgWdt/iSrcWdt<100*iTrgHgt/iSrcHgt) 
    { 
    nhgt=iSrcHgt*iTrgWdt/iSrcWdt;
    iTrgY+=(iTrgHgt-nhgt)/2; iTrgHgt=nhgt;
    }
  else // Adjust width aspect by height aspect
    if (100*iTrgHgt/iSrcHgt<100*iTrgWdt/iSrcWdt)
      { 
      nwdt=iSrcWdt*iTrgHgt/iSrcHgt;
      iTrgX+=(iTrgWdt-nwdt)/2; iTrgWdt=nwdt;
      }
  BufferBlit(bypSource,iSourcePitch,iSrcBufHgt,
             iSrcX,iSrcY,iSrcWdt,iSrcHgt,
             bypTarget,iTargetPitch,iTrgBufHgt,
             iTrgX,iTrgY,iTrgWdt,iTrgHgt);
  }

void StdBlit(BYTE *bypSource, int iSourcePitch, int iSrcBufHgt,
          int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
          BYTE *bypTarget, int iTargetPitch, int iTrgBufHgt, 
          int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt,
					int iBytesPerPixel, bool fFlip)
  {
  if (!bypSource || !bypTarget) return;
  if (!iTrgWdt || !iTrgHgt) return;
  int xcnt,ycnt,zcnt,sline,tline,fy;
  for (ycnt=0; ycnt<iTrgHgt; ycnt++)
    {
    fy = iSrcHgt * ycnt / iTrgHgt;
    if (iSrcBufHgt>0) sline = ( iSrcBufHgt - 1 - iSrcY - fy ) * iSourcePitch;
    else sline = ( iSrcY + fy ) * iSourcePitch;
    if (iTrgBufHgt>0) tline = ( iTrgBufHgt - 1 - iTrgY - ycnt ) * iTargetPitch;
    else tline = ( iTrgY + ycnt ) * iTargetPitch;
		if (!fFlip)
			{
	    for (xcnt=0; xcnt<iTrgWdt; xcnt++)
				for (zcnt=0; zcnt<iBytesPerPixel; zcnt++)
		      bypTarget [ tline + (iTrgX + xcnt) * iBytesPerPixel + zcnt]
			     = bypSource [ sline + (iSrcX + iSrcWdt * xcnt / iTrgWdt) * iBytesPerPixel + zcnt ]; 
			}
		else
			{
	    for (xcnt=0; xcnt<iTrgWdt; xcnt++)
				for (zcnt=0; zcnt<iBytesPerPixel; zcnt++)
		      bypTarget [ tline + (iTrgX + iTrgWdt - 1 -xcnt) * iBytesPerPixel + zcnt]
			     = bypSource [ sline + (iSrcX + iSrcWdt * xcnt / iTrgWdt) * iBytesPerPixel + zcnt ]; 
			}  
    }
  }

//---------------------------------- Global Milliseconds -------------------------------------

DWORD time_msecs()
	{
	struct _timeb timebuffer;
	_ftime(&timebuffer);
	DWORD secs = timebuffer.time;
	DWORD msecs = timebuffer.millitm;
	return secs*1000+msecs;
	}

//------------------------- A global variable used by StdRandom -------------------------------

int RandomCount = 0;

