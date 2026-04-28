/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Operates viewports, message board and draws the game */

class C4GraphicsSystem {
public:
  C4GraphicsSystem();
  ~C4GraphicsSystem();

public:
  C4FacetEx fctLoader;
  C4MessageBoard MessageBoard;
  BOOL RedrawBackground;
  BOOL ShowVertices;
  BOOL ShowAction;
  BOOL ShowCommand;
  BOOL ShowEntrance;
  BOOL ShowPathfinder;
  BOOL ShowNetstatus;
  BOOL ShowSolidMask;
  BYTE DarkColorTable[256];
  BOOL fSetPalette;
  static volatile BOOL GraphicsGo;
  C4Video Video;

protected:
  C4Viewport *FirstViewport;
  BOOL fViewportClassRegistered;
  C4Facet ViewportArea;
  char FlashMessageText[C4MaxTitle + 1];
  int FlashMessageTime, FlashMessageX, FlashMessageY;

public:
  void Default();
  void Clear();
  void Execute();
  void FlashMessage(const char *szMessage);
  void SetDarkColorTable();
  void DeactivateDebugOutput();
  void MouseMove(int iButton, int iX, int iY, WORD wKeyParam);
  void SortViewportsByPlayerControl();
  void ClearPointers(C4Object *pObj);
  void UpdateMouse(HWND hwnd, int iX, int iY, int iB);
  void RecalculateViewports();
  void ColorAnimation();
  BOOL Init();
  BOOL InitLoaderScreen(const char *szGroup);
  BOOL SaveScreenshot();
  BOOL SetPalette();
  BOOL CreateViewport(int iPlayer);
  BOOL CloseViewport(int iPlayer);
  BOOL CloseViewport(HWND hwnd);
  int GetAudibility(int iX, int iY);
  int GetViewportCount();
  C4Viewport *GetViewport(int iPlayer);
  C4Viewport *GetViewport(HWND hwnd);

protected:
  void DrawFlashMessage();
  void DrawHoldMessages();
  void DrawFullscreenBackground();
  BOOL RegisterViewportClass(HINSTANCE hInst);
};
