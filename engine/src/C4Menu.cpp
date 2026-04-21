/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* In-game menu as used by objects, players, and fullscreen options */

#include <C4Include.h>

const int C4MN_SymbolSize		= 16,
					C4MN_SymbolSize2	= 10,
					C4MN_FrameWidth		= 2;

const int C4MN_Frame_Left		= 1,
					C4MN_Frame_Right	= 2,
					C4MN_Frame_Top		= 4,
					C4MN_Frame_Bottom = 8,
					C4MN_Frame_Border = C4MN_Frame_Left | C4MN_Frame_Right | C4MN_Frame_Top | C4MN_Frame_Bottom,
					C4MN_Frame_Outside = 16;
					
const int C4MN_Caption_Delay = 20,
					C4MN_InfoCaption_Delay = 90;	

const int C4MN_AdjustPosition = 1<<31;

/* Obsolete helper function still used by CreateMenu(iSymbol) */

void DrawMenuSymbol(int iMenu, C4Facet &cgo, int iOwner, C4Object *cObj)
	{
	C4Facet ccgo;

  int color=0; 
  if (ValidPlr(iOwner)) color=Game.Players.Get(iOwner)->Color;

  switch (iMenu)
    {
    case C4MN_Bridge:
      break;
    case C4MN_Construction:
			if (C4Id2Def(C4Id("WKS1"))) C4Id2Def(C4Id("WKS1"))->Draw(cgo);
      break;
    case C4MN_Buy:
			ccgo.Set(cgo.Surface,cgo.X,cgo.Y,cgo.Wdt*3/4,cgo.Hgt*3/4);
			Game.GraphicsResource.fctFlag.Draw(ccgo,TRUE,color);
      ccgo.Set(cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt/2-2,cgo.Wdt/2,cgo.Hgt/2);
			Game.GraphicsResource.fctEntry.Draw(ccgo);
      break;
    case C4MN_Sell:
			ccgo.Set(cgo.Surface,cgo.X,cgo.Y,cgo.Wdt*3/4,cgo.Hgt*3/4);
			Game.GraphicsResource.fctFlag.Draw(ccgo,TRUE,color);
      ccgo.Set(cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt/2-2,cgo.Wdt/2,cgo.Hgt/2);
			Game.GraphicsResource.fctEntry.Draw(ccgo,TRUE,1);
      break;
    case C4MN_Put:
			cObj->DrawPicture(cgo);
			ccgo.Set(cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt/2,cgo.Wdt/2,cgo.Hgt/2);
			Game.GraphicsResource.fctEntry.Draw(ccgo,TRUE,3);
      break;
    case C4MN_Activate:
			cObj->Def->Draw(cgo);
			ccgo.Set(cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt/2,cgo.Wdt/2,cgo.Hgt/2);
			Game.GraphicsResource.fctEntry.Draw(ccgo,TRUE,2);
      break;
    case C4MN_Main:
			Game.GraphicsResource.fctFlag.Draw(cgo,TRUE,color);
			ccgo.Set(cgo.Surface,cgo.X,cgo.Y+cgo.Hgt/2,cgo.Wdt,cgo.Hgt/2);
			Game.GraphicsResource.fctCrew.Draw(ccgo,TRUE,color);
      break;
    case C4MN_Hostility:
      break;
    case C4MN_Magic:
			ccgo.Set(cgo.Surface,cgo.X,cgo.Y+2,cgo.Wdt,cgo.Hgt-2*2);
			Game.GraphicsResource.fctMagicSymbol.Draw(ccgo);
      break;
    }
  }

C4MenuItem::C4MenuItem()
	{
	Default();
	}

C4MenuItem::~C4MenuItem()
	{
	Clear();
	}

void C4MenuItem::Default()
	{
	Caption[0]=0;
	Command[0]=0;
	Command2[0]=0;
	InfoCaption[0]=0;
	Symbol.Default();
	Next=NULL;
	Count=C4MN_Item_NoCount;
	Parameter=0;
	id=C4ID_None;
	Object=NULL;
	}

void C4MenuItem::Clear()
	{
	Symbol.Clear();
	}

void C4MenuItem::Set(const char *szCaption, C4FacetEx &fctSymbol, const char *szCommand, 
										 int iCount, C4Object *pObject, int iParameter, const char *szInfoCaption,
										 C4ID idID, const char *szCommand2)
	{
	SCopy(szCaption,Caption,C4MaxTitle);
	SCopy(szCommand,Command,_MAX_FNAME+30);
	SCopy(szCommand2,Command2,_MAX_FNAME+30);
	SCopy(szInfoCaption,InfoCaption,C4MaxTitle);
	Symbol=fctSymbol;
	Count=iCount;
	Object=pObject;
	Parameter=iParameter;
	id=idID;
	}

void C4MenuItem::Draw(C4Facet &cgo, int iIndex, BOOL fSelected, int iStyle, C4Menu *pMenu, C4RegionList *pRegions)
	{
	// Select mark
	if (iStyle!=C4MN_Style_Info)
		if (fSelected)
			Engine.DDraw.DrawBox(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CRed);
	// Symbol/text areas
	C4Facet cgoItemSymbol,cgoItemText;
	cgoItemSymbol=cgoItemText=cgo;
	if (iStyle==C4MN_Style_Context) cgoItemSymbol=cgoItemText.TruncateSection(C4FCT_Left);
	// Draw item symbol
	Symbol.Draw(cgoItemSymbol);
	// Draw item text
	Engine.DDraw.ApplyPrimaryClipper(cgoItemText.Surface); Engine.DDraw.SetPrimaryClipper(cgoItemText.X,cgoItemText.Y,cgoItemText.X+cgoItemText.Wdt-1,cgoItemText.Y+cgoItemText.Hgt-1);
	switch (iStyle)
		{
		case C4MN_Style_Context:	   	
			Engine.DDraw.TextOut(Caption,cgoItemText.Surface,cgoItemText.X,cgoItemText.Y,FWhite,FBlack,ALeft);
			break;
		case C4MN_Style_Info:
			char szText[2*C4MaxTitle+1];
			SCopy(InfoCaption,szText); SWordWrap(szText,' ','|',30);
			Engine.DDraw.TextOut(szText,cgoItemText.Surface,cgoItemText.X,cgoItemText.Y,FWhite,FBlack,ALeft);
			break;
		}
	Engine.DDraw.DetachPrimaryClipper(cgoItemText.Surface); Engine.DDraw.NoPrimaryClipper();
	// Draw count
	if (Count!=C4MN_Item_NoCount)
		{
		char szCount[10+1];
		sprintf(szCount,"%ix",Count); 
	  Engine.DDraw.TextOut(szCount,cgoItemText.Surface,cgoItemText.X+cgoItemText.Wdt-1,cgoItemText.Y+cgoItemText.Hgt-1-Engine.DDraw.TextHeight(),FWhite,FBlack,ARight);
		}
	// Region
	if (pRegions) 
		{
		C4Region rgn;
		rgn.Default(); rgn.Set(cgo);
		rgn.Com=COM_MenuEnter; rgn.RightCom=COM_MenuEnterAll;
		rgn.MoveOverCom=COM_MenuSelect; rgn.Data=iIndex;
		rgn.Target=Object;
		rgn.id=id;
		rgn.Menu=pMenu; // Pointer only to be set on local-control no-object menus!
		pRegions->Add(rgn);
		}
	}    	

C4Menu::C4Menu()
	{
	Default();
	}

C4Menu::~C4Menu()
	{
	Clear();
	}

void C4Menu::Default()
	{
	Active=FALSE;
	Selection=-1;
	Style=C4MN_Style_Normal;
	ItemCount=0;
	ItemWidth=ItemHeight=C4SymbolSize;
	First=NULL;
	Redraw=TRUE;
	NeedRefill=FALSE;
	Symbol.Default();
	Caption[0]=0;
	Player=NO_OWNER;
	Permanent=FALSE;
	Object=NULL;
	Extra=C4MN_Extra_None;
	ExtraData=0;
	Position=0;
	TimeOnSelection=0;
	VisibleCount=0;
	Identification=0;
	LocationSet=FALSE;
	RefillObjectContentsCount=0;
	X=Y=Width=Height=0;
	CloseDown=FALSE;
	ParentObject=NULL;
	RefillObject=NULL;
	Columns=Lines=0;
	Multiline=Scrollbar=FALSE;
	DragScrollRange1=DragScrollRange2=0;
	Alignment= C4MN_Align_Right | C4MN_Align_Bottom;
	Buffer.Default();
	Regions.Default();
	CaptionPlace.Default();
	CaptionText[0]=0;
	InfoCaptionPlace.Default();
	InfoCaptionText[0]=0;
	EnableInfoCaption=FALSE;
	}

void C4Menu::Clear()
	{
	Symbol.Clear();
	Buffer.Clear();
	Regions.Clear();
	C4MenuItem *pNext; 
	while (First) { pNext=First->Next; delete First; First=pNext; }
	}

void C4Menu::Close()
	{
	Clear();
	Active=FALSE;
	}

BOOL C4Menu::Init(C4FacetEx &fctSymbol, const char *szEmpty, int iPlayer, int iExtra, int iExtraData, int iId, int iStyle)
	{
	Clear(); Default();
	Active=TRUE;
	Symbol=fctSymbol;
	SCopy(szEmpty,Caption,C4MaxTitle);
	Player=iPlayer;
	Extra=iExtra; ExtraData=iExtraData;
	Identification=iId;
	Style=iStyle;
	return TRUE;
	}

BOOL C4Menu::Init(C4FacetEx &fctSymbol, const char *szEmpty, C4Object *pObject, int iExtra, int iExtraData, int iId, int iStyle)
	{
	Clear(); Default();
	Active=TRUE;
	Symbol=fctSymbol;
	SCopy(szEmpty,Caption,C4MaxTitle);
	Object=pObject;
	Extra=iExtra; ExtraData=iExtraData;
	Identification=iId;
	ParentObject=GetParentObject();
	Style=iStyle;
	return TRUE;
	}

BOOL C4Menu::Add(const char *szCaption, C4FacetEx &fctSymbol, const char *szCommand, 
								 int iCount, C4Object *pObject, int iParameter, const char *szInfoCaption,
								 C4ID idID, const char *szCommand2)
	{
	if (!Active) return FALSE;
	C4MenuItem *pNew,*pLast;
	// Create and set item
	if (!(pNew = new C4MenuItem)) return FALSE;
	pNew->Set(szCaption,fctSymbol,szCommand,iCount,pObject,iParameter,szInfoCaption,idID,szCommand2);
	// Append item to list
	for (pLast=First; pLast && pLast->Next; pLast=pLast->Next);
	if (pLast) pLast->Next=pNew; else First=pNew;
	// Item count
	ItemCount++; 
	// Init selection
	if (Selection==-1) Selection=0;
	// Redraw
	SetRedraw();
	// Success
	return TRUE;
	}

BOOL C4Menu::Control(BYTE byCom, int iData)
	{
	if (!Active) return FALSE;

	// Redraw
	SetRedraw();

  switch (byCom)
    {
		case COM_MenuEnter: Enter(); break;
		case COM_MenuEnterAll: Enter(TRUE); break;
    case COM_MenuClose: Close(); break;
		case COM_MenuCloseDown: CloseDown=TRUE; break;

		// organize with nicer subfunction...
    case COM_MenuLeft: 
			SetSelection(Selection-1); AdjustPosition(); EnableInfoCaption=TRUE; 
			break;
    case COM_MenuRight: 
			SetSelection(Selection+1); AdjustPosition(); EnableInfoCaption=TRUE; 
			break;
    case COM_MenuUp: 
			if (Selection-Columns>=0) SetSelection(Selection-Columns); AdjustPosition(); EnableInfoCaption=TRUE; 
			break;
    case COM_MenuDown: 
			if (Selection+Columns<ItemCount) SetSelection(Selection+Columns); AdjustPosition(); EnableInfoCaption=TRUE; 
			break;
		case COM_MenuSelect: 
			if (ItemCount) 
				{
				SetSelection(iData & (~C4MN_AdjustPosition)); 
				if (iData & C4MN_AdjustPosition) AdjustPosition();
				EnableInfoCaption=TRUE;
				}
			break;
		case COM_MenuMove: 
			SetSelection(Selection+iData); AdjustPosition();
			EnableInfoCaption=TRUE;
			break;

    }

	return TRUE;
	}

BOOL C4Menu::IsActive()
	{
	return Active;
	}

BOOL C4Menu::Enter(BOOL fRight)
	{
	// Not active
	if (!Active) return FALSE;
	// Get selected item
	C4MenuItem *pItem = GetSelectedItem(); 
	if (!pItem) return FALSE;
	// Copy command to buffer (menu might be cleared)
  char szCommand[_MAX_FNAME+30+1];
	SCopy(pItem->Command,szCommand);
	if (fRight && pItem->Command2[0]) SCopy(pItem->Command2,szCommand);
	
	// Close if not permanent
	if (!Permanent) Close();

	// Object menu
	if (Object)
		{
		Object->MenuCommand(szCommand);
		return TRUE;
		}

	// Player menu
	C4Player *pPlr = Game.Players.Get(Player);
	if (pPlr) 
		{
		pPlr->MenuCommand(szCommand);
		return TRUE;
		}

	// Fullscreen menu
	FullScreen.MenuCommand(szCommand);
	return TRUE;

	}

C4MenuItem* C4Menu::GetItem(int iIndex)
	{
	C4MenuItem *pItem;
	int cIndex;
	for (pItem=First,cIndex=0; pItem; pItem=pItem->Next,cIndex++)
		if (cIndex==iIndex) return pItem;
	return NULL;
	}

int C4Menu::GetItemCount()
	{
	return ItemCount;
	}

BOOL C4Menu::SetSelection(int iSelection)
	{
	// Not active
	if (!Active) return FALSE;
	// Outside Limits
	if ((iSelection<0) || (iSelection>ItemCount-1)) return TRUE;
	// Selection change
	if (iSelection!=Selection)
		{
		// Set selection
		Selection=iSelection;
		// Reset time on selection
		TimeOnSelection=0;
		// Reset caption
		if (Style==C4MN_Style_Normal) ResetCaption();		
		// Reset info caption
		InfoCaptionText[0]=0;
		}
	// Done
	return TRUE;
	}

C4MenuItem* C4Menu::GetSelectedItem()
	{
	C4MenuItem *pItem; int cnt;
	for (cnt=0,pItem=First; pItem; cnt++,pItem=pItem->Next)
		if (cnt==Selection)
			return pItem;
	return NULL;
	}

BOOL C4Menu::ActivateNewPlayer(int iPlayer)
	{
	// Menu symbol/init
	Init(GfxR->fctPlayer.GetPhase(),LoadResStr(IDS_MENU_NOPLRFILES),iPlayer);

	// Search player files
	char szSearch[_MAX_PATH+1],szFilename[_MAX_PATH+1],szCommand[_MAX_PATH+30+1];
	sprintf(szSearch,"%s*.c4p",Config.General.PlayerPath);
  struct _finddata_t fdt; int fdthnd;
  if ((fdthnd=_findfirst(szSearch,&fdt))<0) return FALSE;
  do
    {
    if (fdt.attrib & _A_ALL_FILES)
      {
			// Get filename & check usage
      SCopy(szSearch,szFilename); SCopy(fdt.name,GetFilename(szFilename)); 
			if (Game.Players.FileInUse(szFilename)) continue;
			// Load player info & portrait
			C4Group hGroup; C4PlayerInfoCore C4P;
			C4FacetEx fctPortrait;
			if (!hGroup.Open(szFilename)) continue;
			if (!C4P.Load(hGroup)) { hGroup.Close(); continue; }
			/*if (Config.Graphics.ShowPortraits)
				if (fctPortrait.Load(hGroup,C4CFN_Portrait))
					fctPortrait.SetPalette(Game.GraphicsResource.GamePalette,TRUE);*/
			hGroup.Close();
			// Add player item
			sprintf(szCommand,"JoinPlayer:%s",szFilename);
			sprintf(OSTR,LoadResStr(IDS_MENU_NEWPLAYER),C4P.Name);
			if (fctPortrait.Surface)
				{
				Add(OSTR,fctPortrait,szCommand);
				fctPortrait.Default();
				}
			else
				Add(OSTR,GfxR->fctPlayer.GetPhase(BoundBy(C4P.PrefColor,0,C4MaxColor-1)+2),szCommand);
      }
    }
  while (_findnext(fdthnd,&fdt)==0);
  _findclose(fdthnd);
	
	return TRUE;
	}

void C4Menu::AdjustPosition()
	{
	// Adjust position by selection (works only after InitLocation)
	if (Multiline)
		Position = Columns * BoundBy( (Selection/Columns)-Lines/2, 0, Max(ItemCount/Columns+Min(ItemCount%Columns,1)-Lines,0));
	else
		Position = BoundBy(Selection-VisibleCount/2,0,Max(ItemCount-VisibleCount,0));
	}

int C4Menu::GetSelection()
	{
	return Selection;
	}

int C4Menu::GetPosition()
	{
	return Position;
	}

BOOL C4Menu::SetPosition(int iPosition)
	{
	if (!Active) return FALSE;
	// No safety because necessary VisibleCount might not yet be available 
	Position = iPosition;
	return TRUE;
	}

int C4Menu::GetIdentification()
	{
	return Identification;
	}

void C4Menu::SetLocation(int iX, int iY)
	{
	X=iX; Y=iY;
	}

void C4Menu::InitLocation(C4Facet &cgoArea)
	{

	// Item size by style
	switch (Style)
		{
		case C4MN_Style_Normal: 
			ItemWidth=ItemHeight=C4SymbolSize; 
			break;
		case C4MN_Style_Context: 
			ItemWidth=3*C4SymbolSize+10; ItemHeight=C4MN_SymbolSize; 
			break;
		case C4MN_Style_Info:
			ItemWidth=ItemHeight=0;
			char szText[2*C4MaxTitle+1]; int iWdt,iHgt;
			for (C4MenuItem *pItem=First; pItem; pItem=pItem->Next)
				{
				SCopy(pItem->InfoCaption,szText); SWordWrap(szText,' ','|',30);
				Engine.DDraw.TextExtent(szText,iWdt,iHgt);
				ItemWidth=Max(ItemWidth,iWdt); ItemHeight=Max(ItemHeight,iHgt);
				}
			break;
		}

	// Single line (if low item count, sufficient output width, style normal)
	if ((ItemCount<=5) && (cgoArea.Wdt>=5*ItemWidth+2*C4MN_FrameWidth) && (Style==C4MN_Style_Normal))
		{
		Multiline=FALSE; 
		Columns=5; 
		Lines=1;
		}

	// Multiline
	else
		{
		Multiline=TRUE; 
		Columns=Max(Min(cgoArea.Wdt/Max(ItemWidth,1),5),1);
		if (Style==C4MN_Style_Context) Columns=1;
		if (Style==C4MN_Style_Info) Columns=1;
		Lines=Max(Min((cgoArea.Hgt-100)/Max(ItemHeight,1),ItemCount/Columns+Min(ItemCount%Columns,1)),1);
		}

	// Visible count
	VisibleCount = Columns*Lines;
	// Scrollbar
	Scrollbar = ItemCount>VisibleCount;
	// Size calculation
	Height=C4MN_SymbolSize+Lines*ItemHeight+3*C4MN_FrameWidth;
	if (Extra) Height+=C4MN_SymbolSize+C4MN_FrameWidth;
	Width=Columns*ItemWidth+2*C4MN_FrameWidth;
	if (Scrollbar) Width+=C4MN_SymbolSize2+C4MN_FrameWidth;
	// Alignment
	if (Alignment & C4MN_Align_Left) X=C4SymbolSize;
	if (Alignment & C4MN_Align_Right) X=cgoArea.Wdt-C4SymbolSize-Width;
	if (Alignment & C4MN_Align_Top) Y=C4SymbolSize;
	if (Alignment & C4MN_Align_Bottom) Y=cgoArea.Hgt-C4SymbolSize-Height;
	if (Alignment & C4MN_Align_Free) { X=BoundBy(X,0,cgoArea.Wdt-Width); Y=BoundBy(Y,0,cgoArea.Hgt-Height); }
	// Centered (due to small viewport size)
	if (Width>cgoArea.Wdt-2*C4SymbolSize) X=(cgoArea.Wdt-Width)/2;
	if (Height>cgoArea.Hgt-2*C4SymbolSize) Y=(cgoArea.Hgt-Height)/2;
	
	// Redraw
	SetRedraw();

	}

BOOL C4Menu::Draw(C4Facet &cgo, C4RegionList *pRegions, BOOL fResetLocation)
	{
	// Inactive
	if (!Active) return FALSE;

	// Location
	if (fResetLocation) LocationSet=FALSE;
	if (!LocationSet) { InitLocation(cgo); LocationSet=TRUE; }

	// Check buffer
	if (!CheckBuffer()) return FALSE;
		
	// Redraw buffer
	if (Redraw) { DrawBuffer(); Redraw=FALSE; }

	// Background
	C4Facet cgoOut; cgoOut.Set(cgo.Surface,cgo.X+X,cgo.Y+Y,Width,Height); 
	lpDDraw->DrawBoxColorTable(cgoOut.Surface,cgoOut.X,cgoOut.Y,cgoOut.X+cgoOut.Wdt-1,cgoOut.Y+cgoOut.Hgt-1,Game.GraphicsSystem.DarkColorTable);
	
	// Buffer to output
	Buffer.Draw(cgoOut);

	// Caption
	Engine.DDraw.SubPrimaryClipper(cgoOut.X+CaptionPlace.x,cgoOut.Y+CaptionPlace.y,cgoOut.X+CaptionPlace.x+CaptionPlace.Wdt-1,cgoOut.Y+CaptionPlace.y+CaptionPlace.Hgt-1);
	Engine.DDraw.TextOut(CaptionText,cgoOut.Surface,cgoOut.X+CaptionPlace.x+CaptionPlace.tx,cgoOut.Y+CaptionPlace.y,FWhite,FBlack,ALeft);
	Engine.DDraw.RestorePrimaryClipper();
	// Info caption
	Engine.DDraw.SubPrimaryClipper(cgoOut.X+InfoCaptionPlace.x,cgoOut.Y+InfoCaptionPlace.y,cgoOut.X+InfoCaptionPlace.x+InfoCaptionPlace.Wdt-1,cgoOut.Y+InfoCaptionPlace.y+InfoCaptionPlace.Hgt-1);
	Engine.DDraw.TextOut(InfoCaptionText,cgoOut.Surface,cgoOut.X+InfoCaptionPlace.x+InfoCaptionPlace.tx,cgoOut.Y+InfoCaptionPlace.y,FWhite,FBlack,ALeft);
	Engine.DDraw.RestorePrimaryClipper();

	// Set regions
	if (pRegions) pRegions->Add(Regions,FALSE);

	// Done
	return TRUE;
	}

void C4Menu::DrawBuffer()
	{

	// Store and clear global clipper
	int iX1,iY1,iX2,iY2;
	Engine.DDraw.GetPrimaryClipper(iX1,iY1,iX2,iY2); Engine.DDraw.NoPrimaryClipper();
	
	// Wipe buffer, clear regions
	Buffer.Wipe();
	Regions.Clear(); Regions.Default(); Regions.SetAdjust(X,Y);

	// Reroute to buffer
	C4Facet cgo = Buffer;
	C4RegionList *pRegions = &Regions;

	// Area variables
	C4Facet cgoMenu,cgoArea,cgoCaption,cgoTitle,cgoSymbol,cgoSelection,cgoExtra;
	C4Facet cgoClose,cgoScrollbar;
	C4Region rgn;

	// Calculations
	cgoArea.Set(cgo.Surface,cgo.X,cgo.Y,Width,Height); 
	cgoMenu=cgoArea;
	cgoTitle=cgoArea.Truncate(C4FCT_Top,C4MN_SymbolSize+C4MN_FrameWidth*2);
	cgoTitle.Expand(-C4MN_FrameWidth,-C4MN_FrameWidth,-C4MN_FrameWidth,-C4MN_FrameWidth);
	cgoCaption=cgoTitle; 
	cgoSymbol=cgoCaption.TruncateSection();
	cgoClose=cgoCaption.Truncate(C4FCT_Right,C4MN_SymbolSize+C4MN_FrameWidth);
	cgoClose.Expand(-C4MN_FrameWidth);
	if (Extra)
		{
		cgoExtra=cgoArea.Truncate(C4FCT_Bottom,C4MN_SymbolSize+C4MN_FrameWidth*2);
		cgoExtra.Expand(-C4MN_FrameWidth,-C4MN_FrameWidth,-C4MN_FrameWidth,-C4MN_FrameWidth);
		}
	if (Scrollbar)
		{
		cgoScrollbar=cgoArea.Truncate(C4FCT_Right,C4MN_SymbolSize2+C4MN_FrameWidth*2);
		cgoScrollbar.Expand(-C4MN_FrameWidth,-C4MN_FrameWidth);
		}
	cgoSelection=cgoArea;
	cgoSelection.Expand(-C4MN_FrameWidth,Scrollbar ? 0 : -C4MN_FrameWidth,0,Extra ? 0 : -C4MN_FrameWidth);
	
	// General region
	if (pRegions) pRegions->Add(cgoMenu);

	// Frames
	DrawFrame(cgoTitle,C4MN_Frame_Bottom | C4MN_Frame_Outside);
	DrawFrame(cgoClose,C4MN_Frame_Left | C4MN_Frame_Outside);
	DrawFrame(cgoExtra,C4MN_Frame_Top | C4MN_Frame_Outside);
	DrawFrame(cgoScrollbar,C4MN_Frame_Left | C4MN_Frame_Outside);
	DrawFrame(cgoMenu,C4MN_Frame_Border);

	// Symbol
	Symbol.Draw(cgoSymbol);

	// Title (drag bar)
	if (pRegions) pRegions->Add(cgoTitle,"",COM_MenuDrag,ParentObject,COM_None,COM_None,0,Object ? NULL : this);

	// Close
	GfxR->fctMenuSymbol.Draw(cgoClose,TRUE,C4MN_Symbol_Close+CloseDown);
	if (pRegions) pRegions->Add(cgoClose,"",COM_MenuClose,NULL,COM_None,COM_MenuCloseDown,0,Object ? NULL : this);

	// Scrollbar
	DrawScrollbar(cgoScrollbar,ItemCount/Columns+Min(ItemCount%Columns,1),Lines,Position/Columns,pRegions);

	// Arg!
	DragScrollRange1=cgoScrollbar.Y-cgo.Y; DragScrollRange2=cgoScrollbar.Y-cgo.Y+cgoScrollbar.Hgt/*-iSize*/; // ?

	// Draw items
	C4MenuItem *pItem; int cnt;
	C4Facet cgoItem;
	if (VisibleCount && ItemCount)
		for (cnt=0,pItem=GetItem(Position); pItem && (cnt<VisibleCount); cnt++,pItem=pItem->Next)
			{
			// Item area
			cgoItem.Set(cgoSelection.Surface,cgoSelection.X+ItemWidth*(cnt%Columns),cgoSelection.Y+ItemHeight*(cnt/Columns),ItemWidth,ItemHeight);
			// Draw item
			pItem->Draw(cgoItem,Position+cnt,(Selection==Position+cnt),Style,Object ? NULL : this,pRegions);
			}

	// Default empty caption
	char szCaption[C4MaxTitle+1]; SCopy(Caption,szCaption);

	// Selected item caption & id
	C4ID idSelected = C4ID_None;
	if (pItem = GetSelectedItem())
		{
		// Caption
		if (Style==C4MN_Style_Normal) SCopy(pItem->Caption,szCaption);
		// ID
		idSelected = pItem->id;
		}

	// Set caption
	SCopy(szCaption,CaptionText);
	CaptionPlace.Set(cgoCaption.X,cgoCaption.Y,cgoCaption.Wdt,cgoCaption.Hgt,CaptionPlace.tx,CaptionPlace.ty);

	// Info caption
	InfoCaptionPlace.Set(cgoExtra.X,cgoExtra.Y,cgoExtra.Wdt,cgoExtra.Hgt,InfoCaptionPlace.tx,InfoCaptionPlace.ty);
	
	// Extra information (if no info caption)
	if (!InfoCaptionText[0])
		{
		C4Def *pDef = C4Id2Def(idSelected);
		switch (Extra)
			{
			case C4MN_Extra_Components:
				if (pDef)	pDef->Component.Draw(cgoExtra,-1,Game.Defs,C4D_All,TRUE,C4FCT_Right | C4FCT_Triple | C4FCT_Half);
				break;
			case C4MN_Extra_Value:
				if (pDef) Game.GraphicsResource.fctWealthSymbol.DrawValue(cgoExtra,pDef->Value,0,0,C4FCT_Right);
				// Flag parent object's owner's wealth display
				if (ParentObject && ValidPlr(ParentObject->Owner)) 
					Game.Players.Get(ParentObject->Owner)->ViewWealth = C4ViewDelay;
				break;
			case C4MN_Extra_MagicValue:
				if (pDef) Game.GraphicsResource.fctMagicSymbol.DrawValue2(cgoExtra,pDef->Value,ExtraData,0,0,C4FCT_Right);
				break;
			}
		}

	// Restore global clipper
	Engine.DDraw.SetPrimaryClipper(iX1,iY1,iX2,iY2);

	}

void C4Menu::DrawFrame(SURFACE sfcSurface, int iX, int iY, int iWdt, int iHgt)
	{
	lpDDraw->DrawFrame(sfcSurface,iX+1,iY+1,iX+iWdt-1,iY+iHgt-1,CGray1);
	lpDDraw->DrawFrame(sfcSurface,iX,iY,iX+iWdt-2,iY+iHgt-2,CGray3);
	lpDDraw->SetPixel(sfcSurface,iX+iWdt-1,iY,CGray2);
	lpDDraw->SetPixel(sfcSurface,iX,iY+iHgt-1,CGray2);
	}

void C4Menu::DrawFrame(C4Facet &cgo, int iAlign)
	{
	if (!cgo.Wdt || !cgo.Hgt) return;
	// Left
	if (iAlign & C4MN_Frame_Left)
		if (iAlign & C4MN_Frame_Outside)
			{
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X-2,cgo.Y-1,cgo.Y+cgo.Hgt-1+1,CGray2);
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X-1,cgo.Y,cgo.Y+cgo.Hgt-1,CGray1);
			}
		else
			{
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X,cgo.Y,cgo.Y+cgo.Hgt-1,CGray2);
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X+1,cgo.Y+1,cgo.Y+cgo.Hgt-1-1,CGray1);
			}
	// Right
	if (iAlign & C4MN_Frame_Right)
		if (iAlign & C4MN_Frame_Outside)
			{
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X+cgo.Wdt-1+2,cgo.Y-1,cgo.Y+cgo.Hgt-1+1,CGray1);
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X+cgo.Wdt-1+1,cgo.Y,cgo.Y+cgo.Hgt-1,CGray2);
			}
		else
			{
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y,cgo.Y+cgo.Hgt-1,CGray1);
			lpDDraw->DrawVerticalLine(cgo.Surface,cgo.X+cgo.Wdt-1-1,cgo.Y+1,cgo.Y+cgo.Hgt-1-1,CGray2);
			}
	// Top 
	if (iAlign & C4MN_Frame_Top)
		if (iAlign & C4MN_Frame_Outside)
			{
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X-1,cgo.X+cgo.Wdt-1+1,cgo.Y-2,CGray2);
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X,cgo.X+cgo.Wdt-1,cgo.Y-1,CGray1);
			}
		else
			{
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X,cgo.X+cgo.Wdt-1,cgo.Y,CGray2);
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X+1,cgo.X+cgo.Wdt-1-1,cgo.Y+1,CGray1);
			}
	// Bottom
	if (iAlign & C4MN_Frame_Bottom)
		if (iAlign & C4MN_Frame_Outside)
			{
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X-1,cgo.X+cgo.Wdt-1+1,cgo.Y+cgo.Hgt-1+2,CGray1);
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1+1,CGray2);
			}
		else
			{
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CGray1);
			lpDDraw->DrawHorizontalLine(cgo.Surface,cgo.X+1,cgo.X+cgo.Wdt-1-1,cgo.Y+cgo.Hgt-1-1,CGray2);
			}
	}

void C4Menu::DragLocation(int iX, int iY)
	{
	if (!Active) return;
	X=iX; Y=iY;
	SetRedraw();
	}

void C4Menu::ClearPointers(C4Object *pObj)
	{
	if (Object==pObj) Object=NULL;
	if (ParentObject==pObj) ParentObject=NULL; // Reason for menu close anyway.
	if (RefillObject==pObj) RefillObject=NULL;
	for (C4MenuItem *pItem=First; pItem; pItem=pItem->Next)
		if (pItem->Object==pObj) 
			pItem->Object=NULL;
	Regions.ClearPointers(pObj);
	}

C4Object* C4Menu::GetParentObject()
	{
  C4Object *cObj; C4ObjectLink *cLnk;  
	for (cLnk=Game.Objects.First; cLnk && (cObj=cLnk->Obj); cLnk=cLnk->Next)
		if ( cObj->Menu == this )
			return cObj;
	return NULL;
	}

void C4Menu::DrawScrollbar(C4Facet &cgo, int iTotal, int iVisible, int iPosition, C4RegionList *pRegions)
	{
	if (!iTotal) return;
	if (!cgo.Surface || !cgo.Wdt || !cgo.Hgt) return;
	int iSize,iPos;
	iSize=BoundBy(cgo.Hgt*iVisible/iTotal,C4MN_SymbolSize,cgo.Hgt);
	iPos=BoundBy(cgo.Hgt*iPosition/iTotal,0,cgo.Hgt-iSize);
	C4Facet cgo2; cgo2.Set(cgo.Surface,cgo.X,cgo.Y+iPos,cgo.Wdt,iSize);
	DrawButton(cgo2);
	if (pRegions) pRegions->Add(cgo2,"",COM_MenuDragScroll,ParentObject,COM_None,COM_None,0,Object ? NULL : this);
	}

void C4Menu::DrawButton(C4Facet &cgo)
	{
	Engine.DDraw.DrawFrame(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CGray3);
	Engine.DDraw.DrawFrame(cgo.Surface,cgo.X+1,cgo.Y+1,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CGray1);
	Engine.DDraw.DrawBox(cgo.Surface,cgo.X+1,cgo.Y+1,cgo.X+cgo.Wdt-2,cgo.Y+cgo.Hgt-2,CGray2);	
	}

void C4Menu::DragScroll(int iX, int iY)
	{
	if (!Active) return;
	int iDragRangeSize = DragScrollRange2-DragScrollRange1;
	if (iDragRangeSize<=0) return;
	int iPos = (ItemCount/Columns) * BoundBy(iY-DragScrollRange1,0,iDragRangeSize) / iDragRangeSize;
	Position =  Columns * BoundBy( iPos , 0, Max(ItemCount/Columns+Min(ItemCount%Columns,1)-Lines,0));
	SetRedraw();
	}

void C4Menu::SetAlignment(int iAlignment)
	{
	Alignment = iAlignment;
	}

void C4Menu::SetPermanent(BOOL fPermanent)
	{
	Permanent = fPermanent;
	}

BOOL C4Menu::RefillInternal()
  {
	// Variables
	C4FacetEx fctSymbol;
	C4Object *pObj;
	char szCaption[256+1],szCommand[256+1],szCommand2[256+1];
	int cnt,iCount,iLastItemCount=ItemCount;
	C4Def *pDef;
	C4Player *pPlayer;
	C4IDList ListItems;
	C4Object *pTarget;
	
	// Reset flag
	NeedRefill=FALSE;

	// Refill
	switch (Identification)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		case C4MN_Activate: 
			// Refill target
			if (!(pTarget=RefillObject)) return FALSE;
			// Clear items
			ClearItems();
			// Add target contents items
		  for (cnt=0; pDef=C4Id2Def(pTarget->Contents.GetListID(C4D_Activate,cnt)); cnt++)
				{
				// First contents object
				pObj=pTarget->Contents.Find(pDef->id);
				// Count
				iCount=pTarget->Contents.ObjectCount(pDef->id);
				// Caption
				sprintf(szCaption,LoadResStr(IDS_MENU_ACTIVATE),pDef->Name);
				// Picture
			  fctSymbol.Set(pDef->Bitmap[pObj->Color],pDef->PictureRect.x,pDef->PictureRect.y,pDef->PictureRect.Wdt,pDef->PictureRect.Hgt);
				// Commands
				sprintf(szCommand,"SetCommand(this(),\"Activate\",Object(%d))",pObj->Number);
				sprintf(szCommand2,"SetCommand(this(),\"Activate\",0,%d,0,Object(%d),%s)",pTarget->Contents.ObjectCount(pDef->id),pTarget->Number,C4IdText(pDef->id));
				// Add menu item
				Add(szCaption,fctSymbol,szCommand,iCount,pObj,0,pDef->Desc,pDef->id,szCommand2);
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		case C4MN_Buy: 
			// Refill target
			if (!(pTarget=RefillObject)) return FALSE;
			// Clear items
			ClearItems();
			// Add base owner's homebase material
			if (!(pPlayer = Game.Players.Get(pTarget->Base))) return FALSE;
		  for (cnt=0; pDef=C4Id2Def(pPlayer->HomeBaseMaterial.GetID(Game.Defs,C4D_All,cnt,&iCount)); cnt++)
				{
				// Caption
				sprintf(szCaption,LoadResStr(IDS_MENU_BUY),pDef->Name);
				// Picture
			  fctSymbol.Set(pDef->Bitmap[0],pDef->PictureRect.x,pDef->PictureRect.y,pDef->PictureRect.Wdt,pDef->PictureRect.Hgt);
				// Command
				sprintf(szCommand,"AppendCommand(this(),\"Buy\",Object(%d),%d,0,0,0,%s)",pTarget->Number,1,C4IdText(pDef->id));
				sprintf(szCommand2,"AppendCommand(this(),\"Buy\",Object(%d),%d,0,0,0,%s)",pTarget->Number,iCount,C4IdText(pDef->id));
				// Add menu item
				Add(szCaption,fctSymbol,szCommand,iCount,NULL,0,pDef->Desc,pDef->id,szCommand2);
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		case C4MN_Sell: 
			// Refill target
			if (!(pTarget=RefillObject)) return FALSE;
			// Clear items
			ClearItems();
			// Add target contents items
		  for (cnt=0; pDef=C4Id2Def(pTarget->Contents.GetListID(C4D_Sell,cnt)); cnt++)
				{
				// First contents object
				pObj=pTarget->Contents.Find(pDef->id);
				// Count
				iCount=pTarget->Contents.ObjectCount(pDef->id);
				// Caption
				sprintf(szCaption,LoadResStr(IDS_MENU_SELL),pDef->Name);
				// Picture
			  fctSymbol.Set(pDef->Bitmap[pObj->Color],pDef->PictureRect.x,pDef->PictureRect.y,pDef->PictureRect.Wdt,pDef->PictureRect.Hgt);
				// Commands
				sprintf(szCommand,"AppendCommand(this(),\"Sell\",Object(%d),%d,0,0,0,%s)",pTarget->Number,1,C4IdText(pDef->id));
				sprintf(szCommand2,"AppendCommand(this(),\"Sell\",Object(%d),%d,0,0,0,%s)",pTarget->Number,pTarget->Contents.ObjectCount(pDef->id),C4IdText(pDef->id));
				// Add menu item
				Add(szCaption,fctSymbol,szCommand,iCount,NULL,0,pDef->Desc,pDef->id,szCommand2);
				}
			// Adjust selection
			AdjustSelection();
			// Adjust size
			if (ItemCount>iLastItemCount) LocationSet=FALSE;
			// Reset caption
			if (ItemCount!=iLastItemCount) CaptionPlace.Default();
			// Redraw
			SetRedraw();
			// Success
			return TRUE;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		case C4MN_Get:
			// Refill target
			if (!(pTarget=RefillObject)) return FALSE;
			// Clear items
			ClearItems();
			// Add target contents items
		  for (cnt=0; pDef=C4Id2Def(pTarget->Contents.GetListID(C4D_Get,cnt)); cnt++)
				{
				// First contents object
				pObj=pTarget->Contents.Find(pDef->id);
				// Count
				iCount=pTarget->Contents.ObjectCount(pDef->id);
				// Caption
				sprintf(szCaption,LoadResStr(pObj->OCF & OCF_Carryable ? IDS_MENU_GET : IDS_MENU_ACTIVATE),pDef->Name);
				// Picture
			  fctSymbol.Set(pDef->Bitmap[pObj->Color],pDef->PictureRect.x,pDef->PictureRect.y,pDef->PictureRect.Wdt,pDef->PictureRect.Hgt);
				// Command
				sprintf(szCommand,"SetCommand(this(),\"%s\",Object(%d))",pObj->OCF & OCF_Carryable ? "Get" : "Activate",pObj->Number);
				// Add menu item (with object)
				Add(szCaption,fctSymbol,szCommand,iCount,pObj,0,pDef->Desc,pDef->id);
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		case C4MN_Hostility:
			// Refill player
			if (!(pPlayer = Game.Players.Get(Player))) return FALSE;
			// Clear items
			ClearItems();
			// Refill items
			C4Player *pPlr; int iIndex;
			for (iIndex=0; pPlr = Game.Players.GetByIndex(iIndex); iIndex++)
				{
				// Symbol
				fctSymbol.Create(C4SymbolSize,C4SymbolSize);
				pPlayer->DrawHostility(fctSymbol,iIndex);
				// Message
				if (pPlayer->Hostility.GetIDCount(pPlr->Number+1))
					sprintf(OSTR,LoadResStr(IDS_MENU_ATTACK),pPlr->Name);
				else
					sprintf(OSTR,LoadResStr(IDS_MENU_NOATTACK),pPlr->Name);
				// Command
				char szCommand[1000];
				sprintf(szCommand,"SetHostility:%i",pPlr->Number);
				// Info caption
				char szInfoCaption[C4MaxTitle+1],szFriendly[50],szNot[30]="";
				SCopy(LoadResStr(pPlr->Hostility.GetIDCount(pPlayer->Number+1) ? IDS_MENU_ATTACKHOSTILE : IDS_MENU_ATTACKFRIENDLY),szFriendly);
				if (!pPlayer->Hostility.GetIDCount(pPlr->Number+1)) SCopy(LoadResStr(IDS_MENU_ATTACKNOT),szNot);
				sprintf(szInfoCaption,LoadResStr(IDS_MENU_ATTACKINFO),pPlr->Name,szFriendly,szNot);
				if (iIndex==pPlayer->Number) SCopy(LoadResStr(IDS_MENU_ATTACKSELF),szInfoCaption);
				// Add item
				Add(OSTR,fctSymbol,szCommand,C4MN_Item_NoCount,NULL,0,szInfoCaption);
				fctSymbol.Default();
				}
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		default:
			// Not an internal menu
			return TRUE;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		}

	// Successfull internal refill:
	
	// Adjust selection
	AdjustSelection();
	// Adjust size
	if (ItemCount>iLastItemCount) LocationSet=FALSE;
	// Reset caption
	if (ItemCount!=iLastItemCount) if (Style==C4MN_Style_Normal) ResetCaption();
	// Redraw
	SetRedraw();	
	// Success
	return TRUE;

  }

void C4Menu::ClearItems()
	{
	C4MenuItem *pNext; 
	while (First) { pNext=First->Next; delete First; First=pNext; }
	ItemCount=0;
	}

void C4Menu::SetRefillObject(C4Object *pObj)
	{
	RefillObject=pObj;
	NeedRefill=TRUE;
	}

void C4Menu::Execute()
	{
	if (!Active) return;
	// Immediate refill check by RefillObject contents count check
	if (RefillObject)
		if (RefillObject->Contents.ObjectCount()!=RefillObjectContentsCount)
			{ NeedRefill=TRUE; RefillObjectContentsCount=RefillObject->Contents.ObjectCount(); }
	// Refill (timer or flag)
	if (!Tick35 || NeedRefill) 
		if (!RefillInternal())
			Close();
	// Scroll captions
	if (TimeOnSelection>C4MN_Caption_Delay)
		if (CaptionPlace.tx+Engine.DDraw.TextWidth(CaptionText)>CaptionPlace.Wdt) 
			CaptionPlace.tx-=3;
	InfoCaptionPlace.tx-=3;
	// Time on selection
	TimeOnSelection++;
	// Info caption by selection
	C4MenuItem *pItem;
	if (Style!=C4MN_Style_Info)
		if (EnableInfoCaption)
			if (TimeOnSelection>C4MN_InfoCaption_Delay)
				if (!InfoCaptionText[0])
					if (pItem=GetSelectedItem())
						if (pItem->InfoCaption[0])
							{
							// Copy info caption text from item
							SCopy(pItem->InfoCaption,InfoCaptionText,C4MaxTitle);
							// Start out at right border
							InfoCaptionPlace.tx=Width;
							// Redraw
							SetRedraw();
							// Enable extra if no there yet
							if (!Extra) { Extra=C4MN_Extra_Info; LocationSet=FALSE; }
							}
	}

BOOL C4Menu::Refill()
	{
	if (!Active) return FALSE;
	// Refill (close if failure)
	if (!RefillInternal())
		{ Close(); return FALSE; }
	// Success
	return TRUE;
	}

void C4Menu::AdjustSelection()
	{
	// Selection in valid item range
	if (ItemCount) 
		SetSelection(BoundBy(Selection,0,ItemCount-1));
	// No items, no selection
	else 
		{ Selection=-1; if (Style==C4MN_Style_Normal) ResetCaption(); }
	// Ensure selection visible
	if (Selection>=0)
		if (!Inside(Selection-Position,0,VisibleCount-1))
			AdjustPosition();
	}

BOOL C4Menu::IsMultiline()
	{
	return Multiline;
	}

BOOL C4Menu::ConvertCom(int &rCom, int &rData)
	{
	if (!Active) return FALSE;

	// Plain control to menu conversion
	rCom = Com2MenuCom(rCom,Multiline);

	// Menu selection movement to SetSelect conversion
  switch (rCom)
    {
    case COM_MenuLeft: rCom=COM_MenuMove; rData=-1; break;
    case COM_MenuRight: rCom=COM_MenuMove; rData=+1; break;
    case COM_MenuUp: rCom=COM_MenuMove; rData=-Columns; break;
    case COM_MenuDown: rCom=COM_MenuMove; rData=+Columns; break;
    }

	return TRUE;
	}

void C4Menu::SetRedraw()
	{
	Redraw=TRUE;
	}

BOOL C4Menu::CheckBuffer()
	{
	// Buffer facet incorrect size: clear
	if (Buffer.Surface)
		if ((Buffer.Wdt!=Width) || (Buffer.Hgt!=Height))
			Buffer.Clear();
	// No output facet: create (and redraw)
	if (!Buffer.Surface)
		if (Buffer.Create(Width,Height)) SetRedraw();
		else return FALSE;	
	// Buffer facet ready
	return TRUE;
	}

void C4Menu::ResetCaption()
	{
	CaptionText[0]=0; 
	CaptionPlace.Default(); 
	}

