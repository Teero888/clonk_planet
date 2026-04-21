/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A viewport to each player */

#include <C4Include.h>

const int ViewportScrollSpeed=10;

LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
	// Determine viewport
	C4Viewport *cvp; 
	if (!(cvp=Game.GraphicsSystem.GetViewport(hwnd)))
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	
	// Process message
  switch (uMsg)
    {
    //---------------------------------------------------------------------------------------------------------------------------
    case WM_KEYDOWN:  
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			if (Console.Keydown(wParam)) return 0;
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			switch (wParam)
				{
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case VK_SCROLL:
					cvp->TogglePlayerLock();
					break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				case VK_DELETE:
					Console.EditCursor.Delete();		
					break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				default:
					Game.KeyboardInput(wParam);
				break;
				// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
				}
      return 0;
    //------------------------------------------------------------------------------------------------------------
		case WM_SYSKEYDOWN:
			Game.KeyboardInput(wParam,TRUE);
			return FALSE;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_DESTROY:
			sprintf(OSTR,"Viewport%i",cvp->Player+1);
			StoreWindowPosition(hwnd,OSTR,"Software\\RedWolf Design\\Clonk 4\\Console");	
			break;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_CLOSE:
			Game.GraphicsSystem.CloseViewport(hwnd);
			break;

    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_DROPFILES:
			cvp->DropFiles((HANDLE) wParam);
			break;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_USER_DROPDEF:
			Game.DropDef(lParam,cvp->ViewX+LOWORD(wParam),cvp->ViewY+HIWORD(wParam));
			break;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_SIZE:
			cvp->UpdateOutputSize();
			break;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_PAINT:
			if (Game.Halt) Game.GraphicsSystem.Execute();
			break;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_HSCROLL:
			switch (LOWORD(wParam))
				{
				case SB_THUMBTRACK: case SB_THUMBPOSITION: cvp->ViewX=HIWORD(wParam);	break;
				case SB_LINELEFT: cvp->ViewX-=ViewportScrollSpeed; break;
				case SB_LINERIGHT: cvp->ViewX+=ViewportScrollSpeed; break;
				case SB_PAGELEFT: cvp->ViewX-=cvp->ViewWdt; break;
				case SB_PAGERIGHT: cvp->ViewX+=cvp->ViewWdt; break;
				}
			cvp->Execute();
			cvp->ScrollBarsByViewPosition();
			return 0;
    //----------------------------------------------------------------------------------------------------------------------------------
		case WM_VSCROLL:
			switch (LOWORD(wParam))
				{
				case SB_THUMBTRACK: case SB_THUMBPOSITION: cvp->ViewY=HIWORD(wParam);	break;
				case SB_LINEUP: cvp->ViewY-=ViewportScrollSpeed; break;
				case SB_LINEDOWN: cvp->ViewY+=ViewportScrollSpeed; break;
				case SB_PAGEUP: cvp->ViewY-=cvp->ViewWdt; break;
				case SB_PAGEDOWN: cvp->ViewY+=cvp->ViewWdt; break;
				}
			cvp->Execute();
			cvp->ScrollBarsByViewPosition();
			return 0;
    //----------------------------------------------------------------------------------------------------------------------------------
    }

	// Viewport mouse control
	if (Game.MouseControl.IsViewport(cvp) && (Console.EditCursor.GetMode()==C4CNS_ModePlay))
		{
		switch (uMsg)
			{
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_LBUTTONDOWN: Game.MouseControl.Move(C4MC_Button_LeftDown,LOWORD(lParam),HIWORD(lParam),wParam);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_LBUTTONUP: Game.MouseControl.Move(C4MC_Button_LeftUp,LOWORD(lParam),HIWORD(lParam),wParam);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_RBUTTONDOWN: Game.MouseControl.Move(C4MC_Button_RightDown,LOWORD(lParam),HIWORD(lParam),wParam); break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_RBUTTONUP: Game.MouseControl.Move(C4MC_Button_RightUp,LOWORD(lParam),HIWORD(lParam),wParam); break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_LBUTTONDBLCLK: Game.MouseControl.Move(C4MC_Button_LeftDouble,LOWORD(lParam),HIWORD(lParam),wParam);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_RBUTTONDBLCLK: Game.MouseControl.Move(C4MC_Button_RightDouble,LOWORD(lParam),HIWORD(lParam),wParam);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_MOUSEMOVE:
				if ( Inside(LOWORD(lParam)-cvp->DrawX,0,cvp->ViewWdt-1) 
					&& Inside(HIWORD(lParam)-cvp->DrawY,0,cvp->ViewHgt-1) )
						SetCursor(NULL);
				Game.MouseControl.Move(C4MC_Button_None,LOWORD(lParam),HIWORD(lParam),wParam);	
				break;
			//----------------------------------------------------------------------------------------------------------------------------------
			}
		}
	// Console edit cursor control
	else
		{
		switch (uMsg)
			{
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_LBUTTONDOWN: Console.EditCursor.LeftButtonDown(wParam & MK_CONTROL); break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_LBUTTONUP: Console.EditCursor.LeftButtonUp();	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_RBUTTONDOWN: Console.EditCursor.RightButtonDown(wParam & MK_CONTROL);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_RBUTTONUP: Console.EditCursor.RightButtonUp(); break;
			//----------------------------------------------------------------------------------------------------------------------------------
			case WM_MOUSEMOVE: Console.EditCursor.Move(cvp->ViewX+LOWORD(lParam),cvp->ViewY+HIWORD(lParam),wParam);	break;
			//----------------------------------------------------------------------------------------------------------------------------------
			}
		}

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

C4Viewport::C4Viewport()
	{
	Default();
	}

C4Viewport::~C4Viewport()
	{
	Clear();
	}

HWND C4Viewport::CreateViewportWindow(HWND hParent, HINSTANCE hInst, const char *szCaption)
	{
  HWND hwnd = CreateWindowEx	(
          WS_EX_ACCEPTFILES, 
					C4ViewportClassName, szCaption,						
          C4ViewportWindowStyle,
          CW_USEDEFAULT,CW_USEDEFAULT,400,250,
          hParent,NULL,hInst,NULL);                

	return hwnd;
	}


BOOL C4Viewport::Init(HWND hParent, HINSTANCE hInst, int iPlayer)
	{	
	// Set Player
	if (!ValidPlr(iPlayer)) iPlayer = NO_OWNER;
	Player=iPlayer;
	// Create window
	hWnd = CreateViewportWindow( hParent, hInst,
															 (Player==NO_OWNER) ?
															 LoadResStr(IDS_CNS_VIEWPORT) :
															 Game.Players.Get(Player)->Name );
	if (!hWnd) return FALSE;
	// Position and size
	sprintf(OSTR,"Viewport%i",Player+1);
	if (!RestoreWindowPosition(hWnd,OSTR,"Software\\RedWolf Design\\Clonk 4\\Console"))
		ShowWindow(hWnd,SW_SHOWNORMAL);
	UpdateWindow(hWnd);	
	// Updates
	UpdateOutputSize();
	// Disable player lock on unowned viewports
	if (!ValidPlr(Player)) TogglePlayerLock();	
	// Draw
	Execute();
	// Success
	return TRUE;
	}

void C4Viewport::Clear()
	{
	if (hWnd) DestroyWindow(hWnd); hWnd=NULL;
	Player=NO_OWNER;
	ViewX=ViewY=ViewWdt=ViewHgt=0;
	OutX=OutY=OutWdt=OutHgt=0;
	DrawX=DrawY=0;
	Regions.Clear();
	}

void C4Viewport::DrawOverlay(C4FacetEx &cgo)
	{
	// Player fog of war
	DrawPlayerFogOfWar(cgo);
	// Player info
	DrawCursorInfo(cgo);
	DrawPlayerInfo(cgo);
	DrawMenu(cgo);
	// Game messages
  Game.Messages.Draw(cgo,Player);
	// Mouse control
	if (Game.MouseControl.IsViewport(this))
		{
		DrawMouseButtons(cgo);
		Game.MouseControl.Draw(cgo);
		}
	}

void C4Viewport::DrawCursorInfo(C4FacetEx &cgo)
	{
	C4Facet ccgo,ccgo2; 

	// Get cursor
	C4Object *cursor;
	if (!ValidPlr(Player)) return;
	if (!(cursor=Game.Players.Get(Player)->Cursor)) return;

	// Draw info
	if (Config.Graphics.ShowPlayerInfoAlways)
		if (cursor->Info) 
			{
			ccgo.Set(cgo.Surface,cgo.X+C4SymbolBorder,cgo.Y+C4SymbolBorder,3*C4SymbolSize,C4SymbolSize);
			cursor->Info->Draw(	ccgo,
													Config.Graphics.ShowPortraits, // && Game.Players.Get(Player)->CursorFlash,
													(cursor == Game.Players.Get(Player)->Captain) );
			}

	// Draw contents
	if (cursor->Contents.ObjectCount()==1)
		{
		// Single object
		ccgo.Set(cgo.Surface,cgo.X+C4SymbolBorder,cgo.Y+cgo.Hgt-C4SymbolBorder-C4SymbolSize,C4SymbolSize,C4SymbolSize);
		cursor->Contents.GetObject()->DrawPicture(ccgo,FALSE,SetRegions);
		}
	else
		{
		// Object list
		ccgo.Set(cgo.Surface,cgo.X+C4SymbolBorder,cgo.Y+cgo.Hgt-C4SymbolBorder-C4SymbolSize,7*C4SymbolSize,C4SymbolSize);
		cursor->Contents.DrawIDList(ccgo,-1,Game.Defs,C4D_All,SetRegions,COM_Contents,FALSE);
		}

	// Draw energy levels
	if (cursor->ViewEnergy || Config.Graphics.ShowPlayerInfoAlways)
		if (cgo.Hgt>2*C4SymbolSize+2*C4SymbolBorder)
			{
			int cx = C4SymbolBorder;
			// Energy
			ccgo.Set(cgo.Surface,cgo.X+cx,cgo.Y+C4SymbolSize+C4SymbolBorder,C4SymbolBorder-1,cgo.Hgt-2*C4SymbolBorder-2*C4SymbolSize);
			cursor->DrawEnergy(ccgo); cx+=C4SymbolBorder;
			// Magic energy
			ccgo.Set(cgo.Surface,cgo.X+cx,cgo.Y+C4SymbolSize+C4SymbolBorder,C4SymbolBorder-1,cgo.Hgt-2*C4SymbolBorder-2*C4SymbolSize);
			if (cursor->MagicEnergy) 
				{ cursor->DrawMagicEnergy(ccgo); cx+=C4SymbolBorder; }
			// Breath
			ccgo.Set(cgo.Surface,cgo.X+cx,cgo.Y+C4SymbolSize+C4SymbolBorder,C4SymbolBorder-1,cgo.Hgt-2*C4SymbolBorder-2*C4SymbolSize);
			if (cursor->Breath && (cursor->Breath<cursor->GetPhysical()->Breath))
				{ cursor->DrawBreath(ccgo); cx+=C4SymbolBorder; }
			}

	// Draw commands
	if (Config.Graphics.ShowCommands || Game.MouseControl.IsViewport(this))
		if (cgo.Hgt>C4SymbolSize)
			{
			int iSize = 24;//2*C4SymbolSize/3;
			int iSize2 = 30;//4*C4SymbolSize/9;
			// Primary area (bottom)
			ccgo.Set(cgo.Surface,cgo.X,cgo.Y+cgo.Hgt-iSize,cgo.Wdt,iSize);
			// Secondary area (side)
			ccgo2.Set(cgo.Surface,cgo.X+cgo.Wdt-iSize2,cgo.Y,iSize2,cgo.Hgt-iSize-5);
			// Draw commands
			cursor->DrawCommands(ccgo,ccgo2,SetRegions);
			}

	}
		
void C4Viewport::DrawMenu(C4FacetEx &cgo)
	{

	// Get player
	C4Player *pPlr = Game.Players.Get(Player); 

	// Player eliminated
	if (pPlr && pPlr->Eliminated) 
		{
		sprintf(OSTR,LoadResStr(pPlr->Surrendered ? IDS_PLR_SURRENDERED :	IDS_PLR_ELIMINATED),pPlr->Name);
		Engine.DDraw.TextOut(OSTR,cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+2*cgo.Hgt/3,FRed,FBlack,ACenter);
		return;
		}

	// Player cursor object menu
	if (pPlr && pPlr->Cursor)	pPlr->Cursor->Menu && pPlr->Cursor->Menu->Draw(cgo,SetRegions,ResetMenuPositions);
	// Player menu
	if (pPlr)	pPlr->Menu.Draw(cgo,SetRegions,ResetMenuPositions);
	// Fullscreen menu
	FullScreen.Menu.Draw(cgo,SetRegions,ResetMenuPositions);

	// Flag done
	ResetMenuPositions=FALSE;

	}
		
extern int iLastControlSize,iPacketDelay,ScreenRate;
extern int ControlQueueSize,ControlQueueDataSize;

void C4Viewport::Draw()
	{

	// Current graphics output
	C4FacetEx cgo;
	cgo.Set(Engine.DDraw.lpBack,DrawX,DrawY,ViewWdt,ViewHgt,ViewX,ViewY);

	// Draw Landscape (draw before setting clippers)
	Game.Landscape.Draw(cgo);

  // Set clippers
	Engine.DDraw.SetPrimaryClipper(DrawX,DrawY,DrawX+ViewWdt-1,DrawY+ViewHgt-1); 

	// Draw game
	Game.Draw(cgo);

	// Draw overlay
	if (!Application.Fullscreen) Console.EditCursor.Draw(cgo);
  DrawOverlay(cgo);

	// Netstats
	if (!Application.Fullscreen)
		if (Game.GraphicsSystem.ShowNetstatus)
			{
			sprintf(OSTR,"%s|Behind %i ms|ScreenRate %i|ControlRate %i|Input %i Bytes|Control %i Bytes|Game speed %i FPS|Control queue %i/%i",
				Game.AsynchronousControl ? "Asynchronous" : "Synchronous",
									 Game.ControlTimeBehind,ScreenRate,Game.ControlRate,
									 Game.Input.Size,iLastControlSize,
									 Game.FPS,
									 ControlQueueSize,ControlQueueDataSize);
			Engine.DDraw.TextOut(OSTR,cgo.Surface,cgo.X+20,cgo.Y+60,FWhite,FBlack);
			}

  // Remove clippers  
  Engine.DDraw.NoPrimaryClipper();

	}

void C4Viewport::BlitOutput()
	{
	if (hWnd)
		Engine.DDraw.BlitSurface2Window(
			Engine.DDraw.lpBack,DrawX,DrawY,ViewWdt,ViewHgt,
			hWnd,OutX,OutY,ViewWdt,ViewHgt);
	}

void C4Viewport::Execute()
	{
	// Update regions
	static int RegionUpdate=0;
	SetRegions=NULL;
	RegionUpdate++;
	if (RegionUpdate>=5)
		{
		RegionUpdate=0;
		Regions.Clear(); 
		Regions.SetAdjust(-OutX,-OutY);
		SetRegions=&Regions;
		}
	// Adjust position
	AdjustPosition();
	// Draw
	Draw();
	// Video record & status (developer mode, first player viewport)
	if (!Application.Fullscreen)
		if (Player==0 && (this==Game.GraphicsSystem.GetViewport(0)))
			Game.GraphicsSystem.Video.Execute();	
	// Blit output
	BlitOutput();
	}

void C4Viewport::AdjustPosition()
	{

	// View size
	ViewWdt = BoundBy(OutWdt,0,GBackWdt);
	ViewHgt = BoundBy(OutHgt,0,GBackHgt);
	// View size safety for SBack
	ViewWdt = BoundBy(ViewWdt,0,MaxResX);
	ViewHgt = BoundBy(ViewHgt,0,MaxResY);

	// View position
	if (PlayerLock && ValidPlr(Player))
		{
		int iScrollRange = Min(ViewWdt/10,ViewHgt/10);
		if (Game.Players.Get(Player)->ViewMode==C4PVM_Scrolling) iScrollRange=0;
		int iPlrViewX = Game.Players.Get(Player)->ViewX - ViewWdt/2;
		int iPlrViewY = Game.Players.Get(Player)->ViewY - ViewHgt/2;
		ViewX = BoundBy( ViewX, iPlrViewX-iScrollRange, iPlrViewX+iScrollRange );
		ViewY = BoundBy( ViewY, iPlrViewY-iScrollRange, iPlrViewY+iScrollRange );
		}
	ViewX = BoundBy(ViewX,0,GBackWdt-ViewWdt);
	ViewY = BoundBy(ViewY,0,GBackHgt-ViewHgt);

	}

BOOL C4Viewport::DropFiles(HANDLE hDrop)
	{

	if (!Console.Editing) { Console.Message(LoadResStr(IDS_CNS_NONETEDIT)); return FALSE; }

	int iFileNum = DragQueryFile((HDROP)hDrop,0xFFFFFFFF,NULL,NULL);
	POINT pntPoint;
	char szFilename[500+1];
	for (int cnt=0; cnt<iFileNum; cnt++)
		{
		DragQueryFile((HDROP)hDrop,cnt,szFilename,500);
		DragQueryPoint((HDROP)hDrop,&pntPoint);
		Game.DropFile(szFilename,ViewX+pntPoint.x,ViewY+pntPoint.y);
		}
	DragFinish((HDROP)hDrop);
	return TRUE;
	}

void UpdateWindowLayout(HWND hwnd)
	{
	BOOL fMinimized = IsIconic(hwnd);
	BOOL fMaximized = IsZoomed(hwnd);
	RECT rect;
	GetWindowRect(hwnd,&rect);
	MoveWindow(hwnd,rect.left,rect.top,rect.right-rect.left-1,rect.bottom-rect.top,TRUE);
	MoveWindow(hwnd,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,TRUE);	
	}

BOOL C4Viewport::TogglePlayerLock()
	{
	// Disable player lock
	if (PlayerLock)
		{
		PlayerLock=FALSE;
		SetWindowLong(hWnd,GWL_STYLE,C4ViewportWindowStyle | WS_HSCROLL | WS_VSCROLL);
		UpdateWindowLayout(hWnd);
		ScrollBarsByViewPosition();
		}
	// Enable player lock
	else if (ValidPlr(Player)) 
		{
		PlayerLock=TRUE;
		SetWindowLong(hWnd,GWL_STYLE,C4ViewportWindowStyle);
		UpdateWindowLayout(hWnd);
		}
	return TRUE;
	}

BOOL C4Viewport::ViewPositionByScrollBars()
	{
	if (PlayerLock) return FALSE;
	SCROLLINFO scroll;
	scroll.cbSize=sizeof(SCROLLINFO);
	// Vertical
	scroll.fMask=SIF_POS;
	GetScrollInfo(hWnd,SB_VERT,&scroll);
	ViewY=scroll.nPos;
	// Horizontal
	scroll.fMask=SIF_POS;
	GetScrollInfo(hWnd,SB_HORZ,&scroll);
	ViewX=scroll.nPos;	
	return TRUE;
	}

BOOL C4Viewport::ScrollBarsByViewPosition()
	{
	if (PlayerLock) return FALSE;
	SCROLLINFO scroll;
	scroll.cbSize=sizeof(SCROLLINFO);
	// Vertical
	scroll.fMask=SIF_ALL;
	scroll.nMin=0;
	scroll.nMax=GBackHgt;
	scroll.nPage=OutHgt;
	scroll.nPos=ViewY;
	SetScrollInfo(hWnd,SB_VERT,&scroll,TRUE);
	// Horizontal
	scroll.fMask=SIF_ALL;
	scroll.nMin=0;
	scroll.nMax=GBackWdt;
	scroll.nPage=OutWdt;
	scroll.nPos=ViewX;
	SetScrollInfo(hWnd,SB_HORZ,&scroll,TRUE);
	return TRUE;
	}


BOOL C4Viewport::UpdateOutputSize()
	{
	if (!hWnd) return FALSE;
	// Output size
	RECT rect;
	if (!GetClientRect(hWnd,&rect)) return FALSE;
	OutX=rect.left; OutY=rect.top; 
	OutWdt=rect.right-rect.left; OutHgt=rect.bottom-rect.top;
	// Scroll bars
	ScrollBarsByViewPosition();
	// Reset menus
	ResetMenuPositions=TRUE;
	// Done
	return TRUE;
	}

void C4Viewport::Default()
	{
	hWnd=NULL;
	Player=0;
	ViewX=ViewY=ViewWdt=ViewHgt=0;
	OutX=OutY=OutWdt=OutHgt=0;
	DrawX=DrawY=0;
	Next=NULL;
	PlayerLock=TRUE;
	ResetMenuPositions=FALSE;
	SetRegions=NULL;
	Regions.Default();
	}

void C4Viewport::DrawPlayerInfo(C4FacetEx &cgo)
	{
	C4Facet ccgo; 
	if (!ValidPlr(Player)) return;

	// Wealth
	if (Game.Players.Get(Player)->ViewWealth || Config.Graphics.ShowPlayerInfoAlways)
		{
		int wdt = C4SymbolSize;
		int hgt = C4SymbolSize/2;
		ccgo.Set(cgo.Surface,
						 cgo.X+cgo.Wdt-wdt-C4SymbolBorder,
						 cgo.Y+C4SymbolBorder,
						 wdt,hgt);
		Game.GraphicsResource.fctWealthSymbol.DrawValue(ccgo,Game.Players.Get(Player)->Wealth);
		}
	
	// Value gain
	if ( (Game.C4S.Game.ValueGain && Game.Players.Get(Player)->ViewValue)
		|| Config.Graphics.ShowPlayerInfoAlways)
			{
			int wdt = C4SymbolSize;
			int hgt = C4SymbolSize/2;
			ccgo.Set(cgo.Surface,
							 cgo.X+cgo.Wdt-2*wdt-2*C4SymbolBorder,
							 cgo.Y+C4SymbolBorder,
							 wdt,hgt);
			Game.GraphicsResource.fctValueSymbol.DrawValue(ccgo,Game.Players.Get(Player)->ValueGain);
			}

	// Crew
	if (Config.Graphics.ShowPlayerInfoAlways)
		{
		int wdt = C4SymbolSize;
		int hgt = C4SymbolSize/2;
		ccgo.Set(cgo.Surface,
						 cgo.X+cgo.Wdt-3*wdt-3*C4SymbolBorder,
						 cgo.Y+C4SymbolBorder,
						 wdt,hgt);
		Game.GraphicsResource.fctCrew.DrawValue2(ccgo,Game.Players.Get(Player)->SelectCount,Game.Players.Get(Player)->Crew.ObjectCount(),Game.Players.Get(Player)->Color);
		}

	// Controls
	DrawPlayerControls(cgo);
	DrawPlayerStartup(cgo);

	}

BOOL C4Viewport::Init(int iPlayer)
	{
	// Set Player
	if (!ValidPlr(iPlayer)) iPlayer = NO_OWNER;
	Player=iPlayer;
	return TRUE;
	}

const char *VKeyName(WORD vkey)
	{
	static char name[20+1];
	SCopy("Undefined",name);
	if (Inside(vkey,1,47))
		SCopySegment(LoadResStr(IDS_CON_VKEYSET1),vkey-1,name,'|',20);
	if (Inside(vkey,'0','9') || Inside(vkey,'A','Z'))
		{ name[0]= (char) vkey; name[1]=0; }
	if (Inside(vkey,96,145))
		SCopySegment(LoadResStr(IDS_CON_VKEYSET2),vkey-96,name,'|',20);
	if (Inside(vkey,186,192))
		SCopySegment(LoadResStr(IDS_CON_VKEYSET3),vkey-186,name,'|',20);
	if (Inside(vkey,219,226))
		SCopySegment(LoadResStr(IDS_CON_VKEYSET4),vkey-219,name,'|',20);
	return name;
	}

const char *ControlKeyName(int profile, int ccon)
	{
	return VKeyName(Config.Keyboard[profile][ccon]);
	}

BOOL SideWinderButtonNames=TRUE;

const char *GamePadKeyName(int profile, int ccon)
	{
	static char name[20+1];
	SCopySegment(LoadResStr(IDS_CON_GAMEPAD),ccon,name,'|',20);
	if (name[0]=='-')
		if (SideWinderButtonNames)
			SCopySegment(LoadResStr(IDS_CON_SIDEWINDER),Config.Gamepad.Button[ccon]-1,name,'|',20);
		else
			sprintf(name,"[%d]",Config.Gamepad.Button[ccon]);
	return name;
	}

const char *PlrControlKeyName(int iPlayer, int iControl)
	{	
	if (Inside(Game.Players.Get(iPlayer)->Control,C4P_Control_Keyboard1,C4P_Control_Keyboard4))
		return ControlKeyName(Game.Players.Get(iPlayer)->Control,iControl);
	if (Inside(Game.Players.Get(iPlayer)->Control,C4P_Control_GamePad1,C4P_Control_GamePad1))
		return GamePadKeyName(Game.Players.Get(iPlayer)->Control-C4P_Control_GamePad1,iControl);
	return NULL;
	}

void C4Viewport::DrawPlayerControls(C4FacetEx &cgo)
	{
	if (!ValidPlr(Player)) return;
	if (!Game.Players.Get(Player)->ShowControl) return;
	int size = Min( cgo.Wdt/3, 7*cgo.Hgt/24 );
	int tx = cgo.X+cgo.Wdt/2-size/2;
	int ty = cgo.Y+15;
	if (Game.Players.Get(Player)->ShowControlPos == 1)
		{
		// Position 1: bottom right corner
		tx = cgo.X+cgo.Wdt-size-15;
		ty = cgo.Y+cgo.Hgt/2-size/2;
		}

	int iShowCtrl = Game.Players.Get(Player)->ShowControl;
	int iLastCtrl = Com2Control(Game.Players.Get(Player)->LastCom);
	int scwdt=size/3,schgt=size/4;
	BOOL showtext;

	const C4MaxShowControl = 10;

	for (int iCtrl=0; iCtrl<C4MaxShowControl; iCtrl++)
		if (iShowCtrl & (1<<iCtrl))
			{
			showtext= iShowCtrl & (1<<(iCtrl+C4MaxShowControl)) ;
      if (iShowCtrl & (1<<(iCtrl+2*C4MaxShowControl)))
				if (Tick35>18) showtext=FALSE;
			C4Facet ccgo;
			ccgo.Set(cgo.Surface,tx+scwdt*(iCtrl%3),ty+schgt*(iCtrl/3),scwdt,schgt);
			DrawControlKey(ccgo,iCtrl,(iLastCtrl==iCtrl) ? 1 : 0,
				showtext ? PlrControlKeyName(Player,iCtrl) : NULL);
			}
	}

void C4Viewport::DrawPlayerStartup(C4FacetEx &cgo)
	{
	C4Player *pPlr;
	if (!(pPlr = Game.Players.Get(Player))) return;
	if (!pPlr->LocalControl || !pPlr->ShowStartup) return;
	int iNameHgtOff=0;

	// Control
	if (pPlr->MouseControl)
		GfxR->fctMouse.Draw(cgo.Surface,
												cgo.X+(cgo.Wdt-GfxR->fctKeyboard.Wdt)/2+55,
												cgo.Y+cgo.Hgt*2/3-10,
												0,0);
	if (Inside(pPlr->Control,C4P_Control_Keyboard1,C4P_Control_GamePad1))
		{
		GfxR->fctKeyboard.Draw(cgo.Surface,
													 cgo.X+(cgo.Wdt-GfxR->fctKeyboard.Wdt)/2,
													 cgo.Y+cgo.Hgt*2/3,								
													 pPlr->Control,0);
		iNameHgtOff=GfxR->fctKeyboard.Hgt;
		}

	// Name
	Engine.DDraw.TextOut(pPlr->Name,cgo.Surface,
				cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt*2/3+iNameHgtOff+3,
				FPlayer + pPlr->Color, FBlack, ACenter);

	}

void C4Viewport::SetOutputSize(int iDrawX, int iDrawY, int iOutX, int iOutY, int iOutWdt, int iOutHgt)
	{
	DrawX=iDrawX; DrawY=iDrawY;
	OutX=iOutX; OutY=iOutY;
	OutWdt=iOutWdt; OutHgt=iOutHgt;
	// Reset menus
	ResetMenuPositions=TRUE;
	}

void C4Viewport::ClearPointers(C4Object *pObj)
	{
	Regions.ClearPointers(pObj);
	}

void C4Viewport::DrawMouseButtons(C4FacetEx &cgo)
	{
	C4Facet ccgo;
	C4Region rgn;
	int iSymbolSize = 4*C4SymbolSize/9;
	// Help
	ccgo.Set(cgo.Surface,cgo.X+cgo.Wdt-iSymbolSize,cgo.Y+C4SymbolSize+2*C4SymbolBorder,iSymbolSize,iSymbolSize);
	GfxR->fctKey.Draw(ccgo);
	GfxR->fctOKCancel.Draw(ccgo,TRUE,0,1);
	if (SetRegions) {	rgn.Default(); rgn.Set(ccgo,LoadResStr(IDS_CON_HELP)); rgn.Com=COM_Help; SetRegions->Add(rgn); }
	// Player menu
	ccgo.Y+=iSymbolSize;
	GfxR->fctKey.Draw(ccgo);
	GfxR->fctOKCancel.Draw(ccgo,TRUE,1,1);
	if (SetRegions) {	rgn.Default(); rgn.Set(ccgo,LoadResStr(IDS_CON_PLAYERMENU)); rgn.Com=COM_PlayerMenu; SetRegions->Add(rgn); }
	}

void C4Viewport::DrawPlayerFogOfWar(C4FacetEx &cgo)
	{
	C4Player *pPlr=Game.Players.Get(Player);
	if (!pPlr || !pPlr->FogOfWar) return;
	pPlr->FogOfWar->Draw(cgo);
	}
