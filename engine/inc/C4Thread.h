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

    long Variable[C4ThreadMaxVar];
    long Parameter[C4ThreadMaxPar];
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
								long par0=0, long par1=0, long par2=0, long par3=0, long par4=0,
								long par5=0, long par6=0, long par7=0, long par8=0, long par9=0);
    void SetError(const char *szMessage);
  protected:
    long Execute();
    long ExecuteStatement(); 
  };
