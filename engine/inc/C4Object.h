/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* That which fills the world with life */

/* Action.Dir is the direction the object is actually facing. */

#define DIR_None 0
#define DIR_Left 0
#define DIR_Right 1

/* Action.ComDir tells an active object which way it ought to be going.
         If you set the ComDir to COMD_Stop, the object won't sit still
   immediately but will try to slow down according to it's current Action.
   ComDir values circle clockwise from COMD_Up 1 through COMD_UpLeft 8. */

#define COMD_None 0
#define COMD_Stop 0
#define COMD_Up 1
#define COMD_UpRight 2
#define COMD_Right 3
#define COMD_DownRight 4
#define COMD_Down 5
#define COMD_DownLeft 6
#define COMD_Left 7
#define COMD_UpLeft 8

const int FullCon = 100000;

const int MagicPhysicalFactor = 1000;

#define ANY_CONTAINER ((C4Object *)123)
#define NO_CONTAINER ((C4Object *)124)

class C4Action {
public:
  C4Action();
  ~C4Action();

public:
  char Name[C4D_MaxIDLen + 1];
  int Act; // NoSave //
  int Dir;
  int ComDir;
  int Time;
  int Data;
  int Phase, PhaseDelay;
  int t_attach; // SyncClearance-NoSave //
  C4Object *Target, *Target2;
  C4Facet Facet;      // NoSave //
  int FacetX, FacetY; // NoSave //
public:
  void Default();
};

class C4Object {
public:
  C4Object();
  ~C4Object();

public:
  uint32_t Magic;
  C4ID id;
  int Number;
  int Status;       // NoSave //
  int RemovalDelay; // NoSave //
  int Owner;
  int Controller;
  int Category;
  int x, y, r;
  int motion_x, motion_y;
  int NoCollectDelay;
  int MaskPutMotion;
  int Base;
  int Con;
  int Mass;
  int Damage;
  int Energy;
  int MagicEnergy;
  int Alive;
  int Breath;
  int FirePhase;
  int InMat; // SyncClearance-NoSave //
  int Color;
  int Timer;
  int ViewEnergy;           // NoSave //
  int Audible, LastAudible; // NoSave //
  long Local[C4MaxVariable];
  FIXED fix_x, fix_y, fix_r; // SyncClearance-Fix //
  FIXED xdir, ydir, rdir;
  BOOL Mobile;
  BOOL Select;
  BOOL MaskPut;  // NoSave //
  BOOL Unsorted; // NoSave //
  BOOL OnFire;
  BOOL InLiquid;
  BOOL EntranceStatus;
  BOOL NeedEnergy;
  DWORD t_contact; // SyncClearance-NoSave //
  DWORD OCF;

  // Menu
  C4Menu *Menu; // SyncClearance-NoSave //

  C4Facet BaseFace, TopFace;   // NoSave //
  SURFACE cBitmap, cSolidMask; // NoSave //

  C4Def *Def;
  C4Object *Contained;
  C4ObjectInfo *Info;

  C4Action Action;
  C4Shape Shape;
  C4TargetRect SolidMask;
  C4IDList Component;
  C4ObjectList Contents;
  C4MaterialList *MaterialContents; // SyncClearance-NoSave //

  BOOL PhysicalTemporary;
  C4PhysicalInfo TemporaryPhysical;

  // Commands
  C4Command *Command;

  int nActionTarget1, nActionTarget2;
  int nContained;
  char nInfo[C4MaxName + 1];

public:
  void Resort();
  void UpdateAudible();
  void DigOutMaterialCast(BOOL fRequest);
  void AddMaterialContents(int iMaterial, int iAmount);
  void SetCommand(int iCommand, C4Object *pTarget = NULL, int iTx = 0, int iTy = 0, C4Object *pTarget2 = NULL, BOOL fControl = FALSE, int iData = 0, int iRetries = 0, const char *szText = NULL);
  BOOL AddCommand(int iCommand, C4Object *pTarget = NULL, int iTx = 0, int iTy = 0, int iUpdateInterval = 0, C4Object *pTarget2 = NULL, BOOL fInitEvaluation = TRUE, int iData = 0,
                  BOOL fAppend = FALSE, int iRetries = 0, const char *szText = NULL);
  void ClearCommand(C4Command *pUntil);
  void ClearCommands();
  void DrawSelectMark(C4FacetEx &cgo);
  void UpdateActionFace();
  void SyncClearance();
  void SetSolidMask(int iX, int iY, int iWdt, int iHgt, int iTX, int iTY);
  C4Object *ComposeContents(C4ID id);
  BOOL MenuCommand(const char *szCommand);

  BOOL CallControl(BYTE byCom, int par0 = 0, int par1 = 0, int par2 = 0, int par3 = 0, int par4 = 0, int par5 = 0, int par6 = 0, int par7 = 0, int par8 = 0, int par9 = 0);
  long Call(const char *szFunctionCall, long par0 = 0, long par1 = 0, long par2 = 0, long par3 = 0, long par4 = 0, long par5 = 0, long par6 = 0, long par7 = 0, long par8 = 0, long par9 = 0);
  long Call(C4Thread *pCaller, const char *szFunctionCall, long par0 = 0, long par1 = 0, long par2 = 0, long par3 = 0, long par4 = 0, long par5 = 0, long par6 = 0, long par7 = 0, long par8 = 0,
            long par9 = 0);

  BOOL ContainedControl(BYTE byCom);

  void Clear();
  void ClearInfo(C4ObjectInfo *pInfo);
  BOOL AssignInfo();
  BOOL ValidateOwner();
  void DrawPicture(C4Facet &cgo, BOOL fSelected = FALSE, C4RegionList *pRegions = NULL);
  void DenumeratePointers();
  void EnumeratePointers();
  void Default();
  BOOL Init(C4Def *ndef, int owner, C4ObjectInfo *info, int nx, int ny, int nr, FIXED nxdir, FIXED nydir, FIXED nrdir);
  BOOL Compile(const char *szSource);
  BOOL Decompile(char **ppOutput, int *ipSize);
  void DrawEnergy(C4Facet &cgo);
  void DrawMagicEnergy(C4Facet &cgo);
  void DrawBreath(C4Facet &cgo);
  void DrawLine(C4FacetEx &cgo);
  void DrawCommands(C4Facet &cgo, C4Facet &cgo2, C4RegionList *pRegions);
  void DrawCommand(C4Facet &cgoBar, int iAlign, const char *szFunctionFormat, int iCom, C4RegionList *pRegions, int iPlayer, const char *szDesc = NULL, C4Facet *pfctImage = NULL);
  BOOL SetPhase(int iPhase);
  void AssignRemoval();
  void Draw(C4FacetEx &cgo);
  void DrawTopFace(C4FacetEx &cgo);
  void Execute();
  void ClearPointers(C4Object *ptr);
  BOOL ExecMovement();
  BOOL ExecFire();
  void ExecAction();
  BOOL ExecLife();
  BOOL ExecuteCommand();
  void ExecBase();
  void AssignDeath();
  void ContactAction();
  void DoMovement();
  void Stabilize();
  void SetOCF();
  void UpdateShape();
  void RecheckSolidMask();
  void DestroySolidMask();
  void PutSolidMask();
  void RemoveSolidMask(BOOL fCauseInstability = TRUE);
  void UpdateSolidMask();
  void UpdateMass();
  void ComponentConCutoff();
  void ComponentConGain();
  BOOL ChangeDef(C4ID idNew);
  void UpdateFace();
  BOOL At(int ctx, int cty, DWORD &ocf);
  void CloseMenu();
  BOOL ActivateMenu(int iMenu, int iMenuSelect = 0, int iMenuData = 0, int iMenuPosition = 0, C4Object *pTarget = NULL);
  int ContactCheck(int atx, int aty);
  BOOL Contact(int cnat);
  void TargetBounds(int &ctco, int limit_low, int limit_hi, int cnat_low, int cnat_hi);
  BOOL SetAction(int iAct, C4Object *pTarget = NULL, C4Object *pTarget2 = NULL, BOOL fStartCall = TRUE);
  BOOL SetActionByName(const char *szActName, C4Object *pTarget = NULL, C4Object *pTarget2 = NULL, BOOL fStartCall = TRUE);
  void SetDir(int tdir);
  int GetProcedure();
  BOOL Enter(C4Object *pTarget, BOOL fCalls = TRUE);
  BOOL Exit(int iX = 0, int iY = 0, int iR = 0, FIXED iXDir = 0, FIXED iYDir = 0, FIXED iRDir = 0, BOOL fCalls = TRUE);
  void CopyMotion(C4Object *from);
  void ForcePosition(int tx, int ty);
  void DoMotion(int mx, int my);
  BOOL ActivateEntrance(int by_plr, C4Object *by_obj);
  BOOL Incinerate(int iCausedBy);
  BOOL Extinguish();
  void DoDamage(int iLevel, int iCausedBy);
  void DoEnergy(int iChange);
  void DoBreath(int iChange);
  void DoCon(int iChange, BOOL fInitial = FALSE);
  void DoExperience(int change);
  BOOL Promote(int torank, BOOL exception = FALSE);
  void Explode(int iLevel);
  void Blast(int iLevel, int iCausedBy);
  BOOL Build(int iLevel, C4Object *pBuilder);
  BOOL Chop(C4Object *pByObject);
  BOOL Push(FIXED txdir, FIXED dforce, BOOL fStraighten);
  BOOL Lift(FIXED tydir, FIXED dforce);
  void Fling(FIXED txdir, FIXED tydir);
  C4Object *CreateContents(C4ID n_id);
  BOOL CreateContentsByList(C4IDList &idlist);
  BYTE GetArea(int &aX, int &aY, int &aWdt, int &aHgt);
  BYTE GetEntranceArea(int &aX, int &aY, int &aWdt, int &aHgt);
  BYTE GetMomentum(FIXED &rxdir, FIXED &rydir);
  FIXED GetSpeed();
  C4PhysicalInfo *GetPhysical();
  const char *GetName();
  int GetValue();
  void DirectCom(BYTE byCom, int iData);
  BOOL BuyEnergy();
  void AutoSellContents();
};
