/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Object definition */

const DWORD C4D_None = 0, C4D_All = ~C4D_None,

            C4D_StaticBack = 1 << 0, C4D_Structure = 1 << 1, C4D_Vehicle = 1 << 2, C4D_Living = 1 << 3, C4D_Object = 1 << 4,

            C4D_SortLimit = C4D_StaticBack | C4D_Structure | C4D_Vehicle | C4D_Living | C4D_Object,

            C4D_Goal = 1 << 5, C4D_Environment = 1 << 6,

            C4D_SelectBuilding = 1 << 7, C4D_SelectVehicle = 1 << 8, C4D_SelectMaterial = 1 << 9, C4D_SelectKnowledge = 1 << 10, C4D_SelectHomebase = 1 << 11, C4D_SelectAnimal = 1 << 12,
            C4D_SelectNest = 1 << 13, C4D_SelectInEarth = 1 << 14, C4D_SelectVegetation = 1 << 15,

            C4D_TradeLiving = 1 << 16, C4D_Magic = 1 << 17, C4D_CrewMember = 1 << 18,

            C4D_Rule = 1 << 19;

const DWORD C4D_Grab_Put = 1, C4D_Grab_Get = 2,

            C4D_Border_Sides = 1, C4D_Border_Top = 2, C4D_Border_Bottom = 4,

            C4D_Line_Power = 1, C4D_Line_Source = 2, C4D_Line_Drain = 3, C4D_Line_Lightning = 4, C4D_Line_Volcano = 5, C4D_Line_Rope = 6, C4D_Line_Colored = 7, C4D_Line_Vertex = 8,

            C4D_Power_Input = 1, C4D_Power_Output = 2, C4D_Liquid_Input = 4, C4D_Liquid_Output = 8, C4D_Power_Generator = 16, C4D_Power_Consumer = 32, C4D_Liquid_Pump = 64, C4D_Connect_Rope = 128,

            C4D_Place_Surface = 0, C4D_Place_Liquid = 1, C4D_Place_Air = 2;

const DWORD C4D_VehicleControl_None = 0, C4D_VehicleControl_Outside = 1, C4D_VehicleControl_Inside = 2;

const DWORD C4D_Sell = C4D_StaticBack | C4D_Structure | C4D_Vehicle | C4D_Object | C4D_TradeLiving, C4D_Get = C4D_Sell, C4D_Take = C4D_Get, C4D_Activate = C4D_Get;

const DWORD C4D_Load_None = 0, C4D_Load_Picture = 1, C4D_Load_Bitmap = 2, C4D_Load_Script = 4, C4D_Load_Desc = 8, C4D_Load_ActMap = 16, C4D_Load_Image = 32, C4D_Load_Sounds = 64,
            C4D_Load_FE = C4D_Load_Image | C4D_Load_Desc, C4D_Load_RX = C4D_Load_Bitmap | C4D_Load_Script | C4D_Load_Desc | C4D_Load_ActMap | C4D_Load_Sounds, C4D_Load_Temporary = 512;

const int ActIdle = -1;
const int ActHold = -2;

class C4ActionDef {
public:
  C4ActionDef();

public:
  char Name[C4D_MaxIDLen + 1];
  char ProcedureName[C4D_MaxIDLen + 1];
  int Procedure; // Mapped by C4Def::Load
  int Directions;
  int Length;
  int Delay;
  int Attach;
  char NextActionName[C4D_MaxIDLen + 1];
  int NextAction; // Mapped by C4Def::Load
  char InLiquidAction[C4D_MaxIDLen + 1];
  int FacetBase;
  C4TargetRect Facet;
  int FacetTopFace;
  int NoOtherAction;
  int Disabled;
  int DigFree;
  int FacetTargetStretch;
  char Sound[C4D_MaxIDLen + 1];
  int EnergyUsage;
  int Reverse;
  char StartCall[C4D_MaxIDLen + 1];
  char EndCall[C4D_MaxIDLen + 1];
  char PhaseCall[C4D_MaxIDLen + 2];

public:
  void Default();
};

class C4DefCore {
public:
  C4DefCore();

public:
  C4ID id;
  int rC4XVer1, rC4XVer2, rC4XVer3;
  char Name[C4D_MaxName + 1];
  C4PhysicalInfo Physical;
  C4Shape Shape;
  C4Rect Entrance;
  C4Rect Collection;
  C4Rect PictureRect;
  C4TargetRect SolidMask;
  C4TargetRect TopFace;
  C4IDList Component;
  C4ID BurnTurnTo;
  C4ID BuildTurnTo;
  int GrowthType;
  int Basement;
  int CanBeBase;
  int CrewMember;
  int NativeCrew;
  int Mass;
  int Value;
  int Exclusive;
  int Category;
  int Growth;
  int Rebuyable;
  int ContactIncinerate; // 0 off 1 high - 5 low
  int BlastIncinerate;   // 0 off 1 - x if > damage
  int Constructable;
  int Pushable; // 0 not 1 push 2 grab only
  int Carryable;
  int Rotateable;
  int Chopable;
  int Float;
  int ColorByOwner;
  int NoHorizontalMove;
  int BorderBound;
  int LiftTop;
  int CollectionLimit;
  int GrabPutGet;
  int ContainBlast;
  int UprightAttach;
  int ContactFunctionCalls;
  int MaxUserSelect;
  int Line;
  int LineConnect;
  int LineIntersect;
  int NoBurnDecay;
  int IncompleteActivity;
  int Placement;
  int Prey;
  int Edible;
  int BuildAtOnce;
  int AttractLightning;
  int Oversize;
  int Fragile;
  int Projectile;
  int Explosive;
  int NoPushEnter;
  int DragImagePicture;
  int VehicleControl;
  int Pathfinder;
  int Timer;
  char TimerCall[C4D_MaxIDLen];
  char ColorByMaterial[C4M_MaxName + 1];

public:
  void Default();
  BOOL Load(C4Group &hGroup);
  BOOL Save(C4Group &hGroup);

protected:
  BOOL Compile(const char *szSource);
  BOOL Decompile(char **ppOutput, int *ipSize);
};

class C4Def : public C4DefCore {
  friend C4DefList;

public:
  C4Def();
  ~C4Def();

public:
  HBITMAP Picture;
  HBITMAP Image;
  SURFACE Bitmap[C4MaxColor];
  C4Facet MainFace[C4MaxColor];
  char *Desc;
  int ActNum;
  C4ActionDef *ActMap;
  int ControlFlag, ContainedControlFlag, ActivationControlFlag;
  char Maker[C4MaxName + 1];
  char Filename[_MAX_FNAME + 1];
  int Creation;
  C4ScriptHost Script;

protected:
  C4Def *Next;
  BOOL Temporary;

public:
  int ResolveIncludes(C4DefList &rDefs);
  void Clear();
  void Default();
  void Draw(C4Facet &cgo, BOOL fSelected = FALSE, int iColor = 0);
  BOOL Load(C4Group &hGroup, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL);

protected:
  BOOL ColorizeByMaterial(C4MaterialMap &rMats, BYTE bGBM);
  BOOL LoadActMap(C4Group &hGroup);
  void CrossMapActMap();
  BOOL DecompileActMap(C4ActionDef *pC4A, int iActNum, char **ppOutput, int *ipSize);
  BOOL CompileActMap(const char *szSource, C4ActionDef *pC4A, int iActNum);
};

class C4DefList {
public:
  C4DefList();
  ~C4DefList();

public:
  BOOL LoadFailure;

protected:
  C4Def *FirstDef;

public:
  void ResolveIncludes();
  void Default();
  void Clear();
  int Load(C4Group &hGroup, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL, BOOL fOverload = FALSE, BOOL fSearchMessage = FALSE);
  int Load(const char *szSearch, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL, BOOL fOverload = FALSE);
  int LoadFolderLocal(const char *szPath, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL, BOOL fOverload = FALSE, char *szStoreName = NULL);
  int LoadForScenario(const char *szScenario, const char *szSpecified, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL, BOOL fOverload = FALSE);
  C4Def *ID2Def(C4ID id);
  C4Def *GetDef(int Index, DWORD dwCategory = C4D_All);
  int GetDefCount(DWORD dwCategory = C4D_All);
  int GetIndex(C4ID id);
  int RemoveTemporary();
  int ColorizeByMaterial(C4MaterialMap &rMats, BYTE bGBM);
  int CheckEngineVersion(int ver1, int ver2, int ver3);
  void Draw(C4ID id, C4Facet &cgo, BOOL fSelected, int iColor);
  void Remove(C4Def *def);
  BOOL Remove(C4ID id);
  BOOL Reload(C4Def *pDef, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem = NULL);
  BOOL Add(C4Def *ndef, BOOL fOverload);
};

// Default Action Procedures

#define DFA_NONE -1
#define DFA_WALK 0
#define DFA_FLIGHT 1
#define DFA_KNEEL 2
#define DFA_SCALE 3
#define DFA_HANGLE 4
#define DFA_DIG 5
#define DFA_SWIM 6
#define DFA_THROW 7
#define DFA_BRIDGE 8
#define DFA_BUILD 9
#define DFA_PUSH 10
#define DFA_CHOP 11
#define DFA_LIFT 12
#define DFA_FLOAT 13
#define DFA_ATTACH 14
#define DFA_FIGHT 15
#define DFA_CONNECT 16
#define DFA_PULL 17
