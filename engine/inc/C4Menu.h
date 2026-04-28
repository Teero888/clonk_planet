/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* In-game menu as used by objects, players, and fullscreen options */

const int C4MN_Style_Normal = 0, C4MN_Style_Context = 1, C4MN_Style_Info = 2;

const int C4MN_None = 0, C4MN_Construction = 1, C4MN_Bridge = 2, C4MN_Take = 3,
          C4MN_Buy = 4, C4MN_Sell = 5, C4MN_Activate = 6, C4MN_Hostility = 7,
          C4MN_Surrender = 8, C4MN_Put = 9, C4MN_Magic = 10, C4MN_Main = 12,
          C4MN_Get = 13, C4MN_Context = 14, C4MN_Info = 15;

const int C4MN_MaxMenu = C4MN_Context;

const int C4MN_Extra_None = 0, C4MN_Extra_Components = 1, C4MN_Extra_Value = 2,
          C4MN_Extra_MagicValue = 3, C4MN_Extra_Info = 4;

const int C4MN_Align_Left = 1, C4MN_Align_Right = 2, C4MN_Align_Top = 4,
          C4MN_Align_Bottom = 8, C4MN_Align_Free = 16;

const int C4MN_Symbol_Close = 0, C4MN_Symbol_Help = 2, C4MN_Symbol_Player = 4;

void DrawMenuSymbol(int iMenu, C4Facet &cgo, int iOwner, C4Object *cObj);

const int C4MN_Item_NoCount = 12345678;

class C4MenuItem {
  friend C4Menu;

public:
  C4MenuItem();
  ~C4MenuItem();

protected:
  char Caption[C4MaxTitle + 1];
  char Command[_MAX_FNAME + 30 + 1];
  char Command2[_MAX_FNAME + 30 + 1];
  char InfoCaption[2 * C4MaxTitle + 1];
  int Count;
  int Parameter;
  C4ID id;
  C4Object *Object;
  C4FacetEx Symbol;
  C4MenuItem *Next;

protected:
  void Draw(C4Facet &cgo, int iIndex, BOOL fSelected, int iStyle, C4Menu *pMenu,
            C4RegionList *pRegions);
  void Set(const char *szCaption, C4FacetEx &fctSymbol, const char *szCommand,
           int iCount, C4Object *pObject, int iParameter,
           const char *szInfoCaption, C4ID idID, const char *szCommand2);
  void Clear();
  void Default();
};

class C4Menu {
public:
  C4Menu();
  ~C4Menu();

protected:
  BOOL Active;
  BOOL CloseDown;
  BOOL LocationSet;
  BOOL Permanent;
  BOOL Multiline, Scrollbar;
  BOOL Redraw, NeedRefill;
  int Player;
  C4Object *Object;
  C4Object *ParentObject;
  C4Object *RefillObject;
  int Style;
  int Selection, TimeOnSelection;
  int Position;
  int ItemCount;
  int ItemWidth, ItemHeight;
  int VisibleCount;
  int Extra, ExtraData;
  int Identification;
  int X, Y, Width, Height;
  int Columns, Lines;
  int DragScrollRange1, DragScrollRange2;
  int Alignment;
  int RefillObjectContentsCount;
  char Caption[C4MaxTitle + 1];
  char CaptionText[C4MaxTitle + 1];
  char InfoCaptionText[C4MaxTitle + 1];
  BOOL EnableInfoCaption;
  C4TargetRect CaptionPlace;
  C4TargetRect InfoCaptionPlace;
  C4FacetEx Symbol;
  C4FacetEx Buffer;
  C4RegionList Regions;
  C4MenuItem *First;

public:
  BOOL ConvertCom(int &rCom, int &rData);
  BOOL IsMultiline();
  BOOL Refill();
  void Execute();
  void SetRefillObject(C4Object *pObj);
  void SetPermanent(BOOL fPermanent);
  void SetAlignment(int iAlignment);
  void DragScroll(int iX, int iY);
  void ClearPointers(C4Object *pObj);
  void DragLocation(int iX, int iY);
  int GetIdentification();
  int GetItemCount();
  int GetPosition();
  int GetSelection();
  C4MenuItem *GetSelectedItem();
  C4MenuItem *GetItem(int iIndex);
  BOOL SetSelection(int iSelection);
  BOOL SetPosition(int iPosition);
  void SetLocation(int iX, int iY);
  BOOL Enter(BOOL fRight = FALSE);
  BOOL IsActive();
  BOOL Draw(C4Facet &cgo, C4RegionList *pRegions = NULL,
            BOOL fResetLocation = FALSE);
  BOOL Control(BYTE byCom, int iData);
  BOOL Add(const char *szCaption, C4FacetEx fctSymbol, const char *szCommand,
           int iCount = C4MN_Item_NoCount, C4Object *pObject = NULL,
           int iParameter = 0, const char *szInfoCaption = NULL,
           C4ID idID = C4ID_None, const char *szCommand2 = NULL);
  BOOL Init(C4FacetEx fctSymbol, const char *szEmpty, int iPlayer,
            int iExtra = C4MN_Extra_None, int iExtraData = 0, int iId = 0,
            int iStyle = C4MN_Style_Normal);
  BOOL Init(C4FacetEx fctSymbol, const char *szEmpty, C4Object *pObject,
            int iExtra = C4MN_Extra_None, int iExtraData = 0, int iId = 0,
            int iStyle = C4MN_Style_Normal);
  BOOL ActivateNewPlayer(int iPlayer);
  void Close();
  void Clear();
  void Default();

protected:
  void ResetCaption();
  void DrawBuffer();
  void SetRedraw();
  void AdjustSelection();
  void AdjustPosition();
  void ClearItems();
  BOOL CheckBuffer();
  BOOL RefillInternal();
  void DrawButton(C4Facet &cgo);
  void DrawScrollbar(C4Facet &cgo, int iTotal, int iVisible, int iPosition,
                     C4RegionList *pRegions = NULL);
  void DrawFrame(C4Facet &cgo, int iAlign);
  void DrawFrame(SURFACE sfcSurface, int iX, int iY, int iWdt, int iHgt);
  void InitLocation(C4Facet &cgo);
  C4Object *GetParentObject();
};
