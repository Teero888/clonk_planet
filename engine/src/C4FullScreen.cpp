#include <C4Include.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

C4FullScreen::C4FullScreen() { Default(); }

C4FullScreen::~C4FullScreen() {}

BOOL C4FullScreen::OpenGame(const char *szCmdLine) {
  // Clear and default game before re-init
  Game.Clear();
  Game.Default();
  // Store command line for restart
  SCopy(szCmdLine, CommandLine, 1024);
  // Init game
  if (!Game.Init(CommandLine)) {
    Clear();
    return FALSE;
  }
  // Success
  return TRUE;
}

void C4FullScreen::Clear() {
  // Clear game members
  Menu.Clear();
  Game.Clear();
  // Destroy window
  if (hWindow)
    DestroyWindow(hWindow);
  hWindow = NULL;
}

void C4FullScreen::Default() {
  Active = FALSE;
  hWindow = NULL;
  Menu.Default();
  CommandLine[0] = 0;
  HoldGameOver = FALSE;
  HoldAbort = FALSE;
}

void C4FullScreen::Execute() {
  // Ensure viewport exists
  ViewportCheck();

  // No scenario loaded: activate scenario menu
  if (!Game.ScenarioFilename[0])
    if (!Menu.IsActive()) {
      printf("Activating scenario menu...\n");
      ActivateMenuScenario();
    }

  // Execute menu
  Menu.Execute();

  // Execute game
  Game.Execute();

  // If game over, set HoldGameOver flag
  if (Game.GameOver)
    if (!HoldGameOver) {
      HoldGameOver = TRUE;
      Game.Halt = TRUE;
      Game.GraphicsSystem.Execute();
    }

  // If HoldGameOver, clear on any game pad button
  if (HoldGameOver)
    if (Game.GamePadCon1.AnyButtonDown())
      Clear();
}

HWND C4FullScreen::Init(HINSTANCE hInst) {
  Active = TRUE;
  // Return success dummy, Engine.Init will create the real GLFW window
  return (HWND)1;
}

BOOL C4FullScreen::RegisterWindowClass(HINSTANCE hInst) { return TRUE; }

BOOL C4FullScreen::ViewportCheck() {
  // Not active
  if (!Active)
    return FALSE;
  // Check viewports
  switch (Game.GraphicsSystem.GetViewportCount()) {
  // No viewports: create no-owner viewport
  case 0:
    Game.CreateViewport(NO_OWNER);
    break;
  // One viewport: do nothing
  case 1:
    break;
  // More than one viewport: remove all no-owner viewports
  default:
    Game.GraphicsSystem.CloseViewport(NO_OWNER);
    break;
  }
  // Close menu if necessary
  if (!Game.GraphicsSystem.GetViewport(NO_OWNER))
    if (Menu.IsActive())
      Menu.Close();
  // Done
  return TRUE;
}

BOOL C4FullScreen::MenuCommand(const char *szCommand) {
  // Not active
  if (!Active)
    return FALSE;

  // ActivateMenu
  if (SEqual2(szCommand, "ActivateMenu:")) {
    if (SEqual(szCommand + 13, "NewPlayer"))
      ActivateMenuNewPlayer();
    return TRUE;
  }

  // LoadScenario
  if (SEqual2(szCommand, "LoadScenario:")) {
    char szCmd[2048];
    sprintf(szCmd, "\"%s\" Objects.c4d", szCommand + 13);
    // Add joined players
    int iPlrs = 0;
    for (C4Player *pPlr = Game.Players.First; pPlr; pPlr = pPlr->Next) {
      strcat(szCmd, " \"");
      strcat(szCmd, pPlr->Filename);
      strcat(szCmd, "\"");
      iPlrs++;
    }
    // If no players, add default Twonky
    if (iPlrs == 0)
      strcat(szCmd, " Twonky.c4p");

    OpenGame(szCmd);
    return TRUE;
  }

  // JoinPlayer
  if (SEqual2(szCommand, "JoinPlayer:")) {
    Game.Input.AddJoinPlayer(szCommand + 11);
    return TRUE;
  }

  // Quit
  if (SEqual2(szCommand, "Quit")) {
    Application.Quit();
    return TRUE;
  }

  // No valid command
  return FALSE;
}

BOOL C4FullScreen::ActivateMenuNewPlayer() {
  // Max player limit
  if (Game.Players.GetCount() >= Game.C4S.Head.MaxPlayer)
    return FALSE;
  // Menu
  return Menu.ActivateNewPlayer(NO_OWNER);
}

BOOL C4FullScreen::Restart() {
  // Clear
  Game.Clear();
  Menu.Clear();
  // Default
  Game.Default();
  Menu.Default();
  HoldGameOver = FALSE;
  // Reinitialize
  if (Game.Init(CommandLine))
    return TRUE;
  // Failure (clear for safety)
  Game.Clear();
  Menu.Clear();
  return FALSE;
}

void C4FullScreen::CheckPlayerJoinMenu() {
  static int iDelay = 0;
  iDelay++;
  if (iDelay < 3)
    return;
  iDelay = 0;
  // Automatically open player join menu
  if (Active)
    if (!Menu.IsActive())
      if (!Game.GraphicsSystem.MessageBoard.TypeIn)
        if (Game.GraphicsSystem.GetViewport(NO_OWNER))
          ActivateMenuNewPlayer();
}
BOOL C4FullScreen::ActivateMenuScenario() {
  // Init menu
  Menu.Init(C4FacetEx(), "Select Scenario", NO_OWNER, C4MN_Extra_None);
  Menu.SetLocation(100, 100);

  // Open Worlds.c4f
  C4Group hGroup;
  if (!hGroup.Open("Worlds.c4f"))
    return FALSE;

  // Add scenarios
  char szEntry[_MAX_PATH + 1];
  hGroup.ResetSearch();
  while (hGroup.FindNextEntry("*.c4s", szEntry)) {
    char szCmd[1024];
    sprintf(szCmd, "LoadScenario:Worlds.c4f/%s", szEntry);
    Menu.Add(szEntry, C4FacetEx(), szCmd);
  }
  hGroup.Close();

  return TRUE;
}
