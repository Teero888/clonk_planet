/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Structures for object and player info components */

const int C4MaxPhysical = 100000;

class C4PhysicalInfo
  {
  public:
		C4PhysicalInfo();
  public:
    int Energy;
		int Breath; 
    int Walk;
		int Jump;
		int Scale;
		int Hangle;
		int Dig;
		int Swim;
    int Throw;
		int Push;
		int Fight;
		int Magic;
    int CanScale;
		int CanHangle;
    int CanDig;
		int CanConstruct;
		int CanChop;
    int CanSwimDig;
		int CorrosionResist;
		int BreatheWater;
		int Float;
		int fbuf[22];
	public:
		void Default();
		void PromotionUpdate(int iRank);
  };

class C4ObjectInfoCore
  {
  public:
    C4ObjectInfoCore();
  public:
    int  Ver1,Ver2;
    C4ID id;
    char Name[C4MaxName+1];
    int  Participation; 
    int  Rank;
    char RankName[C4MaxName+1];
    int  Experience,Rounds;
    int  DeathCount;
		char TypeName[C4MaxName+1+1];
		int  Birthday,TotalPlayingTime;
    int  fbuf[40];
    C4PhysicalInfo Physical;
  public:
	  BOOL Save(C4Group &hGroup);
	  BOOL Load(C4Group &hGroup);
    void Default(C4ID n_id=C4ID_None, C4DefList *pDefs=NULL, const char *cpNames=NULL);
    BOOL LoadNext(C4Group &hGroup);
    BOOL Add(C4Group &hGroup);
		void Promote(int iRank, C4RankSystem &rRanks);
	protected:
		BOOL Compile(const char *szSource);
		BOOL Decompile(char **ppOutput, int *ipSize);
  };

class C4RoundResult
  {
	public:
		C4RoundResult();
  public:
    char Title[C4MaxTitle+1];
    long Date;
    int Duration;
    int Won;
    int Score,FinalScore,TotalScore;
		int Bonus;
		int Level;
		int fbuf[10];
	public:
		void Default();
  };

class C4PlayerInfoCore
  {
  public:
    C4PlayerInfoCore();
  public:
    // Version
    int  Ver1,Ver2;
    // Player Info
    char Name[C4MaxName+1];
    char Comment[C4MaxComment+1];
    int  Rank;
    char RankName[C4MaxName+1];
    int  Score;
    int  Rounds,RoundsWon,RoundsLost;
    int  TotalPlayingTime;  
		int  AutomaticReanimation;
		int  PrefMouse;
    int  fbuf[48];
    C4RoundResult LastRound;  
    // Preferences
    int PrefColor;
    int PrefControl;
    int PrefPosition;
  public:
    void Default(C4RankSystem *pRanks=NULL);
		void Promote(int iRank, C4RankSystem &rRanks);
    BOOL Load(C4Group &hGroup);
		BOOL Save(C4Group &hGroup);
		BOOL CheckPromotion(C4RankSystem &rRanks);
	protected:
		BOOL Compile(const char *szSource);
		BOOL Decompile(char **ppOutput, int *ipSize);
  };