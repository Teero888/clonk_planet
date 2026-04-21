/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to initialize configuration and execute the game */

class C4Application  
	{
	public:
		C4Application();
		~C4Application();
	public:
		BOOL Active;
		HINSTANCE hInstance;
		HWND hWindow;
		BOOL Fullscreen;
	public:
		BOOL Init(HINSTANCE hInst, int nCmdShow, char *szCmdLine);
		void Clear();
		void Execute();
		void Run();
		void Quit();
		int HandleMessage();
	protected:
		static BOOL ProcessCallback(const char *szMessage, int iProcess);
	};
