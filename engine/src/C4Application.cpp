/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to initialize configuration and execute the game */

#include <C4Include.h>
#include <thread>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

C4Application::C4Application()
{
	hInstance = NULL;
	hWindow = NULL;
	Active = FALSE;
	Fullscreen = FALSE;
}

C4Application::~C4Application()
{
	Config.General.Definitions[0] = 0;
	Config.Save();
	CloseLog(Config.AtLogPath(C4CFN_Log));
}

extern std::thread::id g_mainThreadId;

BOOL C4Application::Init(HINSTANCE hInst, int nCmdShow, char *szCmdLine)
{
	printf("Application::Init: %s\n", szCmdLine);
	g_mainThreadId = std::this_thread::get_id();

	// Set instance
	hInstance = hInst;

	// Config check
	printf("Config.Init...\n");
	Config.Init();
	printf("Config.Load...\n");
	Config.Load();

	// Init C4Group
	C4Group_SetMaker(Config.General.Name);
	C4Group_SetProcessCallback(&ProcessCallback);
	C4Group_SetTempPath(Config.General.TempPath);

	// Set resource for strings (and extended flag for second language)
	printf("SetStringResource...\n");
	SetStringResource(hInstance, SEqualNoCase(Config.General.Language, "US"));

#ifdef C4SHAREWARE
	// Set external unscramble function for scrambled resource strings
	void UnscrambleString(char *szString);
	SetResourceStringUnscramble(UnscrambleString);
#endif

	// Set unregistered user name
	if (!Config.Registered())
		C4Group_SetMaker(LoadResStr(IDS_PRC_UNREGUSER));

	// Fullscreen mode switch (ignored, we force windowed)
	Fullscreen = FALSE;

	// Init carrier window
	printf("Console.Init...\n");
	if (!(hWindow = Console.Init(hInstance)))
	{
		printf("Console.Init failed!\n");
		Clear();
		return FALSE;
	}
	// Init engine
	printf("Engine.Init...\n");
	if (!Engine.Init(hInstance, hWindow, Fullscreen))
	{
		printf("Engine.Init failed!\n");
		Clear();
		return FALSE;
	}
	// Execute command line
	printf("Console.CommandLine...\n");
	if (!Console.CommandLine(szCmdLine))
	{
		printf("Console.CommandLine failed!\n");
		return FALSE;
	}

	return TRUE;
}

void C4Application::Run()
{
	// Main message loop
	while (TRUE)
		switch (HandleMessage())
		{
		case 0:
			Execute();
			break;
		case 1:
			break;
		case 2:
			return;
		}
}

void C4Application::Clear()
{
	Engine.Clear();
}

void C4Application::Execute()
{
	// Fullscreen mode
	if (Fullscreen)
		FullScreen.Execute();
	// Console mode
	else
		Console.Execute();
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

// Forward declare internal window handle from StdDDraw.cpp
extern "C" void* GetGLFWWindow();

int C4Application::HandleMessage()
{
#ifndef _WIN32
    static double startTime = glfwGetTime();
    if (glfwGetTime() - startTime > 5.0) return 2; // Auto-quit after 5 seconds
    GLFWwindow* window = (GLFWwindow*)GetGLFWWindow();
    if (window && glfwWindowShouldClose(window)) return 2;
    glfwPollEvents();
    return 0; // Drive Execute()
#else
	MSG msg;
	BOOL MsgDone;
	// Peek message
	if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		return 0;
	// Handle message
	if (!GetMessage(&msg, NULL, 0, 0))
		return 2;
	// Dialog message transfer
	MsgDone = FALSE;
	if (!MsgDone)
		if (Console.hDialog && IsDialogMessage(Console.hDialog, &msg))
			MsgDone = TRUE;
	if (!MsgDone)
		if (Console.PropertyDlg.hDialog && IsDialogMessage(Console.PropertyDlg.hDialog, &msg))
			MsgDone = TRUE;
	if (!MsgDone)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// Done
	return 1;
#endif
}
