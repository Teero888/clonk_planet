/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Mouse input */

const int C4MC_MaxCaption = 1024;

const int C4MC_Button_None = 0, C4MC_Button_LeftDown = 1, C4MC_Button_LeftUp = 2, C4MC_Button_RightDown = 3, C4MC_Button_RightUp = 4, C4MC_Button_LeftDouble = 5, C4MC_Button_RightDouble = 6;

class C4MouseControl {
public:
  C4MouseControl();
  ~C4MouseControl();

protected:
  BOOL Active;
  int Player;
  C4Player *pPlayer;    // valid during Move()
  C4Viewport *Viewport; // valid during Move()
  char Caption[C4MC_MaxCaption + 1];
  int Cursor;
  int DownCursor;
  int CaptionBottomY;
  int VpX, VpY, X, Y;
  int DownX, DownY, DownOffsetX, DownOffsetY;
  int ShowPointX, ShowPointY;
  int KeepCaption;
  int ScrollSpeed;
  int Drag, DragSelecting;
  int DragImagePhase;
  bool LeftButtonDown, RightButtonDown, LeftDoubleIgnoreUp;
  bool ButtonDownOnSelection;
  bool ControlDown;
  bool ShiftDown;
  bool Scrolling;
  bool InitCentered;
  bool Help;
  bool FogOfWar;
  bool Visible;
  C4ID DragID;
  C4ObjectList Selection;
  C4FacetEx DragImage;
  // Target object
  C4Object *TargetObject; // valid during Move()
  C4Object *DownTarget;
  int TimeOnTargetObject;
  // Region
  C4Region *TargetRegion; // valid during Move()
  C4Region DownRegion;

public:
  void Default();
  void Clear();
  bool Init(int iPlayer);
  void Execute();
  const char *GetCaption();
  void HideCursor();
  void ShowCursor();
  void Draw(C4FacetEx &cgo);
  void Move(int iButton, int iX, int iY, WORD wKeyFlags);
  bool IsViewport(C4Viewport *pViewport);
  void ClearPointers(C4Object *pObj);

protected:
  void SendPlayerSelectNext();
  void UpdateFogOfWar();
  void RightUpDragNone();
  void ButtonUpDragConstruct();
  void ButtonUpDragMenu();
  void ButtonUpDragMoving();
  void ButtonUpDragSelecting();
  void LeftUpDragNone();
  void DragConstruct();
  void DragMenuScroll();
  void RightUp();
  void RightDown();
  void LeftDouble();
  void DragNone();
  void DragMoving();
  void LeftUp();
  void DragSelect();
  void LeftDown();
  void DragMenu();
  void UpdateTargetRegion();
  void UpdateScrolling();
  void CreateDragImage(C4ID id);
  void UpdateCursorTarget();
  void SendCommand(int iCommand, int iX = 0, int iY = 0, C4Object *pTarget = NULL, C4Object *pTarget2 = NULL, int iData = 0, int iAddMode = C4P_Command_Set);
  int UpdateObjectSelection();
  int UpdateCrewSelection();
  int UpdateSingleSelection();
  BOOL SendControl(int iCom, int iData = 0);
  BOOL IsValidMenu(C4Menu *pMenu);
  BOOL UpdatePutTarget(BOOL fVehicle);
};
