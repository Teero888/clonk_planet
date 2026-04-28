/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to run the game */

#include <C4Include.h>

extern char *C4LogBuf;

char OSTR[500];

#define offsC4G(x) offsetof(C4Game, x)

C4CompilerValue C4CR_Game[] = {

    {"Game", C4CV_Section, 0, 0},
    {"Time", C4CV_Integer, offsC4G(Time), 1},
    {"Frame", C4CV_Integer, offsC4G(FrameCounter), 1},
    {"ControlTick", C4CV_Integer, offsC4G(ControlTick), 1},
    {"ControlRate", C4CV_Integer, offsC4G(ControlRate), 1},
    {"DoControl", C4CV_Integer, offsC4G(DoControl), 1},
    {"InputTick", C4CV_Integer, offsC4G(InputTick), 1},
    {"InputRate", C4CV_Integer, offsC4G(InputRate), 1},
    {"DoInput", C4CV_Integer, offsC4G(DoInput), 1},
    {"SyncTick", C4CV_Integer, offsC4G(SyncTick), 1},
    {"SyncRate", C4CV_Integer, offsC4G(SyncRate), 1},
    {"DoSync", C4CV_Integer, offsC4G(DoSync), 1},
    {"Tick2", C4CV_Integer, offsC4G(iTick2), 1},
    {"Tick3", C4CV_Integer, offsC4G(iTick3), 1},
    {"Tick5", C4CV_Integer, offsC4G(iTick5), 1},
    {"Tick10", C4CV_Integer, offsC4G(iTick10), 1},
    {"Tick35", C4CV_Integer, offsC4G(iTick35), 1},
    {"Tick255", C4CV_Integer, offsC4G(iTick255), 1},
    {"Tick500", C4CV_Integer, offsC4G(iTick500), 1},
    {"Tick1000", C4CV_Integer, offsC4G(iTick1000), 1},
    {"StartupPlayerCount", C4CV_Integer, offsC4G(StartupPlayerCount), 1},
    {"ObjectEnumerationIndex", C4CV_Integer, offsC4G(ObjectEnumerationIndex),
     1},
    {"Rules", C4CV_Integer, offsC4G(Rules), 1},

    {"Script", C4CV_Section, 0, 0},
    {"Go", C4CV_Integer, offsC4G(Script.Go), 1},
    {"Counter", C4CV_Integer, offsC4G(Script.Counter), 1},
    {"Globals", C4CV_Integer, offsC4G(Script.Global), C4MaxGlobal},

    {"Weather", C4CV_Section, 0, 0},
    {"Season", C4CV_Integer, offsC4G(Weather.Season), 1},
    {"YearSpeed", C4CV_Integer, offsC4G(Weather.YearSpeed), 1},
    {"SeasonDelay", C4CV_Integer, offsC4G(Weather.SeasonDelay), 1},
    {"Wind", C4CV_Integer, offsC4G(Weather.Wind), 1},
    {"TargetWind", C4CV_Integer, offsC4G(Weather.TargetWind), 1},
    {"Temperature", C4CV_Integer, offsC4G(Weather.Temperature), 1},
    {"TemperatureRange", C4CV_Integer, offsC4G(Weather.TemperatureRange), 1},
    {"Climate", C4CV_Integer, offsC4G(Weather.Climate), 1},
    {"MeteoriteLevel", C4CV_Integer, offsC4G(Weather.MeteoriteLevel), 1},
    {"VolcanoLevel", C4CV_Integer, offsC4G(Weather.VolcanoLevel), 1},
    {"EarthquakeLevel", C4CV_Integer, offsC4G(Weather.EarthquakeLevel), 1},
    {"LightningLevel", C4CV_Integer, offsC4G(Weather.LightningLevel), 1},

    {"Landscape", C4CV_Section, 0, 0},
    {"ScanX", C4CV_Integer, offsC4G(Landscape.ScanX), 1},
    {"LeftOpen", C4CV_Integer, offsC4G(Landscape.LeftOpen), 1},
    {"RightOpen", C4CV_Integer, offsC4G(Landscape.RightOpen), 1},
    {"TopOpen", C4CV_Integer, offsC4G(Landscape.TopOpen), 1},
    {"BottomOpen", C4CV_Integer, offsC4G(Landscape.BottomOpen), 1},
    {"Gravity", C4CV_Integer, offsC4G(Landscape.Gravity), 1},

    {NULL, C4CV_End, 0, 0}};

volatile BOOL C4Game::GameGo;
volatile BOOL C4Game::TimeGo;

C4Game::C4Game() { Default(); }

C4Game::~C4Game() {}

BOOL C4Game::InitDefs() {
  int iDefs = 0;
  Log(LoadResStr(IDS_PRC_INITDEFS));

  // SaveGame definition preset override (not needed with new scenarios that
  // have def specs in scenario core, keep for downward compatibility)
  if (C4S.Head.SaveGame)
    DefinitionFilenamesFromSaveGame();

  // Load specified defs
  iDefs += Defs.Load(DefinitionFilenames, C4D_Load_RX, Config.General.Language,
                     &SoundSystem, TRUE);

  // Def load failure
  if (Defs.LoadFailure)
    return FALSE;

  // Load scenario local defs
  iDefs += Defs.Load(ScenarioFile, C4D_Load_RX, Config.General.Language,
                     &SoundSystem, TRUE, TRUE);

  // Absolutely no defs: we don't like that
  if (!iDefs) {
    Log(LoadResStr(IDS_PRC_NODEFS));
    return FALSE;
  }

  // Check def engine version (should be done immediately on def load)
  iDefs = Defs.CheckEngineVersion(C4XVer1, C4XVer2, C4XVer3);
  if (iDefs > 0) {
    sprintf(OSTR, LoadResStr(IDS_PRC_DEFSINVC4X), iDefs);
    Log(OSTR);
  }

  // Resolve includes
  Defs.ResolveIncludes();

  // Done
  return TRUE;
}

BOOL C4Game::OpenScenario() {

  // Scenario filename check & log
  if (!ScenarioFilename[0]) {
    Log(LoadResStr(IDS_PRC_NOC4S));
    return FALSE;
  }
  sprintf(OSTR, LoadResStr(IDS_PRC_LOADC4S), ScenarioFilename);
  Log(OSTR);

  // Open scenario file
  if (!ScenarioFile.Open(ScenarioFilename)) {
    Log(LoadResStr(IDS_PRC_FILENOTFOUND));
    return FALSE;
  }

  // Read scenario core
  if (!C4S.Load(ScenarioFile)) {
    printf("C4S.Load failed!\n");
    Log(LoadResStr(IDS_PRC_FILEINVALID));
    return FALSE;
  }

  // Check registration
  if (!CheckScenarioAccess()) {
    Log(LoadResStr(IDS_PRC_NOSWSCENARIO));
    return FALSE;
  }

  // Check minimum engine version
  if (C4S.Head.C4XVer1 * 100 + C4S.Head.C4XVer2 * 10 + C4S.Head.C4XVer3 >
      C4XVer1 * 100 + C4XVer2 * 10 + C4XVer3) {
    sprintf(OSTR, LoadResStr(IDS_PRC_NOREQC4X), C4S.Head.C4XVer1,
            C4S.Head.C4XVer2, C4S.Head.C4XVer3);
    Log(OSTR);
    return FALSE;
  }

  // Network reference: check exact engine version
  if (C4S.Head.NetworkReference)
    if (C4S.Head.C4XVer1 * 100 + C4S.Head.C4XVer2 * 10 + C4S.Head.C4XVer3 !=
        C4XVer1 * 100 + C4XVer2 * 10 + C4XVer3) {
      sprintf(OSTR, LoadResStr(IDS_PRC_NOEXACTC4X), C4S.Head.C4XVer1,
              C4S.Head.C4XVer2, C4S.Head.C4XVer3);
      Log(OSTR);
      return FALSE;
    }

  // Network reference: check developer mode
  if (C4S.Head.NetworkReference)
    if (C4S.Head.NetworkDeveloperMode != Console.Active) {
      Log(LoadResStr(IDS_PRC_WRONGMODE));
      return FALSE;
    }

  // Scenario definition preset
  if (C4S.Definitions.GetModules(DefinitionFilenames)) {
    if (DefinitionFilenames[0])
      Log(LoadResStr(IDS_PRC_SCEOWNDEFS));
    else
      Log(LoadResStr(IDS_PRC_LOCALONLY));
  }

  // Scan folder local definitions
  SAddModules(DefinitionFilenames, FoldersWithLocalsDefs(ScenarioFilename));

  // Check mission access
  if (C4S.Head.MissionAccess[0])
    if (!SIsModule(Config.General.MissionAccess, C4S.Head.MissionAccess)) {
      Log(LoadResStr(IDS_PRC_NOMISSIONACCESS));
      return FALSE;
    }

  // Title
  Title.Load(LoadResStr(IDS_CNS_TITLE), ScenarioFile, C4CFN_Title);
  if (!Title.GetLanguageString(Config.General.Language, ScenarioTitle,
                               C4MaxTitle))
    SCopy(C4S.Head.Title, ScenarioTitle);

  return TRUE;
}

void C4Game::CloseScenario() { ScenarioFile.Close(); }

BOOL C4Game::Init(const char *szCmdLine) {
  printf("Game::Init: %s\n", szCmdLine);

  // Parse command line
  ParseCommandLine(szCmdLine);
  // System
  printf("InitSystem...\n");
  InitSystem();

  // Minimal init for menu if no scenario
  if (!ScenarioFilename[0]) {
    printf("Minimal init for menu...\n");
    // Graphics
    if (!GraphicsResource.Init())
      return FALSE;
    if (!GraphicsSystem.Init())
      return FALSE;
    GraphicsSystem.SetPalette();
    GraphicsSystem.SetDarkColorTable();
    // Sound & Music
    if (Config.Sound.RXSound)
      SoundSystem.Init();
    if (Config.Sound.RXMusic)
      MusicSystem.Init();
    // Drive flags
    GraphicsSystem.GraphicsGo = TRUE;
    GameGo = TRUE;
    return TRUE;
  }

  // Loader screen
  char szParentfolder[_MAX_PATH + 1];
  GetParentPath(ScenarioFilename, szParentfolder, _MAX_PATH);
  if (Application.Fullscreen)
    if (!GraphicsSystem.InitLoaderScreen(ScenarioFilename))
      if (!GraphicsSystem.InitLoaderScreen(szParentfolder))
        GraphicsSystem.InitLoaderScreen(Config.AtExePath(C4CFN_Graphics));

  // Startup message board
  if (Application.Fullscreen)
    if (Config.Graphics.ShowStartupMessages || Config.Network.Active) {
      C4Facet cgo;
      cgo.Set(Engine.DDraw.lpBack, 0, 0, Config.Graphics.ResX,
              Config.Graphics.ResY);
      GraphicsSystem.MessageBoard.Init(C4LogBuf, cgo, TRUE);
    }

  // Additional reg code check
  if (Application.Fullscreen)
    if (Config.Registered())
      if (!Config.ValidRegistrationName(Config.General.Name)) {
        Log(LoadResStr(IDS_PRC_REGNAMEINVALID));
        return FALSE;
      }

  // Init game
  printf("InitGame...\n");
  if (!InitGame()) {
    printf("InitGame failed!\n");
    return FALSE;
  }

  // Init players
  printf("InitPlayers...\n");
  if (!InitPlayers()) {
    printf("InitPlayers failed!\n");
    return FALSE;
  }

  // Final init
  printf("InitGameFinal...\n");
  if (!InitGameFinal()) {
    printf("InitGameFinal failed!\n");
    return FALSE;
  }

  // Network final init
  if (!Network.FinalInit())
    return FALSE;

  // Color palette
  if (Application.Fullscreen)
    Engine.DDraw.WipeSurface(Engine.DDraw.lpPrimary);
  GraphicsSystem.SetPalette();
  GraphicsSystem.SetDarkColorTable();

  // Message board
  if (Application.Fullscreen) {
    C4Facet cgo;
    cgo.Set(Engine.DDraw.lpBack, 0,
            Config.Graphics.ResY - Engine.DDraw.TextHeight(),
            Config.Graphics.ResX, Engine.DDraw.TextHeight());
    GraphicsSystem.MessageBoard.Init(C4LogBuf + SLen(C4LogBuf), cgo, FALSE);
    GraphicsSystem.RecalculateViewports();
  }

  // Start message
  Log(LoadResStr(C4S.Head.NetworkGame ? IDS_PRC_JOIN
                 : C4S.Head.SaveGame  ? IDS_PRC_RESUME
                                      : IDS_PRC_START));

  return TRUE;
}

void C4Game::Clear() {
  // Evaluation
  if (GameOver)
    if (!Evaluated)
      Evaluate();

  // Clear
  MouseControl.Clear();
  Players.Clear();
  C4S.Clear();
  Weather.Clear();
  GraphicsSystem.Clear();
  DeleteObjects();
  Defs.Clear();
  TextureMap.Clear();
  Landscape.Clear();
  PXS.Clear();
  Material.Clear();
  GraphicsResource.Clear();
  MusicSystem.Clear();
  SoundSystem.Clear();
  Messages.Clear();
  Info.Clear();
  Title.Clear();
  Script.Clear();
  Names.Clear();
  GameText.Clear();
  Network.Clear();
  InputHandler.Clear();
  Input.Clear();
  Control.Clear();
  ControlQueue.Clear();
  ControlJoinClient.Default();
  PathFinder.Clear();
  TransferZones.Clear();
  CloseScenario();

  // Message (doubled if console is quit on closed game)
  Log(LoadResStr(IDS_CNS_GAMECLOSED));
}

BOOL C4Game::GameOverCheck() {
  int cnt;
  BOOL fDoGameOver = FALSE;

  // Only every 35 ticks
  if (Tick35)
    return FALSE;

  // All players eliminated: game over
  if (!Players.GetCountNotEliminated())
    fDoGameOver = TRUE;

  // Cooperative game over (obsolete with new game goal objects, kept for
  // downward compatibility with CreateObjects,ClearObjects,ClearMaterial
  // settings)
  C4ID c_id;
  int count, mat;
  BOOL condition_valid, condition_true;
  BOOL game_over_valid = FALSE, game_over = TRUE;
  // CreateObjects
  condition_valid = FALSE;
  condition_true = TRUE;
  for (cnt = 0; (c_id = C4S.Game.CreateObjects.GetID(cnt, &count)); cnt++)
    if (count > 0) {
      condition_valid = TRUE;
      // Count objects, fullsize only
      C4ObjectLink *cLnk;
      int iCount = 0;
      for (cLnk = Game.Objects.First; cLnk; cLnk = cLnk->Next)
        if (cLnk->Obj->Status)
          if (cLnk->Obj->Def->id == c_id)
            if (cLnk->Obj->Con >= FullCon)
              iCount++;
      if (iCount < count)
        condition_true = FALSE;
    }
  if (condition_valid) {
    game_over_valid = TRUE;
    if (!condition_true)
      game_over = FALSE;
  }
  // ClearObjects
  condition_valid = FALSE;
  condition_true = TRUE;
  for (cnt = 0; (c_id = C4S.Game.ClearObjects.GetID(cnt, &count)); cnt++) {
    condition_valid = TRUE;
    // Count objects, if category living, live only
    C4ObjectLink *cLnk;
    C4Def *cdef = C4Id2Def(c_id);
    BOOL alive_only = FALSE;
    if (cdef && (cdef->Category & C4D_Living))
      alive_only = TRUE;
    int iCount = 0;
    for (cLnk = Game.Objects.First; cLnk; cLnk = cLnk->Next)
      if (cLnk->Obj->Status)
        if (cLnk->Obj->Def->id == c_id)
          if (!alive_only || cLnk->Obj->Alive)
            iCount++;
    if (iCount > count)
      condition_true = FALSE;
  }
  if (condition_valid) {
    game_over_valid = TRUE;
    if (!condition_true)
      game_over = FALSE;
  }
  // ClearMaterial
  condition_valid = FALSE;
  condition_true = TRUE;
  for (cnt = 0; cnt < C4MaxNameList; cnt++)
    if (C4S.Game.ClearMaterial.Name[cnt][0])
      if (MatValid(mat = Game.Material.Get(C4S.Game.ClearMaterial.Name[cnt]))) {
        condition_valid = TRUE;
        if (Game.Landscape.MatCountValid) // Valid
        {
          if (Game.Landscape.MatCount[mat] >
              (DWORD)C4S.Game.ClearMaterial.Count[cnt])
            condition_true = FALSE;
        } else // No valid mat count available, wait
        {
          condition_true = FALSE;
        }
      }
  if (condition_valid) {
    game_over_valid = TRUE;
    if (!condition_true)
      game_over = FALSE;
  }

  // Evaluate game over
  if (game_over_valid)
    if (game_over)
      fDoGameOver = TRUE;

  // Message
  if (fDoGameOver)
    DoGameOver();

  return GameOver;
}

int iLastControlSize = 0;
extern int iPacketDelay;

BOOL C4Game::Execute() // Returns true if the game is over
{

  // Graphics
  if (GraphicsSystem.GraphicsGo)
    GraphicsSystem.Execute();

  // Control / Network
  if (!ExecuteControl())
    return FALSE;

  // Halt (Host only)
  if (!Network.Active || Network.Host)
    if (Halt)
      return FALSE;

  // Speed control
  if (!GameGo)
    return FALSE;

  // Ticks
  Ticks();

  // Game
  Landscape.LockQBA();
  ExecObjects();
  PXS.Execute();
  MassMover.Execute();
  Weather.Execute();
  Landscape.Execute();
  Players.Execute();
  Landscape.UnLockQBA();
  GraphicsSystem.ColorAnimation();
  SoundSystem.Execute();
  MusicSystem.Execute();
  Messages.Execute();
  Script.Execute();
  GamePadCon1.Execute();
  MouseControl.Execute();
  UpdateRules();
  GameOverCheck();

  return GameOver;
}

BOOL C4Game::InitMaterialTexture() {
  int tme_count, mat_count, tex_count;
  C4Group hMatFile;

  // Check for local Material.c4g
  if (ScenarioFile.FindEntry(C4CFN_Material, OSTR)) {
    if (!hMatFile.OpenAsChild(&ScenarioFile, C4CFN_Material))
      return FALSE;
    sprintf(OSTR, LoadResStr(IDS_PRC_LOCALMAT),
            GetFilename(ScenarioFile.GetName()),
            GetFilename(hMatFile.GetName()));
    Log(OSTR);
  }
  // Use global Material.c4g
  else {
    if (!hMatFile.Open(Config.AtExePath(C4CFN_Material)))
      return FALSE;
  }

  // Load texture map
  tme_count = TextureMap.LoadMap(hMatFile, C4CFN_TexMap);
  sprintf(OSTR, LoadResStr(IDS_PRC_TEXMAPENTRIES), tme_count);
  Log(OSTR);

  // Load textures
  tex_count = TextureMap.LoadTextures(hMatFile);
  sprintf(OSTR, LoadResStr(IDS_PRC_TEXTURES), tex_count);
  Log(OSTR);

  // Load materials
  mat_count = Material.Load(hMatFile);
  sprintf(OSTR, LoadResStr(IDS_PRC_MATERIALS), mat_count);
  Log(OSTR);

  // Close material file
  hMatFile.Close();

  // Load material enumeration
  if (!Material.LoadEnumeration(ScenarioFile)) {
    Log(LoadResStr(IDS_PRC_NOMATENUM));
    return FALSE;
  }

  // Enumerate materials
  if (!EnumerateMaterials())
    return FALSE;

  return TRUE;
}

void C4Game::ClearObjectPtrs(C4Object *pObj) {
  // May not call Objects.ClearPointers() because that would
  // remove pObj from primary list and pObj is to be kept
  // until CheckObjectRemoval().
  C4Object *cObj;
  C4ObjectLink *clnk;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    cObj->ClearPointers(pObj);
}

void C4Game::ClearPointers(C4Object *pObj) {
  Messages.ClearPointers(pObj);
  ClearObjectPtrs(pObj);
  Players.ClearPointers(pObj);
  GraphicsSystem.ClearPointers(pObj);
  Console.ClearPointers(pObj);
  Script.ClearPointers(pObj);
  MouseControl.ClearPointers(pObj);
  TransferZones.ClearPointers(pObj);
}

C4Object *C4Game::NewObject(C4Def *pDef, int iOwner, C4ObjectInfo *pInfo,
                            int iX, int iY, int iR, FIXED xdir, FIXED ydir,
                            FIXED rdir, int iCon) {
  // Safety
  if (!pDef)
    return NULL;
  // Create object
  C4Object *pObj;
  if (!(pObj = new C4Object))
    return NULL;
  // Initialize object
  pObj->Init(pDef, iOwner, pInfo, iX, iY, iR, xdir, ydir, rdir);
  // Enumerate object
  pObj->Number = ++ObjectEnumerationIndex;
  // Add to object list
  if (!Objects.Add(pObj)) {
    delete pObj;
    return NULL;
  }
  // Do initial con
  pObj->DoCon(iCon, TRUE);
  // Success
  return pObj;
}

void C4Game::DeleteObjects() {
  C4Object *cObj;
  C4ObjectLink *clnk, *next;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = next) {
    next = clnk->Next;
    Objects.Remove(cObj);
    delete cObj;
  }
}

C4Object *C4Game::CreateObject(C4ID id, int iOwner, int x, int y, int r,
                               FIXED xdir, FIXED ydir, FIXED rdir) {
  C4Def *pDef;
  // Get pDef
  if (!(pDef = C4Id2Def(id)))
    return NULL;
  // Create object
  return NewObject(pDef, iOwner, NULL, x, y, r, xdir, ydir, rdir, FullCon);
}

C4Object *C4Game::CreateInfoObject(C4ObjectInfo *cinf, int iOwner, int tx,
                                   int ty) {
  C4Def *def;
  // Valid check
  if (!cinf)
    return NULL;
  // Get def
  if (!(def = C4Id2Def(cinf->id)))
    return NULL;
  // Create object
  return NewObject(def, iOwner, cinf, tx, ty, 0, 0, 0, 0, FullCon);
}

C4Object *C4Game::CreateObjectConstruction(C4ID id, int iOwner, int iX, int iBY,
                                           int iCon, BOOL fTerrain) {
  C4Def *pDef;
  C4Object *pObj;

  // Get def
  if (!(pDef = C4Id2Def(id)))
    return NULL;

  int dx, dy, dwdt, dhgt;
  dwdt = pDef->Shape.Wdt;
  dhgt = pDef->Shape.Hgt;
  dx = iX - dwdt / 2;
  dy = iBY - dhgt;

  // Terrain & Basement
  if (fTerrain) {
    // Clear site background (ignored for ultra-large structures)
    if (dwdt * dhgt < 12000)
      Landscape.DigFreeRect(dx, dy, dwdt, dhgt);
    // Raise Terrain
    Landscape.RaiseTerrain(dx, dy + dhgt, dwdt);
    // Basement
    if (pDef->Basement) {
      const int BasementStrength = 8;
      // Border basement
      if (pDef->Basement > 1) {
        Landscape.DrawMaterialRect(MGranite, dx, dy + dhgt,
                                   Min(pDef->Basement, dwdt), BasementStrength);
        Landscape.DrawMaterialRect(
            MGranite, dx + dwdt - Min(pDef->Basement, dwdt), dy + dhgt,
            Min(pDef->Basement, dwdt), BasementStrength);
      }
      // Normal basement
      else
        Landscape.DrawMaterialRect(MGranite, dx, dy + dhgt, dwdt,
                                   BasementStrength);
    }
  }

  // Create object
  if (!(pObj = NewObject(pDef, iOwner, NULL, iX, iBY, 0, 0, 0, 0, iCon)))
    return NULL;

  return pObj;
}

void C4Game::BlastObjects(int tx, int ty, int level, C4Object *inobj,
                          int iCausedBy) {
  C4Object *cObj;
  C4ObjectLink *clnk;

  // Contained blast
  if (inobj) {
    inobj->Blast(level, iCausedBy);
    for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
      if (cObj->Status)
        if (cObj->Contained == inobj)
          cObj->Blast(level, iCausedBy);
  }

  // Uncontained blast local outside objects
  else {
    for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
      if (cObj->Status)
        if (!cObj->Contained) {
          // Direct hit (5 pixel range to all sides)
          if (Inside(ty - (cObj->y + cObj->Shape.y), -5,
                     cObj->Shape.Hgt - 1 + 10))
            if (Inside(tx - (cObj->x + cObj->Shape.x), -5,
                       cObj->Shape.Wdt - 1 + 10))
              cObj->Blast(level, iCausedBy);
          // Shock wave hit (if in level range, living only)
          if (cObj->Category & C4D_Living)
            if (Abs(ty - cObj->y) <= level)
              if (Abs(tx - cObj->x) <= level) {
                cObj->DoEnergy(-level / 2);
                cObj->DoDamage(level / 2, iCausedBy);
                cObj->Fling(
                    itofix(Sign(cObj->x - tx) * (level - Abs(tx - cObj->x))) /
                        5,
                    itofix(-level + Abs(ty - cObj->y)) / 5);
              }
        }
  }
}

void C4Game::ShakeObjects(int tx, int ty, int range) {
  C4Object *cObj;
  C4ObjectLink *clnk;

  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    if (cObj->Status)
      if (!cObj->Contained)
        if (cObj->Category & C4D_Living)
          if (Abs(ty - cObj->y) <= range)
            if (Abs(tx - cObj->x) <= range)
              if (!Random(3))
                if (cObj->Action.t_attach)
                  if (cObj->Shape.AttachMat != MVehic)
                    cObj->Fling(itofix(Rnd3()), 0);
}

void C4Game::CrossCheckObjects() // Every Tick1 by ExecObjects
{
  C4ObjectLink *clnk, *clnk2;
  C4Object *obj1, *obj2;
  DWORD ocf1, ocf2, focf, tocf;

  // AtObject-Check: Checks for first match of obj1 at obj2

  // Checks for this frame
  focf = tocf = OCF_None;
  // Medium level: Fight
  if (!Tick5) {
    focf |= OCF_FightReady;
    tocf |= OCF_FightReady;
  }
  // Very low level: Incineration
  if (!Tick35) {
    focf |= OCF_OnFire;
    tocf |= OCF_Inflammable;
  }

  if (focf && tocf)
    for (clnk = Objects.First; clnk && (obj1 = clnk->Obj); clnk = clnk->Next)
      if (obj1->Status && !obj1->Contained)
        if (obj1->OCF & focf) {
          ocf1 = obj1->OCF;
          ocf2 = tocf;
          if (obj2 = AtObject(obj1->x, obj1->y, ocf2, obj1)) {
            // Incineration
            if ((ocf1 & OCF_OnFire) && (ocf2 & OCF_Inflammable))
              if (!Random(obj2->Def->ContactIncinerate)) {
                obj2->Incinerate(obj1->Owner);
                continue;
              }
            // Fight
            if ((ocf1 & OCF_FightReady) && (ocf2 & OCF_FightReady))
              if (Players.Hostile(obj1->Owner, obj2->Owner)) {
                ObjectActionFight(obj1, obj2);
                ObjectActionFight(obj2, obj1);
                continue;
              }
          }
        }

  // Reverse area check: Checks for all obj2 at obj1

  focf = tocf = OCF_None;
  // High level: Collection, Hit
  if (!Tick3) {
    focf |= OCF_Collection;
    tocf |= OCF_Carryable;
    focf |= OCF_Living;
    tocf |= OCF_HitSpeed2;
  }

  if (focf && tocf)
    for (clnk = Objects.First; clnk && (obj1 = clnk->Obj); clnk = clnk->Next)
      if (obj1->Status && !obj1->Contained)
        if (obj1->OCF & focf) {
          for (clnk2 = Objects.First; clnk2 && (obj2 = clnk2->Obj);
               clnk2 = clnk2->Next)
            if (obj2->Status && !obj2->Contained && (obj2 != obj1))
              if (obj2->OCF & tocf)
                if (Inside(obj2->x - (obj1->x + obj1->Shape.x), 0,
                           obj1->Shape.Wdt - 1))
                  if (Inside(obj2->y - (obj1->y + obj1->Shape.y), 0,
                             obj1->Shape.Hgt - 1)) {
                    // Hit
                    if ((obj2->OCF & OCF_HitSpeed2) && (obj1->OCF & OCF_Living))
                      if (obj2->Category & C4D_Object) {
                        obj1->DoEnergy(-obj2->Mass / 5);
                        int tmass = Max(obj1->Mass, 50);
                        obj1->Fling(obj2->xdir * 50 / tmass,
                                    -Abs(obj2->ydir / 2) * 50 / tmass);
                        obj1->Call(PSF_CatchBlow, -obj2->Mass / 5, (long)obj2);
                        continue;
                      }
                    // Collection
                    if ((obj1->OCF & OCF_Collection) &&
                        (obj2->OCF & OCF_Carryable))
                      if (Inside(obj2->x - (obj1->x + obj1->Def->Collection.x),
                                 0, obj1->Def->Collection.Wdt - 1))
                        if (Inside(obj2->y -
                                       (obj1->y + obj1->Def->Collection.y),
                                   0, obj1->Def->Collection.Hgt - 1)) {
                          // Special: attached Flag may not be collectable
                          if (obj2->Def->id == C4ID_Flag)
                            if (!(Game.Rules & C4RULE_FlagRemoveable))
                              if (obj2->Action.Act > ActIdle)
                                if (SEqual(obj2->Def->ActMap[obj2->Action.Act]
                                               .Name,
                                           "FlyBase"))
                                  continue;
                          // Check for collect rejection
                          if (obj1->Call(PSF_RejectCollection,
                                         (long)obj2->Def->id, (long)obj2))
                            continue;
                          // Object enter container
                          obj2->Enter(obj1);
                          // Cancel attach (hacky)
                          ObjectComCancelAttach(obj2);
                          // Container Collection call
                          obj1->Call(PSF_Collection, (long)obj2);
                          // Object Hit call
                          if (obj2->OCF & OCF_HitSpeed1)
                            obj2->Call(PSF_Hit);
                          // Continue
                          continue;
                        }
                  }
        }

  // Contained-Check: Checks for matching Contained

  // Checks for this frame
  focf = tocf = OCF_None;
  // Low level: Fight
  if (!Tick10) {
    focf |= OCF_FightReady;
    tocf |= OCF_FightReady;
  }

  if (focf && tocf)
    for (clnk = Objects.First; clnk && (obj1 = clnk->Obj); clnk = clnk->Next)
      if (obj1->Status && obj1->Contained)
        if (obj1->OCF & focf) {
          for (clnk2 = Objects.First; clnk2 && (obj2 = clnk2->Obj);
               clnk2 = clnk2->Next)
            if (obj2->Status && obj2->Contained && (obj2 != obj1))
              if (obj2->OCF & tocf)
                if (obj1->Contained == obj2->Contained) {
                  ocf1 = obj1->OCF;
                  ocf2 = obj2->OCF;

                  // Fight
                  if ((ocf1 & OCF_FightReady) && (ocf2 & OCF_FightReady))
                    if (Players.Hostile(obj1->Owner, obj2->Owner)) {
                      ObjectActionFight(obj1, obj2);
                      ObjectActionFight(obj2, obj1);
                      continue;
                    }
                }
        }
}

C4Object *C4Game::AtObject(int ctx, int cty, DWORD &ocf, C4Object *exclude) {
  DWORD cocf;
  C4Object *cObj;
  C4ObjectLink *clnk;

  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    if (cObj != exclude) {
      cocf = ocf | OCF_Exclusive;
      if (cObj->At(ctx, cty, cocf)) {
        // Search match
        if (cocf & ocf) {
          ocf = cocf;
          return cObj;
        }
        // EXCLUSIVE block
        else
          return NULL;
      }
    }
  return NULL;
}

C4Object *C4Game::OverlapObject(int tx, int ty, int wdt, int hgt,
                                int category) {
  C4Object *cObj;
  C4ObjectLink *clnk;
  C4Rect rect1, rect2;
  rect1.x = tx;
  rect1.y = ty;
  rect1.Wdt = wdt;
  rect1.Hgt = hgt;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    if (cObj->Status)
      if (!cObj->Contained)
        if (cObj->Category & category & C4D_SortLimit) {
          rect2 = cObj->Shape;
          rect2.x += cObj->x;
          rect2.y += cObj->y;
          if (rect1.Overlap(rect2))
            return cObj;
        }
  return NULL;
}

C4Object *C4Game::FindObject(C4ID id, int iX, int iY, int iWdt, int iHgt,
                             DWORD ocf, const char *szAction,
                             C4Object *pActionTarget, C4Object *pExclude,
                             C4Object *pContainer, int iOwner,
                             C4Object *pFindNext) {

  C4Object *pClosest = NULL;
  int iClosest, iDistance, iFartherThan = -1;
  C4Object *cObj;
  C4ObjectLink *cLnk;

  // Finding next closest: find closest but further away than last closest
  if (pFindNext && (iWdt == -1) && (iHgt == -1)) {
    iFartherThan = (int)sqrt((pFindNext->x - iX) * (pFindNext->x - iX) +
                             (pFindNext->y - iY) * (pFindNext->y - iY));
    pFindNext = NULL;
  }

  // Scan all objects
  for (cLnk = Objects.First; cLnk && (cObj = cLnk->Obj); cLnk = cLnk->Next) {

    // Not skipping to find next
    if (!pFindNext)
      // Status
      if (cObj->Status)
        // ID
        if ((id == C4ID_None) || (cObj->Def->id == id))
          // OCF (match any specified)
          if (cObj->OCF & ocf)
            // Exclude
            if (cObj != pExclude)
              // Action
              if (!szAction || !szAction[0] ||
                  ((cObj->Action.Act > ActIdle) &&
                   SEqual(szAction, cObj->Def->ActMap[cObj->Action.Act].Name) &&
                   (!pActionTarget ||
                    ((cObj->Action.Target == pActionTarget) ||
                     (cObj->Action.Target2 == pActionTarget)))))
                // Container
                if (!pContainer || (cObj->Contained == pContainer) ||
                    ((pContainer == NO_CONTAINER) && !cObj->Contained))
                  // Owner
                  if ((iOwner == ANY_OWNER) || (cObj->Owner == iOwner))
                  // Area
                  {
                    // Full range
                    if ((iX == 0) && (iY == 0) && (iWdt == 0) && (iHgt == 0))
                      return cObj;
                    // Point
                    if ((iWdt == 0) && (iHgt == 0)) {
                      if (Inside(iX - (cObj->x + cObj->Shape.x), 0,
                                 cObj->Shape.Wdt - 1))
                        if (Inside(iY - (cObj->y + cObj->Shape.y), 0,
                                   cObj->Shape.Hgt - 1))
                          return cObj;
                      continue;
                    }
                    // Closest
                    if ((iWdt == -1) && (iHgt == -1)) {
                      iDistance = (int)sqrt((cObj->x - iX) * (cObj->x - iX) +
                                            (cObj->y - iY) * (cObj->y - iY));
                      if (!pClosest || (iDistance < iClosest))
                        if (iDistance > iFartherThan) {
                          pClosest = cObj;
                          iClosest = iDistance;
                        }
                      continue;
                    }
                    // Range
                    if (Inside(cObj->x - iX, 0, iWdt - 1) &&
                        Inside(cObj->y - iY, 0, iHgt - 1))
                      return cObj;
                  }

    // Find next mark reached
    if (cObj == pFindNext)
      pFindNext = NULL;
  }

  return pClosest;
}

int C4Game::ObjectCount(C4ID id, int x, int y, int wdt, int hgt, DWORD ocf,
                        const char *szAction, C4Object *pActionTarget,
                        C4Object *pExclude, C4Object *pContainer, int iOwner) {
  int iResult = 0;
  C4Object *cObj;
  C4ObjectLink *clnk;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    // Status
    if (cObj->Status)
      // ID
      if ((id == C4ID_None) || (cObj->Def->id == id))
        // OCF
        if (cObj->OCF & ocf)
          // Exclude
          if (cObj != pExclude)
            // Action
            if (!szAction || !szAction[0] ||
                ((cObj->Action.Act > ActIdle) &&
                 SEqual(szAction, cObj->Def->ActMap[cObj->Action.Act].Name) &&
                 (!pActionTarget || ((cObj->Action.Target == pActionTarget) ||
                                     (cObj->Action.Target2 == pActionTarget)))))
              // Container
              if (!pContainer || (cObj->Contained == pContainer) ||
                  ((pContainer == NO_CONTAINER) && !cObj->Contained))
                // Owner
                if ((iOwner == ANY_OWNER) || (cObj->Owner == iOwner))
                // Area
                {
                  // Full range
                  if ((x == 0) && (y == 0) && (wdt == 0) && (hgt == 0)) {
                    iResult++;
                    continue;
                  }
                  // Point
                  if ((wdt == 0) && (hgt == 0)) {
                    if (Inside(x - (cObj->x + cObj->Shape.x), 0,
                               cObj->Shape.Wdt - 1))
                      if (Inside(y - (cObj->y + cObj->Shape.y), 0,
                                 cObj->Shape.Hgt - 1)) {
                        iResult++;
                        continue;
                      }
                    continue;
                  }
                  // Range
                  if (Inside(cObj->x - x, 0, wdt - 1) &&
                      Inside(cObj->y - y, 0, hgt - 1)) {
                    iResult++;
                    continue;
                  }
                }

  return iResult;
}

// Deletes removal-assigned data from list.
// Pointer clearance is done by AssignRemoval.

void C4Game::ObjectRemovalCheck() // Every Tick255 by ExecObjects
{
  C4Object *cObj;
  C4ObjectLink *clnk, *next;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = next) {
    next = clnk->Next;
    if (!cObj->Status && (cObj->RemovalDelay == 0)) {
      Objects.Remove(cObj);
      delete cObj;
    }
  }
}

void C4Game::ExecObjects() // Every Tick1 by Execute
{
  BOOL fResort = FALSE;

  // Execute objects
  C4Object *cObj;
  C4ObjectLink *clnk;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    if (cObj->Status) {
      // Execute object
      cObj->Execute();
      // Flag if needs to be resorted
      if (cObj->Unsorted)
        fResort = TRUE;
    } else
      // Status reset: process removal delay
      if (cObj->RemovalDelay > 0)
        cObj->RemovalDelay--;

  // Cross check objects
  CrossCheckObjects();

  // Resort
  if (fResort) {
    clnk = Objects.First;
    while (clnk && (cObj = clnk->Obj)) {
      clnk = clnk->Next;
      if (cObj->Unsorted) {
        Objects.Remove(cObj);
        Objects.Add(cObj);
        cObj->Unsorted = FALSE;
      }
    }
  }

  // Removal
  if (!Tick255)
    ObjectRemovalCheck();
}

void C4Game::Draw(C4FacetEx &cgo) {
  // Draw PathFinder
  if (GraphicsSystem.ShowPathfinder)
    PathFinder.Draw(cgo);
  // Draw PXS
  PXS.Draw(cgo);
  // Draw objects
  Objects.Draw(cgo);
  // Draw overlay
  DrawCursors(cgo);
}

int C4Game::CommandLine(const char *szScript) {
  if (!szScript)
    return FALSE;
  C4Thread cthr;
  return cthr.Execute(NULL, szScript, NULL, NULL, NULL);
}

BOOL C4Game::CreateViewport(int iPlayer) {
  return GraphicsSystem.CreateViewport(iPlayer);
}

C4ID DefFileGetID(const char *szFilename) {
  C4Group hDef;
  C4DefCore DefCore;
  if (!hDef.Open(szFilename))
    return C4ID_None;
  if (!DefCore.Load(hDef)) {
    hDef.Close();
    return C4ID_None;
  }
  hDef.Close();
  return DefCore.id;
}

BOOL C4Game::DropFile(const char *szFilename, int iX, int iY) {
  C4ID c_id;
  C4Def *cdef;

  // Drop def to create object
  if (SEqualNoCase(GetExtension(szFilename), "c4d")) {
    // Get id from file
    if (c_id = DefFileGetID(szFilename))
      // Get loaded def or try to load def from file
      if ((cdef = C4Id2Def(c_id)) ||
          (Defs.Load(szFilename, C4D_Load_RX, Config.General.Language,
                     &SoundSystem) &&
           (cdef = C4Id2Def(c_id)))) {
        // Building construction
        if (cdef->Category & C4D_Structure) {
          if (CreateObjectConstruction(c_id, NO_OWNER, iX, iY, FullCon, TRUE)) {
            if (Game.Halt)
              GraphicsSystem.Execute();
            return TRUE;
          }
        }
        // Regular object
        else {
          if (CreateObject(c_id, NO_OWNER, iX, iY)) {
            if (Game.Halt)
              GraphicsSystem.Execute();
            return TRUE;
          }
        }
      }
    // Failure
    sprintf(OSTR, LoadResStr(IDS_CNS_DROPNODEF), GetFilename(szFilename));
    Console.Out(OSTR);
    return FALSE;
  }

  return FALSE;
}

BOOL C4Game::DropDef(C4ID id, int iX, int iY) {
  // Get def
  C4Def *pDef;
  if (pDef = C4Id2Def(id)) {
    // Building construction
    if (pDef->Category & C4D_Structure) {
      if (CreateObjectConstruction(id, NO_OWNER, iX, iY, FullCon, TRUE)) {
        if (Game.Halt)
          GraphicsSystem.Execute();
        return TRUE;
      }
    }
    // Regular object
    else {
      if (CreateObject(id, NO_OWNER, iX, iY)) {
        if (Game.Halt)
          GraphicsSystem.Execute();
        return TRUE;
      }
    }
  }
  // Failure
  sprintf(OSTR, LoadResStr(IDS_CNS_DROPNODEF), C4IdText(id));
  Console.Out(OSTR);
  return FALSE;
}

BOOL C4Game::EnumerateMaterials() {

  // Check material number
  if (Material.Num > C4MaxMaterial) {
    Log(LoadResStr(IDS_PRC_TOOMANYMATS));
    return FALSE;
  }

  // Get hardcoded system material indices
  MVehic = Material.Get("Vehicle");
  MTunnel = Material.Get("Tunnel");
  MWater = Material.Get("Water");
  MSnow = Material.Get("Snow");
  MGranite = Material.Get("Granite");
  MEarth = Material.Get("Earth");
  if ((MVehic == MNone) || (MTunnel == MNone)) {
    Log(LoadResStr(IDS_PRC_NOSYSMATS));
    return FALSE;
  }

  // Map materials to game palette
  int mat, col, rgb;
  for (mat = 0; mat < Game.Material.Num; mat++)
    for (col = 0; col < C4M_ColsPerMat; col++)
      for (rgb = 0; rgb < 3; rgb++) {
        GraphicsResource
            .GamePalette[(GBM + mat * C4M_ColsPerMat + col) * 3 + rgb] =
            Material.Map[mat].Color[col * 3 + rgb];
        GraphicsResource
            .GamePalette[(GBM + IFT + mat * C4M_ColsPerMat + col) * 3 + rgb] =
            Material.Map[mat].Color[col * 3 + rgb];
      }

  return TRUE;
}

void C4Game::CastObjects(C4ID id, int num, int level, int tx, int ty,
                         int iOwner) {
  int cnt;
  for (cnt = 0; cnt < num; cnt++)
    CreateObject(id, iOwner, tx, ty, Random(360),
                 itofix(Random(2 * level + 1) - level) / 10,
                 itofix(Random(2 * level + 1) - level) / 10,
                 itofix(Random(3) + 1));
}

void C4Game::BlastCastObjects(C4ID id, int num, int tx, int ty) {
  int cnt;
  for (cnt = 0; cnt < num; cnt++)
    CreateObject(id, NO_OWNER, tx, ty, Random(360),
                 itofix(Random(61) - 30) / 10, itofix(Random(61) - 40) / 10,
                 itofix(Random(3) + 1));
}

BOOL C4Game::Save(C4Group &hGroup, BOOL fSaveGame, BOOL fNetwork) {
  static BOOL fWasSaveGame = C4S.Head.SaveGame;

  // Use scenario core copy, may not mess with original core
  C4Scenario rC4S = C4S;

  // Set scenario	header
  rC4S.Head.C4XVer1 = C4XVer1;
  rC4S.Head.C4XVer2 = C4XVer2;
  rC4S.Head.C4XVer3 = C4XVer3;
  rC4S.Head.NoInitialize = TRUE;
  rC4S.Head.SaveGame = fSaveGame;
  rC4S.Head.NetworkGame = fNetwork;
  rC4S.Head.NetworkReference = 0;
  rC4S.Head.NetworkHostName[0] = 0;
  rC4S.Head.NetworkHostAddress[0] = 0;
  // Network savegame: store client names (including host)
  if (fSaveGame && Network.Active)
    Network.StoreClientNames(rC4S.Head.NetworkClients);
  // Savegame: set definition specs
  if (fSaveGame)
    rC4S.Definitions.SetModules(DefinitionFilenames, Config.General.ExePath);
  // Icon and title
  if (fSaveGame) {
    if (!fNetwork)
      rC4S.Head.Icon = 29;
    SCopy(ScenarioTitle, rC4S.Head.Title, C4MaxTitle);
  } else {
    if (fWasSaveGame)
      rC4S.Head.Icon = 14;
  }

  // Savegame or fullscreen: enforce exact landscape
  if (fSaveGame || Application.Fullscreen)
    rC4S.SetExactLandscape();

  // Save scenario core
  if (!rC4S.Save(hGroup))
    return FALSE;

  // Landscape plus components (exact)
  if (rC4S.Landscape.ExactLandscape) {
    // Landscape
    Objects.RemoveSolidMasks(FALSE);
    if (!Landscape.Save(hGroup))
      return FALSE;
    Objects.PutSolidMasks();
    // PXS
    if (!PXS.Save(hGroup))
      return FALSE;
    // MassMover (create copy, may not modify running data)
    C4MassMoverSet MassMoverSet;
    MassMoverSet.Copy(MassMover);
    if (!MassMoverSet.Save(hGroup))
      return FALSE;
    // Material enumeration
    if (!Material.SaveEnumeration(hGroup))
      return FALSE;
  }
  // Landscape map (static)
  else if (Landscape.Mode == C4LSC_Static)
    if (!Landscape.SaveMap(hGroup))
      return FALSE;

  // Objects
  if (!Objects.Save(hGroup, fSaveGame))
    return FALSE;

  // Script
  if (!Script.Save(hGroup))
    return FALSE;

  // Title
  if (!Title.Save(hGroup))
    return FALSE;

  // Info
  if (!Info.Save(hGroup))
    return FALSE;

  // Save game component updates
  if (fSaveGame) {
    // Save runtime data
    if (!SaveRuntimeData(hGroup))
      return FALSE;
    // Players
    if (!Players.Save(hGroup, Network.Active))
      return FALSE;
    // Components
    hGroup.Delete(C4CFN_ScenarioTitle);
    hGroup.Delete(C4CFN_ScenarioIcon);
    sprintf(OSTR, C4CFN_ScenarioDesc, "*");
    hGroup.Delete(OSTR);
    hGroup.Delete(C4CFN_Title);
    hGroup.Delete(C4CFN_Info);
    // Desc
    if (!SaveDesc(hGroup, TRUE))
      return FALSE;
    // Title bitmap
    if (!SaveGameTitle(hGroup))
      return FALSE;
  }

  // Save scenario component updates
  else {
    // No Game.txt
    hGroup.Delete(C4CFN_Game);
    // No player files
    hGroup.Delete(C4CFN_PlayerFiles);
    // No desc if was save game
    if (fWasSaveGame) {
      sprintf(OSTR, C4CFN_ScenarioDesc, "*");
      hGroup.Delete(OSTR);
    }
  }

  fWasSaveGame = fSaveGame;

  return TRUE;
}

void C4Game::Sec1Timer() { TimeGo = TRUE; }

void C4Game::Default() {
  FrameCounter = 0;
  GameOver = FALSE;
  ScenarioFilename[0] = 0;
  PlayerFilenames[0] = 0;
  DefinitionFilenames[0] = 0;
  NetworkJoinPlayerFilenames[0] = 0;
  DirectJoinAddress[0] = 0;
  StartupPlayerCount = 0;
  SCopy("Working Title", ScenarioTitle);
  Halt = FALSE;
  fReferenceDefinitionOverride = FALSE;
  AsynchronousControl = FALSE;
  Evaluated = FALSE;
  Time = 0;
  ControlTimeStart = ControlTimeBehind = 0;
  FPS = cFPS = 0;
  fScriptCreatedObjects = FALSE;
  fLobby = FALSE;
  iTick2 = iTick3 = iTick5 = iTick10 = iTick35 = iTick255 = iTick500 =
      iTick1000 = 0;
  ControlTick = 0;
  ControlRate = -1;
  DoControl = TRUE;
  InputTick = 0;
  InputRate = -1;
  DoInput = FALSE;
  SyncTick = 0;
  SyncRate = -1;
  DoSync = FALSE;
  ObjectEnumerationIndex = 0;
  Record = Replay = FALSE;

  Defs.Default();
  Material.Default();
  Objects.Default();
  Players.Default();
  Weather.Default();
  Landscape.Default();
  TextureMap.Default();
  Rank.Default();
  MassMover.Default();
  PXS.Default();
  GraphicsSystem.Default();
  MusicSystem.Default();
  SoundSystem.Default();
  C4S.Default();
  Messages.Default();
  Info.Default();
  Title.Default();
  Names.Default();
  GameText.Default();
  Script.Default();
  Network.Default();
  InputHandler.Default();
  GraphicsResource.Default();
  Input.Default();
  Control.Default();
  ControlQueue.Default();
  ControlJoinClient.Default();
  MouseControl.Default();
  PathFinder.Default();
  TransferZones.Default();
}

void C4Game::Evaluate() {
  // Players
  Players.Evaluate();
  Players.Save();

  // Set game flag
  Log(LoadResStr(IDS_PRC_EVALUATED));
  Evaluated = TRUE;
}

void C4Game::DrawCursors(C4FacetEx &cgo) {
  // Draw cursor mark arrow & cursor object name
  int cox, coy, cphase;
  C4Object *cursor;
  C4Facet &fctCursor = GraphicsResource.fctCursor;
  for (C4Player *pPlr = Players.First; pPlr; pPlr = pPlr->Next)
    if (pPlr->LocalControl)
      if (pPlr->CursorFlash || pPlr->SelectFlash)
        if (pPlr->Cursor) {
          cursor = pPlr->Cursor;
          cox = cursor->x - fctCursor.Wdt / 2 - cgo.TargetX;
          coy = cursor->y - cursor->Def->Shape.Hgt / 2 - fctCursor.Hgt -
                cgo.TargetY;
          if (Inside(cox, 1 - fctCursor.Wdt, cgo.Wdt) &&
              Inside(coy, 1 - fctCursor.Hgt, cgo.Hgt)) {
            cphase = 0;
            if (cursor->Contained)
              cphase = 1;
            fctCursor.Draw(cgo.Surface, cgo.X + cox, cgo.Y + coy, cphase);
            if (cursor->Info) {
              if (cursor->Info->Rank > 0)
                sprintf(OSTR, "%s|%s", cursor->Info->RankName,
                        cursor->Info->Name);
              else
                SCopy(cursor->Info->Name, OSTR);

              Engine.DDraw.TextOut(
                  OSTR, cgo.Surface, cgo.X + cox + fctCursor.Wdt / 2,
                  cgo.Y + coy - 2 - Engine.DDraw.TextHeight(OSTR), FRed, FBlack,
                  ACenter);
            }
          }
        }
}

void C4Game::Ticks() {
  // Frames
  FrameCounter++;
  GameGo = FALSE;
  // Ticks
  iTick2++;
  if (iTick2 == 2)
    iTick2 = 0;
  iTick3++;
  if (iTick3 == 3)
    iTick3 = 0;
  iTick5++;
  if (iTick5 == 5)
    iTick5 = 0;
  iTick10++;
  if (iTick10 == 10)
    iTick10 = 0;
  iTick35++;
  if (iTick35 == 35)
    iTick35 = 0;
  iTick255++;
  if (iTick255 == 255)
    iTick255 = 0;
  iTick500++;
  if (iTick500 == 500)
    iTick500 = 0;
  iTick1000++;
  if (iTick1000 == 1000)
    iTick1000 = 0;
  // FPS / Time
  cFPS++;
  if (TimeGo) {
    TimeGo = FALSE;
    Time++;
    FPS = cFPS;
    cFPS = 0;
  }
  // Control
  ControlTick++;
  if (ControlTick >= ControlRate) {
    ControlTick = 0;
    DoControl = TRUE;
  }
  // Input
  InputTick++;
  if (InputTick >= InputRate) {
    InputTick = 0;
    DoInput = TRUE;
  }
  // Sync (since sync add is done on DoControl, occurence may be irregular)
  SyncTick++;
  if (SyncTick >= SyncRate) {
    SyncTick = 0;
    DoSync = TRUE;
  }
}

BOOL C4Game::Compile(const char *szSource) {
  C4Compiler Compiler;
  // C4Game is not defaulted on compilation.
  // Loading of runtime data only overrides only certain values.
  return Compiler.CompileStructure(C4CR_Game, szSource, this);
}

C4Game binDefault;

void SetClientPrefix(char *szFilename, const char *szClient);

BOOL C4Game::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;

  // Own buffer
  *ppOutput = new char[30000];
  *ppOutput[0] = 0;

  // Compile static sections
  Compiler.DecompileStructure(C4CR_Game, this, &binDefault, ppOutput, NULL);

  // PlayerFiles
  SAppend(LineFeed, *ppOutput);
  SAppend("[PlayerFiles]", *ppOutput);
  SAppend(LineFeed, *ppOutput);
  C4Player *pPlr;
  int iPlrIndex;
  for (pPlr = Players.First, iPlrIndex = 1; pPlr;
       pPlr = pPlr->Next, iPlrIndex++) {
    char szPlayerFilename[_MAX_PATH + 1];
    // Set player filename
    SCopy(pPlr->Filename, szPlayerFilename);
    // Network game host local player: compose correct prefix filename (hack)
    if (Network.Active && Network.Host && pPlr->LocalControl) {
      SCopy(GetFilename(pPlr->Filename), szPlayerFilename);
      SetClientPrefix(szPlayerFilename, Network.LocalName);
    }
    // Append player entry
    sprintf(OSTR, "Player%i=%s%s", iPlrIndex, szPlayerFilename, LineFeed);
    SAppend(OSTR, *ppOutput);
  }

  // DefinitionFiles (now not needed anymore since def specs are also stored in
  // scenario core)
  SAppend(LineFeed, *ppOutput);
  SAppend("[DefinitionFiles]", *ppOutput);
  SAppend(LineFeed, *ppOutput);
  char szDefinition[_MAX_PATH + 1];
  int iDef = 1;
  for (int iSeg = 0; SCopySegment(DefinitionFilenames, iSeg, szDefinition);
       iSeg++) {
    SClearFrontBack(szDefinition);
    if (!szDefinition[0])
      continue;
    sprintf(OSTR, "Definition%i=%s", iDef++,
            Config.AtExeRelativePath(szDefinition));
    SAppend(LineFeed, OSTR);
    SAppend(OSTR, *ppOutput);
  }

  // Append player sections
  for (pPlr = Players.First, iPlrIndex = 1; pPlr;
       pPlr = pPlr->Next, iPlrIndex++) {
    sprintf(OSTR, "%i", iPlrIndex);
    pPlr->Decompile(ppOutput, NULL, OSTR);
  }

  // End of file
  SAppend(EndOfFile, *ppOutput);
  *ipSize = SLen(*ppOutput);

  return TRUE;
}

BOOL C4Game::CompileRuntimeData() {
  // Compile
  if (!Compile(GameText.GetData()))
    return FALSE;
  // Success
  return TRUE;
}

BOOL C4Game::SaveRuntimeData(C4Group &hGroup) {

  // Enumerate pointers
  Script.EnumerateVariablePointers();
  Players.EnumeratePointers();

  // Decompile
  char *Buffer;
  int BufferSize;
  Decompile(&Buffer, &BufferSize);

  // Denumerate pointers
  Script.DenumerateVariablePointers();
  Players.DenumeratePointers();

  // Save
  if (!hGroup.Add(C4CFN_Game, Buffer, BufferSize, FALSE, TRUE)) {
    delete[] Buffer;
    return FALSE;
  }

  return TRUE;
}

BOOL C4Game::LoadPlayerFilenames(C4Group &hGroup) {
  const char *pSource;

  // Use loaded game text component
  if (pSource = GameText.GetData()) {
    PlayerFilenames[0] = 0;
    const char *szPos;
    char szLinebuf[30 + _MAX_PATH + 1];
    if (szPos = SSearch((const char *)pSource, "[PlayerFiles]"))
      while (TRUE) {
        szPos = SAdvanceSpace(szPos);
        SCopyUntil(szPos, szLinebuf, 0x0D, 30 + _MAX_PATH);
        szPos += SLen(szLinebuf);
        if (SEqual2(szLinebuf, "Player") && (SCharPos('=', szLinebuf) > -1)) {
          if (PlayerFilenames[0])
            SAppend(";", PlayerFilenames);
          SAppend(szLinebuf + SCharPos('=', szLinebuf) + 1, PlayerFilenames);
        } else
          break;
      }
    if (PlayerFilenames[0])
      return TRUE;
  }

  return FALSE;
}

BOOL C4Game::SaveDesc(C4Group &hGroup, BOOL fSaveGame, BOOL fReference,
                      BOOL fLobby) {
  char *buffer;
  if (!(buffer = new char[10000]))
    return FALSE;
  buffer[0] = 0;

  // Header
  SAppend("{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1031{\\fonttbl "
          "{\\f0\\fnil\\fcharset0 Times New Roman;}}",
          buffer);
  SAppend(LineFeed, buffer);

  // Scenario title
  sprintf(OSTR, "\\uc1\\pard\\ulnone\\b\\f0\\fs20 %s\\par", ScenarioTitle);
  SAppend(OSTR, buffer);
  SAppend(LineFeed, buffer);
  SAppend("\\b0\\fs16\\par", buffer);
  SAppend(LineFeed, buffer);

  // Network game host
  if (fReference) {
    sprintf(OSTR, LoadResStr(IDS_DESC_HOST), Network.LocalName,
            Network.LocalAddress);
    SAppend(OSTR, buffer);
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Save game date
  if (fSaveGame) {
    time_t tTime;
    time(&tTime);
    struct tm *pLocalTime;
    pLocalTime = localtime(&tTime);
    sprintf(OSTR, LoadResStr(Network.Active ? IDS_DESC_DATENET : IDS_DESC_DATE),
            pLocalTime->tm_mday, pLocalTime->tm_mon + 1,
            pLocalTime->tm_year + 1900, pLocalTime->tm_hour,
            pLocalTime->tm_min);
    SAppend(OSTR, buffer);
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Duration
  if (Time) {
    sprintf(OSTR, LoadResStr(IDS_DESC_DURATION), Time / 3600,
            (Time % 3600) / 60, Time % 60);
    SAppend(OSTR, buffer);
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Lobby: original scenario file
  if (fLobby) {
    // Get file size (if real file)
    int iFilesize = FileSize(ScenarioFilename) / 1024;
    // Print filename
    if (iFilesize)
      sprintf(OSTR, LoadResStr(IDS_DESC_FILE),
              Config.AtExeRelativePath(ScenarioFilename), iFilesize);
    else
      sprintf(OSTR, LoadResStr(IDS_DESC_FILE2),
              Config.AtExeRelativePath(ScenarioFilename));
    // Convert rtf backslashes
    for (char *cpos = OSTR; cpos = (char *)SSearch(cpos, "\\"); cpos++)
      SInsert(cpos, "\\");
    // Append to desc
    SAppend(OSTR, buffer);
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Definition specs
  if (DefinitionFilenames[0]) {
    char szDef[_MAX_PATH + 1];
    // Desc
    SAppend(LoadResStr(IDS_DESC_DEFSPECS), buffer);
    // Get definition modules
    for (int cnt = 0; SGetModule(DefinitionFilenames, cnt, szDef); cnt++) {
      // Get exe relative path
      SCopy(Config.AtExeRelativePath(szDef), OSTR);
      // Convert rtf backslashes
      for (char *cpos = OSTR; cpos = (char *)SSearch(cpos, "\\"); cpos++)
        SInsert(cpos, "\\");
      // Append comma
      if (cnt > 0)
        SAppend(", ", buffer);
      // Apend to desc
      SAppend(OSTR, buffer);
    }
    // End of line
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Network: clients (including host)
  if (Network.Active) {
    // Desc
    SAppend(LoadResStr(IDS_DESC_CLIENTS), buffer);
    // Host name
    SAppend(Network.LocalName, buffer);
    // Client names
    for (C4NetworkClient *pClient = Game.Network.Clients.First; pClient;
         pClient = pClient->Next) {
      SAppend(", ", buffer);
      SAppend(pClient->Name, buffer);
    }
    // End of line
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Players
  if (Players.First) {
    SAppend(LoadResStr(IDS_DESC_PLRS), buffer);
    BOOL fAppendComma = FALSE;
    for (C4Player *pPlr = Players.First; pPlr; pPlr = pPlr->Next) {
      if (fAppendComma)
        SAppend(", ", buffer);
      SAppend(pPlr->Name, buffer);
      if (Network.Active) {
        sprintf(OSTR, " (%s)", pPlr->AtClientName);
        SAppend(OSTR, buffer);
      }
      fAppendComma = TRUE;
    }
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // Reference info
  if (fReference) {
    // Max player
    sprintf(OSTR, LoadResStr(IDS_DESC_MAXPLR), C4S.Head.MaxPlayer);
    SAppend(OSTR, buffer);
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
    // Engine version
    sprintf(OSTR, LoadResStr(IDS_DESC_VERSION), C4XVer1, C4XVer2, C4XVer3);
    SAppend(OSTR, buffer);
    if (C4XVer4) {
      sprintf(OSTR, ".%d", C4XVer4);
      SAppend(OSTR, buffer);
    }
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
    // Asynchronous mode
    if (AsynchronousControl) {
      SAppend(LoadResStr(IDS_DESC_ASYNCMODE), buffer);
      SAppend("\\par", buffer);
      SAppend(LineFeed, buffer);
    }
    // Developer mode
    if (Console.Active) {
      SAppend(LoadResStr(IDS_DESC_DEVMODE), buffer);
      SAppend("\\par", buffer);
      SAppend(LineFeed, buffer);
    }
    // No runtime join
    if (Config.Network.NoRuntimeJoin) {
      SAppend(LoadResStr(IDS_DESC_NORUNTIMEJOIN), buffer);
      SAppend("\\par", buffer);
      SAppend(LineFeed, buffer);
    }
    // Comment
    if (fReference && Config.Network.Comment[0]) {
      sprintf(OSTR, LoadResStr(IDS_DESC_COMMENT), Config.Network.Comment);
      SAppend(OSTR, buffer);
      SAppend("\\par", buffer);
      SAppend(LineFeed, buffer);
    }
  }

  // Lobby: expecting participants
  if (fLobby) {
    // Expecting participants
    if (Game.C4S.Head.NetworkClients.IsEmpty())
      SAppend(LoadResStr(IDS_DESC_EXPECTING), buffer);
    // Expecting specific participants
    else {
      SCopy(LoadResStr(IDS_NET_LOBBYEXPECTING), OSTR);
      for (int cnt = 0; cnt < C4MaxNameList; cnt++)
        if (Game.C4S.Head.NetworkClients.Name[cnt][0]) {
          if (cnt > 0)
            SAppend(", ", OSTR);
          SAppend(Game.C4S.Head.NetworkClients.Name[cnt], OSTR);
        }
      SAppend(OSTR, buffer);
    }
    SAppend("\\par", buffer);
    SAppend(LineFeed, buffer);
  }

  // End of file
  SAppend(LineFeed, buffer);
  SAppend("}", buffer);
  SAppend(LineFeed, buffer);
  SAppend(EndOfFile, buffer);

  sprintf(OSTR, C4CFN_ScenarioDesc, Config.General.Language);
  return hGroup.Add(OSTR, buffer, SLen(buffer), FALSE, TRUE);
}

BOOL C4Game::RecreatePlayerFiles() {
  // Check and recreate files
  char szPlayerFilename[_MAX_PATH + 1];
  char szNewPlayerFilenames[20 * _MAX_PATH + 1] = "";
  for (int iPar = 0;
       SCopySegment(PlayerFilenames, iPar, szPlayerFilename, ';', _MAX_PATH);
       iPar++)
    if (szPlayerFilename[0]) {
      // Network game join or active resume: recreate in network path
      if (C4S.Head.NetworkGame || Network.Active) {
        /* Notice that on network savegame resume, no original player files can
           be used since modified files on different clients would lead to sync
           loss. Player files will not be restored to keep in player path, so no
           original files are overwritten and lost. */
        sprintf(OSTR, "%s%s", Config.Network.WorkPath,
                GetFilename(szPlayerFilename));
        SCopy(OSTR, szPlayerFilename);
        sprintf(OSTR, LoadResStr(IDS_PRC_RECREATE),
                GetFilename(szPlayerFilename));
        Log(OSTR);
        if (!ScenarioFile.Extract(GetFilename(szPlayerFilename),
                                  szPlayerFilename))
          return FALSE;
      }
      // Recreate in player path for regular game (reuse original is present)
      else if (!FileExists(szPlayerFilename)) {
        sprintf(OSTR, "%s%s", Config.General.PlayerPath,
                GetFilename(szPlayerFilename));
        SCopy(OSTR, szPlayerFilename);
        sprintf(OSTR, LoadResStr(IDS_PRC_RECREATE),
                GetFilename(szPlayerFilename));
        Log(OSTR);
        if (!ScenarioFile.Extract(GetFilename(szPlayerFilename),
                                  szPlayerFilename))
          return FALSE;
      }
      // Compose new player file list
      if (szNewPlayerFilenames[0])
        SAppend(";", szNewPlayerFilenames);
      SAppend(szPlayerFilename, szNewPlayerFilenames);
    }
  SCopy(szNewPlayerFilenames, PlayerFilenames);
  return TRUE;
}

BOOL C4Game::SaveGameTitle(C4Group &hGroup) {

  // Game not running
  if (!FrameCounter) {
    BYTE *bpBytes;
    int iSize;
    if (ScenarioFile.LoadEntry(C4CFN_ScenarioTitle, &bpBytes, &iSize))
      hGroup.Add(C4CFN_ScenarioTitle, bpBytes, iSize, FALSE, TRUE);
  }

  // Fullscreen screenshot
  else if (Application.Fullscreen && Application.Active) {
    SURFACE sfcPic;
    int iSfcWdt = 225, iSfcHgt = 150;
    if (!(sfcPic = CreateSurface(iSfcWdt, iSfcHgt)))
      return FALSE;

    // Fullscreen
    Engine.DDraw.Blit(Engine.DDraw.lpBack, 0, 0, Config.Graphics.ResX,
                      Config.Graphics.ResY - Engine.DDraw.TextHeight(), sfcPic,
                      0, 0, iSfcWdt, iSfcHgt);

    if (!SaveSurface(Config.AtTempPath(C4CFN_TempTitle), sfcPic,
                     GraphicsResource.GamePalette)) {
      DestroySurface(sfcPic);
      return FALSE;
    }
    DestroySurface(sfcPic);
    if (!hGroup.Move(Config.AtTempPath(C4CFN_TempTitle), C4CFN_ScenarioTitle))
      return FALSE;
  }

  return TRUE;
}

void C4Game::KeyboardInput(WORD vk_code, BOOL fAlt) {

  // Alt (syskey)
  if (fAlt) {
    // Alt-function keys
    switch (vk_code) {
    case VK_F1:
      if (!Config.Graphics.ShowCommands)
        Config.Graphics.ShowCommands = TRUE;
      else if (!Config.Graphics.ShowCommandKeys)
        Config.Graphics.ShowCommandKeys = TRUE;
      else {
        Config.Graphics.ShowCommands = FALSE;
        Config.Graphics.ShowCommandKeys = FALSE;
      }
      break;
    case VK_F2:
      Network.ToggleReferenceRequest();
      break;
    case VK_F3:
      Network.ToggleRuntimeJoin();
      break;
    case VK_F5:
      Toggle(Config.General.DebugMode);
      if (!Config.General.DebugMode)
        GraphicsSystem.DeactivateDebugOutput();
      break;
    }
    // Debug output toggle
    if (Config.General.DebugMode || Console.Active)
      switch (vk_code) {
      case VK_F7:
        Toggle(Game.GraphicsSystem.ShowPathfinder);
        break;
      case VK_F8:
        Toggle(Game.GraphicsSystem.ShowSolidMask);
        break;
      }
    // Video module control
    if (Config.Graphics.VideoModule)
      switch (vk_code) {
      case VK_ADD:
        GraphicsSystem.Video.Enlarge();
        break;
      case VK_SUBTRACT:
        GraphicsSystem.Video.Reduce();
        break;
      case VK_MULTIPLY:
        GraphicsSystem.Video.Toggle();
        break;
      }
    return;
  }

  // Function keys
  switch (vk_code) {

  // Toggle music
  case VK_F1:
    Config.Sound.RXMusic = !Config.Sound.RXMusic;
    if (!Config.Sound.RXMusic)
      Game.MusicSystem.Stop();
    else
      Game.MusicSystem.Play();
    break;

  // Toggle message type in
  case VK_F2:
    if (GraphicsSystem.MessageBoard.TypeIn)
      GraphicsSystem.MessageBoard.CloseTypeIn();
    else {
      GraphicsSystem.MessageBoard.StartTypeIn(FALSE);
      FullScreen.Menu.Close();
    }
    break;

  // Type in back buffer selection
  case VK_UP:
  case VK_DOWN:
  case VK_SCROLL:
    if (GraphicsSystem.MessageBoard.Control(vk_code))
      return;
    break;

  // Adjust control rate
  case VK_F3:
    Network.AdjustControlRate(-1);
    break;
  case VK_F4:
    Network.AdjustControlRate(+1);
    break;

  // Screenshot
  case VK_F9:
    GraphicsSystem.SaveScreenshot();
    break;

  // Quick save
  case VK_F11:
    QuickSave(TRUE);
    break;
  case VK_F12:
    QuickSave(FALSE);
    break;
  }

  // Debug output toggle
  if (Config.General.DebugMode || Console.Active)
    switch (vk_code) {
    case VK_F5:
      Toggle(Game.GraphicsSystem.ShowVertices);
      break;
    case VK_F6:
      Toggle(Game.GraphicsSystem.ShowAction);
      break;
    case VK_F7:
      Toggle(Game.GraphicsSystem.ShowCommand);
      break;
    case VK_F8:
      Toggle(Game.GraphicsSystem.ShowEntrance);
      break;
    }

  // Fullscreen menu control
  if (Application.Fullscreen)
    if (FullScreen.Menu.IsActive()) {
      FullScreen.Menu.Control(
          VkCode2FullScreenCom(vk_code, FullScreen.Menu.IsMultiline()), 0);
      return;
    }

  // Map player controls
  C4Player *pPlr;
  int iKdbSet, iCtrl;
  if (!GraphicsSystem.MessageBoard.TypeIn)
    for (iKdbSet = C4P_Control_Keyboard1; iKdbSet <= C4P_Control_Keyboard4;
         iKdbSet++)
      for (iCtrl = 0; iCtrl < C4MaxKey; iCtrl++)
        if (Config.Keyboard[iKdbSet][iCtrl] == vk_code)
          if (pPlr = Players.GetLocalByKbdSet(iKdbSet))
            LocalPlayerControl(pPlr->Number, Control2Com(iCtrl));
}

BOOL C4Game::QuickSave(BOOL fSaveGame) {

  // Registered only
  if (!Config.Registered()) {
    Log(LoadResStr(IDS_GAME_NOUNREGSAVE));
    return FALSE;
  }

  // Network hosts only
  if (Network.Active && !Network.Host) {
    Log(LoadResStr(IDS_GAME_NOCLIENTSAVE));
    return FALSE;
  }

  // Wait message
  Log(LoadResStr(fSaveGame ? IDS_HOLD_SAVINGGAME : IDS_HOLD_SAVINGSCENARIO));
  GraphicsSystem.MessageBoard.EnsureLastMessage();

  // Target save folder
  Config.General.CheckSaveFolders(IDS_GAME_SAVEGAMEFOLDER,
                                  IDS_GAME_SAVESCENARIOFOLDER);
  char szFilename[_MAX_FNAME + 1];
  SCopy(Config.General.RoundPath, szFilename);
  SAppend(fSaveGame ? Config.General.SaveGameFolder
                    : Config.General.SaveScenarioFolder,
          szFilename);

  // Check save folder
  if (!FileExists(szFilename)) {
    // Create save folder
    if (!CreateDirectory(szFilename, 0)) {
      sprintf(OSTR, LoadResStr(IDS_GAME_NOSAVEDIR), szFilename);
      Log(OSTR);
      return FALSE;
    }
    // Add to reload list
    SAddModule(Config.Explorer.Reload, szFilename);
  }

  // Compose filename
  AppendBackslash(szFilename);
  SCopy(ScenarioTitle, OSTR);
  SReplaceChar(OSTR, '\\', ' ');
  SReplaceChar(OSTR, '/', ' ');
  SReplaceChar(OSTR, ':', ' ');
  SReplaceChar(OSTR, '?', ' ');
  SReplaceChar(OSTR, '*', ' ');
  SReplaceChar(OSTR, '"', ' ');
  SReplaceChar(OSTR, '>', ' ');
  SReplaceChar(OSTR, '<', ' ');
  SReplaceChar(OSTR, '|', ' ');
  SAppend(OSTR, szFilename);
  SAppend(".c4s", szFilename);

  // Create target scenario file
  BOOL fOkay = TRUE;
  if (!ScenarioFile.Close())
    fOkay = FALSE;
  if (!C4Group_CopyItem(ScenarioFilename, szFilename))
    fOkay = FALSE;
  SCopy(szFilename, ScenarioFilename);
  if (!ScenarioFile.Open(ScenarioFilename))
    fOkay = FALSE;
  if (!fOkay) {
    sprintf(OSTR, LoadResStr(IDS_CNS_SAVEASERROR), szFilename);
    Log(OSTR);
    return FALSE;
  }

  // Save
  if (!Save(ScenarioFile, fSaveGame, FALSE))
    fOkay = FALSE;
  ScenarioFile.Sort(C4FLS_Scenario);
  if (!ScenarioFile.Close())
    fOkay = FALSE;
  if (!ScenarioFile.Open(ScenarioFilename))
    fOkay = FALSE;
  if (!fOkay) {
    Log(LoadResStr(fSaveGame ? IDS_GAME_FAILSAVEGAME : IDS_CNS_SAVERROR));
    return FALSE;
  }

  // Add to reload list
  SAddModule(Config.Explorer.Reload, szFilename);

  // Success
  Log(LoadResStr(fSaveGame ? IDS_CNS_GAMESAVED : IDS_CNS_SCENARIOSAVED));
  return TRUE;
}

int LandscapeFree(int x, int y) {
  if (!Inside(x, 0, GBackWdt - 1) || !Inside(y, 0, GBackHgt - 1))
    return FALSE;
  return !DensitySolid(GBackDensity(x, y));
}

BOOL C4Game::InitGame() {
  printf("InitGame: Network override...\n");
  // Network override StartupPlayerCount
  if (Config.Network.Active)
    StartupPlayerCount = C4S_MaxPlayer;

  printf("InitGame: DirectJoinGetReference...\n");
  // Direct join: get reference
  if (!DirectJoinGetReference())
    return FALSE;

  printf("InitGame: OpenScenario...\n");
  // Open scenario
  if (!OpenScenario()) {
    printf("OpenScenario failed!\n");
    Log(LoadResStr(IDS_PRC_FAIL));
    return FALSE;
  }

  printf("InitGame: InitNetworkScenario...\n");
  // Init network scenario
  if (!InitNetworkScenario()) {
    printf("InitNetworkScenario failed!\n");
    return FALSE;
  }

  printf("InitGame: LoadScenarioComponents...\n");
  // Scenario components
  if (!LoadScenarioComponents()) {
    printf("LoadScenarioComponents failed!\n");
    Log(LoadResStr(IDS_PRC_FAIL));
    return FALSE;
  }

  printf("InitGame: GraphicsResource.Init...\n");
  // Graphics
  Log(LoadResStr(IDS_PRC_GFXRES));
  if (!GraphicsResource.Init()) {
    printf("GraphicsResource.Init failed!\n");
    Log(LoadResStr(IDS_PRC_FAIL));
    return FALSE;
  }

  // Sound
  if (Config.Sound.RXSound)
    if (!SoundSystem.Init()) {
      Log(LoadResStr(IDS_PRC_NOSND));
      return FALSE;
    }

  // Music
  if (Config.Sound.RXMusic)
    if (!MusicSystem.Init())
      Log(LoadResStr(IDS_PRC_NOMUSIC));

  // Rank system
  Rank.Init("Software\\RedWolf Design\\Clonk 4\\ClonkRanks",
            LoadResStr(IDS_GAME_DEFRANKS), 1000);

  // Definitions
  if (!InitDefs())
    return FALSE;

  // Graphics system
  if (!GraphicsSystem.Init()) {
    Log(LoadResStr(IDS_ERR_NOGFXSYS));
    return FALSE;
  }

  // Materials
  if (!InitMaterialTexture())
    return FALSE;

  // Colorize defs by material
  Defs.ColorizeByMaterial(Material, GBM);

  // Landscape
  Log(LoadResStr(IDS_PRC_LANDSCAPE));
  if (!Landscape.Init()) {
    Log(LoadResStr(IDS_ERR_GBACK));
    return FALSE;
  }
  Landscape.LockQBA();
  Landscape.ScenarioInit();

  // Pathfinder
  PathFinder.Init(&LandscapeFree, &TransferZones);

  // PXS
  if (ScenarioFile.FindEntry(C4CFN_PXS))
    if (!PXS.Load(ScenarioFile)) {
      Log(LoadResStr(IDS_ERR_PXS));
      return FALSE;
    }

  // MassMover
  if (ScenarioFile.FindEntry(C4CFN_MassMover))
    if (!MassMover.Load(ScenarioFile)) {
      Log(LoadResStr(IDS_ERR_MOVER));
      return FALSE;
    }

  // Environment
  if (!C4S.Head.NoInitialize) {
    Log(LoadResStr(IDS_PRC_ENVIRONMENT));
    InitVegetation();
    InitInEarth();
    InitAnimals();
    InitEnvironment();
    InitRules();
    InitGoals();
  }

  Landscape.UnLockQBA();

  // Weather
  Weather.ScenarioInit();

  // Compile runtime data
  CompileRuntimeData();

  // Load objects
  int iObjects = Objects.Load(ScenarioFile);
  if (iObjects) {
    sprintf(OSTR, LoadResStr(IDS_PRC_OBJECTSLOADED), iObjects);
    Log(OSTR);
  }

  // Denumerate game data pointers
  Script.DenumerateVariablePointers();

  // Check object enumeration
  if (!CheckObjectEnumeration())
    return FALSE;

  return TRUE;
}

BOOL C4Game::InitGameFinal() {

  // Validate object owners & assign loaded info objects
  Objects.ValidateOwners();
  Objects.AssignInfo();

  // Script constructor call
  int iObjCount = Objects.ObjectCount();
  if (!C4S.Head.SaveGame)
    Script.Call(NULL, PSF_Initialize);
  if (Objects.ObjectCount() != iObjCount)
    fScriptCreatedObjects = TRUE;

  // Player final init
  C4Player *pPlr;
  for (pPlr = Players.First; pPlr; pPlr = pPlr->Next)
    pPlr->FinalInit(!C4S.Head.SaveGame);

  // Create viewports
  for (pPlr = Players.First; pPlr; pPlr = pPlr->Next)
    if (pPlr->LocalControl)
      CreateViewport(pPlr->Number);
  // Check fullscreen viewports
  FullScreen.ViewportCheck();

  // Console and no players: halt
  if (Console.Active)
    if (!fLobby)
      if (Players.GetCount() == 0)
        Halt = TRUE;
  Console.UpdateHaltCtrls(Halt);

  // Clear NoInitialize flag (for any runtime player join)
  C4S.Head.NoInitialize = FALSE;

  // Host: missing clients from network savegame - send client removal packets
  if (Network.Active && Network.Host)
    for (int cnt = 0; cnt < C4MaxNameList; cnt++)
      if (C4S.Head.NetworkClients.Name[cnt][0])
        if (!SEqualNoCase(C4S.Head.NetworkClients.Name[cnt], Network.LocalName))
          if (!Network.GetClientByName(C4S.Head.NetworkClients.Name[cnt]))
            Input.AddRemoveClient(C4NET_NoClient,
                                  C4S.Head.NetworkClients.Name[cnt]);

  return TRUE;
}

BOOL C4Game::InitPlayers() {

  // SaveGame: load player filenames from game
  if (C4S.Head.SaveGame) {
    // Load player filenames from game text
    if (!LoadPlayerFilenames(ScenarioFile)) {
      Log(LoadResStr(IDS_ERR_NOPLRFILENAMES));
      return FALSE;
    }
    // Check original player files / recreate from backup
    if (!RecreatePlayerFiles()) {
      Log(LoadResStr(IDS_ERR_NOPLRFILERECR));
      return FALSE;
    }
  }

  // Join players (start immediate)
  char szPlayerFilename[_MAX_PATH + 1];
  for (int iPar = 0;
       SCopySegment(PlayerFilenames, iPar, szPlayerFilename, ';', _MAX_PATH);
       iPar++)
    if (szPlayerFilename[0])
      if (!Players.Join(szPlayerFilename, !C4S.Head.SaveGame,
                        Network.GetClientNumber(), Network.LocalName))
        return FALSE;

  // Join local players to network game (control queue)
  if (NetworkJoinPlayerFilenames[0]) {
    int iPar;
    for (iPar = 0; SCopySegment(NetworkJoinPlayerFilenames, iPar,
                                szPlayerFilename, ';', _MAX_PATH);
         iPar++)
      if (szPlayerFilename[0])
        Input.AddJoinPlayer(szPlayerFilename);
  }

  // Check valid participating player numbers (host/single only)
  if (!Network.Active || (Network.Host && !fLobby)) {
    // No players in fullscreen
    if (Players.GetCount() == 0)
      if (Application.Fullscreen) {
        sprintf(OSTR, LoadResStr(IDS_CNS_NOFULLSCREENPLRS));
        Log(OSTR);
        return FALSE;
      }
    // Too many players
    if (Players.GetCount() > C4S.Head.MaxPlayer) {
      sprintf(OSTR, LoadResStr(IDS_PRC_TOOMANYPLRS), C4S.Head.MaxPlayer);
      Log(OSTR);
      if (Application.Fullscreen)
        return FALSE;
      else
        Console.Message(OSTR);
    }
  }

  return TRUE;
}

int ListExpandValids(C4IDList &rlist, C4ID *idlist, int maxidlist) {
  int cnt, cnt2, ccount, cpos;
  for (cpos = 0, cnt = 0; rlist.GetID(cnt); cnt++)
    if (C4Id2Def(rlist.GetID(cnt, &ccount)))
      for (cnt2 = 0; cnt2 < ccount; cnt2++)
        if (cpos < maxidlist) {
          idlist[cpos] = rlist.GetID(cnt);
          cpos++;
        }
  return cpos;
}

BOOL C4Game::PlaceInEarth(C4ID id) {
  int cnt, tx, ty;
  for (cnt = 0; cnt < 35; cnt++) // cheap trys
  {
    tx = Random(GBackWdt);
    ty = Random(GBackHgt);
    if (GBackMat(tx, ty) == MEarth)
      if (CreateObject(id, NO_OWNER, tx, ty, Random(360)))
        return TRUE;
  }
  return FALSE;
}

C4Object *C4Game::PlaceVegetation(C4ID id, int iX, int iY, int iWdt, int iHgt,
                                  int iGrowth) {
  int cnt, iTx, iTy, iMaterial;

  // Get definition
  C4Def *pDef;
  if (!(pDef = C4Id2Def(id)))
    return NULL;

  // No growth specified: full or random growth
  if (iGrowth < 0) {
    iGrowth = FullCon;
    if (pDef->Growth)
      if (!Random(3))
        iGrowth = Random(FullCon) + 1;
  }

  // Place by placement type
  switch (pDef->Placement) {

  // Surface soil
  case C4D_Place_Surface:
    for (cnt = 0; cnt < 20; cnt++) {
      // Random hit within target area
      iTx = iX + Random(iWdt);
      iTy = iY + Random(iHgt);
      // Above IFT
      while ((iTy > 0) && GBackIFT(iTx, iTy))
        iTy--;
      // Above semi solid
      long lx = iTx, ly = iTy;
      if (!AboveSemiSolid(lx, ly) || !Inside((int)ly, 50, GBackHgt - 50))
        continue;
      iTx = lx;
      iTy = ly;
      // Free above
      if (GBackSemiSolid(iTx, iTy - pDef->Shape.Hgt) ||
          GBackSemiSolid(iTx, iTy - pDef->Shape.Hgt / 2))
        continue;
      // Soil check
      iTy += 3; // two pix into ground
      iMaterial = GBackMat(iTx, iTy);
      if (iMaterial != MNone)
        if (Material.Map[iMaterial].Soil) {
          if (!pDef->Growth)
            iGrowth = FullCon;
          iTy += 5;
          return CreateObjectConstruction(id, NO_OWNER, iTx, iTy, iGrowth);
        }
    }
    break;

  // Underwater
  case C4D_Place_Liquid:
    // Random range
    iTx = iX + Random(iWdt);
    iTy = iY + Random(iHgt);
    // Find liquid
    {
      long lx = iTx, ly = iTy;
      if (!FindSurfaceLiquid(lx, ly, pDef->Shape.Wdt, pDef->Shape.Hgt))
        if (!FindLiquid(lx, ly, pDef->Shape.Wdt, pDef->Shape.Hgt))
          return NULL;
      iTx = lx;
      iTy = ly;
      // Liquid bottom
      if (!SemiAboveSolid(lx, ly))
        return NULL;
      iTx = lx;
      iTy = ly;
    }
    iTy += 3;
    // Create object
    return CreateObjectConstruction(id, NO_OWNER, iTx, iTy, iGrowth);
  }

  // Undefined placement type
  return NULL;
}

C4Object *C4Game::PlaceAnimal(C4ID idAnimal) {
  C4Def *pDef = C4Id2Def(idAnimal);
  if (!pDef)
    return NULL;
  int iX, iY;
  // Placement
  switch (pDef->Placement) {
  // Running free
  case C4D_Place_Surface:
    iX = Random(GBackWdt);
    iY = Random(GBackHgt);
    {
      long lx = iX, ly = iY;
      if (!FindSolidGround(lx, ly, pDef->Shape.Wdt))
        return NULL;
      iX = lx;
      iY = ly;
    }
    break;
  // In liquid
  case C4D_Place_Liquid:
    iX = Random(GBackWdt);
    iY = Random(GBackHgt);
    {
      long lx = iX, ly = iY;
      if (!FindSurfaceLiquid(lx, ly, pDef->Shape.Wdt, pDef->Shape.Hgt))
        if (!FindLiquid(lx, ly, pDef->Shape.Wdt, pDef->Shape.Hgt))
          return NULL;
      iX = lx;
      iY = ly;
    }
    iY += pDef->Shape.Hgt / 2;
    break;
  // Floating in air
  case C4D_Place_Air:
    iX = Random(GBackWdt);
    for (iY = 0; (iY < GBackHgt) && !GBackSemiSolid(iX, iY); iY++)
      ;
    if (iY <= 0)
      return NULL;
    iY = Random(iY);
    break;
  }
  // Create object
  return CreateObject(idAnimal, NO_OWNER, iX, iY);
}

void C4Game::InitInEarth() {
  const int maxvid = 100;
  int cnt, vidnum;
  C4ID vidlist[maxvid];
  // Amount
  int amt = (GBackWdt * GBackHgt / 5000) *
            C4S.Landscape.InEarthLevel.Evaluate() / 100;
  // List all valid IDs from C4S
  vidnum = ListExpandValids(C4S.Landscape.InEarth, vidlist, maxvid);
  // Place
  if (vidnum > 0)
    for (cnt = 0; cnt < amt; cnt++)
      PlaceInEarth(vidlist[Random(vidnum)]);
}

void C4Game::InitVegetation() {
  const int maxvid = 100;
  int cnt, vidnum;
  C4ID vidlist[maxvid];
  // Amount
  int amt = (GBackWdt / 50) * C4S.Landscape.VegLevel.Evaluate() / 100;
  // Get percentage vidlist from C4S
  vidnum = ListExpandValids(C4S.Landscape.Vegetation, vidlist, maxvid);
  // Place vegetation
  if (vidnum > 0)
    for (cnt = 0; cnt < amt; cnt++)
      PlaceVegetation(vidlist[Random(vidnum)], 0, 0, GBackWdt, GBackHgt, -1);
}

void C4Game::InitAnimals() {
  int cnt, cnt2;
  C4ID idAnimal;
  int iCount;
  // Place animals
  for (cnt = 0; (idAnimal = C4S.Animals.FreeLife.GetID(cnt, &iCount)); cnt++)
    for (cnt2 = 0; cnt2 < iCount; cnt2++)
      PlaceAnimal(idAnimal);
  // Place nests
  for (cnt = 0; (idAnimal = C4S.Animals.EarthNest.GetID(cnt, &iCount)); cnt++)
    for (cnt2 = 0; cnt2 < iCount; cnt2++)
      PlaceInEarth(idAnimal);
}

BOOL C4Game::SaveNetworkReference(C4Group &hGroup, BOOL fDenied) {

  // Scenario
  C4Scenario sC4S;
  sC4S.Head.C4XVer1 = C4XVer1;
  sC4S.Head.C4XVer2 = C4XVer2;
  sC4S.Head.C4XVer3 = C4XVer3;
  sC4S.Head.Icon = C4S.Head.Icon;
  sC4S.Head.EnableRoundOptions = 0;
  sC4S.Head.EnableUnregisteredAccess = C4S.Head.EnableUnregisteredAccess;
  if (!Network.Lobby)
    sC4S.Head.EnableUnregisteredAccess = 0;
  sC4S.Head.MaxPlayer = C4S.Head.MaxPlayer;
  sC4S.Head.NetworkGame = 0;
  sC4S.Head.NetworkReference = 1;
  sC4S.Head.NetworkLobby = Network.Lobby;
  sC4S.Head.NetworkFileCreation = ScenarioFile.GetCreation();
  sC4S.Head.NetworkDeveloperMode = Console.Active;
  sC4S.Head.NetworkNoRuntimeJoin = Config.Network.NoRuntimeJoin;
  SCopy(ScenarioTitle, sC4S.Head.Title, C4MaxTitle);
  SCopy(Network.LocalName, sC4S.Head.NetworkHostName, C4MaxTitle);
  SCopy(Network.LocalAddress, sC4S.Head.NetworkHostAddress, C4MaxTitle);
  SCopy(Config.AtExeRelativePath(ScenarioFilename), sC4S.Head.NetworkFilename,
        C4MaxTitle);
  sC4S.Definitions.SetModules(DefinitionFilenames, Config.General.ExePath);

  // Save scenario core
  if (!sC4S.Save(hGroup))
    return FALSE;

  // Title
  if (!fDenied)
    if (!SaveGameTitle(hGroup))
      return FALSE;

  // Desc
  if (!fDenied) {
    if (!SaveDesc(hGroup, FALSE, TRUE, Network.Lobby))
      return FALSE;
  } else {
    if (!SaveDeniedDesc(hGroup))
      return FALSE;
  }

  // Custom icon
  if (!fDenied)
    hGroup.Add(C4CFN_ScenarioIcon, ScenarioFile);

  return TRUE;
}

BOOL C4Game::RequestNetworkReference(C4Stream *pStream) {
  if (!pStream)
    return FALSE;
  // Message
  sprintf(OSTR, LoadResStr(IDS_NET_REQUESTINF), pStream->GetPeerName());
  Log(OSTR);
  GraphicsSystem.MessageBoard.EnsureLastMessage();
  // Handle
  return Network.HandleReferenceRequest(pStream);
}

void C4Game::ParseCommandLine(const char *szCmdLine) {

  // Definition filenames by registry config
  SCopy(Config.General.Definitions, DefinitionFilenames);

  char cSeparator;
  char szParameter[_MAX_PATH + 1];
  cSeparator = ' ';
  if (SCharCount('"', szCmdLine))
    cSeparator = '"';
  for (int iPar = 0;
       SCopySegment(szCmdLine, iPar, szParameter, cSeparator, _MAX_PATH);
       iPar++) {
    // Scenario file / folder
    if (SEqualNoCase(GetExtension(szParameter), "c4s") ||
        SEqualNoCase(GetExtension(szParameter), "c4f")) {
      SCopy(szParameter, ScenarioFilename, _MAX_PATH);
      continue;
    }

    // Player file
    if (SEqualNoCase(GetExtension(szParameter), "c4p")) {
      SAddModule(PlayerFilenames, szParameter);
      continue;
    }
    // Definition file
    if (SEqualNoCase(GetExtension(szParameter), "c4d")) {
      SAddModule(DefinitionFilenames, szParameter);
      continue;
    }
    // Lobby
    if (SSearchNoCase(szParameter, "/Lobby"))
      if (Config.Network.Active) {
        fLobby = TRUE;
        continue;
      }
    // Direct join
    const char *szAddress;
    if (szAddress = SSearchNoCase(szParameter, "/Join:"))
      if (Config.Network.Active) {
        SCopy(szAddress, DirectJoinAddress, _MAX_PATH);
        continue;
      }
    // Direct join by URL
    if (szAddress = SSearchNoCase(szParameter, "clonk:")) {
      // Store address
      SCopy(szAddress, DirectJoinAddress, _MAX_PATH);
      SClearFrontBack(DirectJoinAddress, '/');
      // Self-enable network
      Config.Network.Active = TRUE;
      continue;
    }
  }

  // Determine startup player count
  StartupPlayerCount = SModuleCount(PlayerFilenames);
}

BOOL C4Game::LoadScenarioComponents() {
  // Info
  Info.Load(LoadResStr(IDS_CNS_INFO), ScenarioFile, C4CFN_Info);
  // Script
  Script.Load(LoadResStr(IDS_CNS_SCRIPT), ScenarioFile, C4CFN_Script,
              Config.General.Language);
  // Names
  Names.Load(LoadResStr(IDS_CNS_NAMES), ScenarioFile, C4CFN_Names);
  // Game (runtime data)
  GameText.Load(C4CFN_Game, ScenarioFile, C4CFN_Game);
  // Success
  return TRUE;
}

void C4Game::InitSystem() {
  // Random seed (host)
  /*if (Config.Network.Active) RandomSeed = 0;
  else*/
  RandomSeed = time(NULL);
  // Randomize
  FixRandom(RandomSeed);
  // Timer flags
  TimeGo = FALSE;
  GameGo = FALSE;
  // Sync rate
  SyncRate = 50;
  // Input rate
  InputRate = 3; // (asynchronous control only)
  // Control mode
  AsynchronousControl = 0; // Config.Network.AsynchronousControl; beta disabled
  // Control time start (host)
  ControlTimeStart = time_msecs();
  // Control rate
  ControlRate = 1;
}

BOOL C4Game::JoinPlayer(const char *szFilename, int iAtClient,
                        const char *szAtClientName) {
  C4Player *pPlr;
  // Join
  if (!(pPlr = Players.Join(szFilename, TRUE, iAtClient, szAtClientName)))
    return FALSE;
  // Player final init
  pPlr->FinalInit(TRUE);
  // Create player viewport
  if (pPlr->LocalControl)
    CreateViewport(pPlr->Number);
  // Check fullscreen viewports
  FullScreen.ViewportCheck();
  // Update menus
  Console.UpdateMenus();
  // Success
  return TRUE;
}

void C4Game::FixRandom(int iSeed) {
  // sprintf(OSTR,"Fixing random to %i",iSeed); Log(OSTR);
  FixedRandom(iSeed);
  InitSafeRandom();
  Randomize3();
}

void C4Game::LocalPlayerControl(int iPlayer, int iCom) {
  C4Player *pPlr = Players.Get(iPlayer);
  if (!pPlr)
    return;
  int iData = 0;
  // Menu button com: activate main menu
  if (iCom == COM_PlayerMenu) {
    pPlr->ActivateMenuMain();
    return;
  }
  // Local player menu active: convert menu com and control local
  if (pPlr->Menu.ConvertCom(iCom, iData)) {
    pPlr->Menu.Control(iCom, iData);
    return;
  }
  // Cursor object menu active: convert menu com for control queue
  if (pPlr->Cursor)
    if (pPlr->Cursor->Menu)
      pPlr->Cursor->Menu->ConvertCom(iCom, iData);
  // Player control: add to control queue
  Input.AddPlayerControl(iPlayer, iCom, iData);
}

BOOL C4Game::DefinitionFilenamesFromSaveGame() {
  const char *pSource;
  char szDefinitionFilenames[20 * _MAX_PATH + 1];
  szDefinitionFilenames[0] = 0;

  // Use loaded game text component
  if (pSource = GameText.GetData()) {
    const char *szPos;
    char szLinebuf[30 + _MAX_PATH + 1];
    // Search def file name section
    if (szPos = SSearch((const char *)pSource, "[DefinitionFiles]"))
      // Scan lines
      while (TRUE) {
        szPos = SAdvanceSpace(szPos);
        SCopyUntil(szPos, szLinebuf, 0x0D, 30 + _MAX_PATH);
        szPos += SLen(szLinebuf);
        // Add definition file name
        if (SEqual2(szLinebuf, "Definition") &&
            (SCharPos('=', szLinebuf) > -1)) {
          SNewSegment(szDefinitionFilenames);
          SAppend(szLinebuf + SCharPos('=', szLinebuf) + 1,
                  szDefinitionFilenames);
        } else
          break;
      }
    // Overwrite prior def file name specification
    if (szDefinitionFilenames[0]) {
      SCopy(szDefinitionFilenames, DefinitionFilenames);
      return TRUE;
    }
  }

  return FALSE;
}

BOOL C4Game::DoGameOver() {
  // Duplication safety
  if (GameOver)
    return FALSE;
  // Flag, log, call
  GameOver = TRUE;
  Log(LoadResStr(IDS_PRC_GAMEOVER));
  Script.Call(NULL, PSF_OnGameOver);
  return TRUE;
}

BOOL C4Game::LocalFileMatch(const char *szFilename, int iCreation) {
  // Check file (szFilename relative to ExePath)
  if (C4Group_GetCreation(Config.AtExePath(szFilename)) == iCreation)
    return TRUE;
  // No match
  return FALSE;
}

BOOL C4Game::RequestJoin(C4Stream *pStream) {
  // Lobby join: handle immediately
  if (Network.Lobby) {
    Network.HandleJoin(pStream);
  }
  // Runtime join: add to control queue
  else {
    Input.AddJoinClient(pStream);
  }
  // Ok
  return TRUE;
}

void C4Game::SyncClearance() { Objects.SyncClearance(); }

void C4Game::Synchronize() {
  // Log
  sprintf(OSTR, "Synchronization (Frame %i)", FrameCounter);
  NetLog(OSTR);
  // Fix random
  FixRandom(RandomSeed);
  // Synchronize members
  Landscape.Synchronize();
  MassMover.Synchronize();
  PXS.Synchronize();
  TransferZones.Synchronize();
}

C4Object *C4Game::FindBase(int iPlayer, int iIndex) {
  C4Object *cObj;
  C4ObjectLink *clnk;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    // Status
    if (cObj->Status)
      // Base
      if (cObj->Base == iPlayer)
        // Index
        if (iIndex == 0)
          return cObj;
        else
          iIndex--;
  // Not found
  return NULL;
}

C4Object *C4Game::FindObjectByCommand(int iCommand, C4Object *pTarget, int iTx,
                                      int iTy, C4Object *pTarget2) {
  C4Object *cObj;
  C4ObjectLink *clnk;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next)
    // Status
    if (cObj->Status)
      // Check commands
      for (C4Command *pCommand = cObj->Command; pCommand;
           pCommand = pCommand->Next)
        // Command
        if (pCommand->Command == iCommand)
          // Target
          if (!pTarget || (pCommand->Target == pTarget))
            // Position
            if ((!iTx && !iTy) ||
                ((pCommand->Tx == iTx) && (pCommand->Ty == iTy)))
              // Target2
              if (!pTarget2 || (pCommand->Target2 == pTarget2))
                // Found
                return cObj;
  // Not found
  return NULL;
}

BOOL C4Game::ReloadDef(C4ID id) {
  C4ObjectLink *clnk;
  C4Def *pDef = Defs.ID2Def(id);
  if (!pDef)
    return FALSE;
  // Message
  sprintf(OSTR, "Reloading %s from %s", C4IdText(pDef->id),
          GetFilename(pDef->Filename));
  Log(OSTR);
  // Reload def
  if (Defs.Reload(pDef, C4D_Load_RX, Config.General.Language, &SoundSystem)) {
    // Success, update all concerned object faces
    for (clnk = Objects.First; clnk && clnk->Obj; clnk = clnk->Next)
      if (clnk->Obj->id == id)
        clnk->Obj->UpdateFace();
  } else {
    // Failure, remove all objects of this type
    for (clnk = Objects.First; clnk && clnk->Obj; clnk = clnk->Next)
      if (clnk->Obj->id == id)
        clnk->Obj->AssignRemoval();
    // Kill def
    Defs.Remove(pDef);
    // Log
    Log("Reloading failure. All objects of this type removed.");
    return FALSE;
  }
  // Success
  return TRUE;
}

BOOL C4Game::InitNetworkScenario() {
  // Network game: mistake (safety)
  if (C4S.Head.NetworkGame) {
    Log(LoadResStr(IDS_NET_NODIRECTSTART));
    return FALSE;
  }

  // Network reference: join game as client
  if (C4S.Head.NetworkReference) {
    // Joining lobby flag
    BOOL fJoiningLobby = C4S.Head.NetworkLobby;
    // Close scenario
    CloseScenario();
    // Init network as client (required)
    if (!Config.Network.Active) {
      Log(LoadResStr(IDS_NET_NOTACTIVE));
      return FALSE;
    }
    if (!Network.Init(FALSE, Config.Network.LocalName,
                      Config.Network.LocalAddress))
      return FALSE;
    // Store local join player filenames for later control queue join
    SCopy(PlayerFilenames, NetworkJoinPlayerFilenames);
    PlayerFilenames[0] = 0;
    // Lobby join: check local scenario file
    BOOL fRetrieveScenario = TRUE;
    if (fJoiningLobby)
      if (LocalFileMatch(C4S.Head.NetworkFilename,
                         C4S.Head.NetworkFileCreation)) {
        Log(LoadResStr(IDS_NET_USINGLOCALFILE));
        // Use local scenario file
        fRetrieveScenario = FALSE;
        // Overwrite ScenarioFilename
        SCopy(Config.AtExePath(C4S.Head.NetworkFilename), ScenarioFilename);
      }
    // Join network game
    if (!Network.Join(C4S.Head.NetworkHostName, C4S.Head.NetworkHostAddress,
                      fRetrieveScenario))
      return FALSE;
    // Reopen scenario
    if (!OpenScenario()) {
      Log(LoadResStr(IDS_PRC_FAIL));
      return FALSE;
    }
    // Check network game data scenario type (safety)
    if (!C4S.Head.NetworkGame && !fJoiningLobby) {
      Log(LoadResStr(IDS_NET_NONETGAME));
      return FALSE;
    }
  }

  // Regular scenario: new game as host
  else {
    // Init network as host (if desired)
    if (Config.Network.Active)
      if (!Network.Init(TRUE, Config.Network.LocalName,
                        Config.Network.LocalAddress))
        return FALSE;
    // Init input handler (if asynchronous)
    if (AsynchronousControl)
      if (!InputHandler.Init(Application.hWindow)) {
        Log(LoadResStr(IDS_PRC_FAILINPUT));
        return FALSE;
      }
    // Execute lobby (if desired)
    if (fLobby) {
      // Do lobby
      if (!Network.DoLobby())
        return FALSE;
      // Store local join player filenames (control queue join only in lobby
      // mode) Not if savegame.
      if (!C4S.Head.SaveGame) {
        SCopy(PlayerFilenames, NetworkJoinPlayerFilenames);
        PlayerFilenames[0] = 0;
      }
    }
  }

  // Success
  return TRUE;
}

BOOL C4Game::CheckObjectEnumeration() {
  // Check valid & maximum number & duplicate numbers
  int iMax = 0;
  C4Object *cObj;
  C4ObjectLink *clnk;
  C4Object *cObj2;
  C4ObjectLink *clnk2;
  for (clnk = Objects.First; clnk && (cObj = clnk->Obj); clnk = clnk->Next) {
    // Invalid number
    if (cObj->Number < 1) {
      Log("Invalid object enumeration number");
      return FALSE;
    }
    // Max
    if (cObj->Number > iMax)
      iMax = cObj->Number;
    // Duplicate
    for (clnk2 = Objects.First; clnk2 && (cObj2 = clnk2->Obj);
         clnk2 = clnk2->Next)
      if (cObj2 != cObj)
        if (cObj->Number == cObj2->Number) {
          sprintf(OSTR, "Duplicate object enumeration number %d (%s and %s)",
                  cObj2->Number, cObj->GetName(), cObj2->GetName());
          Log(OSTR);
          return FALSE;
        }
  }
  // Adjust enumeration index
  if (iMax > ObjectEnumerationIndex)
    ObjectEnumerationIndex = iMax;
  // Done
  return TRUE;
}

BOOL C4Game::CheckScenarioAccess() {

  // Registered: all access
  if (Config.Registered())
    return TRUE;

  // Network reference okay
  if (C4S.Head.NetworkReference)
    return TRUE;

  // Scenario in free shareware folder
  C4Group *pFolder;
  if (C4S.Head.EnableUnregisteredAccess)
    if (pFolder = ScenarioFile.GetMother())
      if (Config.IsFreeFolder(pFolder->GetName(), pFolder->GetMaker()))
        return TRUE;

  // Nope
  return FALSE;
}

BOOL C4Game::DirectJoinGetReference() {

  // No direct join
  if (!DirectJoinAddress[0])
    return TRUE;

  // Log direct join
  sprintf(OSTR, LoadResStr(IDS_NET_DIRECTJOIN), DirectJoinAddress);
  Log(OSTR);

  // Init network as client (reinit in InitNetworkScenario)
  if (!Network.Init(FALSE, Config.Network.LocalName,
                    Config.Network.LocalAddress)) {
    Log(LoadResStr(IDS_NET_NODIRECTJOIN));
    ScenarioFilename[0] = 0;
    return FALSE;
  }

  // Get reference from host to network path (overwrite ScenarioFilename)
  SCopy(Config.Network.WorkPath, ScenarioFilename);
  if (!Network.GetGameReference(DirectJoinAddress, ScenarioFilename)) {
    Log(LoadResStr(IDS_NET_NODIRECTJOIN));
    ScenarioFilename[0] = 0;
    return FALSE;
  }

  // Success
  return TRUE;
}

BOOL C4Game::ExecuteControl() {

  // Asynchronous mode: clients send their input via a separate stream, the host
  // does not wait for the clients but continuously sends out its control
  // stream. This mode was never finished and is disabled in InitSystem
  if (AsynchronousControl) {

    // Execute input
    if (DoInput) {
      // Network: client send input to host / host check for any input from
      // clients
      Network.ExecuteInput(Input);
      // Flag
      DoInput = FALSE;
    }

    // Execute control
    if (DoControl) {
      // Client join
      ControlJoinClient.Execute();
      // Sync: host add sync check to input
      if (Network.Active && Network.Host)
        if (DoSync) {
          Input.AddSyncCheck();
          DoSync = FALSE;
        }
      // Input to control (clients get Control from host in ExecuteControl)
      if (!Network.Active || Network.Host) {
        Control.Copy(Input);
        Input.Clear();
      }
      // Network: host send control to clients / client get control from host
      if (!Network.ExecuteControl(Control))
        return FALSE;
      // for time equalization wait for checkback by all clients here...
      // Execute (and clear)
      iLastControlSize = Control.Size;
      Control.Execute();
      // Player join menu
      FullScreen.CheckPlayerJoinMenu();
      // Flag
      DoControl = FALSE;
    }

  }

  // Normal mode: the host sends its control packets to all clients and receives
  // the input packet immediately. All clients wait for each other.
  else {

    // Control
    if (DoControl) {
      // Client join
      ControlJoinClient.Execute();
      // Sync: host add sync check to input
      if (Network.Active && Network.Host)
        if (DoSync) {
          Input.AddSyncCheck();
          DoSync = FALSE;
        }
      // Network (this will yield the full Control packet and clear the Input
      // packet)
      if (!Network.Execute())
        return FALSE;
      // Control
      iLastControlSize = Control.Size;
      Control.Execute();
      // Player join menu
      FullScreen.CheckPlayerJoinMenu();
      // Flag
      DoControl = FALSE;
    }
  }

  // Done, no halt
  return TRUE;
}

DWORD C4Game::ControlTime() { return time_msecs() - ControlTimeStart; }

const char *C4Game::FoldersWithLocalsDefs(const char *szPath) {
  static char szDefs[10 * _MAX_PATH + 1];
  szDefs[0] = 0;

  // Scan path for folder names
  int cnt, iBackslash;
  char szFoldername[_MAX_PATH + 1];
  C4Group hGroup;
  for (cnt = 0; (iBackslash = SCharPos('\\', Config.AtExeRelativePath(szPath),
                                       cnt)) > -1;
       cnt++) {
    // Get folder name
    SCopy(Config.AtExeRelativePath(szPath), szFoldername, iBackslash);
    // Open folder
    if (SEqualNoCase(GetExtension(szFoldername), "c4f"))
      if (hGroup.Open(szFoldername)) {
        // Check for contained defs
        if (hGroup.FindEntry(C4CFN_DefFiles))
        // Add folder to list
        {
          SNewSegment(szDefs);
          SAppend(szFoldername, szDefs);
        }
        // Close folder
        hGroup.Close();
      }
  }

  return szDefs;
}

void C4Game::InitEnvironment() {
  // Place environment objects
  int cnt, cnt2;
  C4ID idType;
  int iCount;
  for (cnt = 0; (idType = C4S.Environment.Objects.GetID(cnt, &iCount)); cnt++)
    for (cnt2 = 0; cnt2 < iCount; cnt2++)
      CreateObject(idType);
}

void C4Game::InitRules() {
  // Place rule objects
  int cnt, cnt2;
  C4ID idType;
  int iCount;
  for (cnt = 0; (idType = C4S.Game.Rules.GetID(cnt, &iCount)); cnt++)
    for (cnt2 = 0; cnt2 < Max(iCount, 1); cnt2++)
      CreateObject(idType);
  // Update rule flags
  UpdateRules();
}

void C4Game::InitGoals() {
  // Place goal objects
  int cnt, cnt2;
  C4ID idType;
  int iCount;
  for (cnt = 0; (idType = C4S.Game.Goals.GetID(cnt, &iCount)); cnt++)
    for (cnt2 = 0; cnt2 < iCount; cnt2++)
      CreateObject(idType);
}

void C4Game::UpdateRules() {
  if (Tick255)
    return;
  Rules = 0;
  if (Objects.Find(C4Id("ENRG")))
    Rules |= C4RULE_StructuresNeedEnergy;
  if (Objects.Find(C4Id("CNMT")))
    Rules |= C4RULE_ConstructionNeedsMaterial;
  if (Objects.Find(C4Id("FGRV")))
    Rules |= C4RULE_FlagRemoveable;
}

BOOL C4Game::SaveDeniedDesc(C4Group &hGroup) {
  char *buffer;
  if (!(buffer = new char[10000]))
    return FALSE;
  buffer[0] = 0;

  // Header
  SAppend("{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1031{\\fonttbl "
          "{\\f0\\fnil\\fcharset0 Times New Roman;}}",
          buffer);
  SAppend(LineFeed, buffer);

  // Scenario title
  sprintf(OSTR, "\\uc1\\pard\\ulnone\\b\\f0\\fs20 %s\\par", ScenarioTitle);
  SAppend(OSTR, buffer);
  SAppend(LineFeed, buffer);
  SAppend("\\b0\\fs16\\par", buffer);
  SAppend(LineFeed, buffer);

  // Network game host
  sprintf(OSTR, LoadResStr(IDS_DESC_HOST), Network.LocalName,
          Network.LocalAddress);
  SAppend(OSTR, buffer);
  SAppend("\\par", buffer);
  SAppend(LineFeed, buffer);

  SAppend("\\par", buffer);
  SAppend(LineFeed, buffer);
  SAppend(LoadResStr(IDS_NET_REFDENIED), buffer);

  // End of file
  SAppend(LineFeed, buffer);
  SAppend("}", buffer);
  SAppend(LineFeed, buffer);
  SAppend(EndOfFile, buffer);

  sprintf(OSTR, C4CFN_ScenarioDesc, Config.General.Language);
  return hGroup.Add(OSTR, buffer, SLen(buffer), FALSE, TRUE);
}
