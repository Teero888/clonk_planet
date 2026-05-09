/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Player data at runtime */

#include <C4Include.h>

#define offsC4PL(x) offsetof(C4Player, x)

C4CompilerValue C4CR_Player[] = {

    {"Player%s", C4CV_Section, 0, 0},
    {"Status", C4CV_Integer, offsC4PL(Status), 1},
    {"AtClient", C4CV_Integer, offsC4PL(AtClient), 1},
    {"AtClientName", C4CV_String, offsC4PL(AtClientName), C4MaxTitle},
    {"Index", C4CV_Integer, offsC4PL(Number), 1},
    {"Eliminated", C4CV_Integer, offsC4PL(Eliminated), 1},
    {"Surrendered", C4CV_Integer, offsC4PL(Surrendered), 1},
    {"Color", C4CV_Integer, offsC4PL(Color), 1},
    {"Control", C4CV_Integer, offsC4PL(Control), 1},
    {"MouseControl", C4CV_Integer, offsC4PL(MouseControl), 1},
    {"Position", C4CV_Integer, offsC4PL(Position), 1},
    {"ViewMode", C4CV_Integer, offsC4PL(ViewMode), 1},
    {"ViewX", C4CV_Integer, offsC4PL(ViewX), 1},
    {"ViewY", C4CV_Integer, offsC4PL(ViewY), 1},
    {"ViewWealth", C4CV_Integer, offsC4PL(ViewWealth), 1},
    {"ViewValue", C4CV_Integer, offsC4PL(ViewValue), 1},
    {"ShowStartup", C4CV_Integer, offsC4PL(ShowStartup), 1},
    {"ShowControl", C4CV_Integer, offsC4PL(ShowControl), 1},
    {"ShowControlPos", C4CV_Integer, offsC4PL(ShowControlPos), 1},
    {"Wealth", C4CV_Integer, offsC4PL(Wealth), 1},
    {"Points", C4CV_Integer, offsC4PL(Points), 1},
    {"Value", C4CV_Integer, offsC4PL(Value), 1},
    {"InitialValue", C4CV_Integer, offsC4PL(InitialValue), 1},
    {"ValueGain", C4CV_Integer, offsC4PL(ValueGain), 1},
    {"ObjectsOwned", C4CV_Integer, offsC4PL(ObjectsOwned), 1},
    {"Hostile", C4CV_IdList, offsC4PL(Hostility), 1},
    {"ProductionDelay", C4CV_Integer, offsC4PL(ProductionDelay), 1},
    {"ProductionUnit", C4CV_Integer, offsC4PL(ProductionUnit), 1},
    {"SelectCount", C4CV_Integer, offsC4PL(SelectCount), 1},
    {"SelectFlash", C4CV_Integer, offsC4PL(SelectFlash), 1},
    {"CursorFlash", C4CV_Integer, offsC4PL(CursorFlash), 1},
    {"Cursor", C4CV_Integer, offsC4PL(Cursor), 1},
    {"Captain", C4CV_Integer, offsC4PL(Captain), 1},
    {"LastCom", C4CV_Integer, offsC4PL(LastCom), 1},
    {"LastComDel", C4CV_Integer, offsC4PL(LastComDel), 1},
    {"LastComDownDouble", C4CV_Integer, offsC4PL(LastComDownDouble), 1},
    {"CursorSelection", C4CV_Integer, offsC4PL(CursorSelection), 1},
    {"CursorToggled", C4CV_Integer, offsC4PL(CursorToggled), 1},
    {"MessageStatus", C4CV_Integer, offsC4PL(MessageStatus), 1},
    {"MessageBuf", C4CV_String, offsC4PL(MessageBuf), 256},
    {"HomeBaseMaterial", C4CV_IdList, offsC4PL(HomeBaseMaterial), 1},
    {"HomeBaseProduction", C4CV_IdList, offsC4PL(HomeBaseProduction), 1},
    {"Knowledge", C4CV_IdList, offsC4PL(Knowledge), 1},
    {"Magic", C4CV_IdList, offsC4PL(Magic), 1},

    {NULL, C4CV_End, 0, 0}};

C4Player::C4Player() { Default(); }

C4Player::~C4Player() { Clear(); }

BOOL C4Player::ObjectInCrew(C4Object *tobj) {
  C4Object *cobj;
  C4ObjectLink *clnk;
  if (!tobj)
    return FALSE;
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj == tobj)
      return TRUE;
  return FALSE;
}

void C4Player::ClearPointers(C4Object *pObj) {
  // Game
  if (Captain == pObj)
    Captain = NULL;
  // Crew
  while (Crew.Remove(pObj))
    ;
  // Cursor
  if (Cursor == pObj) {
    Cursor = NULL;
    AdjustCursorCommand();
    if (Cursor)
      Cursor->Select = 1;
  }
  // View
  if (ViewTarget == pObj)
    ViewTarget = NULL;
  // Menu
  Menu.ClearPointers(pObj);
}

void C4Player::UpdateValue() {
  int lval = Value, lobj = ObjectsOwned;
  Value = 0;
  ObjectsOwned = 0;

  // Points
  Value += Points;

  // Wealth
  Value += Wealth;

  // Asset all owned objects
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = Game.Objects.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Owner == Number) {
      ObjectsOwned++;
      Value += cobj->GetValue();
    }

  // Value gain
  ValueGain = Value - InitialValue;

  // Update
  if ((Value != lval) || (ObjectsOwned != lobj))
    ViewValue = C4ViewDelay;
}

void C4Player::Execute() {
  if (!Status)
    return;

  // Tick1
  UpdateCounts();
  UpdateView();
  ExecuteControl();
  Menu.Execute();

  // Tick35
  if (!Tick35) {
    ExecHomeBaseProduction();
    UpdateValue();
    CheckElimination();
    UpdateFogOfWar();
  }

  // Delays
  if (MessageStatus > 0)
    MessageStatus--;
  if (RetireDelay > 0)
    RetireDelay--;
  if (ViewWealth > 0)
    ViewWealth--;
  if (ViewValue > 0)
    ViewValue--;
  if (CursorFlash > 0)
    CursorFlash--;
  if (SelectFlash > 0)
    SelectFlash--;
}

BOOL C4Player::Init(int iNumber, int iAtClient, const char *szAtClientName, const char *szFilename, BOOL fScenarioInit) {

  // Status init
  Status = TRUE;
  SCopy(szFilename, Filename);
  printf("C4Player::Init: SCopy(szFilename, Filename): szFilename='%s', Filename='%s'\n", szFilename, Filename);
  Number = iNumber;
  if (Number < C4S_MaxPlayer)
    PlrStartIndex = Number;

  // At client
  AtClient = iAtClient;
  SCopy(szAtClientName, AtClientName, C4MaxTitle);

  // Load core & crew info list
  printf("C4Player::Init: Before Load: Filename='%s'\n", Filename);
  if (!Load(szFilename))
    return FALSE;
  printf("C4Player::Init: After Load: Filename='%s'\n", Filename);

  // Check double players names
  char szOriginalName[C4MaxName + 1];
  SCopy(Name, szOriginalName, C4MaxName - 10);
  for (int iCount = 2; Game.Players.GetByName(Name, Number); iCount++)
    sprintf(Name, "%s (%i)", szOriginalName, iCount);

  // Scenario init
  if (fScenarioInit) {
    if (!ScenarioInit())
      return FALSE;
  }

  // Load runtime data
  else {
    // (compile using DefaultRuntimeData)
    printf("C4Player::Init: Before LoadRuntimeData: Filename='%s'\n", Filename);
    if (!LoadRuntimeData(Game.ScenarioFile))
      return FALSE;
    printf("C4Player::Init: After LoadRuntimeData: Filename='%s'\n", Filename);
    // Reset values default-overriden by old runtime data load (safety?)
    if (Number == C4P_Number_None)
      Number = iNumber;
    SCopy(szFilename, Filename);
    printf("C4Player::Init: Restored Filename='%s'\n", Filename);
    // Reset at-client for local players from no-network savegames
    if (AtClient == C4NET_NoClient)
      if (SEqualNoCase(AtClientName, "Unknown") || SEqualNoCase(AtClientName, "Local")) {
        AtClient = iAtClient;
        SCopy(szAtClientName, AtClientName, C4MaxTitle);
      }
    // Validate at-client in network games (by at-client name)
    if (Game.Network.Active)
      // Local player
      if (SEqualNoCase(AtClientName, Game.Network.LocalName))
        AtClient = Game.Network.GetClientNumber();
      // Remote player
      else {
        AtClient = C4NET_NoClient;
        // The client number is unknown: notice for non-hosts there is no way
        // to determine the presence or correct client number of any other
        // client. Players for which the client is missing cannot be removed and
        // will reside as ghost players. They will even reside if the client was
        // present and quits the game. On rejoin, that client could even
        // recapture control over those players. To avoid ghost players, the
        // host sends a C4PK_RemoveClient for every client from Scenario core
        // client list that is not present.
      }
  }

  // Check local control
  LocalControl = FALSE;
  if (!Game.Network.Active || (AtClient == Game.Network.GetClientNumber()))
    LocalControl = TRUE;

  // Init control method
  InitControl();

  printf("C4Player::Init: End: Filename='%s'\n", Filename);
  return TRUE;
}

BOOL C4Player::Save() {
  C4Group hGroup;
  // Open group
  if (!hGroup.Open(Filename, TRUE)) {
    printf("C4Player::Save failed: hGroup.Open(%s) failed: %s\n", Filename, hGroup.GetError());
    return FALSE;
  }
  // Save core
  if (!C4PlayerInfoCore::Save(hGroup)) {
    printf("C4Player::Save failed: C4PlayerInfoCore::Save failed\n");
    hGroup.Close();
    return FALSE;
  }
  // Save crew
  if (!CrewInfoList.Save(hGroup)) {
    printf("C4Player::Save failed: CrewInfoList.Save failed\n");
    hGroup.Close();
    return FALSE;
  }
  // Close group
  hGroup.Sort(C4FLS_Player);
  if (!hGroup.Close()) {
    printf("C4Player::Save failed: hGroup.Close failed\n");
    return FALSE;
  }
  // Add to reload list
  if (LocalControl)
    SAddModule(Config.Explorer.Reload, Filename);
  // Success
  return TRUE;
}

void C4Player::PlaceReadyCrew(int tx1, int tx2, int ty, C4Object *FirstBase) {
  int cnt, crewnum, ctx, cty;
  C4Object *nobj;
  C4ObjectInfo *pInfo;
  C4Def *pDef;

  // Set name source
  const char *cpNames = Config.AtExePath(C4CFN_Names);
  if (Game.Names.GetData())
    cpNames = Game.Names.GetData();

  // Old specification
  if (Game.C4S.PlrStart[PlrStartIndex].ReadyCrew.IsClear()) {
    // Target number of ready crew
    crewnum = Game.C4S.PlrStart[PlrStartIndex].Crew.Evaluate();
    // Place crew
    for (cnt = 0; cnt < crewnum; cnt++) {
      // Set standard crew
      C4ID idStdCrew = Game.C4S.PlrStart[PlrStartIndex].NativeCrew;
      // Select member from home crew, add new if necessary
      while (!(pInfo = CrewInfoList.GetIdle(idStdCrew, Game.Defs)))
        if (!CrewInfoList.New(idStdCrew, &Game.Defs, cpNames))
          break;
      // Crew placement location
      if (!pInfo || !(pDef = C4Id2Def(pInfo->id)))
        continue;
      ctx = tx1 + Random(tx2 - tx1);
      cty = ty;
      if (!Game.C4S.PlrStart[PlrStartIndex].EnforcePosition) {
        long lctx = ctx, lcty = cty;
        FindSolidGround(lctx, lcty, pDef->Shape.Wdt * 3);
        ctx = (int)lctx;
        cty = (int)lcty;
      }
      // Create object
      if (nobj = Game.CreateInfoObject(pInfo, Number, ctx, cty)) {
        // Add object to crew
        Crew.Add(nobj);
        // If base is present, enter base
        if (FirstBase) {
          nobj->Enter(FirstBase);
          nobj->SetCommand(C4CMD_Exit);
        }
      }
    }
  }

  // New specification
  else {
    // Place crew
    int id, iCount;
    for (cnt = 0; id = Game.C4S.PlrStart[PlrStartIndex].ReadyCrew.GetID(cnt, &iCount); cnt++) {

      // Minimum one clonk if empty id
      iCount = Max(iCount, 1);

      for (int cnt2 = 0; cnt2 < iCount; cnt2++) {
        // Select member from home crew, add new if necessary
        while (!(pInfo = CrewInfoList.GetIdle(id, Game.Defs)))
          if (!CrewInfoList.New(id, &Game.Defs, cpNames))
            break;
        // Safety
        if (!pInfo || !(pDef = C4Id2Def(pInfo->id)))
          continue;
        // Crew placement location
        ctx = tx1 + Random(tx2 - tx1);
        cty = ty;
        if (!Game.C4S.PlrStart[PlrStartIndex].EnforcePosition) {
          long lctx = ctx, lcty = cty;
          FindSolidGround(lctx, lcty, pDef->Shape.Wdt * 3);
          ctx = (int)lctx;
          cty = (int)lcty;
        }
        // Create object
        if (nobj = Game.CreateInfoObject(pInfo, Number, ctx, cty)) {
          // Add object to crew
          Crew.Add(nobj);
          // If base is present, enter base
          if (FirstBase) {
            nobj->Enter(FirstBase);
            nobj->SetCommand(C4CMD_Exit);
          }
        }
      }
    }
  }
}

C4Object *CreateLine(C4ID linetype, int owner, C4Object *fobj, C4Object *tobj);

BOOL CreatePowerConnection(C4Object *fbase, C4Object *tbase) {
  if (CreateLine(C4ID_PowerLine, fbase->Owner, fbase, tbase))
    return TRUE;
  return FALSE;
}

void C4Player::PlaceReadyBase(int &tx, int &ty, C4Object **pFirstBase) {
  int cnt, cnt2, ctx, cty;
  C4Def *def;
  C4ID cid;
  C4Object *cbase, *fpower = NULL;
  // Create ready base structures
  for (cnt = 0; (cid = Game.C4S.PlrStart[PlrStartIndex].ReadyBase.GetID(cnt)); cnt++) {
    if (def = C4Id2Def(cid))
      for (cnt2 = 0; cnt2 < Game.C4S.PlrStart[PlrStartIndex].ReadyBase.GetCount(cnt); cnt2++) {
        ctx = tx;
        cty = ty;
        {
          long lctx = ctx, lcty = cty;
          if (Game.C4S.PlrStart[PlrStartIndex].EnforcePosition || FindConSiteSpot(lctx, lcty, def->Shape.Wdt, def->Shape.Hgt, def->Category, 20)) {
            ctx = (int)lctx;
            cty = (int)lcty;
            if (cbase = Game.CreateObjectConstruction(cid, Number, ctx, cty, FullCon, TRUE)) {
              // FirstBase
              if (!(*pFirstBase))
                if (cbase->Def->CanBeBase) {
                  *pFirstBase = cbase;
                  tx = (*pFirstBase)->x;
                  ty = (*pFirstBase)->y;
                }
              // First power plant
              if (cbase->Def->LineConnect & C4D_Power_Generator)
                if (!fpower)
                  fpower = cbase;
            }
          }
        }
      }
  }

  // Power connections
  C4ObjectLink *clnk;
  C4Object *cobj;
  if (Game.Rules & C4RULE_StructuresNeedEnergy)
    if (fpower)
      for (clnk = Game.Objects.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
        if (cobj->Owner == Number)
          if (cobj->Def->LineConnect & C4D_Power_Consumer)
            CreatePowerConnection(fpower, cobj);
}

void C4Player::PlaceReadyVehic(int tx1, int tx2, int ty, C4Object *FirstBase) {
  int cnt, cnt2, ctx, cty;
  C4Def *def;
  C4ID cid;
  C4Object *cobj;
  for (cnt = 0; (cid = Game.C4S.PlrStart[PlrStartIndex].ReadyVehic.GetID(cnt)); cnt++) {
    if (def = C4Id2Def(cid))
      for (cnt2 = 0; cnt2 < Game.C4S.PlrStart[PlrStartIndex].ReadyVehic.GetCount(cnt); cnt2++) {
        ctx = tx1 + Random(tx2 - tx1);
        cty = ty;
        if (!Game.C4S.PlrStart[PlrStartIndex].EnforcePosition) {
          long lctx = ctx, lcty = cty;
          FindLevelGround(lctx, lcty, def->Shape.Wdt, 6);
          ctx = (int)lctx;
          cty = (int)lcty;
        }
        if (cobj = Game.CreateObject(cid, Number, ctx, cty)) {
          if (FirstBase) // First base overrides target location
          {
            cobj->Enter(FirstBase);
            cobj->SetCommand(C4CMD_Exit);
          }
        }
      }
  }
}

void C4Player::PlaceReadyMaterial(int tx1, int tx2, int ty, C4Object *FirstBase) {
  int cnt, cnt2, ctx, cty;
  C4Def *def;
  C4ID cid;

  // In base
  if (FirstBase) {
    FirstBase->CreateContentsByList(Game.C4S.PlrStart[PlrStartIndex].ReadyMaterial);
  }

  // Outside
  else {
    for (cnt = 0; (cid = Game.C4S.PlrStart[PlrStartIndex].ReadyMaterial.GetID(cnt)); cnt++) {
      if (def = C4Id2Def(cid))
        for (cnt2 = 0; cnt2 < Game.C4S.PlrStart[PlrStartIndex].ReadyMaterial.GetCount(cnt); cnt2++) {
          ctx = tx1 + Random(tx2 - tx1);
          cty = ty;
          if (!Game.C4S.PlrStart[PlrStartIndex].EnforcePosition) {
            long lctx = ctx, lcty = cty;
            FindSolidGround(lctx, lcty, def->Shape.Wdt);
            ctx = (int)lctx;
            cty = (int)lcty;
          }
          Game.CreateObject(cid, Number, ctx, cty);
        }
    }
  }
}

BOOL C4Player::ScenarioInit() {
  int ptx, pty;

  // Set color
  int iColor = PrefColor;
  while (Game.Players.ColorTaken(iColor)) {
    ++iColor %= C4MaxColor;
    if (iColor == PrefColor)
      break;
  }
  Color = iColor;

  // Wealth, home base materials, abilities
  Wealth = Game.C4S.PlrStart[PlrStartIndex].Wealth.Evaluate();
  HomeBaseMaterial = Game.C4S.PlrStart[PlrStartIndex].HomeBaseMaterial;
  HomeBaseMaterial.ConsolidateValids(Game.Defs);
  HomeBaseProduction = Game.C4S.PlrStart[PlrStartIndex].HomeBaseProduction;
  HomeBaseProduction.ConsolidateValids(Game.Defs);
  Knowledge = Game.C4S.PlrStart[PlrStartIndex].BuildKnowledge;
  Knowledge.ConsolidateValids(Game.Defs);
  Magic = Game.C4S.PlrStart[PlrStartIndex].Magic;
  Magic.ConsolidateValids(Game.Defs);
  if (Magic.IsClear())
    Magic.Load(Game.Defs, C4D_Magic); // All magic default if empty
  Magic.SortByValue(Game.Defs);

  // Starting position
  ptx = Game.C4S.PlrStart[PlrStartIndex].PositionX;
  pty = Game.C4S.PlrStart[PlrStartIndex].PositionY;

  // Zoomed position
  if (ptx > -1)
    ptx = BoundBy(ptx * Game.C4S.Landscape.MapZoom.Evaluate(), 0, GBackWdt - 1);
  if (pty > -1)
    pty = BoundBy(pty * Game.C4S.Landscape.MapZoom.Evaluate(), 0, GBackHgt - 1);

  // Standard position (PrefPosition)
  if (ptx < 0)
    if (Game.StartupPlayerCount >= 2) {
      int iMaxPos = Game.StartupPlayerCount;
      // Map preferred position to available positions
      int iStartPos = BoundBy(PrefPosition * iMaxPos / C4P_MaxPosition, 0, iMaxPos - 1);
      int iPosition = iStartPos;
      // Distribute according to availability
      while (Game.Players.PositionTaken(iPosition)) {
        ++iPosition %= iMaxPos;
        if (iPosition == iStartPos)
          break;
      }
      Position = iPosition;
      // Set x position
      ptx = BoundBy(16 + Position * (GBackWdt - 32) / (iMaxPos - 1), 0, GBackWdt - 16);
    }

  // All-random position
  if (ptx < 0)
    ptx = 16 + Random(GBackWdt - 32);
  if (pty < 0)
    pty = 16 + Random(GBackHgt - 32);

  Game.Landscape.LockQBA();

  // Place to solid ground
  if (!Game.C4S.PlrStart[PlrStartIndex].EnforcePosition) {
    long lptx = ptx, lpty = pty;
    FindSolidGround(lptx, lpty, 30);
    ptx = (int)lptx;
    pty = (int)lpty;
  }

  // Place Readies
  C4Object *FirstBase = NULL;
  if (!Game.C4S.Head.NoInitialize) {
    PlaceReadyBase(ptx, pty, &FirstBase);
    PlaceReadyMaterial(ptx - 10, ptx + 10, pty, FirstBase);
    PlaceReadyVehic(ptx - 30, ptx + 30, pty, FirstBase);
  }
  PlaceReadyCrew(ptx - 30, ptx + 30, pty, FirstBase);

  // Scenario script initialization
  Game.Script.Call(NULL, PSF_InitializePlayer, Number, ptx, pty, (long)FirstBase);

  Game.Landscape.UnLockQBA();

  return TRUE;
}

BOOL C4Player::FinalInit(BOOL fInitialValue) {
  if (!Status)
    return TRUE;

  // Init player's gamepad control
  if (LocalControl)
    if (Control == C4P_Control_GamePad1)
      Game.GamePadCon1.Initialize(Number, 0);

  // Init player's mouse control
  if (LocalControl)
    if (MouseControl)
      Game.MouseControl.Init(Number);

  // Melee default hostility
  if (Game.FindObject(C4Id("MELE")) || Game.FindObject(C4Id("MEL2")))
    for (C4Player *pPlr = Game.Players.First; pPlr; pPlr = pPlr->Next)
      if (pPlr != this)
        Hostility.SetIDCount(pPlr->Number + 1, 1, TRUE);

  // Set initial value
  if (fInitialValue) {
    UpdateValue();
    InitialValue = Value;
  }

  // Cursor
  if (!Cursor)
    AdjustCursorCommand();

  // Assign Captain
  if (Game.Objects.Find(C4Id("KILC")))
    if (!Captain)
      Captain = GetHiRankActiveCrew();

  // Update counts, pointers, views, value
  UpdateValue();
  Execute();

  // Mouse control: init fog of war
  if (MouseControl) {
    FogOfWar = new C4FogOfWar;
    FogOfWar->Init(GBackWdt, GBackHgt);
    UpdateFogOfWar();
  }

  return TRUE;
}

BOOL C4Player::Sell2Home(C4Object *pObj) {
  C4Object *cObj;
  // Valid checks
  if (!pObj)
    return FALSE;
  if (Eliminated)
    return FALSE;
  // No crew members
  if (pObj->OCF & OCF_CrewMember)
    return FALSE;
  // Sell contents first
  while (cObj = pObj->Contents.GetObject()) {
    if (pObj->Contained)
      cObj->Enter(pObj->Contained);
    else
      cObj->Exit(cObj->x, cObj->y);
    Sell2Home(cObj);
  }
  // Do transaction
  DoWealth(+pObj->GetValue());
  // Barrel handling (hardcoded crap)
  if ((pObj->Def->id == C4Id("WBRL")) || (pObj->Def->id == C4Id("ABRL")) || (pObj->Def->id == C4Id("OBRL")) || (pObj->Def->id == C4Id("LBRL")))
    pObj->ChangeDef(C4Id("BARL"));
  // Add to homebase material
  HomeBaseMaterial.IncreaseIDCount(pObj->Def->id, pObj->Def->Rebuyable);
  // Remove object
  if (pObj->Contained)
    pObj->Exit();
  pObj->AssignRemoval();
  // Done
  return TRUE;
}

BOOL C4Player::DoWealth(int iChange) {
  Wealth = BoundBy(Wealth + iChange, 0, 10000);
  if (LocalControl) {
    if (iChange > 0)
      SoundEffect("Cash");
    if (iChange < 0)
      SoundEffect("UnCash");
  }
  ViewWealth = C4ViewDelay;
  return TRUE;
}

void C4Player::SetViewMode(int iMode, C4Object *pTarget) {
  ViewMode = iMode;
  ViewTarget = pTarget;
}

void C4Player::Evaluate() {
  const int SuccessBonus = 100;

  // Set last round
  SCopy(Game.ScenarioTitle, LastRound.Title);
  time(&LastRound.Date);
  LastRound.Duration = Game.Time;
  LastRound.Won = !Eliminated;
  // Melee: personal value gain score ...check Game.Objects(C4D_Goal)
  if (Game.C4S.Game.IsMelee())
    LastRound.Score = Max(ValueGain, 0);
  // Cooperative: shared score
  else
    LastRound.Score = Game.Players.AverageValueGain();
  LastRound.Level = 0; // unknown...
  LastRound.Bonus = SuccessBonus * LastRound.Won;
  LastRound.FinalScore = LastRound.Score + LastRound.Bonus;
  LastRound.TotalScore = Score + LastRound.FinalScore;

  // Update player
  Rounds++;
  if (LastRound.Won)
    RoundsWon++;
  else
    RoundsLost++;
  Score = LastRound.TotalScore;
  TotalPlayingTime += Game.Time;

  // Crew
  CrewInfoList.Evaluate();
}

void C4Player::Surrender() {
  if (Surrendered)
    return;
  Surrendered = TRUE;
  Eliminated = TRUE;
  RetireDelay = C4RetireDelay;
  SoundEffect("Eliminated");
  sprintf(OSTR, LoadResStr(IDS_PRC_PLRSURRENDERED), Name);
  Log(OSTR);
}

BOOL C4Player::SetHostility(int iOpponent, int iHostility) {
  // Check opponent valid
  if (!ValidPlr(iOpponent) || (iOpponent == Number))
    return FALSE;
  // Set hostility
  Hostility.SetIDCount(iOpponent + 1, iHostility, TRUE);
  // Announce
  SoundEffect("Trumpet");
  sprintf(OSTR, LoadResStr(iHostility ? IDS_PLR_HOSTILITY : IDS_PLR_NOHOSTILITY), Name, Game.Players.Get(iOpponent)->Name);
  Log(OSTR);
  // Success
  return TRUE;
}

C4Object *C4Player::GetHiExpActiveCrew() {
  C4ObjectLink *clnk;
  C4Object *cobj, *hiexp = NULL;
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Alive)
      if (!hiexp || (cobj->Info->Experience > hiexp->Info->Experience))
        hiexp = cobj;
  return hiexp;
}

C4Object *C4Player::GetHiRankActiveCrew() {
  C4ObjectLink *clnk;
  C4Object *cobj, *hirank = NULL;
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Alive)
      if (!hirank || (cobj->Info->Rank > hirank->Info->Rank))
        hirank = cobj;
  return hirank;
}

void C4Player::CheckCrewExPromotion() {
  C4Object *hirank;
  if (hirank = GetHiRankActiveCrew())
    if (hirank->Info->Rank < 1) // No F�hnrich -> except. promo.
      if (hirank = GetHiExpActiveCrew())
        hirank->Promote(1, TRUE);
}

BOOL C4Player::Message(const char *szMsg) {
  if (!szMsg)
    return FALSE;
  SCopy(szMsg, MessageBuf, 256);
  MessageStatus = SLen(szMsg) * 2;
  return TRUE;
}

void C4Player::Clear() {
  Crew.Clear();
  CrewInfoList.Clear();
  Menu.Clear();
  Portrait.Clear();
  if (FogOfWar)
    delete FogOfWar;
  FogOfWar = NULL;
}

void C4Player::Default() {
  Filename[0] = 0;
  Number = C4P_Number_None;
  DefaultRuntimeData();
  Menu.Default();
  Crew.Default();
  CrewInfoList.Default();
  LocalControl = FALSE;
  Portrait.Default();
  Next = NULL;
  FogOfWar = NULL;
  RedrawFogOfWar = FALSE;
}

BOOL C4Player::Load(const char *szFilename) {
  C4Group hGroup;
  printf("C4Player::Load: Start: Filename='%s'\n", Filename);
  printf("  this=%p, &Filename=%p, end_of_core=%p\n", this, Filename, (char*)this + sizeof(C4PlayerInfoCore));
  // Open group
  if (!hGroup.Open(szFilename))
    return FALSE;
  printf("C4Player::Load: After hGroup.Open: Filename='%s'\n", Filename);
  // Load core
  if (!C4PlayerInfoCore::Load(hGroup)) {
    hGroup.Close();
    return FALSE;
  }
  printf("C4Player::Load: After C4PlayerInfoCore::Load: Filename='%s'\n", Filename);
  // Load portrait
  /*if (Portrait.Load(hGroup,C4CFN_Portrait))
          Portrait.SetPalette(GfxR->GamePalette,TRUE);*/
  // Load crew info list
  CrewInfoList.Load(hGroup);
  printf("C4Player::Load: After CrewInfoList.Load: Filename='%s'\n", Filename);
  // Close group
  hGroup.Close();
  printf("C4Player::Load: End: Filename='%s'\n", Filename);
  // Success
  return TRUE;
}

void C4Player::DrawHostility(C4FacetEx &cgo, int iIndex) {
  C4Player *pPlr;
  if (pPlr = Game.Players.GetByIndex(iIndex)) {
    // Portrait
    /*if (Config.Graphics.ShowPortraits && pPlr->Portrait.Surface)
            pPlr->Portrait.Draw(cgo);
    // Standard player image
    else*/
    Game.GraphicsResource.fctCrew.Draw(cgo, TRUE, pPlr->Color);
    // Hostility flag
    if (pPlr != this)
      Game.GraphicsResource.fctFlag.DrawX(cgo.Surface, cgo.X, cgo.Y + cgo.Hgt / 3, 2 * cgo.Wdt / 3, 2 * cgo.Hgt / 3, Hostility.GetIDCount(pPlr->Number + 1) ? 1 : C4MaxColor);
  }
}

BOOL C4Player::MakeCrewMember(C4Object *pObj) {
  C4ObjectInfo *cInf = NULL;
  if (!pObj || !pObj->Def->CrewMember)
    return FALSE;

  // Find crew info by name
  if (pObj->nInfo[0])
    cInf = CrewInfoList.GetIdle(pObj->nInfo);

  // Set name source
  const char *cpNames = Config.AtExePath(C4CFN_Names);
  if (Game.Names.GetData())
    cpNames = Game.Names.GetData();

  // Find crew info by id
  if (!cInf)
    while (!(cInf = CrewInfoList.GetIdle(pObj->id, Game.Defs)))
      if (!CrewInfoList.New(pObj->id, &Game.Defs, cpNames))
        return FALSE;

  // Set object info
  pObj->Info = cInf;

  // Add to crew
  Crew.Add(pObj);

  return TRUE;
}

void C4Player::ExecuteControl() {

  // LastCom
  if (LastCom != COM_None) {
    // Advance delay counter
    LastComDel++;
    // Check for COM_Single com (after delay)
    if (LastComDel > C4DoubleClick) {
      // Pass additional COM_Single com
      DirectCom(LastCom | COM_Single,
                0); // (currently, com data is not stored by Single coms)
      LastCom = COM_None;
      LastComDel = 0;
    }
  }

  // LastComDownDouble
  if (LastComDownDouble > 0)
    LastComDownDouble--;
}

void C4Player::AdjustCursorCommand() {
  // Reset view
  SetViewMode(C4PVM_Cursor);
  // Set cursor to hirank Select clonk
  C4Object *cobj, *pHiRank = NULL;
  C4ObjectLink *clnk;
  // Find hirank Select
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    if (cobj->Status)
      if (cobj->Select)
        if (!pHiRank || (cobj->Info && (cobj->Info->Rank > pHiRank->Info->Rank)))
          pHiRank = cobj;
  // If none, check non-Selects as well
  if (!pHiRank) {
    for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
      if (cobj->Status)
        if (!pHiRank || (cobj->Info && (cobj->Info->Rank > pHiRank->Info->Rank)))
          pHiRank = cobj;
  }
  Cursor = pHiRank;
  if (Cursor)
    Cursor->Select = 1;

  // Updates
  CursorFlash = 30;
}

void C4Player::CursorRight() {
  C4ObjectLink *cLnk;
  // Get next crew member
  if (cLnk = Crew.GetLink(Cursor))
    for (cLnk = cLnk->Next; cLnk; cLnk = cLnk->Next)
      if (cLnk->Obj->Status)
        break;
  if (!cLnk)
    for (cLnk = Crew.First; cLnk; cLnk = cLnk->Next)
      if (cLnk->Obj->Status)
        break;
  if (cLnk)
    Cursor = cLnk->Obj;
  // Updates
  CursorFlash = 30;
  CursorSelection = 1;
}

void C4Player::CursorLeft() {
  C4ObjectLink *cLnk;
  // Get prev crew member
  if (cLnk = Crew.GetLink(Cursor))
    for (cLnk = cLnk->Prev; cLnk; cLnk = cLnk->Prev)
      if (cLnk->Obj->Status)
        break;
  if (!cLnk)
    for (cLnk = Crew.Last; cLnk; cLnk = cLnk->Prev)
      if (cLnk->Obj->Status)
        break;
  if (cLnk)
    Cursor = cLnk->Obj;
  // Updates
  CursorFlash = 30;
  CursorSelection = 1;
}

void C4Player::UnselectCrew() {
  C4Object *cObj;
  C4ObjectLink *cLnk;
  for (cLnk = Crew.First; cLnk && (cObj = cLnk->Obj); cLnk = cLnk->Next)
    if (cObj->Status)
      cObj->Select = 0;
}

void C4Player::SelectSingleByCursor() {
  // Unselect crew
  UnselectCrew();
  // Select cursor
  if (Cursor)
    Cursor->Select = 1;
  // Updates
  SelectFlash = 30;
  AdjustCursorCommand();
}

void C4Player::SelectSingle(C4Object *tobj) {
  // Set cursor
  Cursor = tobj;
  // Now use PlayerSelectSingleByCursor
  SelectSingleByCursor();
}

void C4Player::CursorToggle() {
  C4ObjectLink *clnk;
  // Selection mode: toggle cursor select
  if (CursorSelection) {
    if (Cursor)
      Cursor->Select = !Cursor->Select;
    CursorToggled = 1;
  }
  // Pure toggle: toggle all Select
  else {
    for (clnk = Crew.First; clnk; clnk = clnk->Next)
      clnk->Obj->Select = !clnk->Obj->Select;
    AdjustCursorCommand();
  }
  // Updates
  SelectFlash = 30;
}

void C4Player::SelectAllCrew() {
  C4ObjectLink *clnk;
  // Select all crew
  for (clnk = Crew.First; clnk; clnk = clnk->Next)
    clnk->Obj->Select = 1;
  // Updates
  AdjustCursorCommand();
  CursorSelection = CursorToggled = 0;
  SelectFlash = 30;
  // Game display
  if (LocalControl)
    SoundEffect("Ding");
}

void C4Player::UpdateSelectionToggleStatus() {
  if (CursorSelection)
    // Select toggled: cursor to hirank
    if (CursorToggled)
      AdjustCursorCommand();
    // Cursor select only: single control
    else
      SelectSingleByCursor();
  CursorSelection = 0;
  CursorToggled = 0;
}

BOOL C4Player::ObjectCom(BYTE byCom, int iData) // By DirectCom
{
  if (Eliminated)
    return FALSE;
  // Hide startup
  if (ShowStartup)
    ShowStartup = FALSE;
  // If regular com, update cursor & selection status
  if (!(byCom & COM_Single) && !(byCom & COM_Double))
    UpdateSelectionToggleStatus();
  // Apply direct com to cursor object
  if (Cursor)
    Cursor->DirectCom(byCom, iData);
  // Done
  return TRUE;
}

BOOL C4Player::ObjectCommand(int iCommand, C4Object *pTarget, int iX, int iY, C4Object *pTarget2, int iData, int iMode) {
  // Eliminated
  if (Eliminated)
    return FALSE;
  // Hide startup
  if (ShowStartup)
    ShowStartup = FALSE;
  // Update selection & toggle status
  UpdateSelectionToggleStatus();

  // Apply to all selected crew members (in cursor range) except pTarget.
  // Set, Add, Append mode flags may be combined and have a priority order.
  C4ObjectLink *cLnk;
  C4Object *cObj;
  for (cLnk = Crew.First; cLnk && (cObj = cLnk->Obj); cLnk = cLnk->Next) {
    if (cObj->Status)
      if (cObj->Select)
        if (!(iMode & C4P_Command_Range) || (Cursor && Inside(cObj->x - Cursor->x, -15, +15) && Inside(cObj->y - Cursor->y, -15, +15)))
          if (cObj != pTarget)

            if (iMode & C4P_Command_Append)
              cObj->AddCommand(iCommand, pTarget, iX, iY, 0, pTarget2, TRUE, iData, TRUE);
            else if (iMode & C4P_Command_Add)
              cObj->AddCommand(iCommand, pTarget, iX, iY, 0, pTarget2, TRUE, iData, FALSE);
            else if (iMode & C4P_Command_Set)
              cObj->SetCommand(iCommand, pTarget, iX, iY, pTarget2, TRUE, iData);
  }

  // Success
  return TRUE;
}

void C4Player::DirectCom(BYTE byCom, int iData) // By InCom or ExecuteControl
{
  switch (byCom) {
  case COM_CursorLeft:
    CursorLeft();
    break;
  case COM_CursorRight:
    CursorRight();
    break;
  case COM_CursorToggle:
    CursorToggle();
    break;
  case COM_CursorToggle_D:
    SelectAllCrew();
    break;

  default:
    ObjectCom(byCom, iData);
    break;
  }
}

void C4Player::InCom(BYTE byCom, int iData) {

  // Menu control: no single/double processing
  if (Inside(byCom, COM_MenuEnter, COM_MenuNavigation2)) {
    DirectCom(byCom, iData);
    return;
  }

  // Reset view
  SetViewMode(C4PVM_Cursor);

  // Check LastCom buffer for prior COM_Single
  if (LastCom != COM_None)
    if (LastCom != byCom)
      DirectCom(LastCom | COM_Single, iData);

  // Check LastCom buffer for COM_Double
  if (LastCom == byCom)
    byCom |= COM_Double;

  // Pass regular/COM_Double byCom to player
  DirectCom(byCom, iData);

  // LastCom/Del process
  LastCom = byCom;
  LastComDel = 0;

  // LastComDownDouble process
  if (byCom == COM_Down_D)
    LastComDownDouble = C4DoubleClick;
}

BOOL C4Player::Compile(const char *szSource, const char *szSecExt) {
  C4Compiler Compiler;
  DefaultRuntimeData();
  return Compiler.CompileStructure(C4CR_Player, szSource, this, szSecExt);
}

BOOL C4Player::Decompile(char **ppOutput, int *ipSize, const char *szSecExt) {
  C4Compiler Compiler;
  C4Player dC4P;
  return Compiler.DecompileStructure(C4CR_Player, this, &dC4P, ppOutput, ipSize, szSecExt);
}

BOOL C4Player::LoadRuntimeData(C4Group &hGroup) {
  const char *pSource;
  // Use loaded game text component
  if (!(pSource = Game.GameText.GetData()))
    return FALSE;
  // Search player section
  sprintf(OSTR, "%i", Game.Players.GetIndex(this) + 1);
  if (!Compile(pSource, OSTR))
    return FALSE;
  // Denumerate pointers
  DenumeratePointers();
  // Success
  return TRUE;
}

void C4Player::ExecHomeBaseProduction() {
  const int MaxHomeBaseProduction = 25;
  ProductionDelay++;
  if (ProductionDelay >= 60) // Minute Production Unit
  {
    ProductionDelay = 0;
    ProductionUnit++;
    for (int cnt = 0; HomeBaseProduction.GetID(cnt); cnt++)
      if (HomeBaseProduction.GetCount(cnt) > 0)
        if (ProductionUnit % BoundBy(11 - HomeBaseProduction.GetCount(cnt), 1, 10) == 0)
          if (HomeBaseMaterial.GetIDCount(HomeBaseProduction.GetID(cnt)) < MaxHomeBaseProduction)
            HomeBaseMaterial.IncreaseIDCount(HomeBaseProduction.GetID(cnt));
  }
}

void C4Player::UpdateCounts() {
  int nclkcnt, nselcnt;
  C4Object *cobj;
  C4ObjectLink *clnk;
  nclkcnt = nselcnt = 0;
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next) {
    nclkcnt++;
    if (cobj->Select)
      nselcnt++;
  }
  if (CrewCnt != nclkcnt)
    CrewCnt = nclkcnt;
  if (SelectCount != nselcnt)
    SelectCount = nselcnt;
}

void C4Player::CheckElimination() {
  // Already eliminated safety
  if (Eliminated)
    return;
  // Do-elimination flag
  BOOL fEliminate = FALSE;
  // Standard elimination: no crew
  if (CrewCnt <= 0)
    fEliminate = TRUE;
  // Do elimination
  if (fEliminate)
    Eliminate();
}

void C4Player::UpdateView() {
  BOOL fUpdateFOW = FALSE;
  switch (ViewMode) {
  case C4PVM_Cursor:
    if (Cursor) {
      // Fow update on long cursor position jump
      if (Distance(ViewX, ViewY, Cursor->x, Cursor->y) > 300)
        fUpdateFOW = TRUE;
      ViewX = Cursor->x;
      ViewY = Cursor->y;
    }
    break;
  case C4PVM_Target:
    if (ViewTarget) {
      ViewX = ViewTarget->x;
      ViewY = ViewTarget->y;
      // High-rate fow update on view target
      if (!Tick5)
        fUpdateFOW = TRUE;
    }
    break;
  case C4PVM_Scrolling:
    break;
  }
  // Update fog of war
  if (fUpdateFOW)
    if (FogOfWar)
      FogOfWar->AddVisibility(ViewX, ViewY);
}

void C4Player::DefaultRuntimeData() {
  Status = 0;
  Eliminated = 0;
  Surrendered = 0;
  AtClient = C4NET_NoClient;
  SCopy("Local", AtClientName);
  Color = -1;
  Control = C4P_Control_None;
  MouseControl = FALSE;
  Position = -1;
  PlrStartIndex = 0;
  RetireDelay = 0;
  ViewMode = C4PVM_Cursor;
  ViewX = ViewY = 0;
  ViewTarget = NULL;
  CursorSelection = CursorToggled = 0;
  ShowStartup = TRUE;
  CrewCnt = 0;
  ViewWealth = ViewValue = 0;
  ShowControl = ShowControlPos = 0;
  Wealth = 0;
  Points = 0;
  Value = InitialValue = ValueGain = 0;
  ObjectsOwned = 0;
  Captain = NULL;
  ProductionDelay = ProductionUnit = 0;
  Cursor = NULL;
  SelectCount = 0;
  SelectFlash = CursorFlash = 30;
  LastCom = 0;
  LastComDel = 0;
  LastComDownDouble = 0;
  CursorSelection = CursorToggled = 0;
  MessageStatus = 0;
  MessageBuf[0] = 0;
  Hostility.Default();
  HomeBaseMaterial.Default();
  HomeBaseProduction.Default();
  Knowledge.Default();
  Magic.Default();
  FlashCom = 0;
}

BOOL C4Player::ActivateMenuMain() {

  // Menu symbol/init
  C4FacetEx fctSymbol;
  fctSymbol.Create(C4SymbolSize, C4SymbolSize);
  DrawMenuSymbol(C4MN_Main, fctSymbol, Number, NULL);
  Menu.Init(fctSymbol, LoadResStr(IDS_MENU_CPMAIN), Number);
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  fctSymbol.Default();

  // Goals
  SCopy(LoadResStr(IDS_MENU_CPGOALSINFO), OSTR);
  Menu.Add(LoadResStr(IDS_MENU_CPGOALS), GfxR->fctMenu.GetPhase(2), "ActivateMenu:Goals", C4MN_Item_NoCount, NULL, 0, OSTR);
  // Rules
  SCopy(LoadResStr(IDS_MENU_CPRULESINFO), OSTR);
  Menu.Add(LoadResStr(IDS_MENU_CPRULES), GfxR->fctMenu.GetPhase(3), "ActivateMenu:Rules", C4MN_Item_NoCount, NULL, 0, OSTR);
  // Hostility
  if (Game.Players.GetCount() > 1) {
    SCopy(LoadResStr(IDS_MENU_CPATTACKINFO), OSTR);
    Menu.Add(LoadResStr(IDS_MENU_CPATTACK), GfxR->fctFlag.GetPhase(1), "ActivateMenu:Hostility", C4MN_Item_NoCount, NULL, 0, OSTR);
  }
  // Surrender
  if (Game.Objects.Find(C4Id("SURR"))) {
    SCopy(LoadResStr(IDS_MENU_CPSURRENDERINFO), OSTR);
    Menu.Add(LoadResStr(IDS_MENU_CPSURRENDER), GfxR->fctSurrender.GetPhase(), "ActivateMenu:Surrender", C4MN_Item_NoCount, NULL, 0, OSTR);
  }
  // NewPlayer
  if (Game.Players.GetCount() < Game.C4S.Head.MaxPlayer) {
    SCopy(LoadResStr(IDS_MENU_CPNEWPLAYERINFO), OSTR);
    Menu.Add(LoadResStr(IDS_MENU_CPNEWPLAYER), GfxR->fctPlayer.GetPhase(), "ActivateMenu:NewPlayer", C4MN_Item_NoCount, NULL, 0, OSTR);
  }
  // Save game
  if (!Game.Network.Active || Game.Network.Host) {
    SCopy(LoadResStr(IDS_MENU_CPSAVEGAMEINFO), OSTR);
    Menu.Add(LoadResStr(IDS_MENU_CPSAVEGAME), GfxR->fctMenu.GetPhase(0), "Save:Game", C4MN_Item_NoCount, NULL, 0, OSTR);
  }
  // Save scenario
  if (!Game.Network.Active || Game.Network.Host) {
    SCopy(LoadResStr(IDS_MENU_CPSAVESCENARIOINFO), OSTR);
    Menu.Add(LoadResStr(IDS_MENU_CPSAVESCENARIO), GfxR->fctMenu.GetPhase(1), "Save:Scenario", C4MN_Item_NoCount, NULL, 0, OSTR);
  }

  // No empty menus
  if (Menu.GetItemCount() == 0)
    Menu.Close();

  return TRUE;
}

BOOL C4Player::MenuCommand(const char *szCommand) {
  // ActivateMenu
  if (SEqual2(szCommand, "ActivateMenu:")) {
    if (SEqual(szCommand + 13, "Hostility"))
      return ActivateMenuHostility();
    if (SEqual(szCommand + 13, "Surrender"))
      return ActivateMenuSurrender();
    if (SEqual(szCommand + 13, "NewPlayer"))
      return ActivateMenuNewPlayer();
    if (SEqual(szCommand + 13, "Goals"))
      return ActivateMenuGoals();
    if (SEqual(szCommand + 13, "Rules"))
      return ActivateMenuRules();
  }
  // JoinPlayer
  if (SEqual2(szCommand, "JoinPlayer:")) {
    Game.Input.AddJoinPlayer(szCommand + 11);
    return TRUE;
  }
  // SetHostility
  if (SEqual2(szCommand, "SetHostility:")) {
    int iOpponent;
    sscanf(szCommand + 13, "%i", &iOpponent);
    Game.Input.AddSetHostility(Number, iOpponent, !Hostility.GetIDCount(iOpponent + 1));
    return TRUE;
  }
  // SurrenderPlayer
  if (SEqual2(szCommand, "Surrender")) {
    Game.Input.AddSurrenderPlayer(Number);
    return TRUE;
  }
  // Save
  if (SEqual2(szCommand, "Save:Game")) {
    Game.QuickSave(TRUE);
    return TRUE;
  }
  // Save
  if (SEqual2(szCommand, "Save:Scenario")) {
    Game.QuickSave(FALSE);
    return TRUE;
  }
  // No valid command
  return FALSE;
}

BOOL C4Player::ActivateMenuNewPlayer() {
  Menu.ActivateNewPlayer(Number);
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  return TRUE;
}

BOOL C4Player::ActivateMenuHostility() {
  // Init menu
  C4FacetEx fctSymbol;
  fctSymbol.Create(C4SymbolSize, C4SymbolSize);
  GfxR->fctFlag.Draw(fctSymbol, TRUE, 1);
  Menu.Init(fctSymbol, LoadResStr(IDS_MENU_CPATTACK), Number, C4MN_Extra_None, 0, C4MN_Hostility);
  fctSymbol.Default();
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  Menu.SetPermanent(TRUE);
  Menu.Refill();
  return TRUE;
}

BOOL C4Player::ActivateMenuSurrender() {
  // Menu symbol/init
  Menu.Init(GfxR->fctSurrender.GetPhase(), LoadResStr(IDS_MENU_CPSURRENDER), Number);
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  // Items
  sprintf(OSTR, "SurrenderPlayer:%i", Number);
  Menu.Add(LoadResStr(IDS_MENU_SURRENDER), GfxR->fctSurrender.GetPhase(), OSTR);
  // Done
  return TRUE;
}

void C4Player::EnumeratePointers() {
  // Cursor
  Cursor = Game.Objects.Enumerated(Cursor);
  // Captain
  Captain = Game.Objects.Enumerated(Captain);
}

void C4Player::DenumeratePointers() {
  // Cursor
  Cursor = Game.Objects.Denumerated(Cursor);
  // Captain
  Captain = Game.Objects.Denumerated(Captain);
}

void C4Player::RemoveCrewObjects() {
  C4Object *cobj;
  C4ObjectLink *clnk;

  // Assign removal crew
  while (cobj = Crew.GetObject())
    cobj->AssignRemoval();

  // Assign removal flags
  for (clnk = Game.Objects.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    // Status
    if (cobj->Status)
      // ID
      if (cobj->id == C4ID_Flag)
        // Owner
        if (cobj->Owner == Number)
          // Remove
          cobj->AssignRemoval();
}

BOOL C4Player::DoPoints(int iChange) {
  Points = BoundBy(Points + iChange, 0, 100000);
  ViewValue = C4ViewDelay;
  return TRUE;
}

void C4Player::SetCursor(C4Object *pObj) {
  // Set cursor
  Cursor = pObj;
  // Select object
  if (Cursor)
    Cursor->Select = 1;
  // View flash
  CursorFlash = 30;
  SelectFlash = 30;
}

void C4Player::SelectCrew(C4ObjectList &rList) {
  // Unselect
  UnselectCrew();
  // Select (does not check whether objects are in crew)
  C4ObjectLink *clnk;
  for (clnk = rList.First; clnk; clnk = clnk->Next)
    clnk->Obj->Select = 1;
  // Updates
  AdjustCursorCommand();
  CursorSelection = CursorToggled = 0;
  SelectFlash = 30;
}

void C4Player::ScrollView(int iX, int iY, int ViewWdt, int ViewHgt) {
  SetViewMode(C4PVM_Scrolling);
  ViewX = BoundBy(ViewX + iX, ViewWdt / 2, GBackWdt - ViewWdt / 2);
  ViewY = BoundBy(ViewY + iY, ViewHgt / 2, GBackHgt - ViewHgt / 2);
}

void C4Player::InitControl() {
  // Set control
  Control = C4P_Control_None;
  // Preferred control
  int iControl = PrefControl;
  // Choose next while control taken
  if (Game.Players.ControlTaken(iControl)) {
    // Preferred control taken, search for available keyboard control
    for (iControl = C4P_Control_Keyboard1; iControl <= C4P_Control_Keyboard4; iControl++)
      if (!Game.Players.ControlTaken(iControl)) // Available control found
        break;
    // No available control found
    if (iControl > C4P_Control_Keyboard4)
      iControl = C4P_Control_None;
  }
  // Set control
  Control = iControl;
  // Mouse
  if (PrefMouse)
    if (Inside(Control, C4P_Control_Keyboard1, C4P_Control_Keyboard4))
      if (!Game.Players.MouseControlTaken())
        MouseControl = TRUE;
}

void C4Player::UpdateFogOfWar() {
  // Eliminated: keep last fog of war
  if (Eliminated)
    return;
  // No fog of war
  if (!FogOfWar)
    return;
  // Clear all visibility
  FogOfWar->ClearVisibility();
  // Add view for all crew members
  C4Object *cobj;
  C4ObjectLink *clnk;
  for (clnk = Crew.First; clnk && (cobj = clnk->Obj); clnk = clnk->Next)
    FogOfWar->AddVisibility(cobj->x, cobj->y);
  // Add view for target view object
  if (ViewMode == C4PVM_Target)
    if (ViewTarget)
      FogOfWar->AddVisibility(ViewTarget->x, ViewTarget->y);
  // Reset flag
  RedrawFogOfWar = FALSE;
}

void C4Player::SelectCrew(C4Object *pObj, BOOL fSelect) {
  // Not a valid crew member
  if (!pObj || !Crew.GetLink(pObj))
    return;
  // Select/unselect
  if (fSelect)
    pObj->Select = 1;
  else
    pObj->Select = 0;
  // Updates
  SelectFlash = 30;
  CursorSelection = CursorToggled = 0;
  AdjustCursorCommand();
}

BOOL C4Player::ActivateMenuGoals() {
  // Menu symbol/init
  C4FacetEx fctSymbol;
  Menu.Init(GfxR->fctMenu.GetPhase(2), LoadResStr(IDS_MENU_CPGOALS), Number);
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  Menu.SetPermanent(TRUE);
  // Items
  int cnt;
  C4ID idGoal;
  C4Def *pDef;
  for (cnt = 0; idGoal = Game.Objects.GetListID(C4D_Goal, cnt); cnt++)
    if (pDef = C4Id2Def(idGoal)) {
      fctSymbol.Create(C4SymbolSize, C4SymbolSize);
      pDef->Draw(fctSymbol);
      Menu.Add(pDef->Name, fctSymbol, "", Game.Objects.ObjectCount(idGoal), NULL, 0, pDef->Desc);
      fctSymbol.Default();
    }
  // Done
  return TRUE;
}

BOOL C4Player::ActivateMenuRules() {
  // Menu symbol/init
  C4FacetEx fctSymbol;
  Menu.Init(GfxR->fctMenu.GetPhase(3), LoadResStr(IDS_MENU_CPRULES), Number);
  Menu.SetAlignment(C4MN_Align_Left | C4MN_Align_Bottom);
  Menu.SetPermanent(TRUE);
  // Items
  int cnt;
  C4ID idGoal;
  C4Def *pDef;
  for (cnt = 0; idGoal = Game.Objects.GetListID(C4D_Rule, cnt); cnt++)
    if (pDef = C4Id2Def(idGoal)) {
      fctSymbol.Create(C4SymbolSize, C4SymbolSize);
      pDef->Draw(fctSymbol);
      Menu.Add(pDef->Name, fctSymbol, "", C4MN_Item_NoCount, NULL, 0, pDef->Desc);
      fctSymbol.Default();
    }
  // Done
  return TRUE;
}

void C4Player::Eliminate() {
  if (Eliminated)
    return;
  Eliminated = TRUE;
  RetireDelay = C4RetireDelay;
  SoundEffect("Eliminated");
  sprintf(OSTR, LoadResStr(IDS_PRC_PLRELIMINATED), Name);
  Log(OSTR);
}
