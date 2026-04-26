/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main program entry point */

#include <C4Include.h>
#include <string>

C4Application Application;
C4Engine Engine;
C4Console Console;
C4FullScreen FullScreen;
C4Game Game;

#ifdef C4SHAREWARE
C4ConfigShareware Config;
#else
C4Config Config;
#endif

int main(int argc, char **argv)
{
	HINSTANCE hInst = 0;
	int nCmdShow = 1;
	std::string cmdLine;
	for (int i = 1; i < argc; i++) {
		if (i > 1) cmdLine += " ";
		cmdLine += argv[i];
	}

	// Init application
	if (!Application.Init(hInst, nCmdShow, (char*)cmdLine.c_str()))
		return C4XRV_Failure;

	// Execute application
	Application.Run();

	// Return exit code
	if (!Game.GameOver)
		return C4XRV_Aborted;
	return C4XRV_Completed;
}
