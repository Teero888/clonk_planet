#include <C4Include.h>

LRESULT APIENTRY ViewportWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }
void UpdateWindowLayout(HWND hwnd) {}

C4Viewport::C4Viewport() { Default(); }
C4Viewport::~C4Viewport() { Clear(); }

void C4Viewport::Default() {
  Player = NO_OWNER;
  ViewX = ViewY = ViewWdt = ViewHgt = 0;
  DrawX = DrawY = OutX = OutY = OutWdt = OutHgt = 0;
  Next = NULL;
  PlayerLock = TRUE;
  ResetMenuPositions = FALSE;
  Regions.Default();
  SetRegions = NULL;
}

void C4Viewport::Clear() { Regions.Clear(); }

void C4Viewport::Execute() {
  // Adjust position
  AdjustPosition();

  // Draw
  Draw();

  // Blit
  BlitOutput();
}

void C4Viewport::Draw() {
  // Current graphics output
  C4FacetEx cgo;
  cgo.Set(Engine.DDraw.lpBack, DrawX, DrawY, ViewWdt, ViewHgt, ViewX, ViewY);

  // Draw Landscape (draw before setting clippers)
  Game.Landscape.Draw(cgo);

  // Set clippers
  Engine.DDraw.SetPrimaryClipper(DrawX, DrawY, DrawX + ViewWdt - 1, DrawY + ViewHgt - 1);

  // Draw game
  Game.Draw(cgo);

  // Draw overlay
  DrawOverlay(cgo);

  // Netstats? (omitted for now)

  // Remove clippers
  Engine.DDraw.NoPrimaryClipper();
}

void C4Viewport::DrawOverlay(C4FacetEx &cgo) {
  // Player fog of war
  DrawPlayerFogOfWar(cgo);
  // Player info
  DrawCursorInfo(cgo);
  DrawPlayerInfo(cgo);
  DrawMenu(cgo);
  // Game messages
  Game.Messages.Draw(cgo, Player);
  // Mouse control
  if (Game.MouseControl.IsViewport(this)) {
    DrawMouseButtons(cgo);
    Game.MouseControl.Draw(cgo);
  }
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
  C4Facet ccgo, ccgo2;
  C4Object *cursor;
  if (!ValidPlr(Player))
    return;
  if (!(cursor = Game.Players.Get(Player)->Cursor))
    return;

  // Draw info
  if (Config.Graphics.ShowPlayerInfoAlways && cursor->Info) {
    ccgo.Set(cgo.Surface, cgo.X + C4SymbolBorder, cgo.Y + C4SymbolBorder, 3 * C4SymbolSize, C4SymbolSize);
    cursor->Info->Draw(ccgo, Config.Graphics.ShowPortraits, (cursor == Game.Players.Get(Player)->Captain));
  }

  // Draw contents
  if (cursor->Contents.ObjectCount() == 1) {
    // Single object
    ccgo.Set(cgo.Surface, cgo.X + C4SymbolBorder, cgo.Y + cgo.Hgt - C4SymbolBorder - C4SymbolSize, C4SymbolSize, C4SymbolSize);
    cursor->Contents.GetObject()->DrawPicture(ccgo, FALSE, SetRegions);
  } else {
    // Object list
    ccgo.Set(cgo.Surface, cgo.X + C4SymbolBorder, cgo.Y + cgo.Hgt - C4SymbolBorder - C4SymbolSize, 7 * C4SymbolSize, C4SymbolSize);
    cursor->Contents.DrawIDList(ccgo, -1, Game.Defs, C4D_All, SetRegions, COM_Contents, FALSE);
  }

  // Draw energy levels
  if (cursor->ViewEnergy || Config.Graphics.ShowPlayerInfoAlways)
    if (cgo.Hgt > 2 * C4SymbolSize + 2 * C4SymbolBorder) {
      int cx = C4SymbolBorder;
      // Energy
      ccgo.Set(cgo.Surface, cgo.X + cx, cgo.Y + C4SymbolSize + C4SymbolBorder, C4SymbolBorder - 1, cgo.Hgt - 2 * C4SymbolBorder - 2 * C4SymbolSize);
      cursor->DrawEnergy(ccgo);
      cx += C4SymbolBorder;
      // Magic energy
      ccgo.Set(cgo.Surface, cgo.X + cx, cgo.Y + C4SymbolSize + C4SymbolBorder, C4SymbolBorder - 1, cgo.Hgt - 2 * C4SymbolBorder - 2 * C4SymbolSize);
      if (cursor->MagicEnergy) {
        cursor->DrawMagicEnergy(ccgo);
        cx += C4SymbolBorder;
      }
      // Breath
      ccgo.Set(cgo.Surface, cgo.X + cx, cgo.Y + C4SymbolSize + C4SymbolBorder, C4SymbolBorder - 1, cgo.Hgt - 2 * C4SymbolBorder - 2 * C4SymbolSize);
      if (cursor->Breath && (cursor->Breath < cursor->GetPhysical()->Breath)) {
        cursor->DrawBreath(ccgo);
        cx += C4SymbolBorder;
      }
    }

  // Draw commands
  if (Config.Graphics.ShowCommands || Game.MouseControl.IsViewport(this))
    if (cgo.Hgt > C4SymbolSize) {
      int iSize = 24;
      int iSize2 = 30;
      // Primary area (bottom)
      ccgo.Set(cgo.Surface, cgo.X, cgo.Y + cgo.Hgt - iSize, cgo.Wdt, iSize);
      // Secondary area (side)
      ccgo2.Set(cgo.Surface, cgo.X + cgo.Wdt - iSize2, cgo.Y, iSize2, cgo.Hgt - iSize - 5);
      // Draw commands
      cursor->DrawCommands(ccgo, ccgo2, SetRegions);
    }
}

void C4Viewport::DrawPlayerInfo(C4FacetEx &cgo) {
  if (!ValidPlr(Player))
    return;
  C4Facet ccgo;
  C4Player *pPlr = Game.Players.Get(Player);

  // Wealth
  if (pPlr->ViewWealth || Config.Graphics.ShowPlayerInfoAlways) {
    int wdt = C4SymbolSize;
    int hgt = C4SymbolSize / 2;
    ccgo.Set(cgo.Surface, cgo.X + cgo.Wdt - wdt - C4SymbolBorder, cgo.Y + C4SymbolBorder, wdt, hgt);
    Game.GraphicsResource.fctWealthSymbol.DrawValue(ccgo, pPlr->Wealth);
  }

  // Value gain
  if ((Game.C4S.Game.ValueGain && pPlr->ViewValue) || Config.Graphics.ShowPlayerInfoAlways) {
    int wdt = C4SymbolSize;
    int hgt = C4SymbolSize / 2;
    ccgo.Set(cgo.Surface, cgo.X + cgo.Wdt - 2 * wdt - 2 * C4SymbolBorder, cgo.Y + C4SymbolBorder, wdt, hgt);
    Game.GraphicsResource.fctValueSymbol.DrawValue(ccgo, pPlr->ValueGain);
  }

  // Crew
  if (Config.Graphics.ShowPlayerInfoAlways) {
    int wdt = C4SymbolSize;
    int hgt = C4SymbolSize / 2;
    ccgo.Set(cgo.Surface, cgo.X + cgo.Wdt - 3 * wdt - 3 * C4SymbolBorder, cgo.Y + C4SymbolBorder, wdt, hgt);
    Game.GraphicsResource.fctCrew.DrawValue2(ccgo, pPlr->SelectCount, pPlr->Crew.ObjectCount(), pPlr->Color);
  }

  // Controls
  DrawPlayerControls(cgo);
  DrawPlayerStartup(cgo);
}

void C4Viewport::DrawPlayerFogOfWar(C4FacetEx &cgo) {
  C4Player *pPlr = Game.Players.Get(Player);
  if (!pPlr || !pPlr->FogOfWar)
    return;
  pPlr->FogOfWar->Draw(cgo);
}

void C4Viewport::DrawMouseButtons(C4FacetEx &cgo) {
  C4Facet ccgo;
  C4Region rgn;
  int iSymbolSize = 4 * C4SymbolSize / 9;
  // Help
  ccgo.Set(cgo.Surface, cgo.X + cgo.Wdt - iSymbolSize, cgo.Y + C4SymbolSize + 2 * C4SymbolBorder, iSymbolSize, iSymbolSize);
  GfxR->fctKey.Draw(ccgo);
  GfxR->fctOKCancel.Draw(ccgo, TRUE, 0, 1);
  if (SetRegions) {
    rgn.Default();
    rgn.Set(ccgo, LoadResStr(IDS_CON_HELP));
    rgn.Com = COM_Help;
    SetRegions->Add(rgn);
  }
  // Player menu
  ccgo.Y += iSymbolSize;
  GfxR->fctKey.Draw(ccgo);
  GfxR->fctOKCancel.Draw(ccgo, TRUE, 1, 1);
  if (SetRegions) {
    rgn.Default();
    rgn.Set(ccgo, LoadResStr(IDS_CON_PLAYERMENU));
    rgn.Com = COM_PlayerMenu;
    SetRegions->Add(rgn);
  }
}

void C4Viewport::DrawPlayerStartup(C4FacetEx &cgo) {
  C4Player *pPlr;
  if (!(pPlr = Game.Players.Get(Player)))
    return;
  if (!pPlr->LocalControl || !pPlr->ShowStartup)
    return;
  int iNameHgtOff = 0;

  // Control
  if (pPlr->MouseControl)
    GfxR->fctMouse.Draw(cgo.Surface, cgo.X + (cgo.Wdt - GfxR->fctKeyboard.Wdt) / 2 + 55, cgo.Y + cgo.Hgt * 2 / 3 - 10, 0, 0);
  if (Inside(pPlr->Control, C4P_Control_Keyboard1, C4P_Control_GamePad1)) {
    GfxR->fctKeyboard.Draw(cgo.Surface, cgo.X + (cgo.Wdt - GfxR->fctKeyboard.Wdt) / 2, cgo.Y + cgo.Hgt * 2 / 3, pPlr->Control, 0);
    iNameHgtOff = GfxR->fctKeyboard.Hgt;
  }

  // Name
  Engine.DDraw.TextOut(pPlr->Name, cgo.Surface, cgo.X + cgo.Wdt / 2, cgo.Y + cgo.Hgt * 2 / 3 + iNameHgtOff + 3, FPlayer + pPlr->Color, FBlack, ACenter);
}

const char *VKeyName(WORD vkey) {
  static char name[20 + 1];
  SCopy("Undefined", name);
  if (Inside((int)vkey, 1, 47))
    SCopySegment(LoadResStr(IDS_CON_VKEYSET1), vkey - 1, name, '|', 20);
  if (Inside((int)vkey, '0', '9') || Inside((int)vkey, 'A', 'Z')) {
    name[0] = (char)vkey;
    name[1] = 0;
  }
  if (Inside((int)vkey, 96, 145))
    SCopySegment(LoadResStr(IDS_CON_VKEYSET2), vkey - 96, name, '|', 20);
  if (Inside((int)vkey, 186, 192))
    SCopySegment(LoadResStr(IDS_CON_VKEYSET3), vkey - 186, name, '|', 20);
  if (Inside((int)vkey, 219, 226))
    SCopySegment(LoadResStr(IDS_CON_VKEYSET4), vkey - 219, name, '|', 20);
  return name;
}

const char *ControlKeyName(int profile, int ccon) { return VKeyName(Config.Keyboard[profile][ccon]); }

BOOL SideWinderButtonNames = TRUE;

const char *GamePadKeyName(int profile, int ccon) {
  static char name[20 + 1];
  SCopySegment(LoadResStr(IDS_CON_GAMEPAD), ccon, name, '|', 20);
  if (name[0] == '-')
    if (SideWinderButtonNames)
      SCopySegment(LoadResStr(IDS_CON_SIDEWINDER), Config.Gamepad.Button[ccon] - 1, name, '|', 20);
    else
      sprintf(name, "[%d]", Config.Gamepad.Button[ccon]);
  return name;
}

const char *PlrControlKeyName(int iPlayer, int iControl) {
  if (Inside(Game.Players.Get(iPlayer)->Control, C4P_Control_Keyboard1, C4P_Control_Keyboard4))
    return ControlKeyName(Game.Players.Get(iPlayer)->Control, iControl);
  if (Inside(Game.Players.Get(iPlayer)->Control, C4P_Control_GamePad1, C4P_Control_GamePad1))
    return GamePadKeyName(Game.Players.Get(iPlayer)->Control - C4P_Control_GamePad1, iControl);
  return NULL;
}

void C4Viewport::DrawPlayerControls(C4FacetEx &cgo) {
  if (!ValidPlr(Player))
    return;
  if (!Game.Players.Get(Player)->ShowControl)
    return;
  int size = Min(cgo.Wdt / 3, 7 * cgo.Hgt / 24);
  int tx = cgo.X + cgo.Wdt / 2 - size / 2;
  int ty = cgo.Y + 15;
  if (Game.Players.Get(Player)->ShowControlPos == 1) {
    // Position 1: bottom right corner
    tx = cgo.X + cgo.Wdt - size - 15;
    ty = cgo.Y + cgo.Hgt / 2 - size / 2;
  }

  int iShowCtrl = Game.Players.Get(Player)->ShowControl;
  int iLastCtrl = Com2Control(Game.Players.Get(Player)->LastCom);
  int scwdt = size / 3, schgt = size / 4;
  BOOL showtext;

  const int C4MaxShowControl = 10;

  for (int iCtrl = 0; iCtrl < C4MaxShowControl; iCtrl++)
    if (iShowCtrl & (1 << iCtrl)) {
      showtext = iShowCtrl & (1 << (iCtrl + C4MaxShowControl));
      if (iShowCtrl & (1 << (iCtrl + 2 * C4MaxShowControl)))
        if (Tick35 > 18)
          showtext = FALSE;
      C4Facet ccgo;
      ccgo.Set(cgo.Surface, tx + scwdt * (iCtrl % 3), ty + schgt * (iCtrl / 3), scwdt, schgt);
      DrawControlKey(ccgo, iCtrl, (iLastCtrl == iCtrl) ? 1 : 0, showtext ? PlrControlKeyName(Player, iCtrl) : NULL);
    }
}

void C4Viewport::AdjustPosition() {
  // View size
  ViewWdt = BoundBy(OutWdt, 0, GBackWdt);
  ViewHgt = BoundBy(OutHgt, 0, GBackHgt);
  // View size safety for SBack
  ViewWdt = BoundBy(ViewWdt, 0, MaxResX);
  ViewHgt = BoundBy(ViewHgt, 0, MaxResY);

  // View position
  if (PlayerLock && ValidPlr(Player)) {
    int iScrollRange = Min(ViewWdt / 10, ViewHgt / 10);
    if (Game.Players.Get(Player)->ViewMode == C4PVM_Scrolling)
      iScrollRange = 0;
    int iPlrViewX = Game.Players.Get(Player)->ViewX - ViewWdt / 2;
    int iPlrViewY = Game.Players.Get(Player)->ViewY - ViewHgt / 2;
    ViewX = BoundBy(ViewX, iPlrViewX - iScrollRange, iPlrViewX + iScrollRange);
    ViewY = BoundBy(ViewY, iPlrViewY - iScrollRange, iPlrViewY + iScrollRange);
  }
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
}

void C4Viewport::ClearPointers(C4Object *pObj) { Regions.ClearPointers(pObj); }
BOOL C4Viewport::DropFiles(HANDLE hDrop) { return FALSE; }
BOOL C4Viewport::TogglePlayerLock() {
  // Disable player lock
  if (PlayerLock) {
    PlayerLock = FALSE;
  }
  // Enable player lock
  else if (ValidPlr(Player)) {
    PlayerLock = TRUE;
  }
  return TRUE;
}

BOOL C4Viewport::UpdateOutputSize() {
  ViewWdt = OutWdt;
  ViewHgt = OutHgt;
  ResetMenuPositions = TRUE;
  return TRUE;
}
