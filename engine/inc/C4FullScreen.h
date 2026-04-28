/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to execute the game fullscreen mode */

class C4FullScreen {
public:
  C4FullScreen();
  ~C4FullScreen();

public:
  BOOL Active;
  HWND hWindow;
  BOOL HoldAbort;
  BOOL HoldGameOver;
  C4Menu Menu;
  char CommandLine[1024 + 1];

public:
  void Default();
  void Clear();
  void Execute();
  void CheckPlayerJoinMenu();
  HWND Init(HINSTANCE hInst);
  BOOL Restart();
  BOOL MenuCommand(const char *szCommand);
  BOOL ViewportCheck();
  BOOL OpenGame(const char *szCmdLine);

protected:
  BOOL ActivateMenuMain();
  BOOL ActivateMenuNewPlayer();
  BOOL ActivateMenuScenario();
  BOOL RegisterWindowClass(HINSTANCE hInst);
};

#define C4FullScreenClassName "C4FullScreen"
