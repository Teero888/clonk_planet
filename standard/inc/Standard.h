/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* All kinds of valuable helpers */

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   UINT;
typedef unsigned long  DWORD;
typedef int            BOOL;

#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL 0
#endif

inline void DWordAlign(int &val)
  {
  if (val%4) { val>>=2; val<<=2; val+=4; }
  }

inline int DWordAligned(int val)
  {
  if (val%4) { val>>=2; val<<=2; val+=4; }
	return val;
  }

inline bool Inside(int ival, int lbound, int rbound)
  {
  if ((ival>=lbound) && (ival<=rbound)) return true;
  return false;
  }

inline int BoundBy(int bval, int lbound, int rbound)
  {
  if (bval<lbound) return lbound;
  if (bval>rbound) return rbound;
  return bval;
  }

inline int ForceLimits(int &rVal, int iLow, int iHi)
	{
	if (rVal<iLow) { rVal=iLow; return -1; }
	if (rVal>iHi)  { rVal=iHi;  return +1; }
	return 0;
	}

inline int Abs(int val)
  {
  if (val<0) return -val;
  return val;
  }

inline int Sign(int val)
  {
  if (val<0) return -1;
  if (val>0) return +1;
  return 0;
  }

void Swap(int &int1, int &int2);
int Max(int val1, int val2);
int Min(int val1, int val2);
void Toggle(BYTE &tbyte);
void Toggle(BOOL &rBool);
int Distance(int iX1, int iY1, int iX2, int iY2);
int Angle(int iX1, int iY1, int iX2, int iY2);

void FillMem(void *lpMem, DWORD dwSize, BYTE bValue);
void ZeroMem(void *lpMem, DWORD dwSize);
bool MemEqual(void *lpMem1, void *lpMem2, DWORD dwSize);
void MemCopy(void *lpMem1, void *lpMem2, DWORD dwSize);

bool ForLine(int x1, int y1, int x2, int y2, 
             BOOL (*fnCallback)(int, int, int), int iPar=0,
						 int *lastx=NULL, int *lasty=NULL);

bool PathMove(int &rX, int &rY, int iTargetX, int iTargetY, int iSteps, int &rDelta);

char CharCapital(char cChar);
bool IsIdentifier(char cChar);
bool IsUpperCase(char cChar);
bool IsLowerCase(char cChar);
bool IsWhiteSpace(char cChar);

int  SLen(const char *sptr);
int  SLenUntil(const char *szStr, char cUntil);

bool SEqual(const char *szStr1, const char *szStr2);
bool SEqual2(const char *szStr1, const char *szStr2);
bool SEqualUntil(const char *szStr1, const char *szStr2, char cWild);
bool SEqualNoCase(const char *szStr1, const char *szStr2, int iLen=-1);

int  SCompare(const char *szStr1, const char *szStr2);

void SCopy(const char *szSource, char *sTarget, int iMaxL=-1);
void SCopyUntil(const char *szSource, char *sTarget, char cUntil, int iMaxL=-1);
void SCopyIdentifier(const char *szSource, char *sTarget, int iMaxL=0);
bool SCopyPrecedingIdentifier(const char *pBegin, const char *pIdentifier, char *sTarget, int iSize);
bool SCopySegment(const char *fstr, int segn, char *tstr, char sepa=';', int iMaxL=-1);
bool SCopyNamedSegment(const char *szString, const char *szName, char *sTarget, char cSeparator=';', char cNameSeparator='=', int iMaxL=-1);
bool SCopyEnclosed(const char *szSource, char cOpen, char cClose, char *sTarget, int iSize);

void SAppend(const char *szSource, char *szTarget);
void SAppendChar(char cChar, char *szStr);

void SInsert(char *szString, const char *szInsert, int iPosition=0);
void SDelete(char *szString, int iLen, int iPosition=0);

int  SCharPos(char cTarget, const char *szInStr, int iIndex=0);
int  SCharLastPos(char cTarget, const char *szInStr);
int  SCharCount(char cTarget, const char *szInStr, const char *cpUntil=NULL);
int  SCharCountEx(const char *szString, const char *szCharList);

void SReplaceChar(char *str, char fc, char tc);

const char *SSearch(const char *szString, const char *szIndex);
const char *SSearchNoCase(const char *szString, const char *szIndex);
const char *SSearchIdentifier(const char *szString, const char *szIndex);
const char *SSearchFunction(const char *szString, const char *szIndex);

const char *SAdvanceSpace(const char *szSPos);
const char *SAdvancePast(const char *szSPos, char cPast);

bool SGetModule(const char *szList, int iIndex, char *sTarget, int iSize=-1);
bool SIsModule(const char *szList, const char *szString, int *ipIndex=NULL, bool fCase=false);
bool SAddModule(char *szList, const char *szModule);
bool SAddModules(char *szList, const char *szModules);
bool SRemoveModule(char *szList, const char *szModule, bool fCase=false);
int SModuleCount(const char *szList);

void SRemoveComments(char *szScript);
void SNewSegment(char *szStr, const char *szSepa=";");
void SCapitalize(char *szString);
void SWordWrap(char *szText, char cSpace, char cSepa, int iMaxLine);
int  SClearFrontBack(char *szString, char cClear=' ');

char *SGetFilename(const char *szFilepath);

#define LineFeed "\x00D\x00A"
#define EndOfFile "\x020"

void BufferBlit(BYTE *bypSource, int iSourcePitch, 
                int iSrcBufHgt, // Positive: Bottom up
                int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
                BYTE *bypTarget, int iTargetPitch, 
                int iTrgBufHgt, // Positive: Bottom up
                int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt);

void BufferBlitAspect(BYTE *bypSource, int iSourcePitch, 
                int iSrcBufHgt, // Positive: Bottom up
                int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
                BYTE *bypTarget, int iTargetPitch, 
                int iTrgBufHgt, // Positive: Bottom up
                int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt);

void StdBlit(BYTE *bypSource, int iSourcePitch, int iSrcBufHgt,
							int iSrcX, int iSrcY, int iSrcWdt, int iSrcHgt,
							BYTE *bypTarget, int iTargetPitch, int iTrgBufHgt,
							int iTrgX, int iTrgY, int iTrgWdt, int iTrgHgt,
							int iBytesPerPixel=1, bool fFlip=false);


#include <Fixed.h>

DWORD time_msecs();