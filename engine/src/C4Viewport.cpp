#include <C4Include.h>

#ifndef _WIN32

LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }
void UpdateWindowLayout(HWND hwnd) {}

C4Viewport::C4Viewport() { Default(); }
C4Viewport::~C4Viewport() { Clear(); }

void C4Viewport::Default() {
    hWnd = NULL;
    Player = 0;
    ViewX = ViewY = ViewWdt = ViewHgt = 0;
    OutX = OutY = OutWdt = OutHgt = 0;
    DrawX = DrawY = 0;
    Next = NULL;
    PlayerLock = TRUE;
    ResetMenuPositions = FALSE;
    SetRegions = NULL;
    Regions.Default();
}

void C4Viewport::Clear() {}
void C4Viewport::Execute() {}
void C4Viewport::ClearPointers(C4Object *pObj) { Regions.ClearPointers(pObj); }
void C4Viewport::SetOutputSize(int iDrawX, int iDrawY, int iOutX, int iOutY, int iOutWdt, int iOutHgt) {
    DrawX = iDrawX; DrawY = iDrawY;
    OutX = iOutX; OutY = iOutY;
    OutWdt = iOutWdt; OutHgt = iOutHgt;
}

BOOL C4Viewport::Init(int iPlayer) { Player = iPlayer; return TRUE; }
BOOL C4Viewport::Init(HWND hParent, HINSTANCE hInst, int iPlayer) { Player = iPlayer; return TRUE; }
BOOL C4Viewport::DropFiles(HANDLE hDrop) { return FALSE; }
BOOL C4Viewport::TogglePlayerLock() { return FALSE; }
void C4Viewport::Draw() {}
const char *PlrControlKeyName(int iPlayer, int iControl) { return ""; }

#endif
