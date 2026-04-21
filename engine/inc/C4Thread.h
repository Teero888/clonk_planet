/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Executes C4Script */

const int C4ThreadMaxVar = C4MaxVariable,
          C4ThreadMaxPar = C4MaxVariable;

class C4ThreadError
	{
	public:
		char Message[1024+1];
		const char *Position;
	};

class C4Thread
  {
  public:
    C4Thread();
    ~C4Thread();
	public:
    C4Object *cObj;
		char Function[100+1];
    
		BOOL SkipNextStatement; // Set by if
		BOOL JumpbackStatement; // Set by while
    BOOL ReturnThread; // Set by return
    BOOL NextStatementAdjacent; // Do not expect statement separator ';'

    int Variable[C4ThreadMaxVar];
    int Parameter[C4ThreadMaxPar];
  protected:
		C4Thread *Caller;
		const char *Script;
    const char *cScr;
		C4ThreadError Error;
  public:
    int Execute(C4Thread *pCaller,
								const char *szScript, 
								const char *szFunction,
								const char *cpPosition,
								C4Object *pObj,
								int par0=0, int par1=0, int par2=0, int par3=0, int par4=0,
								int par5=0, int par6=0, int par7=0, int par8=0, int par9=0);
    void SetError(const char *szMessage);
  protected:
    int Execute();
    int ExecuteStatement(); 
  };
