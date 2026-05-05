/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Functions mapped by C4Script */

#include <C4Include.h>

//========================== Some Support Functions =======================================

char pscOSTR[500];

const int MaxFnStringParLen = 500;
char FnStringParBuf[MaxFnStringParLen + 1];
char FnStringFormatBuf[MaxFnStringParLen + 1];

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static int g_null_fd = -1;
static bool IsPointerValid(const void *ptr) {
  if (!ptr)
    return false;
  if ((uintptr_t)ptr < 65536)
    return false;
  if (g_null_fd == -1)
    g_null_fd = open("/dev/null", O_WRONLY);
  if (g_null_fd == -1)
    return true;
  return write(g_null_fd, ptr, 1) != -1 || errno != EFAULT;
}

static bool IsObjectValid(C4Object *pObj) {
  if (!IsPointerValid(pObj))
    return false;
  return pObj->Magic == 0x434F424A && pObj->Status;
}

const char *FnStringPar(const char *szPar) {
  FnStringParBuf[0] = 0;
  if (IsPointerValid(szPar))
    if (szPar[0] == '"')
      SCopyUntil(szPar + 1, FnStringParBuf, '"', MaxFnStringParLen);
    else
      SCopy(szPar, FnStringParBuf, MaxFnStringParLen);
  return FnStringParBuf;
}

const char *FnStringFormat(const char *szFormatPar, long iPar0 = 0, long iPar1 = 0, long iPar2 = 0, long iPar3 = 0, long iPar4 = 0, long iPar5 = 0, long iPar6 = 0, long iPar7 = 0, long iPar8 = 0,
                           long iPar9 = 0) {
  char szFormat[MaxFnStringParLen + 1];
  SCopy(FnStringPar(szFormatPar), szFormat);
  long iPar[C4ThreadMaxPar];
  iPar[0] = iPar0;
  iPar[1] = iPar1;
  iPar[2] = iPar2;
  iPar[3] = iPar3;
  iPar[4] = iPar4;
  iPar[5] = iPar5;
  iPar[6] = iPar6;
  iPar[7] = iPar7;
  iPar[8] = iPar8;
  iPar[9] = iPar9;

  FnStringFormatBuf[0] = 0;
  const char *cpFormat = szFormat;
  const char *cpType;
  char szField[MaxFnStringParLen + 1];
  int cPar = 0;
  while (*cpFormat) {
    // Copy normal stuff
    while (*cpFormat && (*cpFormat != '%'))
      SAppendChar(*cpFormat++, FnStringFormatBuf);
    // Field
    if (*cpFormat == '%') {
      // Scan field type
      for (cpType = cpFormat + 1; *cpType && (*cpType == '.' || Inside(*cpType, '0', '9')); cpType++)
        ;
      // Copy field
      SCopy(cpFormat, szField, cpType - cpFormat + 1);
      // Insert field by type
      switch (*cpType) {
      // Decimal
      case 'd':
        if (cPar < C4ThreadMaxPar)
          sprintf(FnStringFormatBuf + SLen(FnStringFormatBuf), szField, iPar[cPar++]);
        cpFormat += SLen(szField);
        break;
      // C4ID
      case 'i':
        if (cPar < C4ThreadMaxPar)
          SAppend(C4IdText(iPar[cPar++]), FnStringFormatBuf);
        cpFormat += SLen(szField);
        break;
      // String
      case 's':
        if (cPar < C4ThreadMaxPar)
          sprintf(FnStringFormatBuf + SLen(FnStringFormatBuf), szField, FnStringPar((const char *)(uintptr_t)iPar[cPar++]));
        cpFormat += SLen(szField);
        break;
      // Undefined / Empty
      default:
        SAppendChar(*cpFormat++, FnStringFormatBuf);
        break;
      }
    }
  }

  return FnStringFormatBuf;
}

BOOL CheckEnergyNeedChain(C4Object *pObj, C4ObjectList &rEnergyChainChecked) {

  if (!pObj)
    return FALSE;

  // No recursion, flag check
  if (rEnergyChainChecked.GetLink(pObj))
    return FALSE;
  rEnergyChainChecked.Add(pObj);

  // This object needs energy
  if (pObj->Def->LineConnect & C4D_Power_Consumer)
    if (pObj->NeedEnergy)
      return TRUE;

  // Check all power line connected structures
  C4Object *cline;
  C4ObjectLink *clnk;
  for (clnk = Game.Objects.First; clnk && (cline = clnk->Obj); clnk = clnk->Next)
    if (cline->Status)
      if (cline->Def->id == C4ID_PowerLine)
        if (cline->Action.Target == pObj)
          if (CheckEnergyNeedChain(cline->Action.Target2, rEnergyChainChecked))
            return TRUE;

  return FALSE;
}

#define BarrelFill Action.ComDir
#define MaxBarrelFill 200

C4Object *ObjectFindFillBarrel(C4Object *inobj, C4ID type) {
  C4ObjectLink *clnk;
  C4Object *cobj;
  // Look for same type barrel with space
  for (clnk = inobj->Contents.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Status)
      if (cobj->Def->id == type)
        if (cobj->BarrelFill < MaxBarrelFill)
          return cobj;
  // Look for empty barrel
  for (clnk = inobj->Contents.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Def->id == C4Id("BARL"))
      return cobj;

  return NULL;
}

DWORD StringBitEval(const char *str) {
  DWORD rval = 0;
  for (int cpos = 0; str && str[cpos]; cpos++)
    if ((str[cpos] != '_') && (str[cpos] != ' '))
      rval += 1 << cpos;
  return rval;
}

//=============================== C4Script Functions ====================================

C4Object *Fn_this(C4Thread *cthr) { return cthr->cObj; }

long Fn_return(C4Thread *cthr, long iReturnValue) {
  cthr->ReturnThread = TRUE;
  return iReturnValue;
}

long Fn_goto(C4Thread *cthr, long iCounter) {
  Game.Script.Counter = iCounter;
  cthr->ReturnThread = TRUE;
  return iCounter;
}

long Fn_if(C4Thread *cthr, long fCondition) {
  // Following statement expected to be adjacent
  cthr->NextStatementAdjacent = TRUE;
  // Condition true, continue with next statement
  if (fCondition)
    return TRUE;
  // Condition false, skip next statement
  cthr->SkipNextStatement = TRUE;
  return FALSE;
}

long Fn_while(C4Thread *cthr, long fCondition) {
  // Following statement expected to be adjacent
  cthr->NextStatementAdjacent = TRUE;
  // Condition false, skip next statement
  if (!fCondition) {
    cthr->SkipNextStatement = TRUE;
    return FALSE;
  }
  // Condition true, jump back to while statement at next statement separator
  cthr->JumpbackStatement = TRUE;
  return TRUE;
}

long FnChangeDef(C4Thread *cthr, long to_id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->ChangeDef(to_id);
}

long FnExplode(C4Thread *cthr, long iLevel, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Explode(iLevel);
  return TRUE;
}

long FnIncinerate(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  int iCausedBy = NO_OWNER;
  if (cthr->cObj)
    iCausedBy = cthr->cObj->Owner;
  return pObj->Incinerate(iCausedBy);
}

long FnExtinguish(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Extinguish();
}

long FnSetSolidMask(C4Thread *cthr, long iX, long iY, long iWdt, long iHgt, long iTX, long iTY, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->SetSolidMask(iX, iY, iWdt, iHgt, iTX, iTY);
  return TRUE;
}

long FnSetGravity(C4Thread *cthr, long iGravity) {
  Game.Landscape.Gravity = ftofix(0.2 * (float)BoundBy(iGravity, 5, 300) / 100.0);
  return TRUE;
}

long FnGetGravity(C4Thread *cthr) { return (int)(100.0 * fixtof(Game.Landscape.Gravity) / 0.2); }

long FnDeathAnnounce(C4Thread *cthr) {
  const int MaxDeathMsg = 7;
  if (!cthr->cObj)
    return FALSE;
  sprintf(pscOSTR, LoadResStr(IDS_OBJ_DEATH1 + (WORD)Random(MaxDeathMsg)), cthr->cObj->GetName());
  GameMsgObject(pscOSTR, cthr->cObj);
  return TRUE;
}

long FnGrabContents(C4Thread *cthr, C4Object *from) {
  if (!cthr->cObj)
    return FALSE;
  if (!IsObjectValid(from))
    return FALSE;
  C4Object *thing;
  while (thing = from->Contents.GetObject())
    thing->Enter(cthr->cObj);
  return TRUE;
}

long FnPunch(C4Thread *cthr, C4Object *target, long punch) {
  if (!cthr->cObj)
    return FALSE;
  return ObjectComPunch(cthr->cObj, target, punch);
}

long FnFling(C4Thread *cthr, C4Object *pObj, long iXDir, long iYDir) {
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Fling(itofix(iXDir), itofix(iYDir));
  return TRUE;
}

long FnJump(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return ObjectComJump(pObj);
}

long FnEnter(C4Thread *cthr, C4Object *pTarget, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Enter(pTarget);
}

long FnExit(C4Thread *cthr, C4Object *pObj, long tx, long ty, long tr, long txdir, long tydir, long trdir) {
  if (!IsObjectValid(pObj))
    return FALSE;
  if (cthr->cObj) {
    tx += cthr->cObj->x;
    ty += cthr->cObj->y;
  }
  if (tr == -1)
    tr = Random(360);
  ObjectComCancelAttach(pObj);
  return pObj->Exit(tx, ty + pObj->Shape.y, tr, itofix(txdir), itofix(tydir), ftofix((float)trdir / 10.0));
}

long FnSplit2Components(C4Thread *cthr, C4Object *pObj) {
  C4Object *pThing, *pNew, *pContainer;
  int cnt, cnt2;
  // Pointer required
  if (!IsObjectValid(pObj))
    return FALSE;
  // Store container
  pContainer = pObj->Contained;
  // Contents: exit / transfer to container
  while (pThing = pObj->Contents.GetObject())
    if (pContainer)
      pThing->Enter(pContainer);
    else
      pThing->Exit(pThing->x, pThing->y);
  // Destroy the object, create its components
  if (pObj->Contained)
    pObj->Exit(pObj->x, pObj->y);
  for (cnt = 0; pObj->Component.GetID(cnt); cnt++)
    for (cnt2 = 0; cnt2 < pObj->Component.GetCount(cnt); cnt2++)
      if (pNew = Game.CreateObject(pObj->Component.GetID(cnt), pObj->Owner, pObj->x, pObj->y, Random(360), itofix(Rnd3()), itofix(Rnd3()), itofix(Rnd3()))) {
        if (pObj->OnFire)
          pNew->Incinerate(pObj->Owner);
        if (pContainer)
          pNew->Enter(pContainer);
      }
  pObj->AssignRemoval();
  return TRUE;
}

long FnRemoveObject(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->AssignRemoval();
  return TRUE;
}

long FnForcePosition(C4Thread *cthr, C4Object *pObj, long tx, long ty) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->ForcePosition(tx, ty);
  return TRUE;
}

long FnSetPosition(C4Thread *cthr, long iX, long iY, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->ForcePosition(iX, iY);
  return TRUE;
}

long FnDoCon(C4Thread *cthr, long iChange, C4Object *pObj) // in percent
{
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->DoCon(FullCon * iChange / 100);
  return TRUE;
}

long FnGetCon(C4Thread *cthr, C4Object *pObj) // in percent
{
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return 100 * pObj->Con / FullCon;
}

long FnDoEnergy(C4Thread *cthr, long iChange, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->DoEnergy(iChange);
  return TRUE;
}

long FnDoBreath(C4Thread *cthr, long iChange, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->DoBreath(iChange);
  return TRUE;
}

long FnDoDamage(C4Thread *cthr, long iChange, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  int iCausedBy = NO_OWNER;
  if (cthr->cObj)
    iCausedBy = cthr->cObj->Owner;
  pObj->DoDamage(iChange, iCausedBy);
  return TRUE;
}

long FnDoMagicEnergy(C4Thread *cthr, long iChange, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Physical modification factor
  iChange *= MagicPhysicalFactor;
  // Maximum load
  if (iChange > 0)
    if (pObj->MagicEnergy + iChange > pObj->GetPhysical()->Magic)
      return FALSE;
  // Insufficient load
  if (iChange < 0)
    if (pObj->MagicEnergy + iChange < 0)
      return FALSE;
  // Change energy level
  pObj->MagicEnergy = BoundBy(pObj->MagicEnergy + iChange, 0, pObj->GetPhysical()->Magic);
  pObj->ViewEnergy = C4ViewDelay;
  return TRUE;
}

long FnGetMagicEnergy(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->MagicEnergy / MagicPhysicalFactor;
}

#define offsC4PH(x) offsetof(C4PhysicalInfo, x)

int PhysicalOffset(const char *szPhysical) {

  C4CompilerValue C4CR_Physical[] = {

      {"Physical", C4CV_Section, 0, 0},
      {"Energy", C4CV_Integer, offsC4PH(Energy), 1},
      {"Breath", C4CV_Integer, offsC4PH(Breath), 1},
      {"Walk", C4CV_Integer, offsC4PH(Walk), 1},
      {"Jump", C4CV_Integer, offsC4PH(Jump), 1},
      {"Scale", C4CV_Integer, offsC4PH(Scale), 1},
      {"Hangle", C4CV_Integer, offsC4PH(Hangle), 1},
      {"Dig", C4CV_Integer, offsC4PH(Dig), 1},
      {"Swim", C4CV_Integer, offsC4PH(Swim), 1},
      {"Throw", C4CV_Integer, offsC4PH(Throw), 1},
      {"Push", C4CV_Integer, offsC4PH(Push), 1},
      {"Fight", C4CV_Integer, offsC4PH(Fight), 1},
      {"Magic", C4CV_Integer, offsC4PH(Magic), 1},
      {"Float", C4CV_Integer, offsC4PH(Float), 1},
      {"CanScale", C4CV_Integer, offsC4PH(CanScale), 1},
      {"CanHangle", C4CV_Integer, offsC4PH(CanHangle), 1},
      {"CanDig", C4CV_Integer, offsC4PH(CanDig), 1},
      {"CanConstruct", C4CV_Integer, offsC4PH(CanConstruct), 1},
      {"CanChop", C4CV_Integer, offsC4PH(CanChop), 1},
      {"CanSwimDig", C4CV_Integer, offsC4PH(CanSwimDig), 1},
      {"CorrosionResist", C4CV_Integer, offsC4PH(CorrosionResist), 1},
      {"BreatheWater", C4CV_Integer, offsC4PH(BreatheWater), 1},

      {NULL, C4CV_End, 0, 0}};

  C4CompilerValue *cRef;
  // Find reference in correct section
  char szCurrentSection[100 + 1];
  szCurrentSection[0] = 0;
  for (cRef = C4CR_Physical; cRef->Type != C4CV_End; cRef++) {
    // New section
    if (cRef->Type == C4CV_Section) {
      SCopy(cRef->Name, szCurrentSection);
    }
    // Value
    else {
      // Match section, match value
      if (SEqual("Physical", szCurrentSection))
        if (SEqual(szPhysical, cRef->Name))
          return cRef->Offset;
    }
  }

  // Reference not found
  return -1;
}

long FnSetPhysical(C4Thread *cthr, const char *szPhysical, long iValue, long iMode, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Get physical offset
  int iOffset = PhysicalOffset(FnStringPar(szPhysical));
  if (iOffset < 0)
    return FALSE;
  // Set by mode
  switch (iMode) {
  // Currently active physical
  case 0:
    // Info objects or temporary mode only
    if (!pObj->Info && !pObj->PhysicalTemporary)
      return FALSE;
    // Set physical
    *((int *)(((BYTE *)(pObj->GetPhysical())) + iOffset)) = BoundBy(iValue, 0, 1000000);
    return TRUE;
  // Permanent physical
  case 1:
    // Info objects only
    if (!pObj->Info)
      return FALSE;
    // Set physical
    *((int *)(((BYTE *)&(pObj->Info->Physical)) + iOffset)) = BoundBy(iValue, 0, 1000000);
    return TRUE;
  // Temporary physical
  case 2:
    // Automatically switch to temporary mode
    if (!pObj->PhysicalTemporary) {
      pObj->TemporaryPhysical = *(pObj->GetPhysical());
      pObj->PhysicalTemporary = TRUE;
    }
    // Set physical
    *((int *)(((BYTE *)&(pObj->TemporaryPhysical)) + iOffset)) = BoundBy(iValue, 0, 1000000);
    return TRUE;
  }
  // Invalid mode
  return FALSE;
}

long FnResetPhysical(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;

  // Info objects only
  if (!pObj->Info)
    return FALSE;

  // Reset to permanent physical
  if (pObj->PhysicalTemporary) {
    // Switch to permanent mode
    pObj->TemporaryPhysical.Default();
    pObj->PhysicalTemporary = FALSE;
  }

  return TRUE;
}

long FnGetPhysical(C4Thread *cthr, const char *szPhysical, long iMode, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Get physical offset
  int iOffset = PhysicalOffset(FnStringPar(szPhysical));
  if (iOffset < 0)
    return FALSE;
  // Get by mode
  switch (iMode) {
  // Currently active physical
  case 0:
    // Get physical
    return *((int *)(((BYTE *)(pObj->GetPhysical())) + iOffset));
  // Permanent physical
  case 1:
    // Info objects only
    if (!pObj->Info)
      return FALSE;
    // Get physical
    return *((int *)(((BYTE *)&(pObj->Info->Physical)) + iOffset));
  // Temporary physical
  case 2:
    // Info objects only
    if (!pObj->Info)
      return FALSE;
    // Only if in temporary mode
    if (!pObj->PhysicalTemporary)
      return FALSE;
    // Get physical
    return *((int *)(((BYTE *)&(pObj->TemporaryPhysical)) + iOffset));
  }
  // Invalid mode
  return FALSE;
}

long FnSetEntrance(C4Thread *cthr, long e_status) {
  if (!cthr->cObj)
    return FALSE;
  cthr->cObj->EntranceStatus = e_status;
  return TRUE;
}

long FnSetXDir(C4Thread *cthr, long nxdir, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->xdir = ftofix((float)nxdir / 10.0);
  pObj->Mobile = 1;
  return TRUE;
}

long FnSetRDir(C4Thread *cthr, long nrdir, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->rdir = ftofix((float)nrdir / 10.0);
  pObj->Mobile = 1;
  return TRUE;
}

long FnSetYDir(C4Thread *cthr, long nydir, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->ydir = ftofix((float)nydir / 10.0);
  pObj->Mobile = 1;
  return TRUE;
}

long FnSetR(C4Thread *cthr, long nr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->r = BoundBy(nr, 0, 359);
  pObj->fix_r = itofix(pObj->r);
  pObj->UpdateFace();
  return TRUE;
}

long FnSetAction(C4Thread *cthr, const char *szAction, C4Object *pTarget, C4Object *pTarget2) {
  if (!cthr->cObj)
    return FALSE;
  if (!szAction)
    return FALSE;
  return cthr->cObj->SetActionByName(FnStringPar(szAction), pTarget, pTarget2);
}

long FnSetActionData(C4Thread *cthr, long iData, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Action.Data = iData;
  return TRUE;
}

long FnObjectSetAction(C4Thread *cthr, C4Object *pObj, const char *szAction, C4Object *pTarget, C4Object *pTarget2) {
  if (!szAction || !IsObjectValid(pObj))
    return FALSE;
  return pObj->SetActionByName(FnStringPar(szAction), pTarget, pTarget2);
}

long FnSetComDir(C4Thread *cthr, long ncomdir, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Action.ComDir = ncomdir;
  return TRUE;
}

long FnSetDir(C4Thread *cthr, long ndir, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->SetDir(ndir);
  return TRUE;
}

long FnSetCategory(C4Thread *cthr, long iCategory, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Category = iCategory;
  pObj->Resort();
  pObj->SetOCF();
  return TRUE;
}

long FnSetAlive(C4Thread *cthr, long nalv, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  pObj->Alive = nalv;
  return TRUE;
}

long FnSetOwner(C4Thread *cthr, long iOwner, C4Object *pObj) {
  // Object safety
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Check valid owner
  if (!(ValidPlr(iOwner) || (iOwner == NO_OWNER)))
    return FALSE;
  // Set owner
  pObj->Owner = iOwner;
  // Set color
  if (pObj->Def->ColorByOwner)
    if (ValidPlr(pObj->Owner)) {
      pObj->Color = BoundBy(Game.Players.Get(pObj->Owner)->Color, 0, C4MaxColor - 1);
      pObj->UpdateFace();
    }
  return TRUE;
}

long FnSetPhase(C4Thread *cthr, long iVal, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->SetPhase(iVal);
}

long FnSetCommand(C4Thread *cthr, C4Object *pObj, const char *szCommand, C4Object *pTarget, long iTx, long iTy, C4Object *pTarget2, long iData, long iRetries) {
  // Object
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj) || !szCommand)
    return FALSE;
  // Command
  int iCommand = CommandByName(FnStringPar(szCommand));
  if (!iCommand) {
    pObj->ClearCommands();
    return FALSE;
  }
  // Special: convert iData to szText
  const char *szText = NULL;
  if (iCommand == C4CMD_Call) {
    szText = FnStringPar((const char *)(uintptr_t)iData);
    iData = 0;
  }
  // Set
  pObj->SetCommand(iCommand, pTarget, iTx, iTy, pTarget2, FALSE, iData, iRetries, szText);
  // Success
  return TRUE;
}

long FnAddCommand(C4Thread *cthr, C4Object *pObj, const char *szCommand, C4Object *pTarget, long iTx, long iTy, C4Object *pTarget2, long iUpdateInterval, long iData, long iRetries) {
  // Object
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj) || !szCommand)
    return FALSE;
  // Command
  int iCommand = CommandByName(FnStringPar(szCommand));
  if (!iCommand)
    return FALSE;
  // Special: convert iData to szText
  const char *szText = NULL;
  if (iCommand == C4CMD_Call) {
    szText = FnStringPar((const char *)(uintptr_t)iData);
    iData = 0;
  }
  // Add
  return pObj->AddCommand(iCommand, pTarget, iTx, iTy, iUpdateInterval, pTarget2, TRUE, iData, FALSE, iRetries, szText);
}

long FnAppendCommand(C4Thread *cthr, C4Object *pObj, const char *szCommand, C4Object *pTarget, long iTx, long iTy, C4Object *pTarget2, long iUpdateInterval, long iData, long iRetries) {
  // Object
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj) || !szCommand)
    return FALSE;
  // Command
  int iCommand = CommandByName(FnStringPar(szCommand));
  if (!iCommand)
    return FALSE;
  // Special: convert iData to szText
  const char *szText = NULL;
  if (iCommand == C4CMD_Call) {
    szText = FnStringPar((const char *)(uintptr_t)iData);
    iData = 0;
  }
  // Add
  return pObj->AddCommand(iCommand, pTarget, iTx, iTy, iUpdateInterval, pTarget2, TRUE, iData, TRUE, iRetries, szText);
}

long FnGetCommand(C4Thread *cthr, C4Object *pObj, long iElement) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Object has no command
  if (!pObj->Command)
    return 0;
  // Return command element
  switch (iElement) {
  case 0: // Name
    return (long)CommandName(pObj->Command->Command);
  case 1: // Target
    return (long)pObj->Command->Target;
  case 2: // Tx
    return pObj->Command->Tx;
  case 3: // Ty
    return pObj->Command->Ty;
  case 4: // Target2
    return (long)pObj->Command->Target2;
  }
  // Undefined element
  return 0;
}

const char *FnGetAction(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (pObj->Action.Act <= ActIdle)
    return "Idle";
  return pObj->Def->ActMap[pObj->Action.Act].Name;
}

const char *FnGetName(C4Thread *cthr, C4Object *pObj, C4ID idDef) {
  // Def name
  C4Def *pDef;
  if (idDef)
    if (pDef = C4Id2Def(idDef))
      return pDef->Name;
  // Object name
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->GetName();
}

const char *FnGetDesc(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Def->Desc;
}

const char *FnGetPlayerName(C4Thread *cthr, int iPlayer) {
  if (!ValidPlr(iPlayer))
    return NULL;
  return Game.Players.Get(iPlayer)->Name;
}

C4Object *FnGetActionTarget(C4Thread *cthr, int target_index, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  if (target_index == 0)
    return pObj->Action.Target;
  if (target_index == 1)
    return pObj->Action.Target2;
  return NULL;
}

long FnGetDir(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Action.Dir;
}

long FnGetEntrance(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->EntranceStatus;
}

long FnGetPhase(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Action.Phase;
}

long FnGetEnergy(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return 100 * pObj->Energy / C4MaxPhysical;
}

long FnGetBreath(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return 100 * pObj->Breath / C4MaxPhysical;
}

long FnGetMass(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return pObj->Mass;
}

long FnGetRDir(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return fixtoi(pObj->rdir * 10);
}

long FnGetXDir(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return fixtoi(pObj->xdir * 10);
}

long FnGetYDir(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return fixtoi(pObj->ydir * 10);
}

long FnGetR(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  // Adjust range
  int iR = pObj->r;
  while (iR > 180)
    iR -= 360;
  while (iR < -180)
    iR += 360;
  return iR;
}

long FnGetComDir(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Action.ComDir;
}

long FnGetX(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->x;
}

long FnGetVertexNum(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Shape.VtxNum;
}

long FnGetVertex(C4Thread *cthr, long iIndex, long fYCoordinate, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (pObj->Shape.VtxNum < 1)
    return FALSE;
  iIndex = Min(iIndex, pObj->Shape.VtxNum - 1);
  if (fYCoordinate)
    return pObj->Shape.VtxY[iIndex];
  return pObj->Shape.VtxX[iIndex];
}

long FnSetVertex(C4Thread *cthr, long iIndex, long fYCoordinate, long iValue, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (!Inside(iIndex, 0, C4D_MaxVertex - 1))
    return FALSE;
  if (fYCoordinate)
    pObj->Shape.VtxY[iIndex] = iValue;
  else
    pObj->Shape.VtxX[iIndex] = iValue;
  return TRUE;
}

long FnAddVertex(C4Thread *cthr, long iX, long iY, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Shape.AddVertex(iX, iY);
}

long FnGetY(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->y;
}

long FnGetAlive(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Alive;
}

long FnGetOwner(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return -1;
  return pObj->Owner;
}

long FnCrewMember(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Def->CrewMember;
}

long FnGetController(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return -1;
  return pObj->Controller;
}

long FnGetCategory(C4Thread *cthr, C4Object *pObj, long idDef) {
  // Def category
  C4Def *pDef;
  if (idDef)
    if (pDef = C4Id2Def(idDef))
      return pDef->Category;
  // Object category
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return pObj->Category;
}

long FnGetOCF(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->OCF;
}

long FnGetDamage(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Damage;
}

long FnGetValue(C4Thread *cthr, C4Object *pObj, long idDef) {
  // Def value
  C4Def *pDef;
  if (idDef)
    if (pDef = C4Id2Def(idDef))
      return pDef->Value;
  // Object value
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->GetValue();
}

long FnGetRank(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (!pObj->Info)
    return 0;
  return pObj->Info->Rank;
}

long FnValue(C4Thread *cthr, long id) {
  C4Def *pDef = C4Id2Def(id);
  if (pDef)
    return pDef->Value;
  return 0;
}

long FnGetActTime(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Action.Time;
}

C4ID FnGetID(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return C4ID_None;
  return pObj->Def->id;
}

long FnGetBase(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return -1;
  return pObj->Base;
}

long FnGetMenu(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return -1;
  if (pObj->Menu && pObj->Menu->IsActive())
    return pObj->Menu->GetIdentification();
  return C4MN_None;
}

long FnCreateMenu(C4Thread *cthr, long iSymbol, C4Object *pMenuObj, C4Object *pCommandObj, long iExtra, const char *szCaption, long iExtraData, long iStyle, long fPermanent) {
  if (!pMenuObj)
    pMenuObj = cthr->cObj;
  if (!IsObjectValid(pMenuObj))
    return FALSE;
  if (!pCommandObj)
    pCommandObj = cthr->cObj;
  if (!IsObjectValid(pCommandObj))
    return FALSE;

  // Create symbol
  C4Def *pDef;
  C4FacetEx fctSymbol;
  fctSymbol.Create(C4SymbolSize, C4SymbolSize);
  if (pDef = C4Id2Def(iSymbol))
    pDef->Draw(fctSymbol);

  // Clear any old menu, init new menu
  pMenuObj->CloseMenu();
  pMenuObj->Menu = new C4Menu;
  pMenuObj->Menu->Init(fctSymbol, FnStringPar(szCaption), pCommandObj, iExtra, iExtraData, iSymbol, iStyle);

  // Set permanent
  pMenuObj->Menu->SetPermanent(fPermanent);

  // Default symbol to avoid destruction
  fctSymbol.Default();

  return TRUE;
}

long FnAddMenuItem(C4Thread *cthr, const char *szCaption, const char *szCommand, long idItem, C4Object *pMenuObj, long iCount, long iParameter, const char *szInfoCaption) {
  if (!pMenuObj)
    pMenuObj = cthr->cObj;
  if (!IsObjectValid(pMenuObj))
    return FALSE;
  if (!pMenuObj->Menu)
    return FALSE;

  char caption[256 + 1];
  char command[256 + 1];
  char infocaption[C4MaxTitle + 1];

  // Check specified def
  C4Def *pDef = C4Id2Def(idItem);
  if (!pDef)
    pDef = pMenuObj->Def;

  // Compose caption with def name
  sprintf(caption, FnStringPar(szCaption), pDef->Name);

  // Native script command
  if (SCharCount('(', FnStringPar(szCommand)))
    sprintf(command, FnStringPar(szCommand));
  // Compose command with id and parameter (old style)
  else
    sprintf(command, "%s(%s,%i)", FnStringPar(szCommand), C4IdText(idItem), iParameter);

  // Info caption
  SCopy(FnStringPar(szInfoCaption), infocaption, C4MaxTitle);
  // Default info caption by def desc
  if (!infocaption[0])
    SCopy(pDef->Desc, infocaption, C4MaxTitle);

  // Create symbol by def
  C4FacetEx fctSymbol;
  fctSymbol.Create(C4SymbolSize, C4SymbolSize);
  pDef->Draw(fctSymbol);

  // Convert default zero count to no count
  if (iCount == 0)
    iCount = C4MN_Item_NoCount;

  // Add menu item
  pMenuObj->Menu->Add(caption, fctSymbol, command, iCount, NULL, iParameter, infocaption, idItem);

  // Default symbol to avoid destruction
  fctSymbol.Default();

  return TRUE;
}

long FnSelectMenuItem(C4Thread *cthr, long iItem, C4Object *pMenuObj) {
  if (!pMenuObj)
    pMenuObj = cthr->cObj;
  if (!IsObjectValid(pMenuObj))
    return FALSE;
  if (!pMenuObj->Menu)
    return FALSE;
  return pMenuObj->Menu->SetSelection(iItem);
}

// Check / Status

C4Object *FnContained(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->Contained;
}

C4Object *FnContents(C4Thread *cthr, int index, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  // Special: objects attaching to another object
  //          cannot be accessed by FnContents
  C4Object *cobj;
  while (cobj = pObj->Contents.GetObject(index)) {
    if (cobj->GetProcedure() != DFA_ATTACH)
      return cobj;
    index++;
  }
  return NULL;
}

long FnShiftContents(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;

  // One object type only
  if (pObj->Contents.ListIDCount(C4D_All) < 2)
    return FALSE;

  // Move first object to tail until new id is at front
  C4ID c_id = pObj->Contents.GetObject()->id;

  while (pObj->Contents.GetObject()->id == c_id) {
    C4Object *pMove = pObj->Contents.GetObject();
    if (!pMove)
      return FALSE;

    pObj->Contents.Remove(pMove);
    pObj->Contents.Add(pMove, FALSE);
  }

  return TRUE;
}

C4Object *FnScrollContents(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;

  C4Object *pMove = pObj->Contents.GetObject();
  if (pMove) {
    pObj->Contents.Remove(pMove);
    pObj->Contents.Add(pMove, FALSE);
  }

  return pObj->Contents.GetObject();
}

long FnContentsCount(C4Thread *cthr, long id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return pObj->Contents.ObjectCount(id);
}

C4Object *FnFindContents(C4Thread *cthr, C4ID c_id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->Contents.Find(c_id);
}

C4Object *FnFindOtherContents(C4Thread *cthr, C4ID c_id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->Contents.FindOther(c_id);
}

long FnActIdle(C4Thread *cthr) {
  if (!cthr->cObj)
    return FALSE;
  if (cthr->cObj->Action.Act == ActIdle)
    return TRUE;
  return FALSE;
}

long FnCheckEnergyNeedChain(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  C4ObjectList EnergyChainChecked;
  return CheckEnergyNeedChain(pObj, EnergyChainChecked);
}

long FnEnergyCheck(C4Thread *cthr, long energy, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (!(Game.Rules & C4RULE_StructuresNeedEnergy) || (pObj->Energy >= energy) || !(pObj->Def->LineConnect & C4D_Power_Consumer)) {
    pObj->NeedEnergy = 0;
    return TRUE;
  }
  pObj->NeedEnergy = 1;
  return FALSE;
}

long FnStuck(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Shape.CheckContact(pObj->x, pObj->y);
}

long FnInLiquid(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->InLiquid;
}

long FnOnFire(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->OnFire;
}

long FnComponentAll(C4Thread *cthr, C4Object *pObj, long c_id) {
  int cnt;
  if (!IsObjectValid(pObj))
    return FALSE;
  for (cnt = 0; pObj->Def->Component.GetID(cnt); cnt++)
    if (pObj->Def->Component.GetID(cnt) != c_id)
      if (pObj->Def->Component.GetCount(cnt) > 0)
        return FALSE;
  return TRUE;
}

C4Object *FnCreateObject(C4Thread *cthr, C4ID id, int iXOffset, int iYOffset, int iOwner) {
  if (cthr->cObj) // Local object calls override
  {
    iXOffset += cthr->cObj->x;
    iYOffset += cthr->cObj->y;
    iOwner = cthr->cObj->Owner;
  }

  return Game.CreateObject(id, iOwner, iXOffset, iYOffset);
}

C4Object *FnCreateConstruction(C4Thread *cthr, C4ID id, int iXOffset, int iYOffset, int iOwner, int iCompletion, int fTerrain, int fCheckSite) {
  // Local object calls override position offset, owner
  if (cthr->cObj) {
    iXOffset += cthr->cObj->x;
    iYOffset += cthr->cObj->y;
    iOwner = cthr->cObj->Owner;
  }

  // Check site
  if (fCheckSite)
    if (!ConstructionCheck(id, iXOffset, iYOffset, cthr->cObj))
      return NULL;

  // Create site object
  return Game.CreateObjectConstruction(id, iOwner, iXOffset, iYOffset, iCompletion * FullCon / 100, fTerrain);
}

C4Object *FnCreateContents(C4Thread *cthr, C4ID c_id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->CreateContents(c_id);
}

C4Object *FnComposeContents(C4Thread *cthr, C4ID c_id, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return NULL;
  return pObj->ComposeContents(c_id);
}

long FnFindConstructionSite(C4Thread *cthr, long id, long iVarX, long iVarY) {

  C4Def *pDef;
  if (!(pDef = C4Id2Def(id)))
    return FALSE;
  if (!Inside(iVarX, 0, C4ThreadMaxVar - 1) || !Inside(iVarY, 0, C4ThreadMaxVar - 1))
    return FALSE;

  // Construction check at starting position
  if (ConstructionCheck(id, cthr->Variable[iVarX], cthr->Variable[iVarY]))
    return TRUE;

  // Search
  return FindConSiteSpot(cthr->Variable[iVarX], cthr->Variable[iVarY], pDef->Shape.Wdt, pDef->Shape.Hgt, pDef->Category, 20);
}

C4Object *FnFindBase(C4Thread *cthr, int iOwner, int iIndex) { return Game.FindBase(iOwner, iIndex); }

long FnObjectCount(C4Thread *cthr, long id, long x, long y, long wdt, long hgt, long dwOCF, const char *szAction, C4Object *pActionTarget, C4Object *pContainer, long iOwner) {
  // Local call adjust coordinates
  if (cthr->cObj)
    if (x || y || wdt || hgt) // if not default full range
    {
      x += cthr->cObj->x;
      y += cthr->cObj->y;
    }
  // Adjust default ocf
  if (dwOCF == 0)
    dwOCF = OCF_All;
  // Adjust default owner
  if (iOwner == 0)
    iOwner = ANY_OWNER; // imcomplete useless implementation
  // Find object
  return Game.ObjectCount(id, x, y, wdt, hgt, dwOCF, FnStringPar(szAction), pActionTarget,
                          cthr->cObj, // Local calls exclude self
                          pContainer, iOwner);
}

C4Object *FnFindObject(C4Thread *cthr, C4ID id, int x, int y, int wdt, int hgt, int dwOCF, const char *szAction, C4Object *pActionTarget, C4Object *pContainer, C4Object *pFindNext) {
  // Local call adjust coordinates
  if (cthr->cObj)
    if (x || y || wdt || hgt) // if not default full range
    {
      x += cthr->cObj->x;
      y += cthr->cObj->y;
    }
  // Adjust default ocf
  if (dwOCF == 0)
    dwOCF = OCF_All;
  // Find object
  return Game.FindObject(id, x, y, wdt, hgt, dwOCF, FnStringPar(szAction), pActionTarget,
                         cthr->cObj, // Local calls exclude self
                         pContainer, ANY_OWNER, pFindNext);
}

long FnMakeCrewMember(C4Thread *cthr, C4Object *pObj, long iPlayer) {
  if (!IsObjectValid(pObj) || !ValidPlr(iPlayer))
    return FALSE;
  return Game.Players.Get(iPlayer)->MakeCrewMember(pObj);
}

long FnFlameConsumeMaterial(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  int mat = GBackMat(x, y);
  if (!MatValid(mat))
    return FALSE;
  if (!Game.Material.Map[mat].Inflammable)
    return FALSE;
  if (Game.Landscape.ExtractMaterial(x, y) == MNone)
    return FALSE;
  return TRUE;
}

long FnSmoke(C4Thread *cthr, long tx, long ty, long level) {
  if (cthr->cObj) {
    tx += cthr->cObj->x;
    ty += cthr->cObj->y;
  }
  Smoke(tx, ty, level);
  return TRUE;
}

long FnBubble(C4Thread *cthr, long tx, long ty) {
  if (cthr->cObj) {
    tx += cthr->cObj->x;
    ty += cthr->cObj->y;
  }
  BubbleOut(tx, ty);
  return TRUE;
}

long FnExtractLiquid(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  if (!GBackLiquid(x, y))
    return MNone;
  return Game.Landscape.ExtractMaterial(x, y);
}

long FnInsertMaterial(C4Thread *cthr, long mat, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return Game.Landscape.InsertMaterial(mat, x, y);
}

long FnObjectInsertMaterial(C4Thread *cthr, long mat, C4Object *target) {
  if (!IsObjectValid(target))
    return FALSE;
  if (mat == MNone)
    return FALSE;

  // If common liquid, fill contained barrels (hardcoded crap)
  C4Object *barrel;
  C4ID barreltype = C4ID_None;
  if (mat == Game.Material.Get("Water"))
    barreltype = C4Id("WBRL");
  if (mat == Game.Material.Get("Acid"))
    barreltype = C4Id("ABRL");
  if (mat == Game.Material.Get("Oil"))
    barreltype = C4Id("OBRL");
  if ((mat == Game.Material.Get("Lava")) || (mat == Game.Material.Get("DuroLava")))
    barreltype = C4Id("LBRL");
  if (barreltype)
    if (barrel = ObjectFindFillBarrel(target, barreltype)) {
      if (barrel->Def->id != barreltype)
        barrel->ChangeDef(barreltype);
      barrel->BarrelFill++;
      return TRUE;
    }

  return Game.Landscape.InsertMaterial(mat, target->x, target->y);
}

long FnObjectExtractLiquid(C4Thread *cthr, C4Object *from) {
  if (!IsObjectValid(from))
    return MNone;

  C4ObjectLink *clnk;
  C4Object *pObj;

  // Look for barrel with liquid
  for (clnk = from->Contents.First; clnk && (pObj = clnk->Obj); clnk = clnk->Next)
    if (pObj->Status) {
      if (pObj->Def->id == C4Id("WBRL"))
        if (pObj->BarrelFill > 0) {
          pObj->BarrelFill--;
          if (pObj->BarrelFill < 1)
            pObj->ChangeDef(C4Id("BARL"));
          return Game.Material.Get("Water");
        }
      if (pObj->Def->id == C4Id("ABRL"))
        if (pObj->BarrelFill > 0) {
          pObj->BarrelFill--;
          if (pObj->BarrelFill < 1)
            pObj->ChangeDef(C4Id("BARL"));
          return Game.Material.Get("Acid");
        }
      if (pObj->Def->id == C4Id("OBRL"))
        if (pObj->BarrelFill > 0) {
          pObj->BarrelFill--;
          if (pObj->BarrelFill < 1)
            pObj->ChangeDef(C4Id("BARL"));
          return Game.Material.Get("Oil");
        }
      if (pObj->Def->id == C4Id("LBRL"))
        if (pObj->BarrelFill > 0) {
          pObj->BarrelFill--;
          if (pObj->BarrelFill < 1)
            pObj->ChangeDef(C4Id("BARL"));
          return Game.Material.Get("Lava");
        }
    }

  return MNone;
}

long FnGetMaterialCount(C4Thread *cthr, long iMaterial) { return Game.Landscape.GetMaterialCount(iMaterial); }

long FnGetMaterial(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return GBackMat(x, y);
}

long FnGBackSolid(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return GBackSolid(x, y);
}

long FnGBackSemiSolid(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return GBackSemiSolid(x, y);
}

long FnGBackLiquid(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return GBackLiquid(x, y);
}

long FnExtractMaterialAmount(C4Thread *cthr, long x, long y, long mat, long amount) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  for (int extracted = 0; extracted < amount; extracted++) {
    if (GBackMat(x, y) != mat)
      return extracted;
    if (Game.Landscape.ExtractMaterial(x, y) != mat)
      return extracted;
  }
  return amount;
}

long FnBlastObjects(C4Thread *cthr, long iX, long iY, long iLevel, C4Object *pInObj) {
  int iCausedBy = NO_OWNER;
  if (cthr->cObj)
    iCausedBy = cthr->cObj->Owner;
  Game.BlastObjects(iX, iY, iLevel, pInObj, iCausedBy);
  return TRUE;
}

long FnSound(C4Thread *cthr, const char *szSound, long fGlobal) { return SoundEffect(FnStringPar(szSound), 0, 100, fGlobal ? NULL : cthr->cObj); }

long FnMusic(C4Thread *cthr, const char *szSongname) {
  if (!szSongname) {
    Config.Sound.RXMusic = FALSE;
    Game.MusicSystem.Stop();
  } else {
    Config.Sound.RXMusic = TRUE;
    Game.MusicSystem.Stop();
    if (!Game.MusicSystem.Play(FnStringPar(szSongname))) {
      Config.Sound.RXMusic = FALSE;
      return FALSE;
    }
  }
  return TRUE;
}

long FnSoundLevel(C4Thread *cthr, const char *szSound, long iLevel) { return SoundLevel(FnStringPar(szSound), iLevel); }

long FnGameOver() { return Game.DoGameOver(); }

long FnGainMissionAccess(C4Thread *cthr, const char *szPassword) {
  if (SLen(Config.General.MissionAccess) + SLen(FnStringPar(szPassword)) + 3 > CFG_MaxString)
    return FALSE;
  SAddModule(Config.General.MissionAccess, FnStringPar(szPassword));
  return TRUE;
}

long FnLog(C4Thread *cthr, const char *szMessage, long iPar0, long iPar1, long iPar2, long iPar3, long iPar4, long iPar5, long iPar6, long iPar7, long iPar8) {
  Log(FnStringFormat(szMessage, iPar0, iPar1, iPar2, iPar3, iPar4, iPar5, iPar6, iPar7));
  return TRUE;
}

const char *FnFormat(C4Thread *cthr, const char *szFormat, int iPar0, int iPar1, int iPar2, int iPar3, int iPar4, int iPar5, int iPar6, int iPar7, int iPar8) {
  static char szFnFormatBuf[1024 + 1];
  SCopy(FnStringFormat(szFormat, iPar0, iPar1, iPar2, iPar3, iPar4, iPar5, iPar6, iPar7, iPar8), szFnFormatBuf, 1024);
  return szFnFormatBuf;
}

C4ID FnC4Id(C4Thread *cthr, const char *szID) { return (C4Id(FnStringPar(szID))); }

long FnMessage(C4Thread *cthr, const char *szMessage, C4Object *pObj, long iPar0, long iPar1, long iPar2, long iPar3, long iPar4, long iPar5, long iPar6, long iPar7) {
  char buf[MaxFnStringParLen + 1];
  if (!szMessage)
    return FALSE;

  // Speech
  BOOL fSpoken = FALSE;
  if (SCopySegment(FnStringPar(szMessage), 1, buf, '$'))
    if (SoundEffect(buf, 0, 100, cthr->cObj))
      fSpoken = TRUE;

  // Text
  if (!fSpoken)
    if (SCopySegment(FnStringFormat(szMessage, iPar0, iPar1, iPar2, iPar3, iPar4, iPar5, iPar6, iPar7), 0, buf, '$'))
      if (pObj)
        GameMsgObject(buf, pObj);
      else
        GameMsgGlobal(buf);

  return TRUE;
}

long FnScriptGo(C4Thread *cthr, long go) {
  Game.Script.Go = go;
  return TRUE;
}

long FnCastPXS(C4Thread *cthr, const char *mat_name, long amt, long level, long tx, long ty) {
  if (cthr->cObj) {
    tx += cthr->cObj->x;
    ty += cthr->cObj->y;
  }
  Game.PXS.Cast(Game.Material.Get(FnStringPar(mat_name)), amt, tx, ty, level);
  return TRUE;
}

long FnCastObjects(C4Thread *cthr, long id, long amt, long level, long tx, long ty) {
  if (cthr->cObj) {
    tx += cthr->cObj->x;
    ty += cthr->cObj->y;
  }
  Game.CastObjects(id, amt, level, tx, ty, cthr->cObj ? cthr->cObj->Owner : NO_OWNER);
  return TRUE;
}

long FnMaterial(C4Thread *cthr, const char *mat_name) { return Game.Material.Get(FnStringPar(mat_name)); }

C4Object *FnPlaceVegetation(C4Thread *cthr, C4ID id, long iX, long iY, long iWdt, long iHgt, long iGrowth) {
  // Local call: relative coordinates
  if (cthr->cObj) {
    iX += cthr->cObj->x;
    iY += cthr->cObj->y;
  }
  // Place vegetation
  return Game.PlaceVegetation(id, iX, iY, iWdt, iHgt, iGrowth);
}

C4Object *FnPlaceAnimal(C4Thread *cthr, C4ID id) { return Game.PlaceAnimal(id); }

long FnDrawVolcanoBranch(C4Thread *cthr, long mat, long fx, long fy, long tx, long ty, long size) {
  int cx, cx2, cy;
  for (cy = ty; cy < fy; cy++) {
    cx = fx + (tx - fx) * (cy - fy) / (ty - fy);
    for (cx2 = cx - size / 2; cx2 < cx + size / 2; cx2++)
      SBackPix(cx2, cy, Mat2PixColRnd(mat) + GBackIFT(cx2, cy));
  }
  return TRUE;
}

long FnHostile(C4Thread *cthr, long iPlr1, long iPlr2) { return Hostile(iPlr1, iPlr2); }

long FnSetHostility(C4Thread *cthr, long iPlr, long iPlr2, long fHostile) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->SetHostility(iPlr2, fHostile);
}

long FnSetPlrView(C4Thread *cthr, long iPlr, C4Object *tobj) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->SetViewMode(C4PVM_Target, tobj);
  return TRUE;
}

long FnSetPlrShowControl(C4Thread *cthr, long iPlr, const char *defstring) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->ShowControl = StringBitEval(FnStringPar(defstring));
  return TRUE;
}

long FnSetPlrShowCommand(C4Thread *cthr, long iPlr, long iCom) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->FlashCom = iCom;
  if (!Config.Graphics.ShowCommands)
    Config.Graphics.ShowCommands = TRUE;
  return TRUE;
}

long FnSetPlrShowControlPos(C4Thread *cthr, long iPlr, long pos) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->ShowControlPos = pos;
  return TRUE;
}

long FnGetPlrView(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->ViewMode;
}

long FnDoHomebaseMaterial(C4Thread *cthr, long iPlr, long id, long iChange) {
  if (!ValidPlr(iPlr))
    return FALSE;
  int iLastcount = Game.Players.Get(iPlr)->HomeBaseMaterial.GetIDCount(id);
  return Game.Players.Get(iPlr)->HomeBaseMaterial.SetIDCount(id, iLastcount + iChange, TRUE);
}

long FnGetPlrDownDouble(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->LastComDownDouble;
}

long FnSetPlrKnowledge(C4Thread *cthr, long iPlr, long id) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->Knowledge.SetIDCount(id, 1, TRUE);
}

long FnSetComponent(C4Thread *cthr, long idComponent, long iCount, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return pObj->Component.SetIDCount(idComponent, iCount, TRUE);
}

long FnGetPlrKnowledge(C4Thread *cthr, long iPlr, long id, long iIndex, DWORD dwCategory) {
  if (!ValidPlr(iPlr))
    return FALSE;
  // Search by id, check if available, return BOOL
  if (id)
    return (Game.Players.Get(iPlr)->Knowledge.GetIDCount(id, 1) != 0);
  // Search indexed item of given category, return C4ID
  return Game.Players.Get(iPlr)->Knowledge.GetID(Game.Defs, dwCategory, iIndex);
}

C4ID FnGetDefinition(C4Thread *cthr, int iIndex, DWORD dwCategory) {
  C4Def *pDef;
  // Default: all categories
  if (!dwCategory)
    dwCategory = C4D_All;
  // Get def
  if (!(pDef = Game.Defs.GetDef(iIndex, dwCategory)))
    return C4ID_None;
  // Return id
  return pDef->id;
}

long FnGetComponent(C4Thread *cthr, long idComponent, long iIndex, C4Object *pObj, long idDef) {
  // Def component
  if (idDef) {
    // Get def
    C4Def *pDef = C4Id2Def(idDef);
    if (!pDef)
      return 0;
    // Component count
    if (idComponent)
      return pDef->Component.GetIDCount(idComponent);
    // Indexed component
    return pDef->Component.GetID(iIndex);
  }
  // Object component
  else {
    // Get object
    if (!pObj)
      pObj = cthr->cObj;
    if (!IsObjectValid(pObj))
      return 0;
    // Component count
    if (idComponent)
      return pObj->Component.GetIDCount(idComponent);
    // Indexed component
    return pObj->Component.GetID(iIndex);
  }
  return 0;
}

long FnGetHomebaseMaterial(C4Thread *cthr, long iPlr, long id, long iIndex, DWORD dwCategory) {
  if (!ValidPlr(iPlr))
    return FALSE;
  // Search by id, return available count
  if (id)
    return Game.Players.Get(iPlr)->HomeBaseMaterial.GetIDCount(id);
  // Search indexed item of given category, return C4ID
  return Game.Players.Get(iPlr)->HomeBaseMaterial.GetID(Game.Defs, dwCategory, iIndex);
}

long FnSetPlrMagic(C4Thread *cthr, long iPlr, long id) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->Magic.SetIDCount(id, 1, TRUE);
}

long FnGetPlrMagic(C4Thread *cthr, long iPlr, long id, long iIndex) {
  if (!ValidPlr(iPlr))
    return FALSE;
  // Search by id, check if available, return BOOL
  if (id)
    return (Game.Players.Get(iPlr)->Magic.GetIDCount(id, 1) != 0);
  // Search indexed item of given category, return C4ID
  return Game.Players.Get(iPlr)->Magic.GetID(Game.Defs, C4D_Magic, iIndex);
}

long FnGetWealth(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->Wealth;
}

long FnSetWealth(C4Thread *cthr, long iPlr, long iValue) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->Wealth = BoundBy(iValue, 0, 100000);
  return TRUE;
}

long FnDoScore(C4Thread *cthr, long iPlr, long iChange) {
  if (!ValidPlr(iPlr))
    return FALSE;
  return Game.Players.Get(iPlr)->DoPoints(iChange);
}

long FnGetPlrValue(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->Value;
}

long FnGetPlrValueGain(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->ValueGain;
}

long FnGetScore(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->Points;
}

C4Object *FnGetHiRank(C4Thread *cthr, int iPlr) {
  if (!ValidPlr(iPlr))
    return NULL;
  return Game.Players.Get(iPlr)->GetHiRankActiveCrew();
}

C4Object *FnGetCrew(C4Thread *cthr, int iPlr, int index) {
  if (!ValidPlr(iPlr))
    return NULL;
  return Game.Players.Get(iPlr)->Crew.GetObject(index);
}

long FnGetCrewCount(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->Crew.ObjectCount();
}

long FnGetPlayerCount(C4Thread *cthr) { return Game.Players.GetCount(); }

long FnEliminatePlayer(C4Thread *cthr, long iPlr) {
  C4Player *pPlr = Game.Players.Get(iPlr);
  if (!pPlr)
    return FALSE;
  if (pPlr->Eliminated)
    return FALSE;
  pPlr->Eliminate();
  return TRUE;
}

C4Object *FnGetCursor(C4Thread *cthr, int iPlr) {
  if (!ValidPlr(iPlr))
    return NULL;
  return Game.Players.Get(iPlr)->Cursor;
}

C4Object *FnGetCaptain(C4Thread *cthr, int iPlr) {
  if (!ValidPlr(iPlr))
    return NULL;
  return Game.Players.Get(iPlr)->Captain;
}

long FnSetCursor(C4Thread *cthr, long iPlr, C4Object *pObj) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->SetCursor(pObj);
  return TRUE;
}

long FnSelectCrew(C4Thread *cthr, long iPlr, C4Object *pObj, long fSelect) {
  if (!ValidPlr(iPlr))
    return FALSE;
  Game.Players.Get(iPlr)->SelectCrew(pObj, fSelect);
  return TRUE;
}

long FnGetSelectCount(C4Thread *cthr, long iPlr) {
  if (!ValidPlr(iPlr))
    return 0;
  return Game.Players.Get(iPlr)->SelectCount;
}

long FnGetWind(C4Thread *cthr, long x, long y) {
  if (cthr->cObj) {
    x += cthr->cObj->x;
    y += cthr->cObj->y;
  }
  return Game.Weather.GetWind(x, y);
}

long FnSetWind(C4Thread *cthr, long iWind) {
  Game.Weather.SetWind(iWind);
  return TRUE;
}

long FnSetTemperature(C4Thread *cthr, long iTemperature) {
  Game.Weather.SetTemperature(iTemperature);
  return TRUE;
}

long FnGetTemperature(C4Thread *cthr) { return Game.Weather.GetTemperature(); }

long FnSetSeason(C4Thread *cthr, long iSeason) {
  Game.Weather.SetSeason(iSeason);
  return TRUE;
}

long FnGetSeason(C4Thread *cthr) { return Game.Weather.GetSeason(); }

long FnSetClimate(C4Thread *cthr, long iClimate) {
  Game.Weather.SetClimate(iClimate);
  return TRUE;
}

long FnGetClimate(C4Thread *cthr) { return Game.Weather.GetClimate(); }

long FnSetSkyFade(C4Thread *cthr, long iFromRed, long iFromGreen, long iFromBlue, long iToRed, long iToGreen, long iToBlue) {
  // Create color array
  int iColors[6];
  iColors[0] = iFromRed;
  iColors[1] = iFromGreen;
  iColors[2] = iFromBlue;
  iColors[3] = iToRed;
  iColors[4] = iToGreen;
  iColors[5] = iToBlue;
  // Set sky fade palette
  Game.Landscape.Sky.SetFadePalette(iColors);
  // Copy sky palette to game palette
  MemCopy(Game.Landscape.Sky.Palette, Game.GraphicsResource.GamePalette + 3 * CSkyDef1, 3 * 20);
  // Update graphics system palette (next frame)
  Game.GraphicsSystem.fSetPalette = TRUE;
  // Done
  return TRUE;
}

long FnSetSkyColor(C4Thread *cthr, long iIndex, long iRed, long iGreen, long iBlue) {
  // Set sky palette
  Game.Landscape.Sky.SetColor(iIndex, iRed, iGreen, iBlue);
  // Copy sky palette to game palette
  MemCopy(Game.Landscape.Sky.Palette, Game.GraphicsResource.GamePalette + 3 * CSkyDef1, 3 * 20);
  // Update graphics system palette (next frame)
  Game.GraphicsSystem.fSetPalette = TRUE;
  // Done
  return TRUE;
}

long FnGetSkyColor(C4Thread *cthr, long iIndex, long iRGB) {
  if (!Inside(iIndex, 0, 19) || !Inside(iRGB, 0, 2))
    return 0;
  return Game.Landscape.Sky.Palette[iIndex * 3 + iRGB];
}

long FnLandscapeWidth(C4Thread *cthr) { return GBackWdt; }

long FnLandscapeHeight(C4Thread *cthr) { return GBackHgt; }

long FnLaunchLightning(C4Thread *cthr, long x, long y, long xdir, long xrange, long ydir, long yrange) { return Game.Weather.LaunchLightning(x, y, xdir, xrange, ydir, yrange); }

long FnLaunchVolcano(C4Thread *cthr, long x) { return Game.Weather.LaunchVolcano(Game.Material.Get("Lava"), x, GBackHgt - 1, BoundBy(15 * GBackHgt / 500 + Random(10), 10, 60)); }

long FnLaunchEarthquake(C4Thread *cthr, long x, long y) {
  Game.Weather.LaunchEarthquake(x, y);
  return 1;
}

long FnShakeFree(C4Thread *cthr, long x, long y, long rad) {
  Game.Landscape.ShakeFree(x, y, rad);
  return 1;
}

long FnShakeObjects(C4Thread *cthr, long x, long y, long rad) {
  Game.ShakeObjects(x, y, rad);
  return 1;
}

long FnDigFree(C4Thread *cthr, long x, long y, long rad, long fRequest) {
  Game.Landscape.DigFree(x, y, rad, fRequest, cthr->cObj);
  return 1;
}

long FnDigFreeRect(C4Thread *cthr, long iX, long iY, long iWdt, long iHgt, long fRequest) {
  Game.Landscape.DigFreeRect(iX, iY, iWdt, iHgt, fRequest, cthr->cObj);
  return TRUE;
}

long FnFreeRect(C4Thread *cthr, long iX, long iY, long iWdt, long iHgt) {
  Game.Landscape.ClearRect(iX, iY, iWdt, iHgt);
  return TRUE;
}

long FnPathFree(C4Thread *cthr, long iX1, long iY1, long iX2, long iY2) { return PathFree(iX1, iY1, iX2, iY2); }

long FnSetTransferZone(C4Thread *cthr, long iX, long iY, long iWdt, long iHgt, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  iX += pObj->x;
  iY += pObj->y;
  return Game.TransferZones.Set(iX, iY, iWdt, iHgt, pObj);
}

long FnNot(C4Thread *cthr, long fCondition) { return !fCondition; }

long FnOr(C4Thread *cthr, long fCon1, long fCon2, long fCon3, long fCon4, long fCon5) { return (fCon1 || fCon2 || fCon3 || fCon4 || fCon5); }

long FnAnd(C4Thread *cthr, long fCon1, long fCon2) { return (fCon1 && fCon2); }

long FnBitAnd(C4Thread *cthr, long iVal1, long iVal2) { return (iVal1 & iVal2); }

long FnEqual(C4Thread *cthr, long iVal1, long iVal2) { return (iVal1 == iVal2); }

long FnLessThan(C4Thread *cthr, long iVal1, long iVal2) { return (iVal1 < iVal2); }

long FnGreaterThan(C4Thread *cthr, long iVal1, long iVal2) { return (iVal1 > iVal2); }

long FnSum(C4Thread *cthr, long iVal1, long iVal2, long iVal3, long iVal4) { return (iVal1 + iVal2 + iVal3 + iVal4); }

long FnSub(C4Thread *cthr, long iVal1, long iVal2, long iVal3, long iVal4) { return (iVal1 - iVal2 - iVal3 - iVal4); }

long FnAbs(C4Thread *cthr, long iVal) { return Abs(iVal); }

long FnMul(C4Thread *cthr, long iVal1, long iVal2) { return (iVal1 * iVal2); }

long FnDiv(C4Thread *cthr, long iVal1, long iVal2) {
  if (!iVal2)
    return 0;
  return (iVal1 / iVal2);
}

long FnMod(C4Thread *cthr, long iVal1, long iVal2) {
  if (!iVal2)
    return 0;
  return (iVal1 % iVal2);
}

const double pi = 3.141592654;

long FnSin(C4Thread *cthr, long iAngle, long iRadius) { return (int)(iRadius * sin(2.0 * pi * iAngle / 360.0)); }

long FnCos(C4Thread *cthr, long iAngle, long iRadius) { return (int)(iRadius * cos(2.0 * pi * iAngle / 360.0)); }

long FnSqrt(C4Thread *cthr, long iValue) { return (int)sqrt(iValue); }

long FnAngle(C4Thread *cthr, long iX1, long iY1, long iX2, long iY2) {
  int iAngle;

  iAngle = (int)(180.0 * atan2(Abs(iY1 - iY2), Abs(iX1 - iX2)) / pi);

  if (iX2 > iX1) {
    if (iY2 < iY1)
      iAngle = 90 - iAngle;
    else
      iAngle = 90 + iAngle;
  } else {
    if (iY2 < iY1)
      iAngle = 270 + iAngle;
    else
      iAngle = 270 - iAngle;
  }

  return iAngle;
}

long FnMin(C4Thread *cthr, long iVal1, long iVal2) { return Min(iVal1, iVal2); }

long FnMax(C4Thread *cthr, long iVal1, long iVal2) { return Max(iVal1, iVal2); }

long FnDistance(C4Thread *cthr, long iX1, long iY1, long iX2, long iY2) { return Distance(iX1, iY1, iX2, iY2); }

long FnObjectDistance(C4Thread *cthr, C4Object *pObj2, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj) || !IsObjectValid(pObj2))
    return 0;
  return Distance(pObj->x, pObj->y, pObj2->x, pObj2->y);
}

long FnObjectNumber(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return 0;
  return pObj->Number;
}

C4Object *FnObject(C4Thread *cthr, long iNumber) { return Game.Objects.ObjectPointer(iNumber); }

long FnShowInfo(C4Thread *cthr, C4Object *pObj) {
  if (!cthr->cObj)
    return FALSE;
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  return cthr->cObj->ActivateMenu(C4MN_Info, 0, 0, 0, pObj);
}

long FnBoundBy(C4Thread *cthr, long iVal, long iRange1, long iRange2) { return BoundBy(iVal, iRange1, iRange2); }

long FnInside(C4Thread *cthr, long iVal, long iRange1, long iRange2) { return Inside(iVal, iRange1, iRange2); }

long FnSEqual(C4Thread *cthr, const char *szString1, const char *szString2) {
  SCopy(FnStringPar(szString1), pscOSTR);
  return SEqual(pscOSTR, FnStringPar(szString2));
}

long FnRandom(C4Thread *cthr, long iRange) { return Random(iRange); }

long FnSafeRandom(C4Thread *cthr, long iRange) { return SafeRandom(iRange); }

long FnSetVar(C4Thread *cthr, long iVarIndex, long iValue) {
  if (!Inside(iVarIndex, 0, C4ThreadMaxVar - 1))
    return FALSE;
  cthr->Variable[iVarIndex] = iValue;
  return iValue;
}

long FnIncVar(C4Thread *cthr, long iVarIndex) {
  if (!Inside(iVarIndex, 0, C4ThreadMaxVar - 1))
    return FALSE;
  cthr->Variable[iVarIndex]++;
  return cthr->Variable[iVarIndex];
}

long FnDecVar(C4Thread *cthr, long iVarIndex) {
  if (!Inside(iVarIndex, 0, C4ThreadMaxVar - 1))
    return FALSE;
  cthr->Variable[iVarIndex]--;
  return cthr->Variable[iVarIndex];
}

long FnVar(C4Thread *cthr, long iVarIndex) {
  if (!Inside(iVarIndex, 0, C4ThreadMaxVar - 1))
    return FALSE;
  return cthr->Variable[iVarIndex];
}

long FnSetGlobal(C4Thread *cthr, long iVarIndex, long iValue) {
  if (!Inside(iVarIndex, 0, C4MaxGlobal - 1))
    return FALSE;
  Game.Script.Global[iVarIndex] = iValue;
  return iValue;
}

long FnGlobal(C4Thread *cthr, long iVarIndex) {
  if (!Inside(iVarIndex, 0, C4MaxGlobal - 1))
    return FALSE;
  return Game.Script.Global[iVarIndex];
}

long FnSetLocal(C4Thread *cthr, long iVarIndex, long iValue, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (!Inside(iVarIndex, 0, C4MaxVariable - 1))
    return FALSE;
  pObj->Local[iVarIndex] = iValue;
  return iValue;
}

long FnLocal(C4Thread *cthr, long iVarIndex, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  if (!IsObjectValid(pObj))
    return FALSE;
  if (!Inside(iVarIndex, 0, C4MaxVariable - 1))
    return FALSE;
  return pObj->Local[iVarIndex];
}

long FnPar(C4Thread *cthr, long iParIndex) {
  if (!Inside(iParIndex, 0, C4ThreadMaxPar - 1))
    return FALSE;
  return cthr->Parameter[iParIndex];
}

long FnCall(C4Thread *cthr, const char *szFunction, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  if (!szFunction)
    return FALSE;
  if (!cthr->cObj)
    return 0;
  return cthr->cObj->Call(FnStringPar(szFunction), par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

long FnObjectCall(C4Thread *cthr, C4Object *pObj, const char *szFunction, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  if (!IsObjectValid(pObj) || !szFunction)
    return FALSE;
  // Make failsafe
  char szFunc2[500 + 1];
  sprintf(szFunc2, "~%s", FnStringPar(szFunction));
  // Call
  return pObj->Call(cthr, szFunc2, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

long FnDefinitionCall(C4Thread *cthr, long idID, const char *szFunction, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  if (!idID || !szFunction)
    return FALSE;
  // Make failsafe
  char szFunc2[500 + 1];
  sprintf(szFunc2, "~%s", FnStringPar(szFunction));
  // Get definition
  C4Def *pDef;
  if (!(pDef = C4Id2Def(idID)))
    return FALSE;
  // Call
  return pDef->Script.Call(cthr, szFunc2, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

long FnGameCall(C4Thread *cthr, const char *szFunction, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  if (!szFunction)
    return FALSE;
  // Make failsafe
  char szFunc2[500 + 1];
  sprintf(szFunc2, "~%s", FnStringPar(szFunction));
  // Call
  return Game.Script.Call(cthr, szFunc2, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

long FnEditCursor() { return (long)Console.EditCursor.GetTarget(); }

long FnResort(C4Thread *cthr, C4Object *pObj) {
  if (!pObj)
    pObj = cthr->cObj;
  // Resort single object
  if (IsObjectValid(pObj))
    pObj->Resort();
  // Resort object list
  else
    Game.Objects.SortByCategory();
  return TRUE;
}

long FnC4D_All() { return C4D_All; }
long FnC4D_StaticBack() { return C4D_StaticBack; }
long FnC4D_Structure() { return C4D_Structure; }
long FnC4D_Vehicle() { return C4D_Vehicle; }
long FnC4D_Living() { return C4D_Living; }
long FnC4D_Object() { return C4D_Object; }
long FnC4D_Knowledge() { return C4D_SelectKnowledge; }
long FnC4D_Goal() { return C4D_Goal; }
long FnC4D_Rule() { return C4D_Rule; }

long FnCOMD_None() { return COMD_None; }
long FnCOMD_Stop() { return COMD_Stop; }
long FnCOMD_Up() { return COMD_Up; }
long FnCOMD_UpRight() { return COMD_UpRight; }
long FnCOMD_Right() { return COMD_Right; }
long FnCOMD_DownRight() { return COMD_DownRight; }
long FnCOMD_Down() { return COMD_Down; }
long FnCOMD_DownLeft() { return COMD_DownLeft; }
long FnCOMD_Left() { return COMD_Left; }
long FnCOMD_UpLeft() { return COMD_UpLeft; }

long FnDIR_Left() { return DIR_Left; }
long FnDIR_Right() { return DIR_Right; }

long FnOCF_Prey() { return OCF_Prey; }
long FnOCF_Living() { return OCF_Living; }
long FnOCF_Edible() { return OCF_Edible; }
long FnOCF_Collectible() { return OCF_Carryable; }
long FnOCF_Container() { return OCF_Container; }
long FnOCF_NotContained() { return OCF_NotContained; }
long FnOCF_Fullcon() { return OCF_FullCon; }
long FnOCF_CrewMember() { return OCF_CrewMember; }
long FnOCF_Grab() { return OCF_Grab; }
long FnOCF_OnFire() { return OCF_OnFire; }
long FnOCF_Chop() { return OCF_Chop; }
long FnOCF_Construct() { return OCF_Construct; }
long FnOCF_InLiquid() { return OCF_InLiquid; }
long FnOCF_InSolid() { return OCF_InSolid; }
long FnOCF_InFree() { return OCF_InFree; }
long FnOCF_Available() { return OCF_Available; }
long FnOCF_PowerConsumer() { return OCF_PowerConsumer; }
long FnOCF_PowerSupply() { return OCF_PowerSupply; }
long FnOCF_AttractLightning() { return OCF_AttractLightning; }

C4Object *FnAnyContainer() { return ANY_CONTAINER; }
C4Object *FnNoContainer() { return NO_CONTAINER; }

//=========================== C4Script Function Map ===================================

#define MkFn (long (*)(C4Thread *, long, long, long, long, long, long, long, long, long, long, long, long))

C4ScriptFnDef C4ScriptFnMap[] = {

    {"return", MkFn &Fn_return, 1},
    {"if", MkFn &Fn_if, 1},
    {"while", MkFn &Fn_while, 1},
    {"goto", MkFn &Fn_goto, 1},
    {"this", MkFn &Fn_this, 1},
    {"Or", MkFn &FnOr, 1},
    {"Not", MkFn &FnNot, 1},
    {"And", MkFn &FnAnd, 1},
    {"BitAnd", MkFn &FnBitAnd, 0},
    {"Sum", MkFn &FnSum, 1},
    {"Sub", MkFn &FnSub, 1},
    {"Abs", MkFn &FnAbs, 1},
    {"Min", MkFn &FnMin, 1},
    {"Max", MkFn &FnMax, 1},
    {"Mul", MkFn &FnMul, 1},
    {"Div", MkFn &FnDiv, 1},
    {"Mod", MkFn &FnMod, 1},
    {"Sin", MkFn &FnSin, 1},
    {"Cos", MkFn &FnCos, 1},
    {"Sqrt", MkFn &FnSqrt, 1},
    {"LessThan", MkFn &FnLessThan, 1},
    {"GreaterThan", MkFn &FnGreaterThan, 1},
    {"BoundBy", MkFn &FnBoundBy, 1},
    {"Inside", MkFn &FnInside, 1},
    {"Equal", MkFn &FnEqual, 1},
    {"SEqual", MkFn &FnSEqual, 1},
    {"Random", MkFn &FnRandom, 1},
    {"Var", MkFn &FnVar, 1},
    {"Par", MkFn &FnPar, 1},
    {"AssignVar", MkFn &FnSetVar, 0},
    {"SetVar", MkFn &FnSetVar, 1},
    {"IncVar", MkFn &FnIncVar, 1},
    {"DecVar", MkFn &FnDecVar, 1},
    {"SetGlobal", MkFn &FnSetGlobal, 1},
    {"Global", MkFn &FnGlobal, 1},
    {"SetLocal", MkFn &FnSetLocal, 1},
    {"Local", MkFn &FnLocal, 1},
    {"Explode", MkFn &FnExplode, 1},
    {"DoCon", MkFn &FnDoCon, 1},
    {"GetCon", MkFn &FnGetCon, 1},
    {"DoDamage", MkFn &FnDoDamage, 1},
    {"DoEnergy", MkFn &FnDoEnergy, 1},
    {"DoBreath", MkFn &FnDoBreath, 1},
    {"DoMagicEnergy", MkFn &FnDoMagicEnergy, 1},
    {"GetMagicEnergy", MkFn &FnGetMagicEnergy, 1},
    {"EnergyCheck", MkFn &FnEnergyCheck, 0},
    {"CheckEnergyNeedChain", MkFn &FnCheckEnergyNeedChain, 0},
    {"GetEnergy", MkFn &FnGetEnergy, 1},
    {"Incinerate", MkFn &FnIncinerate, 1},
    {"Extinguish", MkFn &FnExtinguish, 1},
    {"OnFire", MkFn &FnOnFire, 1},
    {"Smoke", MkFn &FnSmoke, 1},
    {"Stuck", MkFn &FnStuck, 1},
    {"InLiquid", MkFn &FnInLiquid, 1},
    {"Bubble", MkFn &FnBubble, 1},
    {"GrabContents", MkFn &FnGrabContents, 0},
    {"Punch", MkFn &FnPunch, 1},
    {"Fling", MkFn &FnFling, 1},
    {"Jump", MkFn &FnJump, 1},
    {"SetAction", MkFn &FnSetAction, 1},
    {"SetActionData", MkFn &FnSetActionData, 0},
    {"ChangeDef", MkFn &FnChangeDef, 1},
    {"GetAction", MkFn &FnGetAction, 1},
    {"GetActTime", MkFn &FnGetActTime, 1},
    {"GetOwner", MkFn &FnGetOwner, 1},
    {"GetMass", MkFn &FnGetMass, 1},
    {"GetBreath", MkFn &FnGetBreath, 1},
    {"GetX", MkFn &FnGetX, 1},
    {"GetY", MkFn &FnGetY, 1},
    {"GetBase", MkFn &FnGetBase, 1},
    {"GetMenu", MkFn &FnGetMenu, 0},
    {"GetVertexNum", MkFn &FnGetVertexNum, 0},
    {"GetVertex", MkFn &FnGetVertex, 0},
    {"SetVertex", MkFn &FnSetVertex, 0},
    {"AddVertex", MkFn &FnAddVertex, 0},
    {"AnyContainer", MkFn &FnAnyContainer, 1},
    {"NoContainer", MkFn &FnNoContainer, 1},
    {"GetController", MkFn &FnGetController, 1},
    {"GetPhase", MkFn &FnGetPhase, 1},
    {"SetPhase", MkFn &FnSetPhase, 1},
    {"GetCategory", MkFn &FnGetCategory, 1},
    {"GetOCF", MkFn &FnGetOCF, 1},
    {"SetAlive", MkFn &FnSetAlive, 1},
    {"GetAlive", MkFn &FnGetAlive, 1},
    {"GetDamage", MkFn &FnGetDamage, 1},
    {"CrewMember", MkFn &FnCrewMember, 0},
    {"ObjectSetAction", MkFn &FnObjectSetAction, 0},
    {"ComponentAll", MkFn &FnComponentAll, 0},
    {"SetComDir", MkFn &FnSetComDir, 1},
    {"GetComDir", MkFn &FnGetComDir, 1},
    {"SetDir", MkFn &FnSetDir, 1},
    {"GetDir", MkFn &FnGetDir, 1},
    {"SetEntrance", MkFn &FnSetEntrance, 1},
    {"GetEntrance", MkFn &FnGetEntrance, 1},
    {"SetCategory", MkFn &FnSetCategory, 1},
    {"Exit", MkFn &FnExit, 1},
    {"Enter", MkFn &FnEnter, 1},
    {"Split2Components", MkFn &FnSplit2Components, 1},
    {"SetCommand", MkFn &FnSetCommand, 1},
    {"AddCommand", MkFn &FnAddCommand, 1},
    {"AppendCommand", MkFn &FnAppendCommand, 1},
    {"GetCommand", MkFn &FnGetCommand, 1},
    {"GetDefinition", MkFn &FnGetDefinition, 1},
    {"ActIdle", MkFn &FnActIdle, 0},
    {"SetRDir", MkFn &FnSetRDir, 1},
    {"GetRDir", MkFn &FnGetRDir, 1},
    {"GetXDir", MkFn &FnGetXDir, 1},
    {"GetYDir", MkFn &FnGetYDir, 1},
    {"GetR", MkFn &FnGetR, 1},
    {"GetName", MkFn &FnGetName, 1},
    {"GetDesc", MkFn &FnGetDesc, 1},
    {"GetPlayerName", MkFn &FnGetPlayerName, 1},
    {"SetXDir", MkFn &FnSetXDir, 1},
    {"SetYDir", MkFn &FnSetYDir, 1},
    {"SetR", MkFn &FnSetR, 1},
    {"SetOwner", MkFn &FnSetOwner, 1},
    {"CreateObject", MkFn &FnCreateObject, 1},
    {"MakeCrewMember", MkFn &FnMakeCrewMember, 1},
    {"CreateContents", MkFn &FnCreateContents, 1},
    {"ShiftContents", MkFn &FnShiftContents, 0},
    {"ComposeContents", MkFn &FnComposeContents, 1},
    {"CreateConstruction", MkFn &FnCreateConstruction, 1},
    {"GetID", MkFn &FnGetID, 1},
    {"Contents", MkFn &FnContents, 1},
    {"ScrollContents", MkFn &FnScrollContents, 0},
    {"Contained", MkFn &FnContained, 1},
    {"ContentsCount", MkFn &FnContentsCount, 1},
    {"FindContents", MkFn &FnFindContents, 0},
    {"FindConstructionSite", MkFn &FnFindConstructionSite, 0},
    {"FindOtherContents", MkFn &FnFindOtherContents, 0},
    {"FindBase", MkFn &FnFindBase, 1},
    {"PathFree", MkFn &FnPathFree, 1},
    {"Sound", MkFn &FnSound, 1},
    {"Music", MkFn &FnMusic, 1},
    {"SoundLevel", MkFn &FnSoundLevel, 0},
    {"DeathAnnounce", MkFn &FnDeathAnnounce, 0},
    {"FindObject", MkFn &FnFindObject, 1},
    {"ObjectCount", MkFn &FnObjectCount, 1},
    {"ObjectCall", MkFn &FnObjectCall, 1},
    {"GameCall", MkFn &FnGameCall, 1},
    {"DefinitionCall", MkFn &FnDefinitionCall, 1},
    {"AssignRemoval", MkFn &FnRemoveObject, 0},
    {"RemoveObject", MkFn &FnRemoveObject, 1},
    {"GetActionTarget", MkFn &FnGetActionTarget, 1},
    {"Call", MkFn &FnCall, 0},
    {"SetPlrView", MkFn &FnSetPlrView, 1},
    {"SetPlrKnowledge", MkFn &FnSetPlrKnowledge, 1},
    {"GetPlrKnowledge", MkFn &FnGetPlrKnowledge, 1},
    {"SetPlrMagic", MkFn &FnSetPlrMagic, 1},
    {"GetPlrMagic", MkFn &FnGetPlrMagic, 1},
    {"GetPlrDownDouble", MkFn &FnGetPlrDownDouble, 0},
    {"GetPlrView", MkFn &FnGetPlrView, 0},
    {"GetWealth", MkFn &FnGetWealth, 1},
    {"SetWealth", MkFn &FnSetWealth, 1},
    {"SetComponent", MkFn &FnSetComponent, 1},
    {"GetComponent", MkFn &FnGetComponent, 1},
    {"DoScore", MkFn &FnDoScore, 1},
    {"GetScore", MkFn &FnGetScore, 1},
    {"GetPlrValue", MkFn &FnGetPlrValue, 1},
    {"GetPlrValueGain", MkFn &FnGetPlrValueGain, 1},
    {"SetPlrShowControl", MkFn &FnSetPlrShowControl, 0},
    {"SetPlrShowControlPos", MkFn &FnSetPlrShowControlPos, 0},
    {"SetPlrShowCommand", MkFn &FnSetPlrShowCommand, 0},
    {"GetWind", MkFn &FnGetWind, 1},
    {"SetWind", MkFn &FnSetWind, 1},
    {"SetSkyFade", MkFn &FnSetSkyFade, 1},
    {"SetSkyColor", MkFn &FnSetSkyColor, 1},
    {"GetSkyColor", MkFn &FnGetSkyColor, 1},
    {"GetTemperature", MkFn &FnGetTemperature, 1},
    {"SetTemperature", MkFn &FnSetTemperature, 1},
    {"LaunchLightning", MkFn &FnLaunchLightning, 1},
    {"LaunchVolcano", MkFn &FnLaunchVolcano, 1},
    {"LaunchEarthquake", MkFn &FnLaunchEarthquake, 1},
    {"ShakeFree", MkFn &FnShakeFree, 1},
    {"ShakeObjects", MkFn &FnShakeObjects, 0},
    {"DigFree", MkFn &FnDigFree, 1},
    {"FreeRect", MkFn &FnFreeRect, 1},
    {"DigFreeRect", MkFn &FnDigFreeRect, 1},
    {"CastPXS", MkFn &FnCastPXS, 1},
    {"CastObjects", MkFn &FnCastObjects, 1},
    {"Hostile", MkFn &FnHostile, 1},
    {"SetHostility", MkFn &FnSetHostility, 1},
    {"PlaceVegetation", MkFn &FnPlaceVegetation, 1},
    {"PlaceAnimal", MkFn &FnPlaceAnimal, 1},
    {"GameOver", MkFn &FnGameOver, 1},
    {"Message", MkFn &FnMessage, 1},
    {"Log", MkFn &FnLog, 1},
    {"Format", MkFn &FnFormat, 1},
    {"C4Id", MkFn &FnC4Id, 1},
    {"ScriptGo", MkFn &FnScriptGo, 1},
    {"GetHiRank", MkFn &FnGetHiRank, 1},
    {"GetCrew", MkFn &FnGetCrew, 1},
    {"GetCrewCount", MkFn &FnGetCrewCount, 1},
    {"GetPlayerCount", MkFn &FnGetPlayerCount, 1},
    {"EliminatePlayer", MkFn &FnEliminatePlayer, 1},
    {"GetCursor", MkFn &FnGetCursor, 1},
    {"GetCaptain", MkFn &FnGetCaptain, 1},
    {"SetCursor", MkFn &FnSetCursor, 1},
    {"SelectCrew", MkFn &FnSelectCrew, 1},
    {"GetSelectCount", MkFn &FnGetSelectCount, 0},
    {"ForcePosition", MkFn &FnForcePosition, 1},
    {"SetPosition", MkFn &FnSetPosition, 1},
    {"ExtractLiquid", MkFn &FnExtractLiquid, 0},
    {"ExtractMaterialAmount", MkFn &FnExtractMaterialAmount, 0},
    {"GetMaterial", MkFn &FnGetMaterial, 1},
    {"GetMaterialCount", MkFn &FnGetMaterialCount, 1},
    {"GBackSolid", MkFn &FnGBackSolid, 1},
    {"GBackSemiSolid", MkFn &FnGBackSemiSolid, 1},
    {"GBackLiquid", MkFn &FnGBackLiquid, 1},
    {"Material", MkFn &FnMaterial, 1},
    {"BlastObjects", MkFn &FnBlastObjects, 1},
    {"InsertMaterial", MkFn &FnInsertMaterial, 1},
    {"DrawVolcanoBranch", MkFn &FnDrawVolcanoBranch, 0},
    {"ObjectInsertMaterial", MkFn &FnObjectInsertMaterial, 0},
    {"ObjectExtractLiquid", MkFn &FnObjectExtractLiquid, 0},
    {"FlameConsumeMaterial", MkFn &FnFlameConsumeMaterial, 0},
    {"LandscapeWidth", MkFn &FnLandscapeWidth, 1},
    {"LandscapeHeight", MkFn &FnLandscapeHeight, 1},
    {"EditCursor", MkFn &FnEditCursor, 0},
    {"Resort", MkFn &FnResort, 0},
    {"CreateMenu", MkFn &FnCreateMenu, 1},
    {"AddMenuItem", MkFn &FnAddMenuItem, 1},
    {"SelectMenuItem", MkFn &FnSelectMenuItem, 1},
    {"SetSolidMask", MkFn &FnSetSolidMask, 1},
    {"SetGravity", MkFn &FnSetGravity, 1},
    {"GetGravity", MkFn &FnGetGravity, 1},
    {"SetSeason", MkFn &FnSetSeason, 1},
    {"GetSeason", MkFn &FnGetSeason, 1},
    {"SetClimate", MkFn &FnSetClimate, 1},
    {"GetClimate", MkFn &FnGetClimate, 1},
    {"Distance", MkFn &FnDistance, 1},
    {"ObjectDistance", MkFn &FnObjectDistance, 1},
    {"GetValue", MkFn &FnGetValue, 1},
    {"GetRank", MkFn &FnGetRank, 1},
    {"Value", MkFn &FnValue, 1},
    {"Angle", MkFn &FnAngle, 1},
    {"GetHomebaseMaterial", MkFn &FnGetHomebaseMaterial, 1},
    {"DoHomebaseMaterial", MkFn &FnDoHomebaseMaterial, 1},
    {"GainMissionAccess", MkFn &FnGainMissionAccess, 1},
    {"SetPhysical", MkFn &FnSetPhysical, 1},
    {"GetPhysical", MkFn &FnGetPhysical, 1},
    {"ResetPhysical", MkFn &FnResetPhysical, 1},
    {"SetTransferZone", MkFn &FnSetTransferZone, 0},

    {"C4D_All", MkFn &FnC4D_All, 1},
    {"C4D_StaticBack", MkFn &FnC4D_StaticBack, 1},
    {"C4D_Structure", MkFn &FnC4D_Structure, 1},
    {"C4D_Vehicle", MkFn &FnC4D_Vehicle, 1},
    {"C4D_Living", MkFn &FnC4D_Living, 1},
    {"C4D_Object", MkFn &FnC4D_Object, 1},
    {"C4D_Knowledge", MkFn &FnC4D_Knowledge, 1},
    {"C4D_Goal", MkFn &FnC4D_Goal, 1},
    {"C4D_Rule", MkFn &FnC4D_Rule, 1},

    {"COMD_None", MkFn &FnCOMD_None, 1},
    {"COMD_Stop", MkFn &FnCOMD_Stop, 1},
    {"COMD_Up", MkFn &FnCOMD_Up, 1},
    {"COMD_UpRight", MkFn &FnCOMD_UpRight, 1},
    {"COMD_Right", MkFn &FnCOMD_Right, 1},
    {"COMD_DownRight", MkFn &FnCOMD_DownRight, 1},
    {"COMD_Down", MkFn &FnCOMD_Down, 1},
    {"COMD_DownLeft", MkFn &FnCOMD_DownLeft, 1},
    {"COMD_Left", MkFn &FnCOMD_Left, 1},
    {"COMD_UpLeft", MkFn &FnCOMD_UpLeft, 1},

    {"DIR_Left", MkFn &FnDIR_Left, 1},
    {"DIR_Right", MkFn &FnDIR_Right, 1},

    {"OCF_Prey", MkFn &FnOCF_Prey, 1},
    {"OCF_Construct", MkFn &FnOCF_Construct, 1},
    {"OCF_CrewMember", MkFn &FnOCF_CrewMember, 1},
    {"OCF_Grab", MkFn &FnOCF_Grab, 1},
    {"OCF_Fullcon", MkFn &FnOCF_Fullcon, 1},
    {"OCF_Chop", MkFn &FnOCF_Chop, 1},
    {"OCF_OnFire", MkFn &FnOCF_OnFire, 1},
    {"OCF_Living", MkFn &FnOCF_Living, 1},
    {"OCF_Collectible", MkFn &FnOCF_Collectible, 1},
    {"OCF_Container", MkFn &FnOCF_Container, 1},
    {"OCF_NotContained", MkFn &FnOCF_NotContained, 1},
    {"OCF_Edible", MkFn &FnOCF_Edible, 1},
    {"OCF_InLiquid", MkFn &FnOCF_InLiquid, 0},
    {"OCF_InSolid", MkFn &FnOCF_InSolid, 0},
    {"OCF_InFree", MkFn &FnOCF_InFree, 0},
    {"OCF_Available", MkFn &FnOCF_Available, 1},
    {"OCF_PowerSupply", MkFn &FnOCF_PowerSupply, 1},
    {"OCF_PowerConsumer", MkFn &FnOCF_PowerConsumer, 1},
    {"OCF_AttractLightning", MkFn &FnOCF_AttractLightning, 1},

    {"Object", MkFn &FnObject, 0},
    {"ObjectNumber", MkFn &FnObjectNumber, 0},
    {"ShowInfo", MkFn &FnShowInfo, 0},

    {NULL, NULL, 0}

};
