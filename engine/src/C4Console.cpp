#include <C4Include.h>

C4Console::C4Console() {
  Active = FALSE;
  Editing = TRUE;

  hDialog = NULL;
  ScriptCounter = 0;
  FrameCounter = 0;
  fGameOpen = FALSE;

  hbmCursor = NULL;
  hbmCursor2 = NULL;
  hbmBrush = NULL;
  hbmBrush2 = NULL;
  hbmPlay = NULL;
  hbmPlay2 = NULL;
  hbmHalt = NULL;
  hbmHalt2 = NULL;

  MenuIndexFile = 0, MenuIndexComponents = 1, MenuIndexPlayer = 2,
  MenuIndexViewport = 3, MenuIndexNet = -1, MenuIndexHelp = 4;
}

C4Console::~C4Console() {}

void C4Console::Default() { hDialog = NULL; }

void C4Console::Clear() { hDialog = NULL; }

HWND C4Console::Init(HINSTANCE hInst) {
  Active = TRUE;
  // Return success dummy, Engine.Init will create the real GLFW window
  return (HWND)1;
}

void C4Console::Execute() {}

BOOL C4Console::Out(const char *szText) {
  if (!szText)
    return FALSE;
  printf("%s\n", szText);
  return TRUE;
}

BOOL C4Console::In(const char *szText) { return TRUE; }

void C4Console::ClearPointers(C4Object *pObj) {
  PropertyDlg.ClearPointers(pObj);
}

BOOL C4Console::UpdateStatusBars() { return TRUE; }

BOOL C4Console::UpdateHaltCtrls(BOOL fHalt) { return TRUE; }

BOOL C4Console::SetCaption(const char *szCaption) { return TRUE; }

BOOL C4Console::FileQuit() { return TRUE; }

BOOL C4Console::SaveGame(BOOL fSaveGame) { return FALSE; }

BOOL C4Console::FileSaveAs(BOOL fSaveGame) { return FALSE; }

BOOL C4Console::Message(const char *szMessage, BOOL fQuery) {
  printf("%s\n", szMessage);
  return TRUE;
}

void C4Console::EnableControls(BOOL fEnable) {}

BOOL C4Console::FileOpen() { return FALSE; }

BOOL C4Console::FileOpenWPlrs() { return FALSE; }

BOOL C4Console::FileSelect(char *szFilename, int iSize, const char *szFilter,
                           DWORD dwFlags, BOOL fSave) {
  return FALSE;
}

void C4Console::HelpAbout() {}

BOOL C4Console::UpdateViewportMenu() { return TRUE; }

void C4Console::ClearViewportMenu() {}

BOOL C4Console::AddMenuItem(HMENU hMenu, DWORD dwID, const char *szString,
                            BOOL fEnabled) {
  return FALSE;
}

BOOL C4Console::Keydown(WORD wParam) { return FALSE; }

BOOL C4Console::UpdateModeCtrls(BOOL iMode) { return TRUE; }

BOOL C4Console::UpdateCursorBar(const char *szCursor) { return TRUE; }

void C4Console::UpdateInputCtrl() {}

BOOL C4Console::UpdatePlayerMenu() { return TRUE; }

void C4Console::ClearPlayerMenu() {}

void C4Console::PlayerJoin() {}

void C4Console::UpdateMenuText(HMENU hMenu) {}

void C4Console::UpdateNetMenu() {}

void C4Console::ClearNetMenu() {}

void C4Console::UpdateMenus() {}

BOOL C4Console::RegisterConsoleWindowClass(HINSTANCE hInst) { return TRUE; }

BOOL C4Console::OpenGame(const char *szCmdLine) { return FALSE; }
BOOL C4Console::CloseGame() { return FALSE; }
BOOL C4Console::FileSave(BOOL fSaveGame) { return SaveGame(fSaveGame); }
BOOL C4Console::FileClose() { return CloseGame(); }
void C4Console::ViewportNew() {}
void C4Console::EditInfo() {}
void C4Console::EditScript() {}
void C4Console::EditTitle() {}
BOOL C4Console::CommandLine(const char *szCmdLine) {
  // Just pass to Game.Init
  if (!Game.Init(szCmdLine))
    return FALSE;
  fGameOpen = TRUE;
  return TRUE;
}
