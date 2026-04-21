/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Fullscreen startup log and chat type-in */

const int C4MSGB_BackBufferMax = 20;

class C4MessageBoard  
	{
	public:
		C4MessageBoard();
		~C4MessageBoard();
	public:
		BOOL TypeIn;
		C4Facet Output;
	protected:
		BOOL Active;
		char Message[C4MaxMessage];
		char TypeInBuf[C4MaxMessage];
		char BackBuffer[C4MSGB_BackBufferMax][C4MaxMessage];
		int Delay,DelayFactor;
		int Fader;
		int BackIndex;
		int Process;
		BOOL TypeInCommand;
		BOOL Startup;
		const char *pLog;
		const char *pCursor;
	public:
		void Default();
		void Clear();
		void Init(const char *szLog, C4Facet &cgo, BOOL fStartup);
		void Execute();
		void Draw(C4Facet &cgo);
		void LogNotify();
		void NotifyProcess(int iValue);
		void EnsureLastMessage();
		BOOL Control(WORD vk_code);
		BOOL Out(const char *szText);
		BOOL CloseTypeIn();
		BOOL StartTypeIn(BOOL fCommand);
		BOOL CharIn(char cChar);
		C4Player* GetMessagePlayer(const char *szMessage);
	protected:
		void StoreBackBuffer(const char *szMessage);
	};

