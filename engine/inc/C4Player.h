/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Player data at runtime */

const int C4PVM_Cursor = 0, C4PVM_Target = 1, C4PVM_Scrolling = 2;

const int C4P_Number_None = -5;

const int C4P_Command_None = 0, C4P_Command_Set = 1, C4P_Command_Add = 2,
          C4P_Command_Append = 4, C4P_Command_Range = 8;

class C4Player : public C4PlayerInfoCore {
  friend C4PlayerList;

public:
  C4Player();
  ~C4Player();

public:
  char Filename[_MAX_PATH + 1];
  BOOL Status;
  BOOL Eliminated;
  BOOL Surrendered;
  int Number;
  int Color;
  int Control;
  int MouseControl;
  int Position;
  int PlrStartIndex;
  int RetireDelay;
  int AtClient;
  char AtClientName[C4MaxTitle + 1];
  BOOL LocalControl; // Local-NoSave //
  // Menu
  C4Menu Menu; // Local-NoSave //
  // View
  int ViewMode;
  int ViewX, ViewY;
  C4Object *ViewTarget; // NoSave //
  int ViewWealth, ViewValue;
  BOOL ShowStartup;
  int ShowControl, ShowControlPos;
  int FlashCom;         // NoSave //
  BOOL RedrawFogOfWar;  // NoSave //
  C4FogOfWar *FogOfWar; // NoSave //
  // Game
  int Wealth, Points;
  int Value, InitialValue, ValueGain;
  int ObjectsOwned;
  C4Object *Captain;
  C4IDList Hostility;
  // Home Base
  C4IDList HomeBaseMaterial;
  C4IDList HomeBaseProduction;
  int ProductionDelay, ProductionUnit;
  // Crew
  C4ObjectInfoList CrewInfoList; // No Save //
  C4ObjectList Crew;             // No Save //
  int CrewCnt;                   // No Save //
  // Knowledge
  C4IDList Knowledge;
  C4IDList Magic;
  // Control
  C4Object *Cursor;
  int SelectCount;
  int SelectFlash, CursorFlash;
  int LastCom;
  int LastComDel;
  int LastComDownDouble;
  int CursorSelection, CursorToggled;
  // Message
  int MessageStatus;
  char MessageBuf[256 + 1];
  // Portrait
  C4FacetEx Portrait;
  // Dynamic list
  C4Player *Next;

public:
  void Eliminate();
  void SelectCrew(C4Object *pObj, BOOL fSelect);
  void Default();
  void Clear();
  void ClearPointers(C4Object *tptr);
  void Execute();
  void ExecuteControl();
  void UpdateValue();
  void SetViewMode(int iMode, C4Object *pTarget = NULL);
  void Evaluate();
  void Surrender();
  void ScrollView(int iX, int iY, int ViewWdt, int ViewHgt);
  void SelectCrew(C4ObjectList &rList);
  void SetCursor(C4Object *pObj);
  void RemoveCrewObjects();
  void DefaultRuntimeData();
  void DrawHostility(C4FacetEx &cgo, int iIndex);
  void AdjustCursorCommand();
  void CursorRight();
  void CursorLeft();
  void UnselectCrew();
  void SelectSingleByCursor();
  void SelectSingle(C4Object *tobj);
  void CursorToggle();
  void SelectAllCrew();
  void UpdateSelectionToggleStatus();
  void DirectCom(BYTE byCom, int iData);
  void InCom(BYTE byCom, int iData);
  BOOL ObjectCom(BYTE byCom, int iData);
  BOOL ObjectCommand(int iCommand, C4Object *pTarget, int iTx, int iTy,
                     C4Object *pTarget2 = NULL, int iData = 0,
                     int iAddMode = C4P_Command_Set);
  BOOL DoPoints(int iChange);
  BOOL Init(int iNumber, int iAtClient, const char *szAtClientName,
            const char *szFilename, BOOL fScenarioInit);
  BOOL ScenarioInit();
  BOOL FinalInit(BOOL fInitialValue);
  BOOL Save();
  BOOL MakeCrewMember(C4Object *pObj);
  BOOL Load(const char *szFilename);
  BOOL Message(const char *szMsg);
  BOOL ObjectInCrew(C4Object *tobj);
  BOOL Sell2Home(C4Object *tobj);
  BOOL DoWealth(int change);
  BOOL SetHostility(int iOpponent, int iHostility);
  BOOL Compile(const char *szSource, const char *szSecExt);
  BOOL Decompile(char **ppOutput, int *ipSize, const char *szSecExt);
  BOOL LoadRuntimeData(C4Group &hGroup);
  BOOL ActivateMenuMain();
  BOOL MenuCommand(const char *szCommand);
  C4Object *GetHiExpActiveCrew();
  C4Object *GetHiRankActiveCrew();

protected:
  void UpdateFogOfWar();
  void InitControl();
  void DenumeratePointers();
  void EnumeratePointers();
  void UpdateView();
  void CheckElimination();
  void UpdateCounts();
  void ExecHomeBaseProduction();
  void PlaceReadyBase(int &tx, int &ty, C4Object **pFirstBase);
  void PlaceReadyVehic(int tx1, int tx2, int ty, C4Object *FirstBase);
  void PlaceReadyMaterial(int tx1, int tx2, int ty, C4Object *FirstBase);
  void PlaceReadyCrew(int tx1, int tx2, int ty, C4Object *FirstBase);
  void CheckCrewExPromotion();
  BOOL ActivateMenuHostility();
  BOOL ActivateMenuNewPlayer();
  BOOL ActivateMenuSurrender();
  BOOL ActivateMenuRules();
  BOOL ActivateMenuGoals();
};
