/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Console mode dialog for object properties and script interface */

#include <C4Include.h>

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
