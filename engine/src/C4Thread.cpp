/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Executes C4Script */

#include <C4Include.h>

extern char pscOSTR[500];

BOOL LooksLikeInteger(const char *str) {
  while (*str) {
    if (!Inside(*str, '0', '9') && (*str != '+') && (*str != '-'))
      return FALSE;
    str++;
  }
  return TRUE;
}

C4Thread::C4Thread() { ZeroMem(this, sizeof(C4Thread)); }

C4Thread::~C4Thread() {}

int SGetLine(const char *szText, const char *cpPosition) {
  if (!szText || !cpPosition)
    return 0;
  int iLines = 0;
  while (*szText && (szText < cpPosition)) {
    if (*szText == 0x0A)
      iLines++;
    szText++;
  }
  return iLines;
}

// by C4ScriptHost::FunctionCall, C4Thread::ExecuteStatement, C4Thread::Execute

void C4Thread::SetError(const char *szMessage) {

  // Line index
  char szLine[10] = "";
  int iLine = SGetLine(Script, cScr);
  if (iLine > 0)
    sprintf(szLine, "(%d)", 1 + iLine);

  // Store message & position
  sprintf(Error.Message, "%s::%s%s %s", cObj ? cObj->Def->Name : "Engine", Function[0] ? Function : "Edit", szLine, szMessage);

  Error.Position = cScr;

  // Console message
  Console.Out(Error.Message);

  // Debug mode object message
  if (Config.General.DebugMode)
    if (cObj)
      GameMsgObject(Error.Message, cObj);
    else
      GameMsgGlobal(Error.Message);
}

long C4Thread::ExecuteStatement() {

  // Advance to value or function
  cScr = SAdvanceSpace(cScr);

  // String value
  if (cScr[0] == '"') {
    // Return string pointer
    long rval = (long)cScr;
    // Advance to end of string
    cScr = SAdvancePast(cScr + 1, '"');
    return rval;
  }

  // Get identifier
  const int max_id_len = 50;
  char identifier[max_id_len + 1];
  SCopyIdentifier(cScr, identifier, max_id_len);

  // Advance to end of identifier
  cScr += SLen(identifier);

  // No identifier
  if (!identifier[0]) {
    SetError("Identifier expected");
    return FALSE;
  }

  // Integer value
  if (LooksLikeInteger(identifier)) {
    int rval;
    sscanf(identifier, "%d", &rval);
    return (long)rval;
  }

  // C4ID value
  if (LooksLikeID(identifier))
    return (long)C4Id(identifier);

  // Function
  int psc;
  for (psc = 0; C4ScriptFnMap[psc].Identifier; psc++)
    if (SEqual(identifier, C4ScriptFnMap[psc].Identifier))
      break;

  // Advance to parameter bracket
  cScr = SAdvanceSpace(cScr);

  // Function parameter bracket syntax error
  if (cScr[0] != '(') {
    SetError("'(' expected");
    return FALSE;
  }

  // Pass bracket
  cScr++;

  // Zero default parameters
  long parameter[C4ThreadMaxPar];
  int cpar;
  for (cpar = 0; cpar < C4ThreadMaxPar; cpar++)
    parameter[cpar] = 0;

  // Evaluate parameter list
  for (cpar = 0; cpar < C4ThreadMaxPar; cpar++) {
    // Advance space
    cScr = SAdvanceSpace(cScr);
    // End of parameter brackets (empty brackets)
    if (cScr[0] == ')') {
      cScr++;
      break;
    }
    // Evaluate parameter
    parameter[cpar] = ExecuteStatement();
    // Advance space
    cScr = SAdvanceSpace(cScr);
    // End of parameter brackets
    if (cScr[0] == ')') {
      cScr++;
      break;
    }
    // Parameter separator syntax error
    if (cScr[0] != ',') {
      SetError("',' expected");
      return FALSE;
    }
    // Pass parameter separator
    cScr++;
  }

  // Too many parameters
  if (cpar >= C4ThreadMaxPar) {
    SetError("Too many parameters");
    return FALSE;
  }

  // Engine function
  if (C4ScriptFnMap[psc].Identifier)
    return (long)C4ScriptFnMap[psc].Function(this, parameter[0], parameter[1], parameter[2], parameter[3], parameter[4], parameter[5], parameter[6], parameter[7], parameter[8], parameter[9], 0, 0);

  // Object script function
  if (cObj)
    return (long)cObj->Call(this, identifier, parameter[0], parameter[1], parameter[2], parameter[3], parameter[4], parameter[5], parameter[6], parameter[7], parameter[8], parameter[9]);

  // Global (scenario) script function
  return (long)Game.Script.Call(this, identifier, parameter[0], parameter[1], parameter[2], parameter[3], parameter[4], parameter[5], parameter[6], parameter[7], parameter[8], parameter[9]);
}

long C4Thread::Execute() {

  long rval_exec = 0;

  const char *cpStatement = cScr;
  bool fStatementDone = true;

  // Safety
  if (!cScr)
    return FALSE;

  // Execute statements
  while (TRUE) {
    // Assume regular connectivity
    NextStatementAdjacent = FALSE;

    // Beginning a new statement
    if (fStatementDone) {
      // Store statement beginning
      cpStatement = cScr;
      // Reset statement flag
      fStatementDone = false;
    }

    // Execute or skip statement
    if (SkipNextStatement) {
      // Pass statement separator
      cScr = SAdvancePast(cScr, ';');
      // Statement done
      fStatementDone = true;
      // Reset skip flag
      SkipNextStatement = FALSE;
    } else {
      // Execute statement
      rval_exec = ExecuteStatement();

      // Statement connectivity
      if (!NextStatementAdjacent) {
        // Advance to end of function statement
        cScr = SAdvanceSpace(cScr);
        // Function separator syntax error
        if (cScr[0] != ';')
          SetError("';' expected");
        // Pass statement separator
        cScr++;
        // Statement done
        fStatementDone = true;
      }
    }

    // Error: abort execution
    if (Error.Message[0]) {
      return FALSE;
    }

    // Return thread
    if (ReturnThread)
      return rval_exec;

    // Jumpback
    if (fStatementDone)
      if (JumpbackStatement) {
        cScr = cpStatement;
        JumpbackStatement = FALSE;
      }
  }
}

long C4Thread::Execute(C4Thread *pCaller, const char *szScript, const char *szFunction, const char *cpPosition, C4Object *pObj, long par0, long par1, long par2, long par3, long par4, long par5,
                       long par6, long par7, long par8, long par9) {

  // Set execution variables
  Caller = pCaller;
  Script = szScript;
  SCopy(szFunction, Function, 100);
  cScr = cpPosition;
  if (!cScr)
    cScr = szScript;
  cObj = pObj;
  Parameter[0] = par0;
  Parameter[1] = par1;
  Parameter[2] = par2;
  Parameter[3] = par3;
  Parameter[4] = par4;
  Parameter[5] = par5;
  Parameter[6] = par6;
  Parameter[7] = par7;
  Parameter[8] = par8;
  Parameter[9] = par9;

  // Execute
  return Execute();
}