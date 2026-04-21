/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to initialize configuration and execute the game */

#include <C4Include.h>

C4Application::C4Application()
	{
	hInstance=NULL;
	hWindow=NULL;
	Active=FALSE;
	Fullscreen=FALSE;
	}

C4Application::~C4Application()
	{
	Config.General.Definitions[0]=0;
	Config.Save();
  CloseLog(Config.AtLogPath(C4CFN_Log));
	}

BOOL C4Application::Init(HINSTANCE hInst, int nCmdShow, char *szCmdLine)
	{

	// Set instance
	hInstance = hInst;

  // Config check
	Config.Init();
	Config.Load();

	// Init C4Group
	C4Group_SetMaker(Config.General.Name); 
	C4Group_SetProcessCallback(&ProcessCallback);
	C4Group_SetTempPath(Config.General.TempPath);

	// Set resource for strings (and extended flag for second language)
	SetStringResource(hInstance,SEqualNoCase(Config.General.Language,"US"));

#ifdef C4SHAREWARE
	// Set external unscramble function for scrambled resource strings
	void UnscrambleString(char *szString);
	SetResourceStringUnscramble(UnscrambleString);
#endif

	// Set unregistered user name
	if (!Config.Registered()) C4Group_SetMaker(LoadResStr(IDS_PRC_UNREGUSER));

	// Check for fullscreen switch in command line
	if (SSearchNoCase(szCmdLine,"/Fullscreen")) 
		Fullscreen = TRUE;

	// Fullscreen mode - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - 
	if (Fullscreen) 
		{
		// Init carrier window
		if (!(hWindow = FullScreen.Init(hInstance)))
			{ Clear(); return FALSE; }
		// Init engine
		if (!Engine.Init(hInstance,hWindow,Fullscreen))
			{ Clear(); return FALSE; }
		// Open game
		if (!FullScreen.OpenGame(szCmdLine)) return FALSE;
		}

	// Console mode - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - -
	else 
		{
		// Init carrier window
		if (!(hWindow = Console.Init(hInstance)))
			{ Clear(); return FALSE; }
		// Init engine
		if (!Engine.Init(hInstance,hWindow,Fullscreen))
			{ Clear(); return FALSE; }
		// Check registration 
		if ( !Config.Registered() )
			{ Log(LoadResStr(IDS_CNS_REGONLY)); Clear(); return FALSE; }
		// Execute command line
		if (!Console.CommandLine(szCmdLine)) return FALSE;
		}

  return TRUE;
	}

void C4Application::Run()
	{
  // Main message loop
  while (TRUE)
		switch (HandleMessage())
			{
			case 0: Execute(); break;
			case 1: break;
			case 2: return;
			}
	}

void C4Application::Clear()
	{
	Engine.Clear();
	}

void C4Application::Execute()
	{	
	// Fullscreen mode
	if (Fullscreen) FullScreen.Execute();
	// Console mode
	else Console.Execute();
	}

void C4Application::Quit()
	{
	PostQuitMessage(0);
	}

BOOL C4Application::ProcessCallback(const char *szMessage, int iProcess)
	{
  Console.Out(szMessage);
	return TRUE;
	}

int C4Application::HandleMessage()
	{
  MSG msg; BOOL MsgDone;
	// Peek message
  if (!PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) return 0;
  // Handle message
  if (!GetMessage(&msg,NULL,0,0)) return 2;
	// Dialog message transfer
	MsgDone=FALSE;
	if (!MsgDone) if (Console.hDialog && IsDialogMessage(Console.hDialog,&msg)) MsgDone=TRUE;
	if (!MsgDone)	if (Console.PropertyDlg.hDialog && IsDialogMessage(Console.PropertyDlg.hDialog,&msg)) MsgDone=TRUE;
	if (!MsgDone)	{	TranslateMessage(&msg); DispatchMessage(&msg); }
	// Done
	return 1;
	}
