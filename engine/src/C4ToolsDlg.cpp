/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

#include <C4Include.h>

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
