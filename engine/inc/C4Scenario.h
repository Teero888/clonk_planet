/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Core component of a scenario file */

#include <stdint.h>

#pragma pack(push, 1)

class C4SVal 
  {
  public:
    C4SVal();
  public:
    int32_t Std,Rnd,Min,Max; 
  public:
	  void Default();
    void Set(int std=0, int rnd=0, int min=0, int max=100);
    int Evaluate();
  };

class C4SHead 
  {
  public:
		int32_t C4XVer1,C4XVer2,C4XVer3;
    char Title[C4MaxTitle+1];
    int32_t EnableRoundOptions;
		int32_t EnableUnregisteredAccess;
    int32_t Icon;
		int32_t NoInitialize;
		int32_t MaxPlayer;
		int32_t SaveGame;
		char MissionAccess[C4MaxTitle+1];
		int32_t NetworkGame;
		int32_t NetworkReference;
		int32_t NetworkLobby;
		char NetworkHostName[C4MaxTitle+1];
		char NetworkHostAddress[C4MaxTitle+1];
		char NetworkFilename[C4MaxTitle+1];
		int32_t NetworkFileCreation;
		int32_t NetworkDeveloperMode;
		int32_t NetworkNoRuntimeJoin;
		C4NameList NetworkClients;
  public:
    void Default();
  };


const int C4S_MaxDefinitions = 10;

class C4SDefinitions
	{
	public:
		int32_t LocalOnly;
		char Definition[C4S_MaxDefinitions][_MAX_PATH+1];
  public:
	  void SetModules(const char *szList, const char *szRelativeToPath=NULL);
	  BOOL GetModules(char *sList);
		BOOL AssertModules(const char *szPath=NULL, char *sMissing=NULL);
    void Default();
	};


class C4SRealism
  {
  public:
    int32_t ConstructionNeedsMaterial;
    int32_t StructuresNeedEnergy;
  public:
    void Default();  
  };


class C4SGame 
  {
  public:
		
		int32_t Mode;
    int32_t Elimination;    
    int32_t EnableRemoveFlag;
		int32_t EnableSurrender; 

    // Player winning - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    int32_t ValueGain; // If nonzero and value gain is equal or higher
    // Cooperative game over - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		C4IDList CreateObjects; // If all these objects exist
    C4IDList ClearObjects; // If fewer than these objects exist
    C4NameList ClearMaterial; // If less than these materials exist
		int32_t CooperativeGoal; // Master selection for Frontend

		C4IDList Goals;
		C4IDList Rules;

  public:
	  BOOL IsMelee();
	  void ConvertGoals(C4SRealism &rRealism);
    void Default();  
		void ClearCooperativeGoals();
  protected:
	  void ClearOldGoals();
	};

// Game mode

const int C4S_Cooperative		= 0,
					C4S_Melee					= 1,
					C4S_MeleeTeamwork	= 2;

// Player elimination

const int C4S_KillTheCaptain = 0, 
          C4S_EliminateCrew  = 1, 
          C4S_CaptureTheFlag = 2;

// Cooperative goals

const int C4S_NoGoal			= 0,
					C4S_Goldmine		= 1,
					C4S_Monsterkill = 2,
					C4S_ValueGain		= 3,
					C4S_Extended    = 4;


class C4SPlrStart 
  {
  public:
		C4ID NativeCrew; // Obsolete
    C4SVal Crew; // Obsolete
    C4SVal Wealth;
    int32_t PositionX,PositionY,EnforcePosition;
		C4IDList ReadyCrew;
    C4IDList ReadyBase;
    C4IDList ReadyVehic;
    C4IDList ReadyMaterial;
    C4IDList BuildKnowledge;
    C4IDList HomeBaseMaterial;
    C4IDList HomeBaseProduction;
		C4IDList Magic;
  public:
    void Default();
		bool EquipmentEqual(const C4SPlrStart &rhs);
	public:
		bool operator==(const C4SPlrStart& rhs)
			{	return MemEqual((BYTE*)this,(BYTE*)&rhs,sizeof(C4SPlrStart)); }
  };


class C4SLandscape
  {
  public:
		int32_t ExactLandscape;
    C4SVal VegLevel;     
    C4IDList Vegetation; 
    C4SVal InEarthLevel; 
    C4IDList InEarth;    
    int32_t BottomOpen,TopOpen; 
    int32_t LeftOpen,RightOpen; 
    int32_t AutoScanSideOpen;   
    char SkyDef[C4MaxDefString+1];     
    int32_t SkyDefFade[6]; 
		int32_t NoSky;    
		C4SVal Gravity;
    // Dynamic map
    C4SVal MapWdt,MapHgt,MapZoom; 
    C4SVal Amplitude,Phase,Period,Random;
    C4SVal LiquidLevel;
    int32_t MapPlayerExtend;
    C4NameList Layers;
		char Material[C4M_MaxDefName+1];
		char Liquid[C4M_MaxDefName+1];
  public:
    void Default();  
		void GetMapSize(int &rWdt, int &rHgt, int iPlayerNum);
  };

class C4SWeather 
  {
  public:
    C4SVal Climate;
    C4SVal StartSeason,YearSpeed;
    C4SVal Rain,Lightning,Wind;
		char Precipitation[C4M_MaxName+1];
  public:
    void Default();  
  };

class C4SAnimals 
  {
  public:
    C4IDList FreeLife;
		C4IDList EarthNest;
  public:
    void Default();  
  };

class C4SEnvironment
  {
  public:
    C4IDList Objects;
  public:
    void Default();  
  };

class C4SDisasters
  {
  public:
    C4SVal Volcano; 
    C4SVal Earthquake;   
    C4SVal Meteorite;    
  public:
    void Default();  
  };

class C4Scenario
  {
  public: 
    C4Scenario();
  public:
    C4SHead					Head;
		C4SDefinitions	Definitions;
    C4SGame					Game;
    C4SPlrStart			PlrStart[C4S_MaxPlayer];
    C4SLandscape		Landscape;
    C4SAnimals			Animals;
    C4SWeather			Weather;
    C4SDisasters		Disasters;
    C4SRealism			Realism;
		C4SEnvironment	Environment;
  public:
	  void SetExactLandscape();
	  void Clear();
    void Default();
    BOOL Load(C4Group &hGroup);
		BOOL Save(C4Group &hGroup);
	protected:
		BOOL Compile(const char *szSource);
		BOOL Decompile(char **ppOutput, int *ipSize);
  };
#pragma pack(pop)
