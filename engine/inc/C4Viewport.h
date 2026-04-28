/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A viewport to each player */

class C4Viewport
	{
	friend C4GraphicsSystem;
	friend C4MouseControl;
	friend LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	public:
		C4Viewport();
		~C4Viewport();
	public:
		C4RegionList Regions;
		int ViewX,ViewY,ViewWdt,ViewHgt;
		int DrawX,DrawY;
	protected:
		HWND hWnd;
		int Player;
		BOOL PlayerLock;
		int OutX,OutY,OutWdt,OutHgt;
		BOOL ResetMenuPositions;
		C4RegionList *SetRegions;
		C4Viewport *Next;
	public:
		void Default();
		void Clear();
		void Execute();
		void ClearPointers(C4Object *pObj);
		void SetOutputSize(int iDrawX, int iDrawY, int iOutX, int iOutY, int iOutWdt, int iOutHgt);
		BOOL Init(int iPlayer);
		BOOL Init(HWND hParent, HINSTANCE hInst, int iPlayer);
		BOOL DropFiles(HANDLE hDrop);
		BOOL TogglePlayerLock();
	protected:
		void DrawPlayerFogOfWar(C4FacetEx &cgo);
		void DrawMouseButtons(C4FacetEx &cgo);
		void DrawPlayerStartup(C4FacetEx &cgo);
		void Draw();
		void DrawOverlay(C4FacetEx &cgo);
		void DrawMenu(C4FacetEx &cgo);
		void DrawCursorInfo(C4FacetEx &cgo);
		void DrawPlayerInfo(C4FacetEx &cgo);
		void DrawPlayerControls(C4FacetEx &cgo);
		void BlitOutput();
		void AdjustPosition();
		BOOL UpdateOutputSize();
		BOOL ViewPositionByScrollBars();
		BOOL ScrollBarsByViewPosition();
		HWND CreateViewportWindow(HWND hParent, HINSTANCE hInst, const char *szCaption);
	};	

#define C4ViewportClassName "C4Viewport"
#define C4ViewportWindowStyle (WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX)
