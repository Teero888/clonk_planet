/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main class to execute the game fullscreen mode */

#include <C4Include.h>

LRESULT APIENTRY FullScreenWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
	
	// Process message
  switch (uMsg)
    {
    //------------------------------------------------------------------------------------------------------------
    case WM_ACTIVATEAPP:
      Application.Active = wParam;
			// Redraw after task switch
			if (Application.Active) 
				if (Game.Halt || Game.Network.Lobby || Game.Network.WaitingForLobby) 
					Game.GraphicsSystem.Execute();
      return FALSE;
    //------------------------------------------------------------------------------------------------------------
    case WM_TIMER:
      if (wParam==SEC1_TIMER) Game.Sec1Timer();
      return TRUE;
    //------------------------------------------------------------------------------------------------------------
		case WM_DESTROY:
			Application.Clear();
      Application.Quit();
			break;
    //------------------------------------------------------------------------------------------------------------
		case WM_CLOSE:
			FullScreen.Clear();
			break;
    //------------------------------------------------------------------------------------------------------------
		case MM_MCINOTIFY:
			if (wParam == MCI_NOTIFY_SUCCESSFUL)
				Game.MusicSystem.NotifySuccess();
			return TRUE;
    //------------------------------------------------------------------------------------------------------------
    case WM_KEYDOWN:  
			// Lobby: close or abort
			if (Game.Network.Lobby)
				{ 
				switch (wParam)
					{
					case VK_ESCAPE: FullScreen.Clear(); break;
					case VK_RETURN: Game.Network.Lobby=FALSE; break;
					}
				return TRUE; 
				}
			// Waiting for lobby: abort
			if (Game.Network.WaitingForLobby)
				{ 
				switch (wParam)
					{
					case VK_ESCAPE: FullScreen.Clear(); break;
					}
				return TRUE; 
				}
			// Hold game over: quit
			if (FullScreen.HoldGameOver)
				{ 
				FullScreen.Clear(); 
				return TRUE; 
				}
			// Hold abort: abort or continue
			if (FullScreen.HoldAbort)
				{
				WORD cYes;
				cYes='J'; if (SEqual(Config.General.Language,"US")) cYes='Y';
				if (wParam==cYes) 
					FullScreen.Clear();
				else 
					{ 
					FullScreen.HoldAbort=FALSE; 
					Game.Halt=FALSE; 
					Game.GraphicsSystem.RedrawBackground=TRUE;
					}
				return TRUE;
				}
			// Game: hold abort, pause, or input
			switch (wParam)
				{
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
				case VK_ESCAPE:
					Game.Halt = TRUE;
					FullScreen.HoldAbort = TRUE;
					Game.GraphicsSystem.Execute();
					break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 	
				case VK_PAUSE:
					Game.Halt = !Game.Halt;
					break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 	
				default:
					Game.KeyboardInput(wParam);
				break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
				}
      return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_SYSKEYDOWN:
			Game.KeyboardInput(wParam,TRUE);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_CHAR:
			if (!FullScreen.HoldAbort && !FullScreen.HoldGameOver)
				Game.GraphicsSystem.MessageBoard.CharIn((char)wParam);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_USER_LOG:
			if (wParam) { sprintf(OSTR,LoadResStr(wParam),(const char *)lParam); Log(OSTR); }
			else Log((const char *)lParam);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_NET_REQUESTREFERENCE:
			Game.RequestNetworkReference((C4Stream*)lParam);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_NET_REQUESTJOIN:
			Game.RequestJoin((C4Stream*)lParam);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_USER_CREATEREFERENCE:
			Game.Network.CreateReference( (const char*) lParam );
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_NET_INPUT:
			C4Control *pInput; pInput = (C4Control*) lParam;
			Game.Input.AddData(pInput->Data,pInput->Size);
			delete pInput;
			return FALSE;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_LBUTTONDOWN: 
			if (FullScreen.HoldGameOver) { FullScreen.Clear(); return TRUE; }
			Game.GraphicsSystem.MouseMove(C4MC_Button_LeftDown,LOWORD(lParam),HIWORD(lParam),wParam); 
			break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_LBUTTONUP: Game.GraphicsSystem.MouseMove(C4MC_Button_LeftUp,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_RBUTTONDOWN: Game.GraphicsSystem.MouseMove(C4MC_Button_RightDown,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_RBUTTONUP: Game.GraphicsSystem.MouseMove(C4MC_Button_RightUp,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_LBUTTONDBLCLK: Game.GraphicsSystem.MouseMove(C4MC_Button_LeftDouble,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_RBUTTONDBLCLK: Game.GraphicsSystem.MouseMove(C4MC_Button_RightDouble,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------
		case WM_MOUSEMOVE: Game.GraphicsSystem.MouseMove(C4MC_Button_None,LOWORD(lParam),HIWORD(lParam),wParam); break;
		//----------------------------------------------------------------------------------------------------------------------------------

    }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

C4FullScreen::C4FullScreen()
	{
	Default();
	}

C4FullScreen::~C4FullScreen()
	{

	}

BOOL C4FullScreen::OpenGame(const char *szCmdLine)
	{
	// Store command line for restart
	SCopy(szCmdLine,CommandLine,1024);
	// Init game
	if (!Game.Init(CommandLine)) { Clear(); return FALSE; }
	// Success
	return TRUE;
	}

void C4FullScreen::Clear()
	{
	// Clear game members
	Menu.Clear();
	Game.Clear();
	// Destroy window
	if (hWindow) DestroyWindow(hWindow); hWindow=NULL;
	}

void C4FullScreen::Default()
	{
	Active=FALSE;
	hWindow=NULL;
	Menu.Default();
	CommandLine[0]=0;
	HoldGameOver=FALSE;
	HoldAbort=FALSE;
	}

void C4FullScreen::Execute()
	{
	
	// Execute game
	Game.Execute();
	
	// If game over, set HoldGameOver flag
	if (Game.GameOver)
		if (!HoldGameOver)
			{ 
			HoldGameOver=TRUE; 
			Game.Halt=TRUE; 
			Game.GraphicsSystem.Execute();
			}
	
	// If HoldGameOver, clear on any game pad button
	if (HoldGameOver)
		if (Game.GamePadCon1.AnyButtonDown())
			Clear();

	// Execute menu
	Menu.Execute();

	}

HWND C4FullScreen::Init(HINSTANCE hInst)
	{
	Active=TRUE;

	// Register window class
	if (!RegisterWindowClass(hInst)) return NULL;

	// Create window
  hWindow = CreateWindowEx	(
							0, 
							C4FullScreenClassName, 
							"Clonk Planet",
							WS_POPUP,
							CW_USEDEFAULT,CW_USEDEFAULT,0,0,
							NULL,NULL,hInst,NULL);                

	// Show & focus
	ShowWindow(hWindow,SW_SHOWNORMAL);
	SetFocus(hWindow);
	ShowCursor(FALSE);

	return hWindow;
	}

BOOL C4FullScreen::RegisterWindowClass(HINSTANCE hInst)
	{
  WNDCLASSEX WndClass;
	WndClass.cbSize=sizeof(WNDCLASSEX);
  WndClass.style         = CS_DBLCLKS;
  WndClass.lpfnWndProc   = FullScreenWinProc;
  WndClass.cbClsExtra    = 0;		 
  WndClass.cbWndExtra    = 0;	 
  WndClass.hInstance     = hInst;              
  WndClass.hCursor       = NULL;
  WndClass.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
  WndClass.lpszMenuName  = NULL;
  WndClass.lpszClassName = C4FullScreenClassName;
	WndClass.hIcon         = LoadIcon (hInst, MAKEINTRESOURCE (IDI_05_C4X) );
  WndClass.hIconSm       = LoadIcon (hInst, MAKEINTRESOURCE (IDI_05_C4X) );  
	return RegisterClassEx(&WndClass);
	}

BOOL C4FullScreen::ViewportCheck()
	{
	// Not active
	if (!Active) return FALSE;
	// Check viewports
	switch (Game.GraphicsSystem.GetViewportCount())
		{
		// No viewports: create no-owner viewport
		case 0:	Game.CreateViewport(NO_OWNER); break;
		// One viewport: do nothing
		case 1: break;
		// More than one viewport: remove all no-owner viewports
		default: Game.GraphicsSystem.CloseViewport(NO_OWNER); break;
		}
	// Close menu if necessary
	if (!Game.GraphicsSystem.GetViewport(NO_OWNER))
		if (Menu.IsActive()) 
			Menu.Close();
	// Done
	return TRUE;
	}

BOOL C4FullScreen::MenuCommand(const char *szCommand)
	{
	// Not active
	if (!Active) return FALSE;

	// ActivateMenu
	if (SEqual2(szCommand,"ActivateMenu:"))
		{
		if (SEqual(szCommand+13,"NewPlayer")) 
			ActivateMenuNewPlayer();
		return TRUE;
		}

	// JoinPlayer
	if (SEqual2(szCommand,"JoinPlayer:"))
		{
		Game.Input.AddJoinPlayer(szCommand+11);
		return TRUE;
		}	

	// Quit
	if (SEqual2(szCommand,"Quit"))
		{
		Clear();
		return TRUE;
		}

	// No valid command
	return FALSE;
	}

BOOL C4FullScreen::ActivateMenuNewPlayer() 
	{
	// Max player limit
	if (Game.Players.GetCount() >= Game.C4S.Head.MaxPlayer) return FALSE;
	// Menu 
	return Menu.ActivateNewPlayer(NO_OWNER);
	}

BOOL C4FullScreen::Restart()
	{
	// Clear
	Game.Clear();
	Menu.Clear();
	// Default
	Game.Default();
	Menu.Default();
	HoldGameOver=FALSE;
	// Reinitialize
	if (Game.Init(CommandLine)) return TRUE;
	// Failure (clear for safety)
	Game.Clear();
	Menu.Clear();
	return FALSE;
	}

void C4FullScreen::CheckPlayerJoinMenu()
	{
	static int iDelay=0;
	iDelay++; if (iDelay<3) return;
	iDelay=0;
	// Automatically open player join menu
	if (Active)
		if (!Menu.IsActive())
			if (!Game.GraphicsSystem.MessageBoard.TypeIn)
				if (Game.GraphicsSystem.GetViewport(NO_OWNER))
					ActivateMenuNewPlayer();
	}
