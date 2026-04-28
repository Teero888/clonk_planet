/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles engine execution in developer mode */

const int C4CNS_ModePlay = 0, C4CNS_ModeEdit = 1, C4CNS_ModeDraw = 2;

#define IDM_NET_CLIENT1 10000
#define IDM_NET_CLIENT2 10100
#define IDM_PLAYER_QUIT1 10200
#define IDM_PLAYER_QUIT2 10300
#define IDM_VIEWPORT_NEW1 10400
#define IDM_VIEWPORT_NEW2 10500

class C4Console {
  friend BOOL CALLBACK ConsoleDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

public:
  C4Console();
  ~C4Console();

public:
  BOOL Active;
  BOOL Editing;
  HWND hDialog;
  C4PropertyDlg PropertyDlg;
  C4ToolsDlg ToolsDlg;
  C4EditCursor EditCursor;

protected:
  int ScriptCounter;
  int FrameCounter;
  int Time, FPS;
  BOOL fGameOpen;
  HBITMAP hbmMouse;
  HBITMAP hbmMouse2;
  HBITMAP hbmCursor;
  HBITMAP hbmCursor2;
  HBITMAP hbmBrush;
  HBITMAP hbmBrush2;
  HBITMAP hbmPlay;
  HBITMAP hbmPlay2;
  HBITMAP hbmHalt;
  HBITMAP hbmHalt2;
  int MenuIndexFile;
  int MenuIndexComponents;
  int MenuIndexPlayer;
  int MenuIndexViewport;
  int MenuIndexNet;
  int MenuIndexHelp;

public:
  void Default();
  void Clear();
  HWND Init(HINSTANCE hInst);
  void Execute();
  void ClearPointers(C4Object *pObj);
  BOOL Message(const char *szMessage, BOOL fQuery = FALSE);
  BOOL SetCaption(const char *szCaption);
  BOOL In(const char *szText);
  BOOL Out(const char *szText);
  BOOL UpdateCursorBar(const char *szCursor);
  BOOL UpdateHaltCtrls(BOOL fHalt);
  BOOL UpdateModeCtrls(BOOL iMode);
  void UpdateInputCtrl();
  void UpdateMenus();
  BOOL Keydown(WORD wParam);
  BOOL CommandLine(const char *szCmdLine);

protected:
  void ClearNetMenu();
  void UpdateNetMenu();
  void UpdateMenuText(HMENU hMenu);
  void PlayerJoin();
  BOOL UpdatePlayerMenu();
  void ClearPlayerMenu();
  void EditInfo();
  void EditScript();
  void EditTitle();
  BOOL UpdateStatusBars();
  void ViewportNew();
  void HelpAbout();
  BOOL FileSelect(char *sFilename, int iSize, const char *szFilter, DWORD dwFlags, BOOL fSave = FALSE);
  BOOL OpenGame(const char *szCmdLine);
  BOOL CloseGame();
  BOOL SaveGame(BOOL fSaveGame);
  BOOL FileSaveAs(BOOL fSaveGame);
  BOOL FileSave(BOOL fSaveGame);
  BOOL FileOpen();
  BOOL FileOpenWPlrs();
  BOOL FileClose();
  BOOL FileQuit();
  void EnableControls(BOOL fEnable);
  BOOL AddMenuItem(HMENU hMenu, DWORD dwID, const char *szString, BOOL fEnabled = TRUE);
  void ClearViewportMenu();
  BOOL UpdateViewportMenu();
  BOOL RegisterConsoleWindowClass(HINSTANCE hInst);
};

#define C4ConsoleWindowClassname "C4Console"