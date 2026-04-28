/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Console mode dialog for object properties and script interface */

#include <C4Include.h>

#ifndef _WIN32
C4PropertyDlg::C4PropertyDlg() { Default(); }
C4PropertyDlg::~C4PropertyDlg() { Clear(); }
void C4PropertyDlg::Default() { hDialog = 0; }
void C4PropertyDlg::Clear() {}
void C4PropertyDlg::Execute() {}
void C4PropertyDlg::ClearPointers(C4Object *pObj) {}
void C4PropertyDlg::UpdateInputCtrl(C4Object *pObj) {}
BOOL C4PropertyDlg::Open() { return FALSE; }
BOOL C4PropertyDlg::Update() { return FALSE; }
BOOL C4PropertyDlg::Update(C4ObjectList &rSelection) { return FALSE; }
#else

BOOL CALLBACK PropertyDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {

  switch (Msg) {
  //------------------------------------------------------------------------------------------------
  case WM_CLOSE:
    Console.PropertyDlg.Clear();
    break;
  //------------------------------------------------------------------------------------------------
  case WM_DESTROY:
    StoreWindowPosition(hDlg, "Property", "Software\\RedWolf Design\\Clonk 4\\Console", FALSE);
    break;
  //------------------------------------------------------------------------------------------------
  case WM_INITDIALOG:
    SendMessage(hDlg, DM_SETDEFID, (WPARAM)IDOK, (LPARAM)0);
    return TRUE;
  //------------------------------------------------------------------------------------------------
  case WM_COMMAND:
    // Evaluate command
    switch (LOWORD(wParam)) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - - - -
    case IDOK:
      // IDC_COMBOINPUT to Console.EditCursor.In()
      char buffer[16000];
      GetDlgItemText(hDlg, IDC_COMBOINPUT, buffer, 16000);
      if (buffer[0])
        Console.EditCursor.In(buffer);
      Console.EditCursor.UpdateGraphicsSystem();
      return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - - - -
    case IDC_BUTTONRELOADDEF:
      Game.ReloadDef(Console.PropertyDlg.idSelectedDef);
      return TRUE;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - - - -
    }
    return FALSE;
    //-----------------------------------------------------------------------------------------------
  }
  return FALSE;
}

C4PropertyDlg::C4PropertyDlg() { Default(); }

C4PropertyDlg::~C4PropertyDlg() { Clear(); }

BOOL C4PropertyDlg::Open() {
  if (hDialog)
    return TRUE;
  hDialog = CreateDialog(Application.hInstance, MAKEINTRESOURCE(IDD_PROPERTIES), Application.hWindow, (DLGPROC)PropertyDlgProc);
  if (!hDialog)
    return FALSE;
  // Set text
  SetWindowText(hDialog, LoadResStr(IDS_DLG_PROPERTIES));
  // Enable controls
  EnableWindow(GetDlgItem(hDialog, IDOK), Console.Editing);
  EnableWindow(GetDlgItem(hDialog, IDC_COMBOINPUT), Console.Editing);
  EnableWindow(GetDlgItem(hDialog, IDC_BUTTONRELOADDEF), Console.Editing);
  // Show window
  RestoreWindowPosition(hDialog, "Property", "Software\\RedWolf Design\\Clonk 4\\Console");
  SetWindowPos(hDialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
  ShowWindow(hDialog, SW_SHOWNORMAL | SW_SHOWNA);

  return TRUE;
}

BOOL C4PropertyDlg::Update(C4ObjectList &rSelection) {
  if (!hDialog)
    return FALSE;
  // Set new selection
  Selection.Copy(rSelection);
  // Update input control
  UpdateInputCtrl(Selection.GetObject());
  // Update contents
  return Update();
}

BOOL IsObjectPointer(int iValue) {
  for (C4ObjectLink *cLnk = Game.Objects.First; cLnk; cLnk = cLnk->Next)
    if (cLnk->Obj == (C4Object *)(long)iValue)
      return TRUE;
  return FALSE;
}

BOOL C4PropertyDlg::Update() {
  if (!hDialog)
    return FALSE;

  C4Object *cobj;
  char output[16000] = "";
  idSelectedDef = C4ID_None;
  int cnt;

  // Compose info text by selected object(s)
  switch (Selection.ObjectCount()) {
  // No selection
  case 0:
    SAppend(LoadResStr(IDS_CNS_NOOBJECT), output);
    break;
  // One selected object
  case 1:
    cobj = Selection.GetObject();
    // Type
    sprintf(OSTR, LoadResStr(IDS_CNS_TYPE), cobj->Def->Name, C4IdText(cobj->Def->id));
    SAppend(OSTR, output);
    SAppend(LineFeed, output);
    // Owner
    if (ValidPlr(cobj->Owner)) {
      sprintf(OSTR, LoadResStr(IDS_CNS_OWNER), Game.Players.Get(cobj->Owner)->Name);
      SAppend(OSTR, output);
      SAppend(LineFeed, output);
    }
    // Contents
    if (cobj->Contents.ObjectCount()) {
      SAppend(LoadResStr(IDS_CNS_CONTENTS), output);
      cobj->Contents.WriteNameList(output + SLen(output), Game.Defs);
      SAppend(LineFeed, output);
    }
    // Action
    if (cobj->Action.Act != ActIdle) {
      SAppend(LoadResStr(IDS_CNS_ACTION), output);
      SAppend(cobj->Def->ActMap[cobj->Action.Act].Name, output);
      SAppend(LineFeed, output);
    }
    // Locals
    int iValue;
    OSTR[0] = 0;
    for (cnt = 0; cnt < C4MaxVariable; cnt++)
      if (iValue = cobj->Local[cnt]) {
        // Header
        if (!OSTR[0])
          SCopy(LoadResStr(IDS_CNS_LOCALS), OSTR);
        // C4ID
        if ((iValue >= 10000) && LooksLikeID(C4IdText(iValue))) {
          sprintf(OSTR + SLen(OSTR), "[%d] %s ", cnt, C4IdText(iValue));
          continue;
        }
        // C4Object*
        if (IsObjectPointer(iValue)) {
          sprintf(OSTR + SLen(OSTR), "[%d] %s ", cnt, ((C4Object *)(long)iValue)->GetName());
          continue;
        }
        // int
        sprintf(OSTR + SLen(OSTR), "[%d] %d ", cnt, iValue);
      }
    if (OSTR[0]) {
      SAppend(OSTR, output);
      SAppend(LineFeed, output);
    }
    // Store selected def
    idSelectedDef = cobj->id;
    break;
  // Multiple selected objects
  default:
    sprintf(OSTR, LoadResStr(IDS_CNS_MULTIPLEOBJECTS), Selection.ObjectCount());
    SAppend(OSTR, output);
    break;
  }
  // Update info edit control
  int iLine = SendDlgItemMessage(hDialog, IDC_EDITOUTPUT, EM_GETFIRSTVISIBLELINE, (WPARAM)0, (LPARAM)0);
  SetDlgItemText(hDialog, IDC_EDITOUTPUT, output);
  SendDlgItemMessage(hDialog, IDC_EDITOUTPUT, EM_LINESCROLL, (WPARAM)0, (LPARAM)iLine);
  UpdateWindow(GetDlgItem(hDialog, IDC_EDITOUTPUT));

  return TRUE;
}

void C4PropertyDlg::Default() {
  hDialog = NULL;
  idSelectedDef = C4ID_None;
  Selection.Default();
}

void C4PropertyDlg::Clear() {
  Selection.Clear();
  if (hDialog)
    DestroyWindow(hDialog);
  hDialog = NULL;
}

void C4PropertyDlg::UpdateInputCtrl(C4Object *pObj) {
  int cnt;
  HWND hCombo = GetDlgItem(hDialog, IDC_COMBOINPUT);

  char szLastText[500 + 1];
  if (hCombo)
    GetWindowText(hCombo, szLastText, 500);

  // Clear & add standard functions
  SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
  for (C4ScriptFnDef *pFn = C4ScriptFnMap; pFn->Identifier; pFn++)
    if (pFn->Public) {
      SCopy(pFn->Identifier, OSTR);
      SAppend("()", OSTR);
      SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)OSTR);
    }

  // Add object script functions
  BOOL fDivider = FALSE;
  C4ScriptFnRef *pRef;
  // Object script available
  if (pObj && pObj->Def)
    // Scan all functions
    for (cnt = 0; pRef = pObj->Def->Script.GetFunctionRef(cnt); cnt++)
      // Public functions only
      if (pRef->Access = C4SCR_Access_Public) {
        // Insert divider if necessary
        if (!fDivider) {
          SendMessage(hCombo, CB_INSERTSTRING, 0, (LPARAM) "----------");
          fDivider = TRUE;
        }
        // Add function
        SCopy(pRef->Name, OSTR);
        SAppend("()", OSTR);
        SendMessage(hCombo, CB_INSERTSTRING, 0, (LPARAM)OSTR);
      }

  if (hCombo)
    SetWindowText(hCombo, szLastText);
}

void C4PropertyDlg::Execute() {
  if (!Tick35)
    Update();
}

void C4PropertyDlg::ClearPointers(C4Object *pObj) { Selection.ClearPointers(pObj); }

#endif // _WIN32
