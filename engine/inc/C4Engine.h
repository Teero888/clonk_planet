/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to handle DirectDraw and critical timers */

const int SEC1_TIMER=1,SEC1_MSEC=1000;
const int CRITICAL_MSEC=28;

class C4Engine  
	{
	public:
		C4Engine();
		~C4Engine();
	public:
		CStdDDraw DDraw;
	protected:
		HINSTANCE hLanguageModule;
		UINT idCriticalTimer;
		BOOL fTimePeriod;
		DWORD DDrawEmulationState;
		BOOL fDDrawEmulationState;
	public:
		void Default();
		void Clear();
		BOOL Init(HINSTANCE hinst, HWND hwnd, BOOL fFullscreen);
	protected:
		BOOL SetCriticalTimer(HWND hwnd);
		void CloseCriticalTimer();
	};

