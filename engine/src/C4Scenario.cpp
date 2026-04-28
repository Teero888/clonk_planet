/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Core component of a scenario file */

#include <C4Include.h>

//==================================== C4SVal
//==============================================

C4SVal::C4SVal() { Default(); }

void C4SVal::Set(int std, int rnd, int min, int max) {
  Std = std;
  Rnd = rnd;
  Min = min;
  Max = max;
}

int C4SVal::Evaluate() {
  return BoundBy(Std + Random(2 * Rnd + 1) - Rnd, Min, Max);
}

void C4SVal::Default() { Set(); }

//=============================== C4Scenario Compilation
//================================================

#define offsC4S(x) offsetof(C4Scenario, x)

C4CompilerValue C4CR_Scenario[] = {

    {"Head", C4CV_Section, 0, 0},
    {"Icon", C4CV_Integer, offsC4S(Head.Icon), 1},
    {"Title", C4CV_String, offsC4S(Head.Title), C4MaxTitle},
    {"Version", C4CV_Integer, offsC4S(Head.C4XVer1), 3},
    {"RoundOptions", C4CV_Integer, offsC4S(Head.EnableRoundOptions), 1},
    {"Access", C4CV_Integer, offsC4S(Head.EnableUnregisteredAccess), 1},
    {"MaxPlayer", C4CV_Integer, offsC4S(Head.MaxPlayer), 1},
    {"SaveGame", C4CV_Integer, offsC4S(Head.SaveGame), 1},
    {"NoInitialize", C4CV_Integer, offsC4S(Head.NoInitialize), 1},
    {"MissionAccess", C4CV_String, offsC4S(Head.MissionAccess), C4MaxTitle},
    {"NetworkGame", C4CV_Integer, offsC4S(Head.NetworkGame), 1},
    {"NetworkReference", C4CV_Integer, offsC4S(Head.NetworkReference), 1},
    {"NetworkLobby", C4CV_Integer, offsC4S(Head.NetworkLobby), 1},
    {"NetworkHostName", C4CV_String, offsC4S(Head.NetworkHostName), C4MaxTitle},
    {"NetworkHostAddress", C4CV_String, offsC4S(Head.NetworkHostAddress),
     C4MaxTitle},
    {"NetworkFilename", C4CV_String, offsC4S(Head.NetworkFilename), C4MaxTitle},
    {"NetworkFileCreation", C4CV_Integer, offsC4S(Head.NetworkFileCreation), 1},
    {"NetworkDeveloperMode", C4CV_Integer, offsC4S(Head.NetworkDeveloperMode),
     1},
    {"NetworkNoRuntimeJoin", C4CV_Integer, offsC4S(Head.NetworkNoRuntimeJoin),
     1},
    {"NetworkClients", C4CV_NameList, offsC4S(Head.NetworkClients), 1},

    {"Definitions", C4CV_Section, 0, 0},
    {"LocalOnly", C4CV_Integer, offsC4S(Definitions.LocalOnly), 1},
    {"Definition1", C4CV_String, offsC4S(Definitions.Definition[0]), _MAX_PATH},
    {"Definition2", C4CV_String, offsC4S(Definitions.Definition[1]), _MAX_PATH},
    {"Definition3", C4CV_String, offsC4S(Definitions.Definition[2]), _MAX_PATH},
    {"Definition4", C4CV_String, offsC4S(Definitions.Definition[3]), _MAX_PATH},
    {"Definition5", C4CV_String, offsC4S(Definitions.Definition[4]), _MAX_PATH},
    {"Definition6", C4CV_String, offsC4S(Definitions.Definition[5]), _MAX_PATH},
    {"Definition7", C4CV_String, offsC4S(Definitions.Definition[6]), _MAX_PATH},
    {"Definition8", C4CV_String, offsC4S(Definitions.Definition[7]), _MAX_PATH},
    {"Definition9", C4CV_String, offsC4S(Definitions.Definition[8]), _MAX_PATH},
    {"Definition10", C4CV_String, offsC4S(Definitions.Definition[9]),
     _MAX_PATH},

    {"Game", C4CV_Section, 0, 0},
    {"Mode", C4CV_Integer, offsC4S(Game.Mode), 1},
    {"Elimination", C4CV_Integer, offsC4S(Game.Elimination), 1},
    {"CooperativeGoal", C4CV_Integer, offsC4S(Game.CooperativeGoal), 1},
    {"CreateObjects", C4CV_IdList, offsC4S(Game.CreateObjects), 1},
    {"ClearObjects", C4CV_IdList, offsC4S(Game.ClearObjects), 1},
    {"ClearMaterials", C4CV_NameList, offsC4S(Game.ClearMaterial), 1},
    {"ValueGain", C4CV_Integer, offsC4S(Game.ValueGain), 1},
    {"EnableRemoveFlag", C4CV_Integer, offsC4S(Game.EnableRemoveFlag), 1},
    {"EnableSurrender", C4CV_Integer, offsC4S(Game.EnableSurrender), 1},
    {"StructNeedMaterial", C4CV_Integer,
     offsC4S(Realism.ConstructionNeedsMaterial), 1},
    {"StructNeedEnergy", C4CV_Integer, offsC4S(Realism.StructuresNeedEnergy),
     1},
    {"Goals", C4CV_IdList, offsC4S(Game.Goals), 1},
    {"Rules", C4CV_IdList, offsC4S(Game.Rules), 1},

    {"Player1", C4CV_Section, 0, 0},
    {"StandardCrew", C4CV_Id, offsC4S(PlrStart[0].NativeCrew), 1},
    {"Clonks", C4CV_Integer, offsC4S(PlrStart[0].Crew), 2},
    {"Wealth", C4CV_Integer, offsC4S(PlrStart[0].Wealth), 2},
    {"Position", C4CV_Integer, offsC4S(PlrStart[0].PositionX), 2},
    {"EnforcePosition", C4CV_Integer, offsC4S(PlrStart[0].EnforcePosition), 1},
    {"Crew", C4CV_IdList, offsC4S(PlrStart[0].ReadyCrew), 1},
    {"Buildings", C4CV_IdList, offsC4S(PlrStart[0].ReadyBase), 1},
    {"Vehicles", C4CV_IdList, offsC4S(PlrStart[0].ReadyVehic), 1},
    {"Material", C4CV_IdList, offsC4S(PlrStart[0].ReadyMaterial), 1},
    {"Knowledge", C4CV_IdList, offsC4S(PlrStart[0].BuildKnowledge), 1},
    {"HomeBaseMaterial", C4CV_IdList, offsC4S(PlrStart[0].HomeBaseMaterial), 1},
    {"HomeBaseProduction", C4CV_IdList, offsC4S(PlrStart[0].HomeBaseProduction),
     1},
    {"Magic", C4CV_IdList, offsC4S(PlrStart[0].Magic), 1},

    {"Player2", C4CV_Section, 0, 0},
    {"StandardCrew", C4CV_Id, offsC4S(PlrStart[1].NativeCrew), 1},
    {"Clonks", C4CV_Integer, offsC4S(PlrStart[1].Crew), 2},
    {"Wealth", C4CV_Integer, offsC4S(PlrStart[1].Wealth), 2},
    {"Position", C4CV_Integer, offsC4S(PlrStart[1].PositionX), 2},
    {"EnforcePosition", C4CV_Integer, offsC4S(PlrStart[1].EnforcePosition), 1},
    {"Crew", C4CV_IdList, offsC4S(PlrStart[1].ReadyCrew), 1},
    {"Buildings", C4CV_IdList, offsC4S(PlrStart[1].ReadyBase), 1},
    {"Vehicles", C4CV_IdList, offsC4S(PlrStart[1].ReadyVehic), 1},
    {"Material", C4CV_IdList, offsC4S(PlrStart[1].ReadyMaterial), 1},
    {"Knowledge", C4CV_IdList, offsC4S(PlrStart[1].BuildKnowledge), 1},
    {"HomeBaseMaterial", C4CV_IdList, offsC4S(PlrStart[1].HomeBaseMaterial), 1},
    {"HomeBaseProduction", C4CV_IdList, offsC4S(PlrStart[1].HomeBaseProduction),
     1},
    {"Magic", C4CV_IdList, offsC4S(PlrStart[1].Magic), 1},

    {"Player3", C4CV_Section, 0, 0},
    {"StandardCrew", C4CV_Id, offsC4S(PlrStart[2].NativeCrew), 1},
    {"Clonks", C4CV_Integer, offsC4S(PlrStart[2].Crew), 2},
    {"Wealth", C4CV_Integer, offsC4S(PlrStart[2].Wealth), 2},
    {"EnforcePosition", C4CV_Integer, offsC4S(PlrStart[2].EnforcePosition), 1},
    {"Position", C4CV_Integer, offsC4S(PlrStart[2].PositionX), 2},
    {"Crew", C4CV_IdList, offsC4S(PlrStart[2].ReadyCrew), 1},
    {"Buildings", C4CV_IdList, offsC4S(PlrStart[2].ReadyBase), 1},
    {"Vehicles", C4CV_IdList, offsC4S(PlrStart[2].ReadyVehic), 1},
    {"Material", C4CV_IdList, offsC4S(PlrStart[2].ReadyMaterial), 1},
    {"Knowledge", C4CV_IdList, offsC4S(PlrStart[2].BuildKnowledge), 1},
    {"HomeBaseMaterial", C4CV_IdList, offsC4S(PlrStart[2].HomeBaseMaterial), 1},
    {"HomeBaseProduction", C4CV_IdList, offsC4S(PlrStart[2].HomeBaseProduction),
     1},
    {"Magic", C4CV_IdList, offsC4S(PlrStart[2].Magic), 1},

    {"Player4", C4CV_Section, 0, 0},
    {"StandardCrew", C4CV_Id, offsC4S(PlrStart[3].NativeCrew), 1},
    {"Clonks", C4CV_Integer, offsC4S(PlrStart[3].Crew), 2},
    {"Wealth", C4CV_Integer, offsC4S(PlrStart[3].Wealth), 2},
    {"Position", C4CV_Integer, offsC4S(PlrStart[3].PositionX), 2},
    {"EnforcePosition", C4CV_Integer, offsC4S(PlrStart[3].EnforcePosition), 1},
    {"Crew", C4CV_IdList, offsC4S(PlrStart[3].ReadyCrew), 1},
    {"Buildings", C4CV_IdList, offsC4S(PlrStart[3].ReadyBase), 1},
    {"Vehicles", C4CV_IdList, offsC4S(PlrStart[3].ReadyVehic), 1},
    {"Material", C4CV_IdList, offsC4S(PlrStart[3].ReadyMaterial), 1},
    {"Knowledge", C4CV_IdList, offsC4S(PlrStart[3].BuildKnowledge), 1},
    {"HomeBaseMaterial", C4CV_IdList, offsC4S(PlrStart[3].HomeBaseMaterial), 1},
    {"HomeBaseProduction", C4CV_IdList, offsC4S(PlrStart[3].HomeBaseProduction),
     1},
    {"Magic", C4CV_IdList, offsC4S(PlrStart[3].Magic), 1},

    {"Landscape", C4CV_Section, 0, 0},
    {"ExactLandscape", C4CV_Integer, offsC4S(Landscape.ExactLandscape), 1},
    {"Vegetation", C4CV_IdList, offsC4S(Landscape.Vegetation), 0},
    {"VegetationLevel", C4CV_Integer, offsC4S(Landscape.VegLevel), 4},
    {"InEarth", C4CV_IdList, offsC4S(Landscape.InEarth), 0},
    {"InEarthLevel", C4CV_Integer, offsC4S(Landscape.InEarthLevel), 4},
    {"Sky", C4CV_String, offsC4S(Landscape.SkyDef), C4MaxDefString},
    {"SkyFade", C4CV_Integer, offsC4S(Landscape.SkyDefFade), 6},
    {"NoSky", C4CV_Integer, offsC4S(Landscape.NoSky), 1},
    {"BottomOpen", C4CV_Integer, offsC4S(Landscape.BottomOpen), 1},
    {"TopOpen", C4CV_Integer, offsC4S(Landscape.TopOpen), 1},
    {"LeftOpen", C4CV_Integer, offsC4S(Landscape.LeftOpen), 1},
    {"RightOpen", C4CV_Integer, offsC4S(Landscape.RightOpen), 1},
    {"AutoScanSideOpen", C4CV_Integer, offsC4S(Landscape.AutoScanSideOpen), 1},
    {"MapWidth", C4CV_Integer, offsC4S(Landscape.MapWdt), 4},
    {"MapHeight", C4CV_Integer, offsC4S(Landscape.MapHgt), 4},
    {"MapZoom", C4CV_Integer, offsC4S(Landscape.MapZoom), 4},
    {"Amplitude", C4CV_Integer, offsC4S(Landscape.Amplitude), 4},
    {"Phase", C4CV_Integer, offsC4S(Landscape.Phase), 4},
    {"Period", C4CV_Integer, offsC4S(Landscape.Period), 4},
    {"Random", C4CV_Integer, offsC4S(Landscape.Random), 4},
    {"Material", C4CV_String, offsC4S(Landscape.Material), C4M_MaxDefName},
    {"Liquid", C4CV_String, offsC4S(Landscape.Liquid), C4M_MaxDefName},
    {"LiquidLevel", C4CV_Integer, offsC4S(Landscape.LiquidLevel), 4},
    {"MapPlayerExtend", C4CV_Integer, offsC4S(Landscape.MapPlayerExtend), 1},
    {"Layers", C4CV_NameList, offsC4S(Landscape.Layers), 0},
    {"Gravity", C4CV_Integer, offsC4S(Landscape.Gravity), 4},

    {"Animals", C4CV_Section, 0, 0},
    {
        "Animal",
        C4CV_IdList,
        offsC4S(Animals.FreeLife),
        0,
    },
    {
        "Nest",
        C4CV_IdList,
        offsC4S(Animals.EarthNest),
        0,
    },

    {"Weather", C4CV_Section, 0, 0},
    {"Climate", C4CV_Integer, offsC4S(Weather.Climate), 2},
    {"StartSeason", C4CV_Integer, offsC4S(Weather.StartSeason), 2},
    {"YearSpeed", C4CV_Integer, offsC4S(Weather.YearSpeed), 2},
    {"Rain", C4CV_Integer, offsC4S(Weather.Rain), 2},
    {"Wind", C4CV_Integer, offsC4S(Weather.Wind), 2},
    {"Lightning", C4CV_Integer, offsC4S(Weather.Lightning), 2},
    {"Precipitation", C4CV_String, offsC4S(Weather.Precipitation), C4M_MaxName},

    {"Disasters", C4CV_Section, 0, 0},
    {"Meteorite", C4CV_Integer, offsC4S(Disasters.Meteorite), 2},
    {"Volcano", C4CV_Integer, offsC4S(Disasters.Volcano), 2},
    {"Earthquake", C4CV_Integer, offsC4S(Disasters.Earthquake), 2},

    {"Environment", C4CV_Section, 0, 0},
    {
        "Objects",
        C4CV_IdList,
        offsC4S(Environment.Objects),
        0,
    },

    {NULL, C4CV_End, 0, 0}};

//================================ C4Scenario
//====================0======================

C4Scenario::C4Scenario() { Default(); }

void C4Scenario::Default() {
  int cnt;
  Head.Default();
  Definitions.Default();
  Game.Default();
  for (cnt = 0; cnt < C4S_MaxPlayer; cnt++)
    PlrStart[cnt].Default();
  Landscape.Default();
  Animals.Default();
  Weather.Default();
  Disasters.Default();
  Realism.Default();
  Environment.Default();
}

BOOL C4Scenario::Load(C4Group &hGroup) {
  BYTE *pSource;
  // Load
  if (!hGroup.LoadEntry(C4CFN_ScenarioCore, &pSource, NULL, 1))
    return FALSE;
  // Compile
  if (!Compile((char *)pSource)) {
    delete[] pSource;
    return FALSE;
  }
  delete[] pSource;
  // Convert
  Game.ConvertGoals(Realism);
  // Success
  return TRUE;
}

BOOL C4Scenario::Save(C4Group &hGroup) {
  char *Buffer;
  int BufferSize;
  if (!Decompile(&Buffer, &BufferSize))
    return FALSE;
  if (!hGroup.Add(C4CFN_ScenarioCore, Buffer, BufferSize, FALSE, TRUE)) {
    delete[] Buffer;
    return FALSE;
  }
  return TRUE;
}
void C4SDefinitions::Default() { ZeroMem(this, sizeof(C4SDefinitions)); }

void C4SHead::Default() {
  ZeroMem(this, sizeof(C4SHead));

  EnableRoundOptions = 1;
  Icon = 18;
  MaxPlayer = C4S_MaxPlayer;
  SCopy("Default Title", Title, C4MaxTitle);
}

void C4SGame::Default() {
  Elimination = C4S_EliminateCrew;
  ValueGain = 0;
  CreateObjects.Clear();
  ClearObjects.Clear();
  ClearMaterial.Clear();
  Mode = C4S_Cooperative;
  CooperativeGoal = C4S_NoGoal;
  EnableRemoveFlag = 0;
  EnableSurrender = 1;
  Goals.Clear();
  Rules.Clear();
}

void C4SGame::ClearCooperativeGoals() {
  ValueGain = 0;
  CreateObjects.Clear();
  ClearObjects.Clear();
  ClearMaterial.Clear();
}

void C4SPlrStart::Default() {
  NativeCrew = C4ID_None;
  Crew.Set(1, 0, 1, 10);
  Wealth.Set(0, 0, 0, 250);
  PositionX = -1;
  PositionY = -1;
  EnforcePosition = 0;
  ReadyCrew.Default();
  ReadyBase.Default();
  ReadyVehic.Default();
  ReadyMaterial.Default();
  BuildKnowledge.Default();
  HomeBaseMaterial.Default();
  HomeBaseProduction.Default();
  Magic.Default();
}

bool C4SPlrStart::EquipmentEqual(const C4SPlrStart &rhs) {
  C4SPlrStart plr1, plr2;
  plr1 = *this;
  plr2 = rhs;
  plr1.PositionX = plr1.PositionY = 0;
  plr2.PositionX = plr2.PositionY = 0;
  return (plr1 == plr2);
}

void C4SLandscape::Default() {
  BottomOpen = 0;
  TopOpen = 1;
  LeftOpen = 0;
  RightOpen = 0;
  AutoScanSideOpen = 1;
  SkyDef[0] = 0;
  NoSky = 0;
  for (int cnt = 0; cnt < 6; cnt++)
    SkyDefFade[cnt] = 0;
  VegLevel.Set(50, 30, 0, 100);
  InEarthLevel.Set(50, 0, 0, 100);
  MapWdt.Set(100, 0, 64, 250);
  MapHgt.Set(50, 0, 40, 250);
  MapZoom.Set(10, 0, 5, 15);
  Amplitude.Set(0, 0);
  Phase.Set(50);
  Period.Set(15);
  Random.Set(0);
  MapPlayerExtend = 0;
  SCopy("Earth", Material, C4M_MaxName);
  SCopy("Water", Liquid, C4M_MaxName);
  ExactLandscape = 0;
  Gravity.Set(100, 0, 10, 200);
}

void C4SLandscape::GetMapSize(int &rWdt, int &rHgt, int iPlayerNum) {
  rWdt = MapWdt.Evaluate();
  rHgt = MapHgt.Evaluate();
  iPlayerNum = Max(iPlayerNum, 1);
  if (MapPlayerExtend)
    rWdt = Min(rWdt * iPlayerNum, MapWdt.Max);
}

void C4SWeather::Default() {
  Climate.Set(50, 10);
  StartSeason.Set(50, 50);
  YearSpeed.Set(50);
  Wind.Set(0, 70, -100, +100);
  SCopy("Water", Precipitation, C4M_MaxName);
}

void C4SAnimals::Default() {
  FreeLife.Clear();
  EarthNest.Clear();
}

void C4SEnvironment::Default() { Objects.Clear(); }

void C4SRealism::Default() {
  ConstructionNeedsMaterial = 0;
  StructuresNeedEnergy = 1;
}

void C4SDisasters::Default() {
  Volcano.Default();
  Earthquake.Default();
  Meteorite.Default();
}

BOOL C4Scenario::Compile(const char *szSource) {
  C4Compiler Compiler;
  Default();
  return Compiler.CompileStructure(C4CR_Scenario, szSource, this);
}

BOOL C4Scenario::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;
  C4Scenario dC4S;
  *ppOutput = NULL;
  return Compiler.DecompileStructure(C4CR_Scenario, this, &dC4S, ppOutput,
                                     ipSize);
}

void C4Scenario::Clear() {}

void C4Scenario::SetExactLandscape() {
  if (Landscape.ExactLandscape)
    return;
  int iMapZoom = Landscape.MapZoom.Std;
  // Set landscape
  Landscape.ExactLandscape = 1;
  Landscape.MapZoom.Set(1, 0, 1, 1);
  // Zoom player starting positions
  for (int cnt = 0; cnt < C4S_MaxPlayer; cnt++) {
    if (PlrStart[cnt].PositionX >= -1)
      PlrStart[cnt].PositionX = PlrStart[cnt].PositionX * iMapZoom;
    if (PlrStart[cnt].PositionY >= -1)
      PlrStart[cnt].PositionY = PlrStart[cnt].PositionY * iMapZoom;
  }
}

BOOL C4SDefinitions::GetModules(char *sList) {
  // Safety
  if (!sList)
    return FALSE;
  // Local only
  if (LocalOnly) {
    sList[0] = 0;
    return TRUE;
  }
  // Scan for any valid entries
  BOOL fSpecified = FALSE;
  for (int cnt = 0; cnt < C4S_MaxDefinitions; cnt++)
    if (Definition[cnt][0])
      fSpecified = TRUE;
  // No valid entries
  if (!fSpecified)
    return FALSE;
  // Compose entry list
  sList[0] = 0;
  int cnt;
  for (cnt = 0; cnt < C4S_MaxDefinitions; cnt++)
    if (Definition[cnt][0])
      SAddModule(sList, Definition[cnt]);
  // Done
  return TRUE;
}

void C4SDefinitions::SetModules(const char *szList,
                                const char *szRelativeToPath) {
  int cnt;

  // Empty list: local only
  if (!SModuleCount(szList)) {
    LocalOnly = TRUE;
    for (cnt = 0; cnt < C4S_MaxDefinitions; cnt++)
      Definition[cnt][0] = 0;
    return;
  }

  // Set list
  LocalOnly = FALSE;
  for (cnt = 0; cnt < C4S_MaxDefinitions; cnt++) {
    SGetModule(szList, cnt, Definition[cnt], _MAX_PATH);
    // Make relative path
    if (szRelativeToPath)
      if (SEqualNoCase(Definition[cnt], szRelativeToPath,
                       SLen(szRelativeToPath)))
        SCopy(Definition[cnt] + SLen(szRelativeToPath), Definition[cnt]);
  }
}

BOOL C4SDefinitions::AssertModules(const char *szPath, char *sMissing) {
  // Local only
  if (LocalOnly)
    return TRUE;

  // Check all listed modules for availability
  BOOL fAllAvailable = TRUE;
  char szModule[_MAX_PATH + 1];
  if (sMissing)
    sMissing[0] = 0;
  // Check all definition files
  for (int cnt = 0; cnt < C4S_MaxDefinitions; cnt++)
    if (Definition[cnt][0]) {
      // Compose filename using path specified by caller
      szModule[0] = 0;
      if (szPath)
        SCopy(szPath, szModule);
      if (szModule[0])
        AppendBackslash(szModule);
      SAppend(Definition[cnt], szModule);
      // Missing
      if (!C4Group_IsGroup(szModule)) {
        // Add to list
        if (sMissing) {
          SNewSegment(sMissing, ", ");
          SAppend(Definition[cnt], sMissing);
        }
        fAllAvailable = FALSE;
      }
    }

  return fAllAvailable;
}

void C4SGame::ConvertGoals(C4SRealism &rRealism) {

  // Convert mode to goals
  switch (Mode) {
  case C4S_Melee:
    Goals.SetIDCount(C4Id("MELE"), 1, TRUE);
    ClearOldGoals();
    break;
  case C4S_MeleeTeamwork:
    Goals.SetIDCount(C4Id("MEL2"), 1, TRUE);
    ClearOldGoals();
    break;
  }
  Mode = 0;

  // Convert goals (master selection)
  switch (CooperativeGoal) {
  case C4S_Goldmine:
    Goals.SetIDCount(C4Id("GLDM"), 1, TRUE);
    ClearOldGoals();
    break;
  case C4S_Monsterkill:
    Goals.SetIDCount(C4Id("MNTK"), 1, TRUE);
    ClearOldGoals();
    break;
  case C4S_ValueGain:
    Goals.SetIDCount(C4Id("VALG"), Max(ValueGain / 100, 1), TRUE);
    ClearOldGoals();
    break;
  }
  CooperativeGoal = 0;
  // CreateObjects,ClearObjects,ClearMaterials are still valid but invisible

  // Convert realism to rules
  if (rRealism.ConstructionNeedsMaterial)
    Rules.SetIDCount(C4Id("CNMT"), 1, TRUE);
  rRealism.ConstructionNeedsMaterial = 0;
  if (rRealism.StructuresNeedEnergy)
    Rules.SetIDCount(C4Id("ENRG"), 1, TRUE);
  rRealism.StructuresNeedEnergy = 0;

  // Convert rules
  if (EnableSurrender)
    Rules.SetIDCount(C4Id("SURR"), 1, TRUE);
  EnableSurrender = 0;
  if (EnableRemoveFlag)
    Rules.SetIDCount(C4Id("FGRV"), 1, TRUE);
  EnableRemoveFlag = 0;

  // Convert eliminiation to rules
  switch (Elimination) {
  case C4S_KillTheCaptain:
    Rules.SetIDCount(C4Id("KILC"), 1, TRUE);
    break;
  case C4S_CaptureTheFlag:
    Rules.SetIDCount(C4Id("CTFL"), 1, TRUE);
    break;
  }
  Elimination = 1; // unconvertible default crew elimination

  // CaptureTheFlag requires FlagRemoveable
  if (Rules.GetIDCount(C4Id("CTFL")))
    Rules.SetIDCount(C4Id("FGRV"), 1, TRUE);
}

void C4SGame::ClearOldGoals() {
  CreateObjects.Clear();
  ClearObjects.Clear();
  ClearMaterial.Clear();
  ValueGain = 0;
}

BOOL C4SGame::IsMelee() {
  return (Goals.GetIDCount(C4Id("MELE")) || Goals.GetIDCount(C4Id("MEL2")));
}
