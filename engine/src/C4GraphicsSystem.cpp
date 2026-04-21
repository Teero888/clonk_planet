/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Operates viewports, message board and draws the game */

#include <C4Include.h>

LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

volatile BOOL C4GraphicsSystem::GraphicsGo;

C4GraphicsSystem::C4GraphicsSystem()
	{
	fViewportClassRegistered=FALSE;
	Default();
	}

C4GraphicsSystem::~C4GraphicsSystem()
	{
	Clear();
	}

BOOL C4GraphicsSystem::RegisterViewportClass(HINSTANCE hInst)
	{
  WNDCLASSEX WndClass;
	WndClass.cbSize=sizeof(WNDCLASSEX);
  WndClass.style         = CS_DBLCLKS | CS_BYTEALIGNCLIENT;
  WndClass.lpfnWndProc   = ViewportWinProc;
  WndClass.cbClsExtra    = 0;		 
  WndClass.cbWndExtra    = 0;	 
  WndClass.hInstance     = hInst;              
  WndClass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  WndClass.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
  WndClass.lpszMenuName  = NULL;
  WndClass.lpszClassName = C4ViewportClassName;
	WndClass.hIcon         = LoadIcon (hInst, MAKEINTRESOURCE (IDI_01_C4S) );
  WndClass.hIconSm       = LoadIcon (hInst, MAKEINTRESOURCE (IDI_01_C4S) );  
	return RegisterClassEx(&WndClass);
	}

BOOL C4GraphicsSystem::Init()
	{
	// Register viewport class
	if (!fViewportClassRegistered)
		if (!RegisterViewportClass(Application.hInstance)) 
			return FALSE;
	fViewportClassRegistered=TRUE;
	// Init video module
	if (Config.Graphics.VideoModule)
		Video.Init(Engine.DDraw.lpBack);
	// Success
	return TRUE;
	}

void C4GraphicsSystem::Clear()
	{
	// Clear message board
	MessageBoard.Clear();
	// Clear loader
	fctLoader.Clear();
	// Close viewports
	C4Viewport *next;
	while (FirstViewport) 
		{
		next=FirstViewport->Next;
		delete FirstViewport;
		FirstViewport=next;
		}
	FirstViewport=NULL;
	// Clear video system
	Video.Clear();
	}

BOOL C4GraphicsSystem::SetPalette()
	{
	// Set primary palette by game palette
  if (!Engine.DDraw.SetPrimaryPalette(Game.GraphicsResource.GamePalette)) return FALSE;
	return TRUE;
	}

extern int iLastControlSize,iPacketDelay;
extern int ControlQueueSize,ControlQueueDataSize;

int ScreenTick=0, ScreenRate=1;

void C4GraphicsSystem::Execute()
	{

	// Execution timing flag
	GraphicsGo=FALSE;

	// Message board
	MessageBoard.Execute();

	// If lobby running, message board only (page flip done by startup message board)
	if (Game.Network.Lobby || Game.Network.WaitingForLobby) return;

	// Adjust screen rate by behind value
	if (Game.AsynchronousControl && Game.Network.Active)
		ScreenRate = BoundBy( 1 + Game.ControlTimeBehind/250, 1, 25 );
	else
		ScreenRate = 1;

	// Screen rate skip frame draw
	ScreenTick++; if (ScreenTick>=ScreenRate) ScreenTick=0;
	if (Game.AsynchronousControl && Game.Network.Active)
		if (ScreenTick) 
			return;

	// Background redraw
	if (Application.Fullscreen)
		if (RedrawBackground)
			DrawFullscreenBackground();

	// Reset object audibility
	Game.Objects.ResetAudibility();

	// Viewports
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)	
		cvp->Execute();

	// Debug
	if (Application.Fullscreen)
		if (Game.GraphicsSystem.ShowNetstatus)
			{
			sprintf(OSTR,"%s|Behind %i ms|ScreenRate %i|ControlRate %i|Input %i Bytes|Control %i Bytes|Game speed %i FPS|Control queue %i/%i",
				Game.AsynchronousControl ? "Asynchronous" : "Synchronous",
									 Game.ControlTimeBehind,ScreenRate,Game.ControlRate,
									 Game.Input.Size,iLastControlSize,
									 Game.FPS,
									 ControlQueueSize,ControlQueueDataSize);
			Engine.DDraw.TextOut(OSTR,Engine.DDraw.lpBack,20,40,FWhite,FBlack);
			}

	// Messages
	DrawHoldMessages();
	DrawFlashMessage();

	// Palette update
	if (fSetPalette) { SetPalette(); /*SetDarkColorTable();*/ }
	fSetPalette=FALSE;

	// Video record & status (fullsrceen)
	if (Application.Fullscreen)
		Video.Execute();
	
	// Fullscreen page flip
	if (Application.Fullscreen)
		Engine.DDraw.PageFlip();

	}

BOOL C4GraphicsSystem::CloseViewport(HWND hwnd)
	{
	// Close all matching viewports
	int iLastCount = GetViewportCount();
	C4Viewport *next,*prev=NULL;
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=next)
		{
		next=cvp->Next;
		if (cvp->hWnd==hwnd)
			{			
			delete cvp;	
			if (prev) prev->Next=next; 
			else FirstViewport=next;	
			}
		else
			prev=cvp;
		}
	// Recalculate viewports
	RecalculateViewports();
	// Action sound
	if (GetViewportCount()!=iLastCount) SoundEffect("CloseViewport");
	// Done
	return TRUE;
	}

BOOL C4GraphicsSystem::CreateViewport(int iPlayer)
	{
	// Create and init new viewport, add to viewport list
	int iLastCount = GetViewportCount();
	C4Viewport *nvp = new C4Viewport;
	BOOL fOkay = FALSE;
	if (Application.Fullscreen) 
		fOkay = nvp->Init(iPlayer);
	else 
		fOkay = nvp->Init(Application.hWindow,Application.hInstance,iPlayer); 
	if (!fOkay)	{ delete nvp; return FALSE; }
	for (C4Viewport *pLast=FirstViewport; pLast && pLast->Next; pLast=pLast->Next);
	if (pLast) pLast->Next=nvp; else FirstViewport=nvp;
	// Recalculate viewports
	RecalculateViewports();
	// Action sound
	if (GetViewportCount()!=iLastCount) SoundEffect("CloseViewport");
	return TRUE;
	}

C4Viewport* C4GraphicsSystem::GetViewport(HWND hwnd)
	{
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)
		if (cvp->hWnd==hwnd)
			return cvp;
	return NULL;
	}

void C4GraphicsSystem::ClearPointers(C4Object *pObj)
	{
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)
		cvp->ClearPointers(pObj);
	}

void C4GraphicsSystem::Default()
	{
	MessageBoard.Default();
	FirstViewport=NULL;
	RedrawBackground=TRUE;
	ViewportArea.Default();
	fctLoader.Default();
	ShowVertices=FALSE;
	ShowAction=FALSE;
	ShowCommand=FALSE;
	ShowEntrance=FALSE;
	ShowPathfinder=FALSE;
	ShowNetstatus=FALSE;
	ShowSolidMask=FALSE;
	GraphicsGo=FALSE;
	FlashMessageText[0]=0;
	FlashMessageTime=0; FlashMessageX=FlashMessageY=0;
	fSetPalette=FALSE;
	Video.Default();
	}

void C4GraphicsSystem::DrawFullscreenBackground()
	{
	Engine.DDraw.BlitSurfaceTile(Game.GraphicsResource.fctBackground.Surface,Engine.DDraw.lpPrimary,0,0,Config.Graphics.ResX,Config.Graphics.ResY);
	Engine.DDraw.BlitSurfaceTile(Game.GraphicsResource.fctBackground.Surface,Engine.DDraw.lpBack,0,0,Config.Graphics.ResX,Config.Graphics.ResY);
	RedrawBackground=FALSE;
	}

void OnSurfaceRestore()
	{
	Game.GraphicsSystem.RedrawBackground=TRUE;
	}

BOOL C4GraphicsSystem::InitLoaderScreen(const char *szGroup)
	{
  int iLoaders;
  C4Group hGroup;
	C4Facet cgo;
  if (hGroup.Open(szGroup))
    {
    iLoaders = hGroup.EntryCount("Loader*.bmp"); 
		sprintf(OSTR,"Loader%d.bmp",1+SafeRandom(iLoaders));
		if (fctLoader.Load(hGroup,OSTR))
			{
      Engine.DDraw.SetPrimaryPalette(fctLoader.GetPalette());
			cgo.Set(Engine.DDraw.lpPrimary,0,0,Config.Graphics.ResX,Config.Graphics.ResY); 
			fctLoader.Draw(cgo,FALSE);
			hGroup.Close();
			return TRUE;
			}
    hGroup.Close();
    }
	return FALSE;
	}

void C4GraphicsSystem::ColorAnimation()
	{
	// Color animation not enabled
	if (!Config.Graphics.ColorAnimation) return;
	// Get game palette
	BYTE *pPalette = Game.GraphicsResource.GamePalette;
	// Only every tick 10
	if (!Tick10)
		{
		// Material color animation
		for (int mat=0; mat<Game.Material.Num; mat++)
			if (Game.Material.Map[mat].ColorAnimation)
				{
				int mcol = GBM + C4M_ColsPerMat * mat;
				BYTE buf[3];
				MemCopy( pPalette+3*(mcol+0), buf, 3);
				MemCopy( pPalette+3*(mcol+1), pPalette+3*(mcol+0), 6);
				MemCopy( buf, pPalette+3*(mcol+2), 3);
				MemCopy( pPalette+3*(mcol+IFT+0), buf, 3);
				MemCopy( pPalette+3*(mcol+IFT+1), pPalette+3*(mcol+IFT+0), 6);
				MemCopy( buf, pPalette+3*(mcol+IFT+2), 3);
				}
		// Force field color animation
		pPalette[3*191+0]=0;
		pPalette[3*191+1]=0;
		pPalette[3*191+2]=128+SafeRandom(128);
		// Flag palette update
		fSetPalette=TRUE;
		}
	}

BOOL C4GraphicsSystem::CloseViewport(int iPlayer)
	{
	// Close all matching viewports
	int iLastCount = GetViewportCount();
	C4Viewport *next,*prev=NULL;
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=next)
		{
		next=cvp->Next;
		if (cvp->Player==iPlayer)
			{			
			delete cvp;	
			if (prev) prev->Next=next; 
			else FirstViewport=next;	
			}
		else
			prev=cvp;
		}
	// Recalculate viewports
	RecalculateViewports();
	// Action sound
	if (GetViewportCount()!=iLastCount) SoundEffect("CloseViewport");
	return TRUE;
	}

void C4GraphicsSystem::RecalculateViewports()
	{

	// Fullscreen only
	if (!Application.Fullscreen) return;

	// Sort viewports
	SortViewportsByPlayerControl();

	// Viewport area
	ViewportArea.Set(Engine.DDraw.lpBack,0,0,
									 Config.Graphics.ResX,
									 Config.Graphics.ResY-MessageBoard.Output.Hgt);

	// Redraw flag
	RedrawBackground=TRUE;

	// Viewports
	C4Viewport *cvp;
	int iViews = 0;
	for (cvp=FirstViewport; cvp; cvp=cvp->Next) iViews++;
	if (!iViews) return;
	int iViewsH = (int) sqrt(iViews);
	int iViewsX = iViews / iViewsH;
	int iViewsL = iViews % iViewsH;
	int cViewH,cViewX,ciViewsX;
	int cViewWdt,cViewHgt,cOffWdt,cOffHgt,cOffX,cOffY;
	cvp=FirstViewport;
	for (cViewH=0; cViewH<iViewsH; cViewH++)
		{
		ciViewsX = iViewsX;	if (cViewH<iViewsL) ciViewsX++;
		for (cViewX=0; cViewX<ciViewsX; cViewX++)
			{
			cViewWdt = Min(ViewportArea.Wdt/ciViewsX,GBackWdt);
			cViewHgt = Min(ViewportArea.Hgt/iViewsH,GBackHgt);
			cOffX = ViewportArea.X;
			cOffY = ViewportArea.Y;
			cOffWdt=cOffHgt=0;
			if (ciViewsX*cViewWdt<ViewportArea.Wdt)
				cOffX=(ViewportArea.Wdt-ciViewsX*cViewWdt)/2;
			if (iViewsH*cViewHgt<ViewportArea.Hgt)
				cOffY=(ViewportArea.Hgt-iViewsH*cViewHgt)/2;
			if (Config.Graphics.SplitscreenDividers) 
				{
				if (cViewX<ciViewsX-1) cOffWdt=4;
				if (cViewH<iViewsH-1) cOffHgt=4;
				}
			cvp->SetOutputSize(cOffX+cViewX*cViewWdt,cOffY+cViewH*cViewHgt,
												 cOffX+cViewX*cViewWdt,cOffY+cViewH*cViewHgt,
												 cViewWdt-cOffWdt,cViewHgt-cOffHgt);
			cvp=cvp->Next;
			}
		}

	}

int C4GraphicsSystem::GetViewportCount()
	{
	int iResult = 0;
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)	iResult++;
	return iResult;
	}

C4Viewport* C4GraphicsSystem::GetViewport(int iPlayer)
	{
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)
		if (cvp->Player==iPlayer)
			return cvp;
	return NULL;
	}

int LayoutOrder(int iControl)
	{
	// Convert keyboard control index to keyboard layout order
	switch (iControl)
		{
		case C4P_Control_Keyboard1: return 0;
		case C4P_Control_Keyboard2: return 3;
		case C4P_Control_Keyboard3: return 1;
		case C4P_Control_Keyboard4: return 2;
		}
	return iControl;
	}

void C4GraphicsSystem::SortViewportsByPlayerControl()
	{

	// Sort
	BOOL fSorted;
	C4Player *pPlr1,*pPlr2;
	C4Viewport *pView,*pNext,*pPrev;
	do
		{
		fSorted = TRUE;
		for (pPrev=NULL,pView=FirstViewport; pView && (pNext = pView->Next); pView=pNext)	
			{
			// Get players
			pPlr1 = Game.Players.Get(pView->Player);
			pPlr2 = Game.Players.Get(pNext->Player);
			// Swap order
			if (pPlr1 && pPlr2 && ( LayoutOrder(pPlr1->Control) > LayoutOrder(pPlr2->Control) ))
				{
				if (pPrev) pPrev->Next = pNext; else FirstViewport = pNext;
				pView->Next = pNext->Next;
				pNext->Next = pView;
				pPrev = pNext;
				pNext = pView;
				fSorted = FALSE;
				}
			// Don't swap
			else
				{
				pPrev = pView;
				}
			}
		}
	while (!fSorted);

	}

void C4GraphicsSystem::MouseMove(int iButton, int iX, int iY, WORD wKeyParam)
	{	
	// Pass on to mouse controlled viewport
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)
		if (Game.MouseControl.IsViewport(cvp))
			Game.MouseControl.Move( iButton, 
															BoundBy(iX-cvp->OutX,0,cvp->ViewWdt-1), 
															BoundBy(iY-cvp->OutY,0,cvp->ViewHgt-1), 
															wKeyParam );
	}

BOOL C4GraphicsSystem::SaveScreenshot()
	{
	// Fullscreen only
	if (!Application.Fullscreen) return FALSE;
	// Filename
	char szFilename[_MAX_PATH+1];
	int iScreenshotIndex=1;
	do
		{
		sprintf(szFilename,"Screenshot%03i.bmp",iScreenshotIndex++);
		}
	while (FileExists(szFilename));
	// Save primary surface
	if (!SaveSurface(szFilename,Engine.DDraw.lpBack,Game.GraphicsResource.GamePalette))
		Log(LoadResStr(IDS_PRC_SCREENSHOTERROR));
	else
		Log(LoadResStr(IDS_PRC_SCREENSHOT));
	return TRUE;
	}

void C4GraphicsSystem::DeactivateDebugOutput()
	{
	ShowVertices=FALSE;
	ShowAction=FALSE;
	ShowCommand=FALSE;
	ShowEntrance=FALSE;
	ShowPathfinder=FALSE;
	}

void C4GraphicsSystem::DrawHoldMessages()
	{
	if (Application.Fullscreen)
		{
		if (FullScreen.HoldAbort) 
			Engine.DDraw.TextOut(LoadResStr(IDS_HOLD_ABORT),Engine.DDraw.lpBack,Config.Graphics.ResX/2,Config.Graphics.ResY/2,FWhite,FBlack,ACenter);
		if (FullScreen.HoldGameOver) 
			Engine.DDraw.TextOut(LoadResStr(IDS_HOLD_GAMEOVER),Engine.DDraw.lpBack,Config.Graphics.ResX/2,Config.Graphics.ResY/2,FWhite,FBlack,ACenter);
		}
	}

BYTE FindPaletteColor(BYTE *bypPalette, int iRed, int iGreen, int iBlue)
	{
	int iClosest=0;
	for (int iColor=1; iColor<256; iColor++)
		if (Abs(bypPalette[iColor*3+0]-iRed)+Abs(bypPalette[iColor*3+1]-iGreen)+Abs(bypPalette[iColor*3+2]-iBlue)
			< Abs(bypPalette[iClosest*3+0]-iRed)+Abs(bypPalette[iClosest*3+1]-iGreen)+Abs(bypPalette[iClosest*3+2]-iBlue) )
				iClosest = iColor;
	return iClosest;
	}

void C4GraphicsSystem::SetDarkColorTable()
	{
	const int iDarkening=80;
	// Using GamePalette
	BYTE *bypPalette = Game.GraphicsResource.GamePalette;
	for (int iColor=0; iColor<256; iColor++)
		DarkColorTable[iColor]=FindPaletteColor(bypPalette,Max(bypPalette[iColor*3+0]-iDarkening,0),Max(bypPalette[iColor*3+1]-iDarkening,0),Max(bypPalette[iColor*3+2]-iDarkening,0));
	}

// FlashMessage is not designed for multi-viewports (status text mix / border gfx trash)

void C4GraphicsSystem::FlashMessage(const char *szMessage)
	{
	SCopy(szMessage,FlashMessageText,C4MaxTitle);
	FlashMessageTime=SLen(FlashMessageText)*2;
	FlashMessageX=-1; FlashMessageY=5;
	}

void C4GraphicsSystem::DrawFlashMessage()
	{
	if (!FlashMessageTime) return;
	if (!Application.Fullscreen) return;
	Engine.DDraw.TextOut(FlashMessageText,Engine.DDraw.lpBack,
											 (FlashMessageX==-1) ? Config.Graphics.ResX/2 : FlashMessageX,
											 (FlashMessageY==-1) ? Config.Graphics.ResY/2 : FlashMessageY,
											 FWhite,FBlack,
											 (FlashMessageX==-1) ? ACenter : ALeft);
	FlashMessageTime--;
	//if (!FlashMessageTime) RedrawBackground=TRUE;
	}

int C4GraphicsSystem::GetAudibility(int iX, int iY)
	{
	// Accumulate audibility by viewports
	int iAudible=0;
	for (C4Viewport *cvp=FirstViewport; cvp; cvp=cvp->Next)
		iAudible = Max( iAudible, 
										BoundBy(100-100*Distance(cvp->ViewX+cvp->ViewWdt/2,cvp->ViewY+cvp->ViewHgt/2,iX,iY)/700,0,100) );
	return iAudible;
	}
