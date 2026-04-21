/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles engine execution in developer mode */

#include <C4Include.h>

BOOL SetMenuItemText(HMENU hMenu, WORD id, const char *szText);

C4Console::C4Console()
	{
	Active = FALSE;
	Editing = TRUE;

	hDialog=NULL;
	ScriptCounter=0;
	FrameCounter=0;
	fGameOpen=FALSE;
	
	hbmCursor=NULL;
	hbmCursor2=NULL;
	hbmBrush=NULL;
	hbmBrush2=NULL;
	hbmPlay=NULL;
	hbmPlay2=NULL;
	hbmHalt=NULL;
	hbmHalt2=NULL;

	MenuIndexFile				=  0,
	MenuIndexComponents =  1,
	MenuIndexPlayer			=  2,
	MenuIndexViewport		=  3,
	MenuIndexNet				= -1,
	MenuIndexHelp				=  4; 
	}
		
C4Console::~C4Console()
	{
	if (hbmCursor) DeleteObject(hbmCursor);
	if (hbmCursor2) DeleteObject(hbmCursor2);
	if (hbmBrush) DeleteObject(hbmBrush);
	if (hbmBrush2) DeleteObject(hbmBrush2);
	if (hbmPlay) DeleteObject(hbmPlay);
	if (hbmPlay2) DeleteObject(hbmPlay2);
	if (hbmHalt) DeleteObject(hbmHalt);
	if (hbmHalt2) DeleteObject(hbmHalt2);
	}

BOOL CALLBACK ConsoleDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
	
	switch (Msg)
		{
    //------------------------------------------------------------------------------------------------------------
    case WM_ACTIVATEAPP:
      Application.Active = wParam;
      return TRUE;    
    //------------------------------------------------------------------------------------------------------------
    case WM_TIMER:
      if (wParam==SEC1_TIMER) Game.Sec1Timer();
      return TRUE;
    //------------------------------------------------------------------------------------------------------------
    case WM_DESTROY:
			StoreWindowPosition(hDlg,"Main","Software\\RedWolf Design\\Clonk 4\\Console",FALSE);
			Application.Clear();
      Application.Quit();
			return TRUE;
    //------------------------------------------------------------------------------------------------------------
		case WM_CLOSE:
			Console.FileQuit();
			return TRUE;
    //------------------------------------------------------------------------------------------------------------
		case MM_MCINOTIFY:
			if (wParam == MCI_NOTIFY_SUCCESSFUL)
				Game.MusicSystem.NotifySuccess();
			return TRUE;
    //------------------------------------------------------------------------------------------------------------
		case WM_INITDIALOG:  
      SendMessage(hDlg,DM_SETDEFID,(WPARAM)IDOK,(LPARAM)0);  
			Console.UpdateMenuText(GetMenu(hDlg));
      return TRUE; 
    //------------------------------------------------------------------------------------------------------------
		case WM_COMMAND:  
			// Evaluate command 
			switch (LOWORD(wParam))
				{
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDOK:
					// IDC_COMBOINPUT to Console.In()
					char buffer[16000];
					GetDlgItemText(hDlg,IDC_COMBOINPUT,buffer,16000);
					if (buffer[0])
						Console.In(buffer);
					Console.EditCursor.UpdateGraphicsSystem();
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDC_BUTTONHALT:
					Game.Halt=TRUE;
					Console.UpdateHaltCtrls(Game.Halt);
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDC_BUTTONPLAY:
					// Lobby continue
					if (Game.Network.Lobby) Game.Network.Lobby=FALSE;
					// Regular pause continue
					Game.Halt=FALSE;
					Console.UpdateHaltCtrls(Game.Halt);
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDC_BUTTONMODEPLAY:
					Console.EditCursor.SetMode(C4CNS_ModePlay);
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDC_BUTTONMODEEDIT:
					Console.EditCursor.SetMode(C4CNS_ModeEdit);
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDC_BUTTONMODEDRAW:
					Console.EditCursor.SetMode(C4CNS_ModeDraw);
					return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_QUIT:	Console.FileQuit();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_SAVEAS:	Console.FileSaveAs(FALSE);	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_SAVE:	Console.FileSave(FALSE);	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_SAVEGAMEAS:	Console.FileSaveAs(TRUE);	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_SAVEGAME:	Console.FileSave(TRUE);	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_OPEN:	Console.FileOpen();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_OPENWPLRS:	Console.FileOpenWPlrs(); return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_FILE_CLOSE: Console.FileClose();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_HELP_ABOUT: Console.HelpAbout();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_PLAYER_JOIN: Console.PlayerJoin();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_VIEWPORT_NEW: Console.ViewportNew();	return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_COMPONENTS_TITLE: Console.EditTitle(); return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_COMPONENTS_INFO: Console.EditInfo(); return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case IDM_COMPONENTS_SCRIPT: Console.EditScript(); return TRUE;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				}
			// New player viewport
			if (Inside(LOWORD(wParam),IDM_VIEWPORT_NEW1,IDM_VIEWPORT_NEW2))
				{
				Game.CreateViewport(LOWORD(wParam)-IDM_VIEWPORT_NEW1);
				return TRUE;
				}
			// Remove player
			if (Inside(LOWORD(wParam),IDM_PLAYER_QUIT1,IDM_PLAYER_QUIT2))
				{
				Game.Input.AddRemovePlayer(LOWORD(wParam)-IDM_PLAYER_QUIT1);
				return TRUE;
				}
			// Remove client
			if (Inside(LOWORD(wParam),IDM_NET_CLIENT1,IDM_NET_CLIENT2))
				{
				Game.Input.AddRemoveClient( LOWORD(wParam)-IDM_NET_CLIENT1, Game.Network.GetClientName(LOWORD(wParam)-IDM_NET_CLIENT1) );
				return TRUE;
				}
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_USER_LOG:
			if (wParam) { sprintf(OSTR,LoadResStr(wParam),(const char *)lParam); Log(OSTR); }
			else Log((const char *)lParam);
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_USER_NETLOG:
			if (wParam) { sprintf(OSTR,LoadResStr(wParam),(const char *)lParam); NetLog(OSTR); }
			else NetLog((const char *)lParam);
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
		case WM_USER_RELOADDEF:
			Game.ReloadDef( (C4ID) lParam );
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_USER_CREATEREFERENCE:
			Game.Network.CreateReference( (const char*) lParam );
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		case WM_NET_INPUT:
			C4Control *pInput = (C4Control*) lParam;
			Game.Input.AddData(pInput->Data,pInput->Size);
			delete pInput;
			return FALSE;
    //------------------------------------------------------------------------------------------------------------
		}

	return FALSE;
	}

HWND C4Console::Init(HINSTANCE hInst)
	{
	// Active
	Active = TRUE;
	// Editing (disable if network)
	Editing = !Config.Network.Active;
	// Create dialog window
	if (!( hDialog = CreateDialog( 
										 hInst,
										 MAKEINTRESOURCE(IDD_CONSOLE),
										 NULL,
										 (DLGPROC) ConsoleDlgProc ) )) return NULL;
	RestoreWindowPosition(hDialog,"Main","Software\\RedWolf Design\\Clonk 4\\Console");
	// Set icon
	SendMessage(hDialog,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_00_C4X)));
	SendMessage(hDialog,WM_SETICON,ICON_SMALL,(LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_00_C4X)));
	// Set text
	SetCaption(LoadResStr(IDS_CNS_CONSOLE));
	// Load bitmaps
	hbmMouse=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MOUSE));
	hbmMouse2=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_MOUSE2));
	hbmCursor=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CURSOR));
	hbmCursor2=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_CURSOR2));
	hbmBrush=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BRUSH));
	hbmBrush2=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_BRUSH2));
	hbmPlay=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_PLAY));
	hbmPlay2=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_PLAY2));
	hbmHalt=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_HALT));
	hbmHalt2=(HBITMAP)LoadBitmap(hInst,MAKEINTRESOURCE(IDB_HALT2));
	// Enable controls
	UpdateHaltCtrls(TRUE);
	EnableControls(fGameOpen);
	ClearViewportMenu();
	// Show window and set focus
	ShowWindow(hDialog,SW_SHOWNORMAL);
	UpdateWindow(hDialog);
	SetFocus(hDialog);
	ShowCursor(TRUE);
	// Success
	return hDialog;
	}

BOOL C4Console::In(const char *szText)
	{
	if (!Active) return FALSE;	
	// Create valid single statement
	int iResult; char buffer[1000]; 
	sprintf(buffer,"return(%s);",szText);
	iResult=Game.CommandLine(buffer);
	// Result output
	sprintf(OSTR,"%s = %i",szText,iResult); Out(OSTR);
	return TRUE;
	}

BOOL C4Console::Out(const char *szText)
	{
	if (!Active) return FALSE;
	int len,lines; char *buffer;
	len = 65000;//SendDlgItemMessage(hDialog,IDC_EDITOUTPUT,EM_LINELENGTH,(WPARAM)0,(LPARAM)0); 	
	buffer = new char [len+5000];
	buffer[0]=0;
	GetDlgItemText(hDialog,IDC_EDITOUTPUT,buffer,len);
	if (buffer[0]) SAppend("\r\n",buffer);
	SAppend(szText,buffer);
	SetDlgItemText(hDialog,IDC_EDITOUTPUT,buffer);
	delete [] buffer;
	lines = SendDlgItemMessage(hDialog,IDC_EDITOUTPUT,EM_GETLINECOUNT,(WPARAM)0,(LPARAM)0);
	SendDlgItemMessage(hDialog,IDC_EDITOUTPUT,EM_LINESCROLL,(WPARAM)0,(LPARAM)lines);
	UpdateWindow(hDialog);
	return TRUE;
	}

BOOL C4Console::UpdateStatusBars()
	{
	if (!Active) return FALSE;
	// Frame counter
	if (Game.FrameCounter!=FrameCounter)
		{
		FrameCounter=Game.FrameCounter;
		sprintf(OSTR,"Frame: %i",FrameCounter);
		SetDlgItemText(hDialog,IDC_STATICFRAME,OSTR);
		UpdateWindow(GetDlgItem(hDialog,IDC_STATICFRAME));
		}
	// Script counter
	if (Game.Script.Counter!=ScriptCounter)
		{
		ScriptCounter=Game.Script.Counter;
		sprintf(OSTR,"Script: %i",ScriptCounter);
		SetDlgItemText(hDialog,IDC_STATICSCRIPT,OSTR);
		UpdateWindow(GetDlgItem(hDialog,IDC_STATICSCRIPT));
		}
	// Time & FPS
	if ((Game.Time!=Time) || (Game.FPS!=FPS))
		{
		Time=Game.Time;
		FPS=Game.FPS;
		sprintf(OSTR,"%02ld:%02ld:%02ld (%i FPS)",Time/3600,(Time%3600)/60,Time%60,FPS);
		SetDlgItemText(hDialog,IDC_STATICTIME,OSTR);
		UpdateWindow(GetDlgItem(hDialog,IDC_STATICTIME));
		}
	return TRUE;
	}

BOOL C4Console::UpdateHaltCtrls(BOOL fHalt)
	{
	if (!Active) return FALSE;
	SendDlgItemMessage(hDialog,IDC_BUTTONPLAY,BM_SETSTATE,!fHalt,0);
	UpdateWindow(GetDlgItem(hDialog,IDC_BUTTONPLAY));
	SendDlgItemMessage(hDialog,IDC_BUTTONHALT,BM_SETSTATE,fHalt,0);
	UpdateWindow(GetDlgItem(hDialog,IDC_BUTTONHALT));	
	return TRUE;
	}

void C4Console::Execute()
	{
	if (fGameOpen) Game.Execute();
	EditCursor.Execute();
	PropertyDlg.Execute();
	UpdateStatusBars();
	}

BOOL C4Console::SetCaption(const char *szCaption)
	{
	if (!Active) return FALSE;
	// Sorry, the window caption needs to be constant so
	// the window can be found using FindWindow
	return SetWindowText(hDialog,C4EngineCaption); 
	//return SetWindowText(hDialog,szCaption);
	}

BOOL C4Console::FileQuit()
	{
	Game.Clear();
	PropertyDlg.Clear();
	EditCursor.Clear();
	if (hDialog) DestroyWindow(hDialog); hDialog=NULL;
	return TRUE;
	}

BOOL C4Console::SaveGame(BOOL fSaveGame) 
	{
	
	// Network hosts only
	if (Game.Network.Active && !Game.Network.Host)
		{ Message(LoadResStr(IDS_GAME_NOCLIENTSAVE)); return FALSE; }

	// Can't save to child groups
	if (Game.ScenarioFile.GetMother())
		{
		sprintf(OSTR,LoadResStr(IDS_CNS_NOCHILDSAVE),
						GetFilename(Game.ScenarioFile.GetName()));
		Message(OSTR);
		return FALSE;
		}

	// Save game to open scenario file
	BOOL fOkay=TRUE;
	SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_WAIT)));

	if (!Game.Save(Game.ScenarioFile,fSaveGame,FALSE)) 
		{ Out("Game::Save failed"); fOkay=FALSE; }	

	// Close and reopen scenario file to fix file changes
	Game.ScenarioFile.Sort(C4FLS_Scenario);
	if (!Game.ScenarioFile.Close()) 
		{ Out("ScenarioFile::Close failed"); fOkay=FALSE; }
	if (!Game.ScenarioFile.Open(Game.ScenarioFilename)) 
		{ Out("ScenarioFile::Open failed"); fOkay=FALSE; }

	SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_ARROW)));

	// Initialize/script notification
	if (Game.fScriptCreatedObjects)
		if (!fSaveGame)
			{
			SCopy(LoadResStr(IDS_CNS_SCRIPTCREATEDOBJECTS),OSTR);
			SAppend(LoadResStr(IDS_CNS_WARNDOUBLE),OSTR);
			Message(OSTR);
			Game.fScriptCreatedObjects=FALSE;
			}

	// Status report
	if (!fOkay) Message(LoadResStr(IDS_CNS_SAVERROR));
	else Out(LoadResStr(fSaveGame ? IDS_CNS_GAMESAVED : IDS_CNS_SCENARIOSAVED));

	return fOkay;
	}

BOOL C4Console::FileSave(BOOL fSaveGame)
	{	
	// Don't quicksave games over scenarios
	if (fSaveGame)
		if (!Game.C4S.Head.SaveGame)
			{
			Message(LoadResStr(IDS_CNS_NOGAMEOVERSCEN));
			return FALSE;
			}
	// Save game
	return SaveGame(fSaveGame);	
	}

BOOL C4Console::FileSaveAs(BOOL fSaveGame)
	{
	// Do save-as dialog
	char filename[512+1];
	SCopy(Game.ScenarioFile.GetName(),filename);
	if (!FileSelect(filename,512,
									"Clonk 4 Scenario\0*.c4s\0\0",
									OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
									TRUE)) return FALSE;
	DefaultExtension(filename,"c4s");
	BOOL fOkay=TRUE;
	// Close current scenario file
	if (!Game.ScenarioFile.Close()) fOkay=FALSE;
	// Copy current scenario file to target
	if (!C4Group_CopyItem(Game.ScenarioFilename,filename)) fOkay=FALSE;
	// Open new scenario file
	SCopy(filename,Game.ScenarioFilename);
	SetCaption(GetFilename(Game.ScenarioFilename));
	if (!Game.ScenarioFile.Open(Game.ScenarioFilename)) fOkay=FALSE;
	// Failure message
	if (!fOkay) 
		{
		sprintf(OSTR,LoadResStr(IDS_CNS_SAVEASERROR),Game.ScenarioFilename);
		Message(OSTR); return FALSE;
		}
	// Save game
	return SaveGame(fSaveGame);
	}

BOOL C4Console::Message(const char *szMessage, BOOL fQuery)
	{
	if (!Active) return FALSE;
	return (IDOK==MessageBox(hDialog,szMessage,"Clonk Planet",fQuery ? (MB_OKCANCEL | MB_ICONEXCLAMATION) : MB_ICONEXCLAMATION));
	}

void C4Console::EnableControls(BOOL fEnable)
	{
	if (!Active) return;
	
	// Set button images (edit modes & halt controls)
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEPLAY,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)(fEnable ? hbmMouse : hbmMouse2));
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEEDIT,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)((fEnable && Editing) ? hbmCursor : hbmCursor2));
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEDRAW,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)((fEnable && Editing) ? hbmBrush : hbmBrush2));
	SendDlgItemMessage(hDialog,IDC_BUTTONPLAY,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)(Game.Network.Lobby || fEnable ? hbmPlay : hbmPlay2));
	SendDlgItemMessage(hDialog,IDC_BUTTONHALT,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)(Game.Network.Lobby || fEnable ? hbmHalt : hbmHalt2));
	
	// OK
	EnableWindow( GetDlgItem(hDialog,IDOK), (fEnable && Editing) );

	// Halt controls
	EnableWindow(GetDlgItem(hDialog,IDC_BUTTONPLAY), Game.Network.Lobby || fEnable);
	EnableWindow(GetDlgItem(hDialog,IDC_BUTTONHALT), Game.Network.Lobby || fEnable);

	// Edit modes
	EnableWindow(GetDlgItem(hDialog,IDC_BUTTONMODEPLAY),(fEnable));
	EnableWindow(GetDlgItem(hDialog,IDC_BUTTONMODEEDIT),(fEnable && Editing));
	EnableWindow(GetDlgItem(hDialog,IDC_BUTTONMODEDRAW),(fEnable && Editing));

	// Console input
	EnableWindow(GetDlgItem(hDialog,IDC_COMBOINPUT), (fEnable && Editing));

	// File menu
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_OPEN, MF_BYCOMMAND | ((Game.Network.Lobby | Game.Network.WaitingForLobby) ? MF_GRAYED : MF_ENABLED ));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_OPENWPLRS, MF_BYCOMMAND | ((Game.Network.Lobby | Game.Network.WaitingForLobby) ? MF_GRAYED : MF_ENABLED ));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_SAVEGAME, MF_BYCOMMAND | ((fEnable && Game.Players.GetCount()) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_SAVEGAMEAS, MF_BYCOMMAND | ((fEnable && Game.Players.GetCount()) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_SAVE, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_SAVEAS, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_FILE_CLOSE, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
	
	// Components menu
	EnableMenuItem(GetMenu(hDialog),IDM_COMPONENTS_SCRIPT, MF_BYCOMMAND | ((fEnable && Editing) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_COMPONENTS_INFO, MF_BYCOMMAND | ((fEnable && Editing) ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_COMPONENTS_TITLE, MF_BYCOMMAND | ((fEnable && Editing) ? MF_ENABLED : MF_GRAYED));

	// Player & viewport menu
	EnableMenuItem(GetMenu(hDialog),IDM_PLAYER_JOIN, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(GetMenu(hDialog),IDM_VIEWPORT_NEW, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));

	}

BOOL C4Console::FileOpen()
	{
	// Get scenario file name
	char c4sfile[512+1]="";
	if (!FileSelect(c4sfile,512,
									"Clonk 4 Scenario\0*.c4s;*.c4f\0\0",
									OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
									)) return FALSE;
	// Compose command line
	char cmdline[2000]="";
	SAppend("\"",cmdline); SAppend(c4sfile,cmdline); SAppend("\" ",cmdline); 
	// Open game
	OpenGame(cmdline);
	return TRUE;
	}

BOOL C4Console::FileOpenWPlrs()
	{
	// Get scenario file name
	char c4sfile[512+1]="";
	if (!FileSelect(c4sfile,512,
									"Clonk 4 Scenario\0*.c4s;*.c4f\0\0",
									OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
									)) return FALSE;
	// Get player file name(s)
	char c4pfile[4096+1]="";
	if (!FileSelect(c4pfile,4096,
									"Clonk 4 Player\0*.c4p\0\0",
									OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER
									)) return FALSE;
	// Compose command line
	char cmdline[6000]="";
	SAppend("\"",cmdline); SAppend(c4sfile,cmdline); SAppend("\" ",cmdline); 
	if (FileAttributes(c4pfile) & _A_SUBDIR) // Multiplayer
		{
		const char *cptr = c4pfile+SLen(c4pfile)+1;
		while (*cptr)
			{
			SAppend("\"",cmdline);
			SAppend(c4pfile,cmdline);	SAppend("\\",cmdline); 
			SAppend(cptr,cmdline); SAppend("\" ",cmdline);
			cptr += SLen(cptr)+1;
			}
		}
	else // Single player
		{
		SAppend("\"",cmdline); SAppend(c4pfile,cmdline); SAppend("\" ",cmdline); 
		}
	// Open game
	OpenGame(cmdline);
	return TRUE;
	}

BOOL C4Console::FileClose()
	{
	return CloseGame();
	}

BOOL C4Console::CloseGame()
	{
	if (!fGameOpen) return FALSE;
	Game.Clear();
	Game.GameOver=FALSE; // No leftover values when exiting on closed game
	Clear();
	fGameOpen=FALSE;
	EnableControls(fGameOpen);
	SetCaption(LoadResStr(IDS_CNS_CONSOLE));
	return TRUE;
	}

BOOL C4Console::OpenGame(const char *szCmdLine)
	{

	// Close any old game
	CloseGame();

	// Default game dependent members
	Default();
	
	// Init game dependent members
	if (!EditCursor.Init(Application.hInstance)) return FALSE;	
	
	// Default game
	Game.Default();

	// Init game
	if (!Game.Init(szCmdLine))
		{
		Game.Clear();
		return FALSE;
		}

	// Console updates
	fGameOpen=TRUE;
	UpdateInputCtrl();
	EnableControls(fGameOpen);
	UpdatePlayerMenu();
	UpdateViewportMenu();
	SetCaption(GetFilename(Game.ScenarioFile.GetName()));
	
	return TRUE;
	}

BOOL C4Console::FileSelect(char *sFilename, int iSize, const char * szFilter, DWORD dwFlags, BOOL fSave)
	{
	OPENFILENAME ofn;
	ZeroMem(&ofn,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.hwndOwner=hDialog;
	ofn.lpstrFilter=szFilter;
	ofn.lpstrFile=sFilename;
	ofn.nMaxFile=iSize;
	ofn.nFileOffset= GetFilename(sFilename) - sFilename;
	ofn.nFileExtension= GetExtension(sFilename) - sFilename;
	ofn.Flags=dwFlags;

	if (fSave) return GetSaveFileName(&ofn);
	else return GetOpenFileName(&ofn);

	return FALSE;
	}

void C4Console::ClearPointers(C4Object *pObj)
	{
	EditCursor.ClearPointers(pObj);
	PropertyDlg.ClearPointers(pObj);
	}

void C4Console::Default()
	{
	EditCursor.Default();
	PropertyDlg.Default();
	ToolsDlg.Default();
	}

void C4Console::Clear()
	{
	EditCursor.Clear();
	PropertyDlg.Clear();
	ToolsDlg.Clear();
	ClearViewportMenu();
	ClearPlayerMenu();
	ClearNetMenu();
	}

void C4Console::HelpAbout()
	{
	sprintf(OSTR,LoadResStr(IDS_CNS_ABOUT),C4XVer1,C4XVer2,C4XVer3);
	MessageBox(NULL,OSTR,"Clonk Planet",MB_ICONINFORMATION | MB_TASKMODAL);
	}

void C4Console::ViewportNew()
	{
	Game.CreateViewport(NO_OWNER);
	}

BOOL C4Console::UpdateViewportMenu()
	{
	if (!Active) return FALSE;
	ClearViewportMenu();
	HMENU hMenu = GetSubMenu(GetMenu(hDialog),MenuIndexViewport); 
	for (C4Player *pPlr=Game.Players.First; pPlr; pPlr=pPlr->Next)
		{
		sprintf(OSTR,LoadResStr(IDS_CNS_NEWPLRVIEWPORT),pPlr->Name);
		AddMenuItem(hMenu,IDM_VIEWPORT_NEW1+pPlr->Number,OSTR);
		} 
	return TRUE;
	}

void C4Console::ClearViewportMenu()
	{
	if (!Active) return;
	HMENU hMenu = GetSubMenu(GetMenu(hDialog),MenuIndexViewport); 
	while (DeleteMenu(hMenu,1,MF_BYPOSITION));
	}

BOOL C4Console::AddMenuItem(HMENU hMenu, DWORD dwID, const char *szString, BOOL fEnabled)
	{
	if (!Active) return FALSE;
	MENUITEMINFO minfo;
	ZeroMem(&minfo,sizeof(minfo));
	minfo.cbSize = sizeof(minfo);
	minfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA | MIIM_STATE;
	minfo.fType = MFT_STRING;
	minfo.wID = dwID;
	minfo.dwTypeData = (char*) szString;
	minfo.cch = SLen(szString);
	if (!fEnabled) minfo.fState|=MFS_GRAYED;
	return InsertMenuItem(hMenu,0,FALSE,&minfo);
	}

BOOL C4Console::CommandLine(const char *szCmdLine)
	{
	if (szCmdLine[0]) 
		if (!OpenGame(szCmdLine))
			return FALSE;
	return TRUE;
	}

BOOL C4Console::Keydown(WORD wParam)
	{
	if (!Active) return FALSE;
	// Global keyboard controls
	switch (wParam)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case VK_PAUSE:
			Game.Halt = !Game.Halt;
			UpdateHaltCtrls(Game.Halt);
			return TRUE;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case VK_SPACE:
			EditCursor.ToggleMode();
			return TRUE;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case VK_ADD:
			ToolsDlg.ChangeGrade(+5);
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case VK_SUBTRACT:
			ToolsDlg.ChangeGrade(-5);
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case 'M':
			if (GetKeyState(VK_CONTROL)<0) ToolsDlg.PopMaterial();
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case 'T':
			if (GetKeyState(VK_CONTROL)<0) ToolsDlg.PopTextures();
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case 'I':
			if (GetKeyState(VK_CONTROL)<0) ToolsDlg.SetIFT(!ToolsDlg.ModeIFT);
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case 'W':
			if (GetKeyState(VK_CONTROL)<0) ToolsDlg.SetTool((ToolsDlg.Tool+1)%4);
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	return FALSE;
	}

BOOL C4Console::UpdateModeCtrls(BOOL iMode)
	{
	if (!Active) return FALSE;
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEPLAY,BM_SETSTATE,(iMode==C4CNS_ModePlay),0);
	UpdateWindow(GetDlgItem(hDialog,IDC_BUTTONMODEPLAY));	
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEEDIT,BM_SETSTATE,(iMode==C4CNS_ModeEdit),0);
	UpdateWindow(GetDlgItem(hDialog,IDC_BUTTONMODEEDIT));	
	SendDlgItemMessage(hDialog,IDC_BUTTONMODEDRAW,BM_SETSTATE,(iMode==C4CNS_ModeDraw),0);
	UpdateWindow(GetDlgItem(hDialog,IDC_BUTTONMODEDRAW));
	return TRUE;
	}

BOOL C4Console::UpdateCursorBar(const char *szCursor)
	{
	if (!Active) return FALSE;
	// Cursor 
	SetDlgItemText(hDialog,IDC_STATICCURSOR,szCursor);
	UpdateWindow(GetDlgItem(hDialog,IDC_STATICCURSOR));
	return TRUE;
	}

void C4Console::EditTitle()
	{
	Game.Title.Open();
	}

void C4Console::EditScript()
	{
	Game.Script.Open();
	}

void C4Console::EditInfo()
	{
	Game.Info.Open();
	}

void C4Console::UpdateInputCtrl()
	{
	int cnt;
	HWND hCombo = GetDlgItem(hDialog,IDC_COMBOINPUT);
	// Clear & add standard functions
	SendMessage(hCombo,CB_RESETCONTENT,0,0);
	for (C4ScriptFnDef *pFn = C4ScriptFnMap; pFn->Identifier; pFn++)
		if (pFn->Public)
			{
			SCopy(pFn->Identifier,OSTR); SAppend("()",OSTR);
			SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)OSTR);
			}
	// Add scenario script functions
	C4ScriptFnRef *pRef;
	if (pRef=Game.Script.GetFunctionRef(0))
		SendMessage(hCombo,CB_INSERTSTRING,0,(LPARAM)"----------");
	for (cnt=0; pRef=Game.Script.GetFunctionRef(cnt); cnt++)
		{
		SCopy(pRef->Name,OSTR); SAppend("()",OSTR);
		SendMessage(hCombo,CB_INSERTSTRING,0,(LPARAM)OSTR);
		}
	}

BOOL C4Console::UpdatePlayerMenu()
	{
	if (!Active) return FALSE;
	ClearPlayerMenu();
	HMENU hMenu = GetSubMenu(GetMenu(hDialog),MenuIndexPlayer); 
	for (C4Player *pPlr=Game.Players.First; pPlr; pPlr=pPlr->Next)
		{
		if (Game.Network.Active) sprintf(OSTR,LoadResStr(IDS_CNS_PLRQUITNET),pPlr->Name,pPlr->AtClientName);
		else sprintf(OSTR,LoadResStr(IDS_CNS_PLRQUIT),pPlr->Name);
		AddMenuItem(hMenu,IDM_PLAYER_QUIT1+pPlr->Number,OSTR,(!Game.Network.Active || Game.Network.Host));
		} 
	return TRUE;
	}

void C4Console::ClearPlayerMenu()
	{
	if (!Active) return;
	HMENU hMenu = GetSubMenu(GetMenu(hDialog),MenuIndexPlayer); 
	while (DeleteMenu(hMenu,1,MF_BYPOSITION));
	}

void C4Console::UpdateMenus()
	{
	if (!Active) return;
	EnableControls(fGameOpen);
	UpdatePlayerMenu();
	UpdateViewportMenu();
	UpdateNetMenu();
	}

void C4Console::PlayerJoin()
	{

	// Get player file name(s)
	char c4pfile[4096+1]="";
	if (!FileSelect(c4pfile,4096,
									"Clonk 4 Player\0*.c4p\0\0",
									OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT | OFN_EXPLORER
									)) return;
	
	// Compose player file list
	char c4plist[6000]="";
	// Multiple players
	if (FileAttributes(c4pfile) & _A_SUBDIR) 
		{
		const char *cptr = c4pfile+SLen(c4pfile)+1;
		while (*cptr)
			{
			SNewSegment(c4plist);
			SAppend(c4pfile,c4plist);	SAppend("\\",c4plist); SAppend(cptr,c4plist);
			cptr += SLen(cptr)+1;
			}
		}
	// Single player
	else 
		{
		SAppend(c4pfile,c4plist);
		}
	
	// Join players
	char szPlayerFilename[_MAX_PATH+1];
	for (int iPar=0; SCopySegment(c4plist,iPar,szPlayerFilename,';',_MAX_PATH); iPar++)
		if (szPlayerFilename[0])
			Game.Input.AddJoinPlayer(szPlayerFilename);

	}

void C4Console::UpdateMenuText(HMENU hMenu)
	{
	HMENU hSubMenu;
	if (!Active) return;
	// File
	ModifyMenu(hMenu,MenuIndexFile,MF_BYPOSITION | MF_STRING,0,LoadResStr(IDS_MNU_FILE));
	hSubMenu = GetSubMenu(hMenu,MenuIndexFile); 
	SetMenuItemText(hSubMenu,IDM_FILE_OPEN,LoadResStr(IDS_MNU_OPEN));
	SetMenuItemText(hSubMenu,IDM_FILE_OPENWPLRS,LoadResStr(IDS_MNU_OPENWPLRS));
	SetMenuItemText(hSubMenu,IDM_FILE_SAVE,LoadResStr(IDS_MNU_SAVESCENARIO));
	SetMenuItemText(hSubMenu,IDM_FILE_SAVEAS,LoadResStr(IDS_MNU_SAVESCENARIOAS));
	SetMenuItemText(hSubMenu,IDM_FILE_SAVEGAME,LoadResStr(IDS_MNU_SAVEGAME));
	SetMenuItemText(hSubMenu,IDM_FILE_SAVEGAMEAS,LoadResStr(IDS_MNU_SAVEGAMEAS));
	SetMenuItemText(hSubMenu,IDM_FILE_CLOSE,LoadResStr(IDS_MNU_CLOSE));
	SetMenuItemText(hSubMenu,IDM_FILE_QUIT,LoadResStr(IDS_MNU_QUIT));
	// Components
	ModifyMenu(hMenu,MenuIndexComponents,MF_BYPOSITION | MF_STRING,0,LoadResStr(IDS_MNU_COMPONENTS));
	hSubMenu = GetSubMenu(hMenu,MenuIndexComponents); 
	SetMenuItemText(hSubMenu,IDM_COMPONENTS_SCRIPT,LoadResStr(IDS_MNU_SCRIPT));
	SetMenuItemText(hSubMenu,IDM_COMPONENTS_TITLE,LoadResStr(IDS_MNU_TITLE));
	SetMenuItemText(hSubMenu,IDM_COMPONENTS_INFO,LoadResStr(IDS_MNU_INFO));
	// Player
	ModifyMenu(hMenu,MenuIndexPlayer,MF_BYPOSITION | MF_STRING,0,LoadResStr(IDS_MNU_PLAYER));
	hSubMenu = GetSubMenu(hMenu,MenuIndexPlayer); 
	SetMenuItemText(hSubMenu,IDM_PLAYER_JOIN,LoadResStr(IDS_MNU_JOIN));
	// Viewport
	ModifyMenu(hMenu,MenuIndexViewport,MF_BYPOSITION | MF_STRING,0,LoadResStr(IDS_MNU_VIEWPORT));
	hSubMenu = GetSubMenu(hMenu,MenuIndexViewport); 
	SetMenuItemText(hSubMenu,IDM_VIEWPORT_NEW,LoadResStr(IDS_MNU_NEW));
	// Help
	hSubMenu = GetSubMenu(hMenu,MenuIndexHelp); 
	SetMenuItemText(hSubMenu,IDM_HELP_ABOUT,LoadResStr(IDS_MNU_ABOUT));
	}

void C4Console::UpdateNetMenu()
	{
	// Active & network hosting check
	if (!Active) return; 
	if (!Game.Network.Host || !Game.Network.Active) return;
	// Clear old
	ClearNetMenu();
	// Insert menu
	if (!InsertMenu(GetMenu(hDialog),MenuIndexHelp,MF_BYPOSITION | MF_POPUP,(UINT)CreateMenu(),LoadResStr(IDS_MNU_NET))) return;
	MenuIndexNet=MenuIndexHelp;
	MenuIndexHelp++;								
	DrawMenuBar(hDialog);
	// Update menu
	HMENU hMenu = GetSubMenu(GetMenu(hDialog),MenuIndexNet); 
	// Host
	sprintf(OSTR,LoadResStr(IDS_MNU_NETHOST),Game.Network.LocalName,Game.Network.Client.Number);
	AddMenuItem(hMenu,IDM_NET_CLIENT1+Game.Network.Client.Number,OSTR);
	// Clients
	for (C4NetworkClient *pClient=Game.Network.Clients.First; pClient; pClient=pClient->Next)
		{
		char clientname[C4MaxTitle+2]; SCopy(pClient->Name,clientname); if (pClient->OutOfSync) sprintf(clientname,"~%s",pClient->Name);
		sprintf(OSTR, LoadResStr(pClient->Active ? IDS_MNU_NETCLIENT : IDS_MNU_NETCLIENTDE),
									clientname, pClient->Number);
		AddMenuItem(hMenu,IDM_NET_CLIENT1+pClient->Number,OSTR);
		} 
	return;
	}

void C4Console::ClearNetMenu()
	{
	if (!Active) return;
	if (MenuIndexNet<0) return;
	DeleteMenu(GetMenu(hDialog),MenuIndexNet,MF_BYPOSITION);
	MenuIndexNet=-1;
	MenuIndexHelp--;
	DrawMenuBar(hDialog);
	}
