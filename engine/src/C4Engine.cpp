/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to handle DirectDraw and critical timers */

#include <C4Include.h>

BOOL InitDirectSound(HWND hwnd);
void DeInitDirectSound();

C4Engine::C4Engine()
	{
	Default();
	}

C4Engine::~C4Engine()
	{

	}

BOOL C4Engine::Init(HINSTANCE hinst, HWND hwnd, BOOL fFullscreen)
  {
  // Init log, engine header message
  DeleteFile(Config.AtLogPath(C4CFN_Log));
  Log(C4EngineInfo);
	sprintf(OSTR,C4XVer4 ? "Version: %d.%d%d.%d" : "Version: %d.%d%d",C4XVer1,C4XVer2,C4XVer3,C4XVer4);
  Log(OSTR);

  // Init game timers
  if ( !SetTimer(hwnd,SEC1_TIMER,SEC1_MSEC,NULL)
    || !SetCriticalTimer(hwnd) )
      { Log(LoadResStr(IDS_ERR_TIMER));	return FALSE; }

	// DirectDraw software emulation mode (set in fullscreen only, developer mode 
	// needs to use same as frontend)
	if (fFullscreen)
		{
		if (GetRegistryDWord(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\DirectDraw","EmulationOnly",&DDrawEmulationState))
			fDDrawEmulationState=TRUE;
		if (!Config.Graphics.DDrawAccel) 
			SetRegistryDWord(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\DirectDraw","EmulationOnly",1);
		}

  // Initialize DirectDraw
	if (!DDraw.Init(hwnd,fFullscreen,Config.Graphics.ResX,Config.Graphics.ResY,FALSE)) 
    { Log(LoadResStr(IDS_ERR_DDRAW)); return FALSE; }  
	if (Config.General.RXFontName[0])
		DDraw.InitFont(Config.General.RXFontName,Config.General.RXFontSize);

  // Init DirectSound
  if (Config.Sound.RXSound)
    if (!InitDirectSound(hwnd))
			{ Config.Sound.RXSound=FALSE; Log(LoadResStr(IDS_ERR_DSOUND)); }  

  return TRUE;
  }

void C4Engine::Clear()
  {
	// Clear direct sound
  DeInitDirectSound();
	// Clear direct draw
  DDraw.Clear();	
	// Restore DirectDraw emulation state
	if (fDDrawEmulationState)
		SetRegistryDWord(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\DirectDraw","EmulationOnly",DDrawEmulationState);
	// Close timers
	CloseCriticalTimer();
	// Log
	Log(LoadResStr(IDS_PRC_DEINIT));
	// Free language module
	if (hLanguageModule) FreeLibrary(hLanguageModule);
  }

void CALLBACK EngineCriticalTimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
  {
  Game.GameGo=TRUE;
	Game.GraphicsSystem.GraphicsGo=TRUE;
  }

BOOL C4Engine::SetCriticalTimer(HWND hwnd)
  {
  // Establish minimum resolution
  if (timeBeginPeriod(CRITICAL_MSEC)!=TIMERR_NOERROR)
    return FALSE;
  fTimePeriod=TRUE;
  // Set critical timer
  if (!(idCriticalTimer=timeSetEvent(
               CRITICAL_MSEC,CRITICAL_MSEC,
               &EngineCriticalTimer,0,TIME_PERIODIC))) return FALSE;
  return TRUE;
  }

void C4Engine::CloseCriticalTimer()
  {
  if (idCriticalTimer) timeKillEvent(idCriticalTimer);
  if (fTimePeriod) timeEndPeriod(CRITICAL_MSEC);
  }

void C4Engine::Default()
	{
	hLanguageModule=NULL;
	idCriticalTimer=0;
	fTimePeriod=FALSE;
	DDrawEmulationState=0;
	fDDrawEmulationState=FALSE;
	}

