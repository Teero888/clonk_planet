/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Executes C4Script */

const int C4ThreadMaxVar = C4MaxVariable, C4ThreadMaxPar = C4MaxVariable;

class C4ThreadError {
public:
  char Message[1024 + 1];
  const char *Position;
};

class C4Thread {
public:
  C4Thread();
  ~C4Thread();

public:
  C4Object *cObj;
  char Function[100 + 1];

  BOOL SkipNextStatement;     // Set by if
  BOOL JumpbackStatement;     // Set by while
  BOOL ReturnThread;          // Set by return
  BOOL NextStatementAdjacent; // Do not expect statement separator ';'

  intptr_t Variable[C4ThreadMaxVar];
  intptr_t Parameter[C4ThreadMaxPar];

protected:
  C4Thread *Caller;
  const char *Script;
  const char *cScr;
  C4ThreadError Error;

public:
  intptr_t Execute(C4Thread *pCaller, const char *szScript, const char *szFunction, const char *cpPosition, C4Object *pObj, intptr_t par0 = 0, intptr_t par1 = 0, intptr_t par2 = 0, intptr_t par3 = 0, intptr_t par4 = 0,
               intptr_t par5 = 0, intptr_t par6 = 0, intptr_t par7 = 0, intptr_t par8 = 0, intptr_t par9 = 0);
  void SetError(const char *szMessage);

protected:
  intptr_t Execute();
  intptr_t ExecuteStatement();
};
