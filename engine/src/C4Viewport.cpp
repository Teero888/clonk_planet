#include <C4Include.h>

#ifndef _WIN32
LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }
void UpdateWindowLayout(HWND hwnd) {}
#endif

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

void C4Viewport::Clear() {
#ifdef _WIN32
  if (hWnd)
    DestroyWindow(hWnd);
#endif
  hWnd = NULL;
}

void C4Viewport::Execute() {
  // Update regions
  static int RegionUpdate = 0;
  SetRegions = NULL;
  RegionUpdate++;
  if (RegionUpdate >= 5) {
    RegionUpdate = 0;
    Regions.Clear();
    Regions.SetAdjust(-OutX, -OutY);
    SetRegions = &Regions;
  }
  // Adjust position
  AdjustPosition();
  // Draw
  Draw();
  // Blit output (only for windowed mode on Windows)
#ifdef _WIN32
  if (hWnd && !Application.Fullscreen)
    BlitOutput();
#endif
}

void C4Viewport::Draw() {
  // Current graphics output
  C4FacetEx cgo;
  cgo.Set(Engine.DDraw.lpBack, DrawX, DrawY, ViewWdt, ViewHgt, ViewX, ViewY);

  // Draw Landscape
  Game.Landscape.Draw(cgo);

  // Set clippers
  Engine.DDraw.SetPrimaryClipper(DrawX, DrawY, DrawX + ViewWdt - 1, DrawY + ViewHgt - 1);

  // Draw game
  Game.Draw(cgo);

  // Draw overlay
  DrawOverlay(cgo);

  // Remove clippers
  Engine.DDraw.NoPrimaryClipper();
}

void C4Viewport::DrawOverlay(C4FacetEx &cgo) {
  // Player info
  DrawCursorInfo(cgo);
  DrawPlayerInfo(cgo);
  DrawMenu(cgo);
  // Game messages
  Game.Messages.Draw(cgo, Player);
}

void C4Viewport::DrawMenu(C4FacetEx &cgo) {
  C4Player *pPlr = Game.Players.Get(Player);
  if (pPlr && pPlr->Eliminated) {
    sprintf(OSTR, LoadResStr(pPlr->Surrendered ? IDS_PLR_SURRENDERED : IDS_PLR_ELIMINATED), pPlr->Name);
    Engine.DDraw.TextOut(OSTR, cgo.Surface, cgo.X + cgo.Wdt / 2, cgo.Y + 2 * cgo.Hgt / 3, FRed, FBlack, ACenter);
    return;
  }
  if (pPlr && pPlr->Cursor && pPlr->Cursor->Menu)
    pPlr->Cursor->Menu->Draw(cgo, SetRegions, ResetMenuPositions);
  if (pPlr)
    pPlr->Menu.Draw(cgo, SetRegions, ResetMenuPositions);
  FullScreen.Menu.Draw(cgo, SetRegions, ResetMenuPositions);
  ResetMenuPositions = FALSE;
}

void C4Viewport::DrawCursorInfo(C4FacetEx &cgo) {
  C4Facet ccgo;
  C4Object *cursor;
  if (!ValidPlr(Player))
    return;
  if (!(cursor = Game.Players.Get(Player)->Cursor))
    return;

  if (Config.Graphics.ShowPlayerInfoAlways && cursor->Info) {
    ccgo.Set(cgo.Surface, cgo.X + C4SymbolBorder, cgo.Y + C4SymbolBorder, 3 * C4SymbolSize, C4SymbolSize);
    cursor->Info->Draw(ccgo, Config.Graphics.ShowPortraits, (cursor == Game.Players.Get(Player)->Captain));
  }
}

void C4Viewport::DrawPlayerInfo(C4FacetEx &cgo) {
  if (!ValidPlr(Player))
    return;
  C4Facet ccgo;
  if (Game.Players.Get(Player)->ViewWealth || Config.Graphics.ShowPlayerInfoAlways) {
    ccgo.Set(cgo.Surface, cgo.X + cgo.Wdt - C4SymbolSize - C4SymbolBorder, cgo.Y + C4SymbolBorder, C4SymbolSize, C4SymbolSize / 2);
    Game.GraphicsResource.fctWealthSymbol.DrawValue(ccgo, Game.Players.Get(Player)->Wealth);
  }
}

void C4Viewport::AdjustPosition() {
  ViewWdt = BoundBy(OutWdt, 0, GBackWdt);
  ViewHgt = BoundBy(OutHgt, 0, GBackHgt);
  ViewX = BoundBy(ViewX, 0, GBackWdt - ViewWdt);
  ViewY = BoundBy(ViewY, 0, GBackHgt - ViewHgt);
}

void C4Viewport::SetOutputSize(int iDrawX, int iDrawY, int iOutX, int iOutY, int iOutWdt, int iOutHgt) {
  DrawX = iDrawX;
  DrawY = iDrawY;
  OutX = iOutX;
  OutY = iOutY;
  OutWdt = iOutWdt;
  OutHgt = iOutHgt;
  ResetMenuPositions = TRUE;
}

BOOL C4Viewport::Init(int iPlayer) {
  if (!ValidPlr(iPlayer))
    iPlayer = NO_OWNER;
  Player = iPlayer;
  // For GLFW, we just use the main window dimensions
  OutX = 0;
  OutY = 0;
  OutWdt = Config.Graphics.ResX;
  OutHgt = Config.Graphics.ResY;
  UpdateOutputSize();
  return TRUE;
}

BOOL C4Viewport::Init(HWND hParent, HINSTANCE hInst, int iPlayer) { return Init(iPlayer); }

void C4Viewport::BlitOutput() {
#ifdef _WIN32
  if (hWnd)
    Engine.DDraw.BlitSurface2Window(Engine.DDraw.lpBack, DrawX, DrawY, ViewWdt, ViewHgt, hWnd, OutX, OutY, ViewWdt, ViewHgt);
#endif
}

void C4Viewport::ClearPointers(C4Object *pObj) { Regions.ClearPointers(pObj); }
BOOL C4Viewport::DropFiles(HANDLE hDrop) { return FALSE; }
BOOL C4Viewport::TogglePlayerLock() { return FALSE; }

BOOL C4Viewport::UpdateOutputSize() {
  ViewWdt = OutWdt;
  ViewHgt = OutHgt;
  ResetMenuPositions = TRUE;
  return TRUE;
}

const char *PlrControlKeyName(int iPlayer, int iControl) { return ""; }
