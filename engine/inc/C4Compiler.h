/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Scan ini-type text component files to run-time data structures */

const int C4CV_String			= 1,
					C4CV_Id					= 2,
					C4CV_Integer		= 3,
					C4CV_NameList		= 4,
					C4CV_IdList			= 5,
					C4CV_ObjectList	= 6,

					C4CV_Section		= 10,
					C4CV_End				= 0;

const int C4EnumPointer1 = 1000000000,
					C4EnumPointer2 = 1001000000;

#define offsetof(s,m)	(size_t)&(((s *)0)->m)

class C4CompilerValue 
	{ 
	public:
		char *Name;
    int Type;
    int Offset;
    int Size;
  };

class C4Compiler
	{
	public:
		BOOL CompileStructure(C4CompilerValue *pRefs, const char *szSource, void *vpTarget, const char *szSecExt=NULL);
		BOOL DecompileStructure(C4CompilerValue *pRefs, void *vpData, void *vpDefault, char **ppOutput, int *ipSize, const char *szSecExt=NULL);
	protected:
		BOOL CompileLine(C4CompilerValue *pRefs, const char *szSection, const char *szLine, BYTE *pTarget, const char *szSecExt);
	};