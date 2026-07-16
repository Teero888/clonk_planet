/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to handle DirectDraw and critical timers */

#include <C4Include.h>

BOOL InitDirectSound(HWND hwnd);
void DeInitDirectSound();

C4Engine::C4Engine() { Default(); }

C4Engine::~C4Engine() {}

BOOL C4Engine::Init(HINSTANCE hinst, HWND hwnd, BOOL fFullscreen) {
  // Init log, engine header message
  EraseItem(Config.AtLogPath(C4CFN_Log));
  Log(C4EngineInfo);
  if (C4XVer4)
    sprintf(OSTR, "Version: %d.%d%d.%d", C4XVer1, C4XVer2, C4XVer3, C4XVer4);
  else
    sprintf(OSTR, "Version: %d.%d%d", C4XVer1, C4XVer2, C4XVer3);
  Log(OSTR);

  // Initialize DirectDraw
  if (!DDraw.Init(hwnd, fFullscreen, Config.Graphics.ResX, Config.Graphics.ResY, FALSE)) {
    Log(LoadResStr(IDS_ERR_DDRAW));
    return FALSE;
  }
  DDraw.InitFont(Config.General.RXFontName, Config.General.RXFontSize);

  // DirectDraw software emulation mode (set in fullscreen only, developer mode
  // needs to use same as frontend)
  if (fFullscreen) {
    if (GetRegistryDWord(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectDraw", "EmulationOnly", &DDrawEmulationState))
      fDDrawEmulationState = TRUE;
    if (!Config.Graphics.DDrawAccel)
      SetRegistryDWord(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\DirectDraw", "EmulationOnly", 1);
  }

  // On Linux, we need some way to drive Game.GameGo and GraphicsGo
  // For now, let's just set them to TRUE or handle them in the loop.
  Game.GameGo = TRUE;
  Game.GraphicsSystem.GraphicsGo = TRUE;

  // Init DirectSound
  if (Config.Sound.RXSound)
    if (!InitDirectSound(hwnd)) {
      Config.Sound.RXSound = FALSE;
      Log(LoadResStr(IDS_ERR_DSOUND));
    }

  return TRUE;
}

void C4Engine::Clear() {
  // Clear direct sound
  DeInitDirectSound();
  // Clear direct draw
  DDraw.Clear();

  // Log
  Log(LoadResStr(IDS_PRC_DEINIT));
  }


void CALLBACK EngineCriticalTimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) {
  Game.GameGo = TRUE;
  Game.GraphicsSystem.GraphicsGo = TRUE;
}
BOOL C4Engine::SetCriticalTimer(HWND hwnd) {
  return TRUE;
}

void C4Engine::CloseCriticalTimer() {
}


void C4Engine::Default() {
  hLanguageModule = NULL;
  idCriticalTimer = 0;
  fTimePeriod = FALSE;
  DDrawEmulationState = 0;
  fDDrawEmulationState = FALSE;
}
