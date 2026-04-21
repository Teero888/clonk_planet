/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main program entry point */

#include <C4Include.h>

C4Application Application;
C4Engine			Engine; 
C4Console			Console;
C4FullScreen	FullScreen;
C4Game				Game;

#ifdef C4SHAREWARE
C4ConfigShareware Config;
#else
C4Config	Config;
#endif

int WINAPI WinMain (HINSTANCE hInst, 
										HINSTANCE hPrevInstance,
                    LPSTR lpszCmdParam, 
										int nCmdShow)
  {

	// Init application
	if (!Application.Init(hInst,nCmdShow,lpszCmdParam)) 
		return C4XRV_Failure;

	// Execute application
	Application.Run();

	// Return exit code
	if (!Game.GameOver) return C4XRV_Aborted;
	return C4XRV_Completed;

	}


