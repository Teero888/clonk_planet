/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to run the game */

class C4Game {
public:
  C4Game();
  ~C4Game();

public:
  C4DefList Defs;
  C4TextureMap TextureMap;
  C4RankSystem Rank;
  C4GraphicsSystem GraphicsSystem;
  C4GraphicsResource GraphicsResource;
  C4MusicSystem MusicSystem;
  C4SoundSystem SoundSystem;
  C4Network Network;
  C4InputHandler InputHandler;
  C4GameMessageList Messages;
  C4GamePadControl GamePadCon1;
  C4MouseControl MouseControl;
  C4Weather Weather;
  C4MaterialMap Material;
  C4ObjectList Objects;
  C4Landscape Landscape;
  C4Scenario C4S;
  C4ComponentHost Info;
  C4ComponentHost Title;
  C4ComponentHost Names;
  C4ComponentHost GameText;
  C4ScriptHost Script;
  C4MassMoverSet MassMover;
  C4PXSSystem PXS;
  C4PlayerList Players;
  C4Control Control, Input;
  C4ControlJoinClient ControlJoinClient;
  C4ControlQueue ControlQueue;
  C4PathFinder PathFinder;
  C4TransferZones TransferZones;
  C4Group ScenarioFile;
  char ScenarioFilename[_MAX_PATH + 1];
  char ScenarioTitle[C4MaxTitle + 1];
  char PlayerFilenames[20 * _MAX_PATH + 1];
  char NetworkJoinPlayerFilenames[20 * _MAX_PATH + 1];
  char DefinitionFilenames[20 * _MAX_PATH + 1];
  char DirectJoinAddress[_MAX_PATH + 1];
  int StartupPlayerCount;
  int FPS, cFPS;
  BOOL Halt;
  BOOL GameOver;
  BOOL Evaluated;
  BOOL fScriptCreatedObjects;
  BOOL fLobby;
  BOOL fReferenceDefinitionOverride;
  BOOL AsynchronousControl;
  BOOL Record, Replay;
  int FrameCounter;
  int iTick2, iTick3, iTick5, iTick10, iTick35, iTick255, iTick500, iTick1000;
  int Time;
  int ControlTimeStart, ControlTimeBehind;
  int ControlTick, ControlRate;
  int InputTick, InputRate;
  int SyncTick, SyncRate;
  BOOL DoControl, DoInput, DoSync;
  int RandomSeed;
  int ObjectEnumerationIndex;
  int Rules;
  static volatile BOOL GameGo;
  static volatile BOOL TimeGo;

public:
  // Init and execution
  void Default();
  void Clear();
  void Evaluate();
  void Sec1Timer();
  void KeyboardInput(WORD vk_code, BOOL fAlt = FALSE);
  void Draw(C4FacetEx &cgo);
  void DrawCursors(C4FacetEx &cgo);
  void LocalPlayerControl(int iPlayer, int iCom);
  void FixRandom(int iSeed);
  BOOL Init(const char *szCmdLine);
  BOOL Execute();
  BOOL Save(C4Group &hGroup, BOOL fSaveGame, BOOL fNetwork);
  BOOL SaveNetworkReference(C4Group &hGroup, BOOL fDenied = FALSE);
  BOOL JoinPlayer(const char *szFilename, int iAtClient, const char *szAtClientName);
  BOOL DoGameOver();
  BOOL QuickSave(BOOL fSaveGame);
  BOOL ExecuteControl();
  DWORD ControlTime();
  // Network
  void Synchronize();
  void SyncClearance();
  BOOL RequestJoin(C4Stream *pStream);
  BOOL RequestNetworkReference(C4Stream *pStream);
  // Editing
  int CommandLine(const char *szScript);
  BOOL DropFile(const char *szFilename, int iX, int iY);
  BOOL CreateViewport(int iPlayer);
  BOOL DropDef(C4ID id, int iX, int iY);
  BOOL LoadDef(const char *szFilename);
  BOOL ReloadDef(C4ID id);
  // Object functions
  void ClearPointers(C4Object *cobj);
  C4Object *CreateObject(C4ID type, int owner = NO_OWNER, int x = 50, int y = 50, int r = 0, FIXED xdir = 0, FIXED ydir = 0, FIXED rdir = 0);
  C4Object *CreateObjectConstruction(C4ID type, int owner, int ctx = 0, int bty = 0, int con = 1, BOOL terrain = FALSE);
  C4Object *CreateInfoObject(C4ObjectInfo *cinf, int owner, int tx = 50, int ty = 50);
  void BlastObjects(int tx, int ty, int level, C4Object *inobj, int iCausedBy);
  void ShakeObjects(int tx, int ry, int range);
  C4Object *AtObject(int ctx, int cty, DWORD &ocf, C4Object *exclude = NULL);
  C4Object *OverlapObject(int tx, int ty, int wdt, int hgt, int category);
  C4Object *FindObject(C4ID id, int x = 0, int y = 0, int wdt = 0, int hgt = 0, DWORD ocf = OCF_All, const char *szAction = NULL, C4Object *pActionTarget = NULL, C4Object *pExclude = NULL,
                       C4Object *pContainer = NULL, int iOwner = ANY_OWNER, C4Object *pFindNext = NULL);
  int ObjectCount(C4ID id, int x = 0, int y = 0, int wdt = 0, int hgt = 0, DWORD ocf = OCF_All, const char *szAction = NULL, C4Object *pActionTarget = NULL, C4Object *pExclude = NULL,
                  C4Object *pContainer = NULL, int iOwner = ANY_OWNER);
  C4Object *FindBase(int iPlayer, int iIndex);
  C4Object *FindObjectByCommand(int iCommand, C4Object *pTarget = NULL, int iTx = 0, int iTy = 0, C4Object *pTarget2 = NULL);
  void CastObjects(C4ID id, int num, int level, int tx, int ty, int iOwner = NO_OWNER);
  void BlastCastObjects(C4ID id, int num, int tx, int ty);
  C4Object *PlaceVegetation(C4ID id, int iX, int iY, int iWdt, int iHgt, int iGrowth);
  C4Object *PlaceAnimal(C4ID idAnimal);

protected:
  void InitSystem();
  void InitInEarth();
  void InitVegetation();
  void InitAnimals();
  void InitGoals();
  void InitRules();
  void InitEnvironment();
  void UpdateRules();
  void ParseCommandLine(const char *szCmdLine);
  void CloseScenario();
  void DeleteObjects();
  void ExecObjects();
  void Ticks();
  const char *FoldersWithLocalsDefs(const char *szPath);
  BOOL SaveDeniedDesc(C4Group &hGroup);
  BOOL DirectJoinGetReference();
  BOOL CheckScenarioAccess();
  BOOL CheckObjectEnumeration();
  BOOL InitNetworkScenario();
  BOOL LocalFileMatch(const char *szFilename, int iCreation);
  BOOL DefinitionFilenamesFromSaveGame();
  BOOL LoadScenarioComponents();
  BOOL SaveGameTitle(C4Group &hGroup);
  BOOL InitGame();
  BOOL InitGameFinal();
  BOOL InitPlayers();
  BOOL OpenScenario();
  BOOL InitDefs();
  BOOL InitMaterialTexture();
  BOOL EnumerateMaterials();
  BOOL GameOverCheck();
  BOOL PlaceInEarth(C4ID id);
  BOOL Compile(const char *szSource);
  BOOL Decompile(char **ppOutput, int *ipSize);
  BOOL LoadPlayerFilenames(C4Group &hGroup);
  BOOL SaveRuntimeData(C4Group &hGroup);
  BOOL CompileRuntimeData();
  BOOL SaveDesc(C4Group &hGroup, BOOL fSaveGame = FALSE, BOOL fReference = FALSE, BOOL fLobby = FALSE);
  BOOL StoreParticipantPlayers();
  BOOL RecreatePlayerFiles();

  // Object function internals
  C4Object *NewObject(C4Def *ndef, int owner, C4ObjectInfo *info, int tx, int ty, int tr, FIXED xdir, FIXED ydir, FIXED rdir, int con);
  void ClearObjectPtrs(C4Object *tptr);
  void ClearPlayerPtrs(C4Object *tptr);
  void CrossCheckObjects();
  void ObjectRemovalCheck();
};

const int C4RULE_StructuresNeedEnergy = 1, C4RULE_ConstructionNeedsMaterial = 2, C4RULE_FlagRemoveable = 4;

extern char OSTR[500];
