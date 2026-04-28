/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles viewport editing in console mode */

class C4EditCursor {
public:
  C4EditCursor();
  ~C4EditCursor();

protected:
  int Mode;
  int X, Y, X2, Y2;
  BOOL Hold, DragFrame, DragLine;
  C4Object *Target, *DropTarget;
  HMENU hMenu;
  C4ObjectList Selection;

public:
  void Default();
  void Clear();
  void Execute();
  void ClearPointers(C4Object *pObj);
  void ToggleMode();
  void Draw(C4FacetEx &cgo);
  int GetMode();
  C4Object *GetTarget();
  BOOL SetMode(int iMode);
  void UpdateGraphicsSystem();
  BOOL In(const char *szText);
  BOOL Duplicate();
  BOOL OpenPropTools();
  BOOL Delete();
  BOOL LeftButtonUp();
  BOOL LeftButtonDown(BOOL fControl);
  BOOL RightButtonUp();
  BOOL RightButtonDown(BOOL fControl);
  BOOL Move(int iX, int iY, WORD wKeyFlags);
  BOOL Init(HINSTANCE hInst);

protected:
  void ApplyToolPicker();
  void ToolFailure();
  BOOL EditingOK();
  void PutContents();
  void UpdateDropTarget(WORD wKeyFlags);
  void GrabContents();
  BOOL DoContextMenu();
  BOOL SetToolPatCol(BYTE &bCol);
  void ApplyToolFill();
  void ApplyToolRect();
  void ApplyToolLine();
  void ApplyToolBrush();
  void DrawSelectMark(C4Facet &cgo);
  void FrameSelection();
  void MoveSelection(int iXOff, int iYOff);
  BOOL UpdateStatusBar();
};
