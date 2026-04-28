/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

#include <C4Include.h>

#ifndef _WIN32
C4ToolsDlg::C4ToolsDlg() { Default(); }
C4ToolsDlg::~C4ToolsDlg() {}
void C4ToolsDlg::Default() { hDialog = NULL; }
void C4ToolsDlg::Clear() {}
void C4ToolsDlg::PopTextures() {}
void C4ToolsDlg::PopMaterial() {}
void C4ToolsDlg::ChangeGrade(int iChange) {}
void C4ToolsDlg::UpdatePreview() {}
BOOL C4ToolsDlg::Open() { return FALSE; }
BOOL C4ToolsDlg::SetGrade(int iGrade) { return FALSE; }
BOOL C4ToolsDlg::SetTool(int iTool) { return FALSE; }
BOOL C4ToolsDlg::SetLandscapeMode(int iMode) { return FALSE; }
BOOL C4ToolsDlg::SetIFT(BOOL fIFT) { return FALSE; }
BOOL C4ToolsDlg::SelectTexture(const char *szTexture) { return FALSE; }
BOOL C4ToolsDlg::SelectMaterial(const char *szMaterial) { return FALSE; }
#else

/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Drawing tools dialog for landscape editing in console mode */

#include <C4Include.h>
#include <commctrl.h>

BOOL CALLBACK ToolsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {
  int iValue;
  switch (Msg) {
  //----------------------------------------------------------------------------------------------
  case WM_CLOSE:
    Console.ToolsDlg.Clear();
    break;
  //----------------------------------------------------------------------------------------------
  case WM_DESTROY:
    StoreWindowPosition(hDlg, "Property", "Software\\RedWolf Design\\Clonk 4\\Console", FALSE);
    break;
  //----------------------------------------------------------------------------------------------
  case WM_INITDIALOG:
    return TRUE;
  //----------------------------------------------------------------------------------------------
  case WM_PAINT:
    PostMessage(hDlg, WM_USER, 0, 0); // For user paint
    return FALSE;
  //----------------------------------------------------------------------------------------------
  case WM_USER:
    Console.ToolsDlg.UpdatePreview();
    return TRUE;
  //----------------------------------------------------------------------------------------------
  case WM_VSCROLL:
    switch (LOWORD(wParam)) {
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      iValue = HIWORD(wParam);
      Console.ToolsDlg.SetGrade(C4TLS_GradeMax - iValue);
      break;
    case SB_PAGEUP:
    case SB_PAGEDOWN:
    case SB_LINEUP:
    case SB_LINEDOWN:
      iValue = SendDlgItemMessage(hDlg, IDC_SLIDERGRADE, TBM_GETPOS, 0, 0);
      Console.ToolsDlg.SetGrade(C4TLS_GradeMax - iValue);
      break;
    }
    return TRUE;
  //----------------------------------------------------------------------------------------------
  case WM_COMMAND:
    // Evaluate command
    switch (LOWORD(wParam)) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDOK:
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONMODEDYNAMIC:
      Console.ToolsDlg.SetLandscapeMode(C4LSC_Dynamic);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONMODESTATIC:
      Console.ToolsDlg.SetLandscapeMode(C4LSC_Static);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONMODEEXACT:
      Console.ToolsDlg.SetLandscapeMode(C4LSC_Exact);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONBRUSH:
      Console.ToolsDlg.SetTool(C4TLS_Brush);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONLINE:
      Console.ToolsDlg.SetTool(C4TLS_Line);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONRECT:
      Console.ToolsDlg.SetTool(C4TLS_Rect);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONFILL:
      Console.ToolsDlg.SetTool(C4TLS_Fill);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONPICKER:
      Console.ToolsDlg.SetTool(C4TLS_Picker);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONIFT:
      Console.ToolsDlg.SetIFT(TRUE);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_BUTTONNOIFT:
      Console.ToolsDlg.SetIFT(FALSE);
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_COMBOMATERIAL:
      switch (HIWORD(wParam)) {
      case CBN_SELCHANGE:
        int cursel = SendDlgItemMessage(hDlg, IDC_COMBOMATERIAL, CB_GETCURSEL, 0, 0);
        SendDlgItemMessage(hDlg, IDC_COMBOMATERIAL, CB_GETLBTEXT, cursel, (LPARAM)OSTR);
        Console.ToolsDlg.SetMaterial(OSTR);
        break;
      }
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
    case IDC_COMBOTEXTURE:
      switch (HIWORD(wParam)) {
      case CBN_SELCHANGE:
        int cursel = SendDlgItemMessage(hDlg, IDC_COMBOTEXTURE, CB_GETCURSEL, 0, 0);
        SendDlgItemMessage(hDlg, IDC_COMBOTEXTURE, CB_GETLBTEXT, cursel, (LPARAM)OSTR);
        Console.ToolsDlg.SetTexture(OSTR);
        break;
      }
      return TRUE;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - -
    }
    return FALSE;
    //----------------------------------------------------------------------------------------
  }
  return FALSE;
}

C4ToolsDlg::C4ToolsDlg() {
  Default();
  hbmBrush = hbmLine = hbmRect = hbmFill = NULL;
  hbmIFT = hbmNoIFT = NULL;
}

C4ToolsDlg::~C4ToolsDlg() {
  Clear();
  // Unload bitmaps
  if (hbmBrush)
    DeleteObject(hbmBrush);
  if (hbmLine)
    DeleteObject(hbmLine);
  if (hbmRect)
    DeleteObject(hbmRect);
  if (hbmFill)
    DeleteObject(hbmFill);
  if (hbmIFT)
    DeleteObject(hbmIFT);
  if (hbmNoIFT)
    DeleteObject(hbmNoIFT);
}

BOOL C4ToolsDlg::Open() {
  // Create dialog window
  if (hDialog)
    return TRUE;
  hDialog = CreateDialog(Application.hInstance, MAKEINTRESOURCE(IDD_TOOLS), Application.hWindow, (DLGPROC)ToolsDlgProc);
  if (!hDialog)
    return FALSE;
  // Set text
  SetWindowText(hDialog, LoadResStr(IDS_DLG_TOOLS));
  SetDlgItemText(hDialog, IDC_STATICMATERIAL, LoadResStr(IDS_CTL_MATERIAL));
  SetDlgItemText(hDialog, IDC_STATICTEXTURE, LoadResStr(IDS_CTL_TEXTURE));
  // Load bitmaps if necessary
  LoadBitmaps();
  // Init slider
  InitGradeCtrl();
  // Show window
  RestoreWindowPosition(hDialog, "Property", "Software\\RedWolf Design\\Clonk 4\\Console");
  SetWindowPos(hDialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
  ShowWindow(hDialog, SW_SHOWNORMAL | SW_SHOWNA);
  // Update contols
  UpdateLandscapeModeCtrls();
  UpdateToolCtrls();
  UpdateIFTControls();
  InitMaterialCtrls();
  EnableControls();
  return TRUE;
}

void C4ToolsDlg::Default() {
  hDialog = NULL;
  Tool = C4TLS_Brush;
  Grade = C4TLS_GradeDefault;
  ModeIFT = TRUE;
  SCopy("Earth", Material);
  SCopy("Rough", Texture);
}

void C4ToolsDlg::Clear() {
  if (hDialog)
    DestroyWindow(hDialog);
  hDialog = NULL;
}

BOOL C4ToolsDlg::SetTool(int iTool) {
  Tool = iTool;
  UpdateToolCtrls();
  UpdatePreview();
  return TRUE;
}

void C4ToolsDlg::UpdateToolCtrls() {
  int iTool = Tool;
  SendDlgItemMessage(hDialog, IDC_BUTTONBRUSH, BM_SETSTATE, (iTool == C4TLS_Brush), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONBRUSH));
  SendDlgItemMessage(hDialog, IDC_BUTTONLINE, BM_SETSTATE, (iTool == C4TLS_Line), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONLINE));
  SendDlgItemMessage(hDialog, IDC_BUTTONRECT, BM_SETSTATE, (iTool == C4TLS_Rect), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONRECT));
  SendDlgItemMessage(hDialog, IDC_BUTTONFILL, BM_SETSTATE, (iTool == C4TLS_Fill), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONFILL));
  SendDlgItemMessage(hDialog, IDC_BUTTONPICKER, BM_SETSTATE, (iTool == C4TLS_Picker), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONPICKER));
}

void C4ToolsDlg::InitMaterialCtrls() {
  int cnt;
  const char *szTexture;
  // Materials
  SendDlgItemMessage(hDialog, IDC_COMBOMATERIAL, CB_ADDSTRING, 0, (LPARAM)C4TLS_MatSky);
  for (cnt = 0; cnt < Game.Material.Num; cnt++)
    SendDlgItemMessage(hDialog, IDC_COMBOMATERIAL, CB_ADDSTRING, 0, (LPARAM)Game.Material.Map[cnt].Name);
  SendDlgItemMessage(hDialog, IDC_COMBOMATERIAL, CB_SELECTSTRING, 0, (LPARAM)Material);
  // Textures
  for (cnt = 0; (szTexture = Game.TextureMap.GetTexture(cnt)); cnt++)
    SendDlgItemMessage(hDialog, IDC_COMBOTEXTURE, CB_ADDSTRING, 0, (LPARAM)szTexture);
  SendDlgItemMessage(hDialog, IDC_COMBOTEXTURE, CB_SELECTSTRING, 0, (LPARAM)Texture);
}

void C4ToolsDlg::SetMaterial(const char *szMaterial) {
  SCopy(szMaterial, Material, C4M_MaxName);
  EnableControls();
  UpdatePreview();
  AssertValidTexture();
}

void C4ToolsDlg::SetTexture(const char *szTexture) {
  SCopy(szTexture, Texture, C4M_MaxName);
  UpdatePreview();
}

BOOL C4ToolsDlg::SetIFT(BOOL fIFT) {
  if (fIFT)
    ModeIFT = 1;
  else
    ModeIFT = 0;
  UpdateIFTControls();
  UpdatePreview();
  return TRUE;
}

void C4ToolsDlg::UpdatePreview() {
  if (!hDialog)
    return;

  SURFACE sfcPreview;

  int iPrvWdt, iPrvHgt;
  iPrvWdt = iPrvHgt = 50;

  if (!(sfcPreview = CreateSurface(iPrvWdt, iPrvHgt)))
    return;

  Engine.DDraw.DrawBox(sfcPreview, 0, 0, iPrvWdt - 1, iPrvHgt - 1, CGray4);

  BYTE bCol = Mat2PixCol(Game.Material.Get(Material));

  if (SEqual(Material, C4TLS_MatSky))
    Engine.DDraw.DefinePattern(Game.Landscape.Sky.Surface);
  else
    Engine.DDraw.DefinePattern(Game.TextureMap.GetTexture(Texture));

  if (IsWindowEnabled(GetDlgItem(hDialog, IDC_PREVIEW)))
    Engine.DDraw.DrawCircle(sfcPreview, iPrvWdt / 2, iPrvHgt / 2, Grade, bCol);

  Engine.DDraw.NoPattern();

  Engine.DDraw.AttachPrimaryPalette(sfcPreview);

  RECT rect;
  GetClientRect(GetDlgItem(hDialog, IDC_PREVIEW), &rect);

  Engine.DDraw.BlitSurface2Window(sfcPreview, 0, 0, iPrvWdt, iPrvHgt, GetDlgItem(hDialog, IDC_PREVIEW), rect.left, rect.top, rect.right, rect.bottom);

  DestroySurface(sfcPreview);
}

void C4ToolsDlg::InitGradeCtrl() {
  if (!hDialog)
    return;
  HWND hwndTrack = GetDlgItem(hDialog, IDC_SLIDERGRADE);
  SendMessage(hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)5);
  SendMessage(hwndTrack, TBM_SETLINESIZE, 0, (LPARAM)1);
  SendMessage(hwndTrack, TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(C4TLS_GradeMin, C4TLS_GradeMax));
  SendMessage(hwndTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)C4TLS_GradeMax - Grade);
  UpdateWindow(hwndTrack);
}

BOOL C4ToolsDlg::SetGrade(int iGrade) {
  Grade = BoundBy(iGrade, C4TLS_GradeMin, C4TLS_GradeMax);
  UpdatePreview();
  return TRUE;
}

void C4ToolsDlg::ChangeGrade(int iChange) {
  Grade = BoundBy(Grade + iChange, C4TLS_GradeMin, C4TLS_GradeMax);
  UpdatePreview();
  InitGradeCtrl();
}

void C4ToolsDlg::PopMaterial() {
  if (!hDialog)
    return;
  SetFocus(GetDlgItem(hDialog, IDC_COMBOMATERIAL));
  SendDlgItemMessage(hDialog, IDC_COMBOMATERIAL, CB_SHOWDROPDOWN, TRUE, 0);
}

void C4ToolsDlg::PopTextures() {
  if (!hDialog)
    return;
  SetFocus(GetDlgItem(hDialog, IDC_COMBOTEXTURE));
  SendDlgItemMessage(hDialog, IDC_COMBOTEXTURE, CB_SHOWDROPDOWN, TRUE, 0);
}

void C4ToolsDlg::UpdateIFTControls() {
  if (!hDialog)
    return;
  SendDlgItemMessage(hDialog, IDC_BUTTONNOIFT, BM_SETSTATE, (ModeIFT == 0), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONNOIFT));
  SendDlgItemMessage(hDialog, IDC_BUTTONIFT, BM_SETSTATE, (ModeIFT == 1), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONIFT));
}

void C4ToolsDlg::UpdateLandscapeModeCtrls() {
  int iMode = Game.Landscape.Mode;
  // Dynamic: enable only if dynamic anyway
  SendDlgItemMessage(hDialog, IDC_BUTTONMODEDYNAMIC, BM_SETSTATE, (iMode == C4LSC_Dynamic), 0);
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONMODEDYNAMIC), (iMode == C4LSC_Dynamic));
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONMODEDYNAMIC));
  // Static: enable only if map available
  SendDlgItemMessage(hDialog, IDC_BUTTONMODESTATIC, BM_SETSTATE, (iMode == C4LSC_Static), 0);
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONMODESTATIC), (Game.Landscape.Map != NULL));
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONMODESTATIC));
  // Exact: enable always
  SendDlgItemMessage(hDialog, IDC_BUTTONMODEEXACT, BM_SETSTATE, (iMode == C4LSC_Exact), 0);
  UpdateWindow(GetDlgItem(hDialog, IDC_BUTTONMODEEXACT));
  // Set dialog caption
  SetWindowText(hDialog, LoadResStr(iMode == C4LSC_Dynamic ? IDS_DLG_DYNAMIC : iMode == C4LSC_Static ? IDS_DLG_STATIC : IDS_DLG_EXACT));
}

BOOL C4ToolsDlg::SetLandscapeMode(int iMode) {
  int iLastMode = Game.Landscape.Mode;
  // Exact to static: confirm data loss warning
  if (iLastMode == C4LSC_Exact)
    if (iMode == C4LSC_Static)
      if (!Console.Message(LoadResStr(IDS_CNS_EXACTTOSTATIC), TRUE))
        return FALSE;
  // Set landscape mode
  Game.Landscape.SetMode(iMode);
  // Exact to static: redraw landscape from map
  if (iLastMode == C4LSC_Exact)
    if (iMode == C4LSC_Static) {
      Game.Landscape.MapToLandscape();
      Console.EditCursor.UpdateGraphicsSystem();
    }
  // Assert valid tool
  if (iMode != C4LSC_Exact)
    if (Tool == C4TLS_Fill)
      SetTool(C4TLS_Brush);
  // Update controls
  UpdateLandscapeModeCtrls();
  EnableControls();
  // Success
  return TRUE;
}

void C4ToolsDlg::EnableControls() {
  int iLandscapeMode = Game.Landscape.Mode;
  // Set bitmap buttons
  SendDlgItemMessage(hDialog, IDC_BUTTONBRUSH, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)((iLandscapeMode >= C4LSC_Static) ? hbmBrush : hbmBrush2));
  SendDlgItemMessage(hDialog, IDC_BUTTONLINE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)((iLandscapeMode >= C4LSC_Static) ? hbmLine : hbmLine2));
  SendDlgItemMessage(hDialog, IDC_BUTTONRECT, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)((iLandscapeMode >= C4LSC_Static) ? hbmRect : hbmRect2));
  SendDlgItemMessage(hDialog, IDC_BUTTONFILL, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)((iLandscapeMode >= C4LSC_Exact) ? hbmFill : hbmFill2));
  SendDlgItemMessage(hDialog, IDC_BUTTONPICKER, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)((iLandscapeMode >= C4LSC_Static) ? hbmPicker : hbmPicker2));
  SendDlgItemMessage(hDialog, IDC_BUTTONIFT, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmIFT);
  SendDlgItemMessage(hDialog, IDC_BUTTONNOIFT, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmNoIFT);
  SendDlgItemMessage(hDialog, IDC_BUTTONMODEDYNAMIC, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmDynamic);
  SendDlgItemMessage(hDialog, IDC_BUTTONMODESTATIC, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmStatic);
  SendDlgItemMessage(hDialog, IDC_BUTTONMODEEXACT, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmExact);
  // Enable drawing controls
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONBRUSH), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONLINE), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONRECT), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONFILL), (iLandscapeMode >= C4LSC_Exact));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONPICKER), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONIFT), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONNOIFT), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_COMBOMATERIAL), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_COMBOTEXTURE), (iLandscapeMode >= C4LSC_Static) && !SEqual(Material, C4TLS_MatSky));
  EnableWindow(GetDlgItem(hDialog, IDC_STATICMATERIAL), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_STATICTEXTURE), (iLandscapeMode >= C4LSC_Static) && !SEqual(Material, C4TLS_MatSky));
  EnableWindow(GetDlgItem(hDialog, IDC_SLIDERGRADE), (iLandscapeMode >= C4LSC_Static));
  EnableWindow(GetDlgItem(hDialog, IDC_PREVIEW), (iLandscapeMode >= C4LSC_Static));
  UpdatePreview();
}

void C4ToolsDlg::LoadBitmaps() {
  HINSTANCE hInst = Application.hInstance;
  if (!hbmBrush)
    hbmBrush = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BRUSH));
  if (!hbmLine)
    hbmLine = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LINE));
  if (!hbmRect)
    hbmRect = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RECT));
  if (!hbmFill)
    hbmFill = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FILL));
  if (!hbmPicker)
    hbmPicker = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PICKER));
  if (!hbmBrush2)
    hbmBrush2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BRUSH2));
  if (!hbmLine2)
    hbmLine2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LINE2));
  if (!hbmRect2)
    hbmRect2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RECT2));
  if (!hbmFill2)
    hbmFill2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FILL2));
  if (!hbmPicker2)
    hbmPicker2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PICKER2));
  if (!hbmIFT)
    hbmIFT = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_IFT));
  if (!hbmNoIFT)
    hbmNoIFT = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NOIFT));
  if (!hbmDynamic)
    hbmDynamic = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_DYNAMIC));
  if (!hbmStatic)
    hbmStatic = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STATIC));
  if (!hbmExact)
    hbmExact = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXACT));
}

void C4ToolsDlg::AssertValidTexture() {
  // Static map mode only
  if (Game.Landscape.Mode != C4LSC_Static)
    return;
  // Ignore if sky
  if (SEqual(Material, C4TLS_MatSky))
    return;
  // Current material-texture valid
  sprintf(OSTR, "%s-%s", Material, Texture);
  if (Game.TextureMap.GetIndex(OSTR, FALSE))
    return;
  // Find valid material-texture
  const char *szTexture;
  for (int iTexture = 0; szTexture = Game.TextureMap.GetTexture(iTexture); iTexture++) {
    sprintf(OSTR, "%s-%s", Material, szTexture);
    if (Game.TextureMap.GetIndex(OSTR, FALSE)) {
      SelectTexture(szTexture);
      return;
    }
  }
  // No valid texture found
}

BOOL C4ToolsDlg::SelectTexture(const char *szTexture) {
  SendDlgItemMessage(hDialog, IDC_COMBOTEXTURE, CB_SELECTSTRING, 0, (LPARAM)szTexture);
  SetTexture(szTexture);
  return TRUE;
}

BOOL C4ToolsDlg::SelectMaterial(const char *szMaterial) {
  SendDlgItemMessage(hDialog, IDC_COMBOMATERIAL, CB_SELECTSTRING, 0, (LPARAM)szMaterial);
  SetMaterial(szMaterial);
  return TRUE;
}

#endif
