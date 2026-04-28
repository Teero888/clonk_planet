/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Mouse input */

#include <C4Include.h>

const int C4MC_Drag_None					= 0,
					C4MC_Drag_Selecting			= 1,
					C4MC_Drag_Moving				= 2,
					C4MC_Drag_Menu					= 3,
					C4MC_Drag_MenuScroll		= 4,
					C4MC_Drag_Construct			= 5,

					C4MC_Selecting_Unknown	= 0,
					C4MC_Selecting_Crew			= 1,
					C4MC_Selecting_Objects	= 2;
					
const int C4MC_DragSensitivity = 5;

const int C4MC_Cursor_Region			= 0,
					C4MC_Cursor_Crosshair		= 1,
					C4MC_Cursor_Enter				= 2,
					C4MC_Cursor_Grab				= 3,
					C4MC_Cursor_Chop				= 4,
					C4MC_Cursor_Dig					= 5,
					C4MC_Cursor_Build				= 6,
					C4MC_Cursor_Select			= 7,
					C4MC_Cursor_Object			= 8,
					C4MC_Cursor_Ungrab			= 9,
					C4MC_Cursor_Up					= 10,
					C4MC_Cursor_Down				= 11,
					C4MC_Cursor_Left				= 12,
					C4MC_Cursor_Right				= 13,
					C4MC_Cursor_UpLeft			= 14,
					C4MC_Cursor_UpRight			= 15,
					C4MC_Cursor_DownLeft		= 16,
					C4MC_Cursor_DownRight		= 17,
					C4MC_Cursor_JumpLeft		= 18,
					C4MC_Cursor_JumpRight		= 19,
					C4MC_Cursor_Drop				= 20,
					C4MC_Cursor_ThrowRight	= 21,
					C4MC_Cursor_Put					= 22,
					C4MC_Cursor_DragMenu		= 23,
					C4MC_Cursor_Vehicle			= 24,
					C4MC_Cursor_VehiclePut	= 25,
					C4MC_Cursor_ThrowLeft		= 26,
					C4MC_Cursor_Point				= 27,
					C4MC_Cursor_DigObject		= 28,
					C4MC_Cursor_Help				= 29,
					C4MC_Cursor_DigMaterial	= 30,
					C4MC_Cursor_Add					= 31,
					C4MC_Cursor_Construct		= 32,
					C4MC_Cursor_Attack			= 33,
					C4MC_Cursor_Nothing			= 34;

const int C4MC_Time_on_Target			= 10;

C4MouseControl::C4MouseControl()
	{
	Default();
	}

C4MouseControl::~C4MouseControl()
	{
	Clear();
	}

void C4MouseControl::Default()
	{
	Active=FALSE;
	Player=NO_OWNER;
	pPlayer=NULL;
	Viewport=NULL;
	Cursor=DownCursor=0;
	Caption[0]=0;
	CaptionBottomY=0;
	VpX=VpY=X=Y=DownX=DownY=DownOffsetX=DownOffsetY=0;
	ShowPointX=ShowPointY=-1;
	LeftButtonDown=RightButtonDown=false;
	LeftDoubleIgnoreUp=false;
	ButtonDownOnSelection=false;
	Visible=true;
	InitCentered=false;
	Help=false;
	FogOfWar=false;
	DragID=C4ID_None;
	KeepCaption=0;
	Drag=C4MC_Drag_None; DragSelecting=C4MC_Selecting_Unknown;
	Selection.Default();
	TargetObject=DownTarget=NULL;
	TimeOnTargetObject=0;
	ControlDown=false;
	ShiftDown=false;
	Scrolling=false;
	ScrollSpeed=10;
	TargetRegion=NULL;
	DownRegion.Default();
	DragImage.Default();
	DragImagePhase=0;
	}

void C4MouseControl::Clear()
	{
	Selection.Clear();
	}

void C4MouseControl::Execute()
	{

	if (!Active) return;

	// Scrolling/continuous update
	if (Scrolling || !Tick5) 
		{
		WORD wKeyState=0;
		if (GetKeyState(VK_CONTROL)<0) wKeyState|=MK_CONTROL;
		if (GetKeyState(VK_SHIFT)<0) wKeyState|=MK_SHIFT;
		Move(C4MC_Button_None,VpX,VpY,wKeyState);
		}
	}

bool C4MouseControl::Init(int iPlayer)
	{
	Clear(); Default();
	Active=TRUE;
	Player = iPlayer;
	InitCentered=false;
	return true;
	}

void C4MouseControl::ClearPointers(C4Object *pObj)
	{
	if (TargetObject==pObj) TargetObject=NULL;
	if (DownTarget==pObj) DownTarget=NULL;
	Selection.ClearPointers(pObj);
	}

bool C4MouseControl::IsViewport(C4Viewport *pViewport)
	{
	return (Viewport==pViewport);
	}

void C4MouseControl::Move(int iButton, int iX, int iY, WORD wKeyFlags)
	{
	// Active
	if (!Active) return;
	// Execute caption
	if (KeepCaption) KeepCaption--; else { Caption[0]=0; CaptionBottomY=0; }
	// Check player
	if (!(pPlayer=Game.Players.Get(Player))) { Active=FALSE; return; }
	// Check viewport
	if (!(Viewport=Game.GraphicsSystem.GetViewport(Player))) return;
	// First time viewport attachment: center mouse
	if (!InitCentered) { iX=Viewport->ViewWdt/2; iY=Viewport->ViewHgt/2; InitCentered=true; }
	// Position
	VpX=iX; VpY=iY; X=Viewport->ViewX+VpX; Y=Viewport->ViewY+VpY;	
	// Control state
	ControlDown=false; if (wKeyFlags & MK_CONTROL) ControlDown=true;
	ShiftDown=false; if (wKeyFlags & MK_SHIFT) ShiftDown=true;
	
	// Target region
	UpdateTargetRegion();
	// Scrolling
	UpdateScrolling();
	// Fog of war
	UpdateFogOfWar();

	// Blocked by fog of war: evaluate button up only
	if (FogOfWar) 
		{
		// Left button up
		if (iButton==C4MC_Button_LeftUp) 
			{
			LeftButtonDown=false;
			// End any drag
			Drag=C4MC_Drag_None;
			}
		// Right button up
		if (iButton==C4MC_Button_RightUp)
			{
			RightButtonDown=false;
			// Evaluate single right click: select next crew
			if (Drag==C4MC_Drag_None)
				SendPlayerSelectNext();
			}
		return;
		}

	// Check QBA
	BOOL fQBA=!Game.Landscape.IsQBALocked();
	if (fQBA) Game.Landscape.LockQBA();

	// Move execution by button/drag status
	switch (iButton)
		{
		//------------------------------------------------------------------------------------------
		case C4MC_Button_None:
			switch (Drag)
				{
				case C4MC_Drag_None: DragNone(); break;
				case C4MC_Drag_Selecting: DragSelect(); break;
				case C4MC_Drag_Menu: DragMenu(); break;
				case C4MC_Drag_MenuScroll: DragMenuScroll(); break;
				case C4MC_Drag_Moving: DragMoving(); break;
				case C4MC_Drag_Construct: DragConstruct(); break;
				}			
			break;
		//------------------------------------------------------------------------------------------
		case C4MC_Button_LeftDown: LeftDown(); break;
		//------------------------------------------------------------------------------------------
		case C4MC_Button_LeftUp: LeftUp(); break;
		//------------------------------------------------------------------------------------------
		case C4MC_Button_LeftDouble: LeftDouble(); break;
		//------------------------------------------------------------------------------------------
		case C4MC_Button_RightDown: RightDown(); break;
		//------------------------------------------------------------------------------------------
		case C4MC_Button_RightUp: RightUp(); break;
		//------------------------------------------------------------------------------------------
		}

	// Check QBA
	if (fQBA) Game.Landscape.UnLockQBA();

	}

void C4MouseControl::Draw(C4FacetEx &cgo)
	{
	int iOffsetX,iOffsetY;

	// Hidden
	if (!Visible) return;

	// Draw selection
	Selection.DrawSelectMark(cgo);

	// Draw control
	switch (Drag)
		{
		//------------------------------------------------------------------------------------------
		case C4MC_Drag_None: case C4MC_Drag_Moving: case C4MC_Drag_Construct:
			// Hotspot offset
			switch (Cursor)
				{
				case C4MC_Cursor_Region: case C4MC_Cursor_Select: case C4MC_Cursor_DragMenu:
					iOffsetX=iOffsetY=0;
					break;
				case C4MC_Cursor_Dig: case C4MC_Cursor_DigMaterial:
					iOffsetX=0; iOffsetY=GfxR->fctMouseCursor.Hgt;
					break;
				case C4MC_Cursor_Construct:
					iOffsetX=DragImage.Wdt/2; iOffsetY=DragImage.Hgt;
					break;
				default:
					iOffsetX = GfxR->fctMouseCursor.Wdt/2;
					iOffsetY = GfxR->fctMouseCursor.Hgt/2;
					break;
				}
			// Add mark
			bool fAddMark; fAddMark=false;
			if (ShiftDown)
				if ((Cursor!=C4MC_Cursor_Region) && (Cursor!=C4MC_Cursor_Select) && (Cursor!=C4MC_Cursor_DragMenu)
				 && (Cursor!=C4MC_Cursor_JumpLeft) && (Cursor!=C4MC_Cursor_JumpRight))
					fAddMark=true;
			// Drag image
			if (DragImage.Surface)
				DragImage.Draw(cgo.Surface,cgo.X+VpX-iOffsetX,cgo.Y+VpY-iOffsetY,DragImagePhase);
			// Cursor
			else
				GfxR->fctMouseCursor.Draw(cgo.Surface,cgo.X+VpX-iOffsetX,cgo.Y+VpY-iOffsetY,Cursor);
			// Point
			if ((ShowPointX!=-1) && (ShowPointY!=-1))
				GfxR->fctMouseCursor.Draw( cgo.Surface,
																	 cgo.X+ShowPointX-cgo.TargetX-GfxR->fctMouseCursor.Wdt/2,
																	 cgo.Y+ShowPointY-cgo.TargetY-GfxR->fctMouseCursor.Hgt/2,
																	 C4MC_Cursor_Point );
			// Add mark
			if (fAddMark)
				GfxR->fctMouseCursor.Draw( cgo.Surface,
																	 cgo.X+VpX-iOffsetX+8,
																	 cgo.Y+VpY-iOffsetY+8,
																	 C4MC_Cursor_Add );
			break;
		//------------------------------------------------------------------------------------------
		case C4MC_Drag_Selecting:
			// Draw frame
			Engine.DDraw.DrawFrame( cgo.Surface, 
															cgo.X+VpX,
															cgo.Y+VpY,
															DownX+cgo.X-cgo.TargetX,
															DownY+cgo.Y-cgo.TargetY,
															CRed );
			break;
		//------------------------------------------------------------------------------------------
		}

	// Draw caption
	if (Caption[0])
		{
		int iWdt=Engine.DDraw.TextWidth(Caption);
		int iHgt=Engine.DDraw.TextHeight(Caption);
		Engine.DDraw.TextOut( Caption,
													cgo.Surface,
													cgo.X+BoundBy(VpX,iWdt/2+1,cgo.Wdt-iWdt/2-1),
													cgo.Y+Min( CaptionBottomY ? CaptionBottomY-iHgt-1 : VpY+13, cgo.Hgt-iHgt),
													FRed,FBlack,ACenter);
		}

	}

void C4MouseControl::UpdateCursorTarget()
	{
	int iLastCursor = Cursor;
  
	// Scrolling: no other target
	if (Scrolling) { TargetObject=NULL; return; }

	// On target region
	if (TargetRegion) 
		{ 
		TargetObject=NULL; 
		if (Help) Cursor=C4MC_Cursor_Help;
		return; 
		}

	// Check player cursor
	C4Object *pPlrCursor = pPlayer->Cursor;
	
	// Target object
  DWORD ocf=OCF_Grab | OCF_Chop | OCF_Container | OCF_Construct | OCF_Living | OCF_Carryable | OCF_Container;
	if (Help) ocf|=OCF_All;
  TargetObject=Game.AtObject(X,Y,ocf);
  
  // Movement
  Cursor=C4MC_Cursor_Crosshair;
	
	// Dig
	if (GBackSolid(X,Y))
		{
		Cursor=C4MC_Cursor_Dig;
		if (ControlDown) Cursor=C4MC_Cursor_DigMaterial;
		}

	// Target action
  if (TargetObject)
    {
		// Enter (containers)
    if (ocf & OCF_Container) 
			if (TargetObject->OCF & OCF_Entrance)
				Cursor=C4MC_Cursor_Enter;
		// Grab / Ungrab
    if (ocf & OCF_Grab) 
			{
			Cursor=C4MC_Cursor_Grab;
			if (pPlrCursor)
				if (pPlrCursor->GetProcedure()==DFA_PUSH)
					if (pPlrCursor->Action.Target==TargetObject)
						Cursor=C4MC_Cursor_Ungrab;
			}
		// Object
		if (ocf & OCF_Carryable) 
			{
			Cursor=C4MC_Cursor_Object;
			if (ocf & OCF_InSolid) Cursor=C4MC_Cursor_DigObject;
			}
		// Chop (reduced range)
    if (ocf & OCF_Chop) 
			if (Inside(X-TargetObject->x,-TargetObject->Shape.Wdt/3,+TargetObject->Shape.Wdt/3))
				if (Inside(Y-TargetObject->y,-TargetObject->Shape.Wdt/2,+TargetObject->Shape.Wdt/3))
					Cursor=C4MC_Cursor_Chop;
		// Enter
    if (ocf & OCF_Entrance)
			Cursor=C4MC_Cursor_Enter;
		// Build
    if (ocf & OCF_Construct) Cursor=C4MC_Cursor_Build;
		// Select
    if (ocf & OCF_Living)
      if (ValidPlr(Player))
        if (Game.Players.Get(Player)->ObjectInCrew(TargetObject))
          Cursor=C4MC_Cursor_Select;
		// Attack
    if (ocf & OCF_Living)
			if (TargetObject->Alive)
				if (Hostile(Player,TargetObject->Owner))
					Cursor=C4MC_Cursor_Attack;
		}

	// Jump
	if (pPlrCursor)
		if (!pPlrCursor->Contained)
			if (pPlrCursor->GetProcedure()==DFA_WALK)
				if (Inside(Y-pPlrCursor->y,-25,-10))
					{
					if (Inside(X-pPlrCursor->x,-15,-1)) Cursor=C4MC_Cursor_JumpLeft;
					if (Inside(X-pPlrCursor->x,+1,+15)) Cursor=C4MC_Cursor_JumpRight;
					}

	// Help
	if (Help) 
		Cursor=C4MC_Cursor_Help;

	// Time on target: caption
	if (Cursor==iLastCursor)
		{
		TimeOnTargetObject++;
		if (TimeOnTargetObject>=C4MC_Time_on_Target)
			{
			WORD idCaption=0; const char *szName=""; bool fDouble=false; C4Def *pDef;
			if (TargetObject) szName=TargetObject->GetName();
			// Target caption by cursor
			switch (Cursor)
				{
				case C4MC_Cursor_Select: idCaption=IDS_CON_SELECT; break;
				case C4MC_Cursor_JumpLeft: case C4MC_Cursor_JumpRight: idCaption=IDS_CON_JUMP; break;
				case C4MC_Cursor_Grab: idCaption=IDS_CON_GRAB; fDouble=true; break;
				case C4MC_Cursor_Ungrab: idCaption=IDS_CON_UNGRAB; fDouble=true; break;
				case C4MC_Cursor_Build: idCaption=IDS_CON_BUILD; fDouble=true; break;
				case C4MC_Cursor_Chop: idCaption=IDS_CON_CHOP; fDouble=true; break;
				case C4MC_Cursor_Object: idCaption=IDS_CON_COLLECT; fDouble=true; break;
				case C4MC_Cursor_DigObject: idCaption=IDS_CON_DIGOUT; fDouble=true; break;
				case C4MC_Cursor_Enter: idCaption=IDS_CON_ENTER; fDouble=true; break;
				case C4MC_Cursor_Attack: idCaption=IDS_CON_ATTACK; fDouble=true; break;
				case C4MC_Cursor_Help: idCaption=IDS_CON_NAME; break;
				case C4MC_Cursor_DigMaterial: 
					if (MatValid(GBackMat(X,Y))) 
						if (pDef=C4Id2Def(Game.Material.Map[GBackMat(X,Y)].Dig2Object))
							{ idCaption=IDS_CON_DIGOUT; fDouble=true; szName=pDef->Name; }
					break;
				}
			// Set caption
			if (idCaption) if (!KeepCaption)
				{
				// Caption by cursor
				sprintf(Caption,LoadResStr(idCaption),szName);
				if (fDouble) { SAppendChar('|',Caption); SAppend(LoadResStr(IDS_CON_DOUBLECLICK),Caption); }
				}
			}
		}
	else
		TimeOnTargetObject=0;

	}

int C4MouseControl::UpdateCrewSelection()
	{
	Selection.Clear();
	// Add all crew objects in drag frame to Selection
  C4Object *cObj; C4ObjectLink *cLnk;
  for (cLnk=pPlayer->Crew.First; cLnk && (cObj=cLnk->Obj); cLnk=cLnk->Next)
		if (Inside(cObj->x,Min(X,DownX),Max(X,DownX)))
			if (Inside(cObj->y,Min(Y,DownY),Max(Y,DownY)))
				Selection.Add(cObj);	
	return Selection.ObjectCount();
	}

int C4MouseControl::UpdateObjectSelection()
	{
	Selection.Clear();
	// Add all collectible objects in drag frame to Selection
  C4Object *cObj; C4ObjectLink *cLnk;
  for (cLnk=Game.Objects.First; cLnk && (cObj=cLnk->Obj); cLnk=cLnk->Next)
		if (cObj->Status)
			if (cObj->OCF & OCF_Carryable)
				if (!cObj->Contained)
					if (Inside(cObj->x,Min(X,DownX),Max(X,DownX)))
						if (Inside(cObj->y,Min(Y,DownY),Max(Y,DownY)))
							{
							Selection.Add(cObj);
							if (Selection.ObjectCount()>=20) break; // max. 20 objects
							}
	return Selection.ObjectCount();
	}

int C4MouseControl::UpdateSingleSelection()
	{
	
	// Set single crew selection if cursor on crew (clear prior object selection)
	if (TargetObject && (Cursor==C4MC_Cursor_Select))
		{	Selection.Clear(); Selection.Add(TargetObject);	}

	// Cursor has moved off single crew selection: clear selection
	else if (Game.Players.Get(Player)->ObjectInCrew(Selection.GetObject()))
		Selection.Clear();

	return Selection.ObjectCount();
	}

void C4MouseControl::UpdateScrolling()
	{
	// Assume no scrolling
	Scrolling=false;
	// No scrolling if on region
	if (TargetRegion) return;
	// Scrolling on border
	if (VpX==0)
		{ Cursor=C4MC_Cursor_Left; pPlayer->ScrollView(-ScrollSpeed,0,Viewport->ViewWdt,Viewport->ViewHgt); Scrolling=true; }
	if (VpY==0) 
		{ Cursor=C4MC_Cursor_Up; pPlayer->ScrollView(0,-ScrollSpeed,Viewport->ViewWdt,Viewport->ViewHgt); Scrolling=true; }
	if (VpX==Viewport->ViewWdt-1)
		{ Cursor=C4MC_Cursor_Right; pPlayer->ScrollView(+ScrollSpeed,0,Viewport->ViewWdt,Viewport->ViewHgt); Scrolling=true; }
	if (VpY==Viewport->ViewHgt-1)
		{ Cursor=C4MC_Cursor_Down; pPlayer->ScrollView(0,+ScrollSpeed,Viewport->ViewWdt,Viewport->ViewHgt); Scrolling=true; }
	// Set correct cursor
	if ((VpX==0) && (VpY==0))	Cursor=C4MC_Cursor_UpLeft;
	if ((VpX==Viewport->ViewWdt-1) && (VpY==0))	Cursor=C4MC_Cursor_UpRight;
	if ((VpX==0) && (VpY==Viewport->ViewHgt-1))	Cursor=C4MC_Cursor_DownLeft;
	if ((VpX==Viewport->ViewWdt-1) && (VpY==Viewport->ViewHgt-1))	Cursor=C4MC_Cursor_DownRight;
	}

void C4MouseControl::UpdateTargetRegion()
	{
	// Assume no region
	TargetRegion=NULL;
	// Find region
	if (!(TargetRegion=Viewport->Regions.Find(VpX,VpY))) return;
	// Region found: no target object
	TargetObject=NULL;
	// Cursor
	Cursor=C4MC_Cursor_Region;
	// Stop drag selecting (reset down cursor, too)
	if (Drag==C4MC_Drag_Selecting)
		{ Drag=C4MC_Drag_None; DownCursor=C4MC_Cursor_Nothing; }
	// Caption
	SCopy(TargetRegion->Caption,Caption,C4MC_MaxCaption);
	CaptionBottomY=TargetRegion->Y; KeepCaption=0;
	// Help region caption by region target object
	if (Help)
		if (TargetRegion->Target)
			{
			sprintf(Caption,"%s: ",TargetRegion->Target->Def->Name);						
			SCopy(TargetRegion->Target->Def->Desc,Caption+SLen(Caption),C4MC_MaxCaption-SLen(Caption));
			SWordWrap(Caption,' ','|',50);
			}
	// Menu drag
	if ((TargetRegion->Com==COM_MenuDrag) || (TargetRegion->Com==COM_MenuDragScroll))
		Cursor=C4MC_Cursor_DragMenu;
	// MoveOverCom (on region change)
	static int iLastRegionX,iLastRegionY;
	if (TargetRegion->MoveOverCom)
		{
		if ((TargetRegion->X!=iLastRegionX) || (TargetRegion->Y!=iLastRegionY))
			{
			iLastRegionX=TargetRegion->X; iLastRegionY=TargetRegion->Y;

			// Local control menu
			if (TargetRegion->Menu)						// use SendControl (but that uses DownRegion.Menu...)
				{
				if (IsValidMenu(TargetRegion->Menu))
					TargetRegion->Menu->Control(TargetRegion->MoveOverCom,TargetRegion->Data);
				}
			// Control queue
			else
				{
				Game.Input.AddPlayerControl(Player,TargetRegion->MoveOverCom,TargetRegion->Data);
				}

			}
		}
	else
		{
		iLastRegionX=iLastRegionY=-1;
		}

	}

BOOL C4MouseControl::UpdatePutTarget(BOOL fVehicle)
	{
	
	// Target object
  DWORD ocf=OCF_Container;
	char szName[256+1];
  if (TargetObject=Game.AtObject(X,Y,ocf))
		{
		// Cursor
		if (fVehicle) Cursor=C4MC_Cursor_VehiclePut;
		else Cursor=C4MC_Cursor_Put;
		// Caption
		if (Selection.GetObject())
			{
			// Single object name
			SCopy(Selection.GetObject()->GetName(),szName);
			// Multiple object name
			if (Selection.ObjectCount()>1)
				sprintf(szName,"%d %s",Selection.ObjectCount(),LoadResStr(fVehicle ? IDS_CON_VEHICLES : IDS_CON_ITEMS) );
			// Set caption
			sprintf(Caption,LoadResStr(fVehicle ? IDS_CON_VEHICLEPUT : IDS_CON_PUT),szName,TargetObject->GetName());
			}
		// Put target found
		return TRUE;
		}
  
	return FALSE;
	}

void C4MouseControl::DragMenu()
	{
	// Local control menu
	if (DownRegion.Menu)
		{
		if (!IsValidMenu(DownRegion.Menu))	return;
		DownRegion.Menu->DragLocation( VpX+DownOffsetX, VpY+DownOffsetY );
		}
	// DownTarget's object menu
	else
		{
		if (!DownTarget || !DownTarget->Menu) return;
		DownTarget->Menu->DragLocation( VpX+DownOffsetX, VpY+DownOffsetY );
		}
	}

void C4MouseControl::DragMenuScroll()
	{
	// Local control menu
	if (DownRegion.Menu)
		{
		if (!IsValidMenu(DownRegion.Menu))	return;
		DownRegion.Menu->DragScroll( VpX+DownOffsetX, VpY+DownOffsetY );
		}
	// DownTarget's object menu
	else
		{
		if (!DownTarget || !DownTarget->Menu) return;
		DownTarget->Menu->DragScroll( VpX+DownOffsetX, VpY+DownOffsetY );
		}
	}

void C4MouseControl::LeftDown()
	{
	// Set flag
	LeftButtonDown=true; 
	// Store down values (same MoveRightDown -> use StoreDown)
	DownX=X; DownY=Y;
	DownCursor=Cursor;
	DownTarget=TargetObject;
	DownRegion.Default();
	if (TargetRegion) 
		{
		DownRegion=(*TargetRegion);
		DownTarget=TargetRegion->Target;
		DownOffsetX=TargetRegion->X-VpX; DownOffsetY=TargetRegion->Y-VpY; 
		}
	}

void C4MouseControl::DragSelect()
	{
	switch (DragSelecting)
		{
		case C4MC_Selecting_Unknown:
			// Determine selection type
			if (UpdateCrewSelection()) { DragSelecting=C4MC_Selecting_Crew; break; }
			if (UpdateObjectSelection()) { DragSelecting=C4MC_Selecting_Objects; break; }
			break;
		case C4MC_Selecting_Crew:
			// Select crew
			UpdateCrewSelection();
			break;
		case C4MC_Selecting_Objects:
			// Select objects
			UpdateObjectSelection();
			break;
		}
	}

void C4MouseControl::LeftUp()
	{
	// Update status flag
	LeftButtonDown=false;
	// Ignore left up after double click
	if (LeftDoubleIgnoreUp) { LeftDoubleIgnoreUp=false; return; }
	// Evaluate by drag status
	switch (Drag)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_None: LeftUpDragNone(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Selecting:	ButtonUpDragSelecting(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Moving: ButtonUpDragMoving(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Construct: ButtonUpDragConstruct(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Menu: case C4MC_Drag_MenuScroll: ButtonUpDragMenu(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}			
	}

void C4MouseControl::DragMoving()
	{
	ShowPointX=ShowPointY=-1;

	// Carryable object
	if (Selection.GetObject() && (Selection.GetObject()->OCF & OCF_Carryable))
		{
		// Default object cursor
		Cursor = C4MC_Cursor_Object;
		// Check for put target
		if (ControlDown)
			if (UpdatePutTarget(FALSE))
				return;
		// In liquid: drop
		if (GBackLiquid(X,Y))
			{ Cursor = C4MC_Cursor_Drop; return; }
		// In free: drop or throw
		if (!GBackSolid(X,Y))
			{
			// Check drop
			int iX=X,iY=Y;
			while ((iY<GBackHgt) && !GBackSolid(iX,iY)) iY++;
			if (Inside(X-iX,-5,+5) && Inside(Y-iY,-5,+5))
				{ Cursor = C4MC_Cursor_Drop; return; }
			// Throwing physical
			FIXED fixThrow = ftofix(4.0*(float)50000/(float)C4MaxPhysical);
			if (pPlayer->Cursor) fixThrow=ftofix(4.0*(float)pPlayer->Cursor->GetPhysical()->Throw/(float)C4MaxPhysical);
			// Preferred throwing direction
			int iDir=+1; if (pPlayer->Cursor) if (pPlayer->Cursor->x > X) iDir=-1;
			// Throwing height
			int iHeight=20; if (pPlayer->Cursor) iHeight=pPlayer->Cursor->Shape.Hgt;
			// Check throw
            long lx=iX, ly=iY;
			if (FindThrowingPosition(X,Y,fixThrow*iDir,-fixThrow,iHeight,lx,ly)
			 || FindThrowingPosition(X,Y,fixThrow*(iDir*=-1),-fixThrow,iHeight,lx,ly))
				{ 
                iX=(int)lx; iY=(int)ly;
				Cursor = (iDir==-1) ? C4MC_Cursor_ThrowLeft : C4MC_Cursor_ThrowRight; 
				ShowPointX=iX; ShowPointY=iY; 
				return;
				}
			}
		}

	// Vehicle
	else
		{
		// PushTo
		Cursor = C4MC_Cursor_Vehicle;
		// Check for put target
		if (ControlDown)
			UpdatePutTarget(TRUE);
		}

	}

void C4MouseControl::DragNone()
	{
	
	// Holding left down
	if (LeftButtonDown)
		{
		switch (Cursor)
			{
			// Hold down on region
			case C4MC_Cursor_Region:
				if (!Tick5)
					if (DownRegion.HoldCom)
						SendControl(DownRegion.HoldCom);
				break;
			}
		}

	// Button down: begin drag
	if ( (LeftButtonDown || RightButtonDown)
		&& ((Abs(X-DownX)>C4MC_DragSensitivity) || (Abs(Y-DownY)>C4MC_DragSensitivity)) )
		{
		switch (DownCursor)
			{
			// Drag start selecting in landscape
			case C4MC_Cursor_Crosshair: case C4MC_Cursor_Dig:
				Selection.Clear();
				Drag=C4MC_Drag_Selecting; DragSelecting=C4MC_Selecting_Unknown; 
				break;
			// Drag object from landscape
			case C4MC_Cursor_Object: case C4MC_Cursor_DigObject:
				if (DownTarget) 
					{ 
					Drag=C4MC_Drag_Moving; 
					// Down target is not part of selection: drag single object
					if (!Selection.GetLink(DownTarget))
						{	Selection.Clear(); Selection.Add(DownTarget); }
					}
				break;
			// Drag vehicle from landscape
			case C4MC_Cursor_Grab: case C4MC_Cursor_Ungrab:
				if (DownTarget) 
					if (DownTarget->Def->Pushable == 1)
						{ Drag=C4MC_Drag_Moving; Selection.Clear(); Selection.Add(DownTarget); }
				break;
			// Drag menu (scroll)
			case C4MC_Cursor_DragMenu:
				if (DownTarget || DownRegion.Menu)
					{ Drag=C4MC_Drag_Menu; if (DownRegion.Com==COM_MenuDragScroll) Drag=C4MC_Drag_MenuScroll; Selection.Clear(); } 
				break;
			// Drag from region
			case C4MC_Cursor_Region:
				// Drag object(s) or vehicle(s)
				if ( DownRegion.Target 
					&& ((DownRegion.Target->OCF & OCF_Carryable) || (DownRegion.Target->Def->Pushable==1)) )
						{ 
						Drag=C4MC_Drag_Moving;
						Selection.Clear();
						// Multiple object selection from container
						if (RightButtonDown && DownRegion.Target->Contained && (DownRegion.Target->Contained->Contents.ObjectCount(DownRegion.Target->id)>1) )
							{
							for (C4ObjectLink *cLnk=DownRegion.Target->Contained->Contents.First; cLnk && cLnk->Obj; cLnk=cLnk->Next)
								if (cLnk->Obj->id==DownRegion.Target->id)
									Selection.Add(cLnk->Obj);
							}
						// Single object selection
						else
							Selection.Add(DownRegion.Target); 
						break; 
						}
				// Drag id (construction)
				C4Def *pDef;
				if (DownRegion.id)
					if ((pDef=C4Id2Def(DownRegion.id)) && pDef->Constructable)
						{ Drag=C4MC_Drag_Construct; DragID=DownRegion.id; CreateDragImage(DragID); Selection.Clear(); break; }
				break;
			// Help: no dragging
			case C4MC_Cursor_Help:
				break;
			}
		}
	
	// Regular cursor movement
	else
		{
		// Cursor movement
		UpdateCursorTarget();
		// Update selection
		UpdateSingleSelection();
		}

	}

void C4MouseControl::LeftDouble()
	{
	// Update status flag
	LeftButtonDown=false;
	// Set ignore flag for next left up
	LeftDoubleIgnoreUp=true;
	// Evaluate left double by drag status (can only be C4MC_Drag_None really)
	switch (Drag)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_None:
			// Double left click (might be on a target)
			switch (Cursor)
				{
				case C4MC_Cursor_Attack: SendCommand(C4CMD_Attack,X,Y,TargetObject); break;
				case C4MC_Cursor_Grab: SendCommand(C4CMD_Grab,X,Y,TargetObject); break;
				case C4MC_Cursor_Ungrab: SendCommand(C4CMD_UnGrab,X,Y,TargetObject); break;
				case C4MC_Cursor_Build: SendCommand(C4CMD_Build,X,Y,TargetObject); break;
				case C4MC_Cursor_Chop: SendCommand(C4CMD_Chop,X,Y,TargetObject); break;
				case C4MC_Cursor_Enter: SendCommand(C4CMD_Enter,X,Y,TargetObject); break;
				case C4MC_Cursor_Object: case C4MC_Cursor_DigObject: SendCommand(C4CMD_Get,X,Y,TargetObject); break;
				case C4MC_Cursor_Dig: SendCommand(C4CMD_Dig,X,Y,NULL); break;
				case C4MC_Cursor_DigMaterial: SendCommand(C4CMD_Dig,X,Y,NULL,NULL,TRUE); break;
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	}

void C4MouseControl::RightDown()
	{
	// Update status flag
	RightButtonDown=true;
	// Store down values (same MoveLeftDown -> use StoreDown)
	DownX=X; DownY=Y;
	DownCursor=Cursor;
	DownTarget=TargetObject;
	DownRegion.Default();
	if (TargetRegion) 
		{
		DownRegion=(*TargetRegion);
		DownTarget=TargetRegion->Target;
		DownOffsetX=TargetRegion->X-VpX; DownOffsetY=TargetRegion->Y-VpY; 
		}
	}

void C4MouseControl::RightUp()
	{
	// Update status flag
	RightButtonDown=false;
	// Evaluate by drag status
	switch (Drag)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_None: RightUpDragNone(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Selecting:	ButtonUpDragSelecting(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Moving: ButtonUpDragMoving(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Construct: ButtonUpDragConstruct(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4MC_Drag_Menu: case C4MC_Drag_MenuScroll: ButtonUpDragMenu(); break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}			
	}

BOOL C4MouseControl::IsValidMenu(C4Menu *pMenu)
	{
	// Local control fullscreen menu
	if (pMenu == &(FullScreen.Menu)) 
		if (pMenu->IsActive())
			return TRUE;
	// Local control player menu
	C4Player *pPlr;
	for (int cnt=0; pPlr=Game.Players.Get(cnt); cnt++)
		if (pMenu == &(pPlr->Menu))
			if (pMenu->IsActive())
				return TRUE;
	// No match found
	return FALSE;
	}

BOOL C4MouseControl::SendControl(int iCom, int iData)
	{
	// Help
	if (iCom==COM_Help)
		{
		Help=true;
		return TRUE;
		}
	// Activate player menu (local control)
	if (iCom==COM_PlayerMenu)
		{
		pPlayer->ActivateMenuMain();
		return TRUE;
		}
	// Local control player menu
	if (DownRegion.Menu) // use other general ControlMenu pointer... or use iPlayer
		{
		if (!IsValidMenu(DownRegion.Menu))	return FALSE;
		DownRegion.Menu->Control(iCom,iData);
		return TRUE;
		}
	// Player control queue
	Game.Input.AddPlayerControl(Player,iCom,iData);
	// Done
	return TRUE;
	}

void C4MouseControl::CreateDragImage(C4ID id)
	{
	// Clear old image
	DragImage.Clear(); DragImage.Default();
	// Get definition
	C4Def *pDef=C4Id2Def(id); if (!pDef) return;
	// Create image
	if (pDef->DragImagePicture)
		{
		if (!DragImage.Create(pDef->PictureRect.Wdt*2,pDef->PictureRect.Hgt)) return;
		DragImage.Set(DragImage.Surface,0,0,pDef->PictureRect.Wdt,pDef->PictureRect.Hgt);
		pDef->Draw(DragImage);
		}
	else
		{
		if (!DragImage.Create(pDef->MainFace[0].Wdt*2,pDef->MainFace[0].Hgt)) return;
		DragImage.Set(DragImage.Surface,0,0,pDef->MainFace[0].Wdt,pDef->MainFace[0].Hgt);
		pDef->MainFace[0].Draw(DragImage);
		}
	// Make color mask
	BYTE RedTable[256]; FillMem(RedTable,256,45); RedTable[0]=0;
	C4Facet cgo=DragImage.GetPhase(0);
	Engine.DDraw.DrawBoxColorTable(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,RedTable);
	Engine.DDraw.DrawInline(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,16,45,0);
	Engine.DDraw.DrawInline(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,43,45,16);
	C4Facet fctPhase = DragImage.GetPhase(1);
	DragImage.Draw(fctPhase);
	ZeroMem(RedTable,256); RedTable[16]=43; RedTable[43]=45;
	cgo=DragImage.GetPhase(1);
	Engine.DDraw.DrawBoxColorTable(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,RedTable);
	}

void C4MouseControl::DragConstruct()
	{
	Cursor=C4MC_Cursor_Construct;
	// Check site
	DragImagePhase=1;
	if (ConstructionCheck(DragID,X,Y)) DragImagePhase=0;
	}

void C4MouseControl::LeftUpDragNone()
	{
	// Single left click (might be on a target)
	switch (Cursor)
		{
		// Region 
		case C4MC_Cursor_Region:							
			// Help click on region: ignore
			if (Help) break;
			// Region com & data
			SendControl(DownRegion.Com,DownRegion.Data);
			break;
		// Selection
		case C4MC_Cursor_Select:
			// Crew selection to control queue
			Game.Input.AddPlayerSelection(Player,Selection);
			break;
		// Jump
		case C4MC_Cursor_JumpLeft: case C4MC_Cursor_JumpRight:
			SendCommand(C4CMD_Jump,X,Y,NULL);
			break;
		// Help
		case C4MC_Cursor_Help:
			if (DownTarget)
				{
				sprintf(Caption,"%s: ",DownTarget->Def->Name);						
				SCopy(DownTarget->Def->Desc,Caption+SLen(Caption),C4MC_MaxCaption-SLen(Caption));
				SWordWrap(Caption,' ','|',50);
				KeepCaption=SLen(Caption)/2;
				}
			break;
		// Nothing
		case C4MC_Cursor_Nothing:
			break;
		// Movement
		default:
			// MoveTo command to control queue
			SendCommand(C4CMD_MoveTo,X,Y,NULL);
			break;
		}
	// Clear selection
	Selection.Clear();						
	}

void C4MouseControl::ButtonUpDragSelecting()
	{
	// Finish drag
	Drag=C4MC_Drag_None;
	// Crew selection to control queue
	if (DragSelecting==C4MC_Selecting_Crew)
		{
		Game.Input.AddPlayerSelection(Player,Selection);
		Selection.Clear();
		}
	// Object selection: just keep selection
	if (DragSelecting==C4MC_Selecting_Objects)
		{

		}
	}

void C4MouseControl::ButtonUpDragMoving()
	{
	// Finish drag
	Drag=C4MC_Drag_None;
	// Evaluate to command by cursor for each selected object
	C4ObjectLink *pLnk; C4Object *pObj;
	int iCommand; C4Object *pTarget1,*pTarget2;
	int iAddMode; iAddMode=C4P_Command_Set;
	for (pLnk=Selection.First; pLnk && (pObj=pLnk->Obj); pLnk=pLnk->Next)
		{
		iCommand=C4CMD_None; pTarget1=pTarget2=NULL;
		switch (Cursor)
			{
			case C4MC_Cursor_ThrowLeft: case C4MC_Cursor_ThrowRight:
				iCommand=C4CMD_Throw; pTarget1=pObj; ShowPointX=ShowPointY=-1; break;
			case C4MC_Cursor_Drop:
				iCommand=C4CMD_Drop; pTarget1=pObj; break;
			case C4MC_Cursor_Put:
				iCommand=C4CMD_Put; pTarget1=TargetObject; pTarget2=pObj; break;
			case C4MC_Cursor_Vehicle:
				iCommand=C4CMD_PushTo; pTarget1=pObj; break;
			case C4MC_Cursor_VehiclePut:
				iCommand=C4CMD_PushTo; pTarget1=pObj; pTarget2=TargetObject; break;
			}
		// Set first command, append all following
		SendCommand(iCommand,X,Y,pTarget1,pTarget2,0,iAddMode);
		iAddMode=C4P_Command_Append;
		}
	// Clear selection
	Selection.Clear();
	}

void C4MouseControl::ButtonUpDragMenu()
	{
	// Finish drag
	Drag=C4MC_Drag_None;
	// Clear selection (necessary?)
	Selection.Clear();
	}

void C4MouseControl::ButtonUpDragConstruct()
	{
	// Finish drag
	Drag=C4MC_Drag_None; 
	DragImage.Clear(); DragImage.Default();
	// Command
	if (DragImagePhase==0) // if ConstructionCheck was okay (check again?)
		SendCommand(C4CMD_Construct,X,Y,NULL,NULL,DragID);
	// Clear selection (necessary?)
	Selection.Clear();
	}

void C4MouseControl::SendCommand(int iCommand, int iX, int iY, C4Object *pTarget, C4Object *pTarget2, int iData, int iAddMode)
	{
	// User add multiple command mode
	if (ShiftDown) iAddMode|=C4P_Command_Append;
	// Command to control queue
	Game.Input.AddPlayerCommand(Player,iCommand,iX,iY,pTarget,pTarget2,iData,iAddMode);
	}

void C4MouseControl::RightUpDragNone()
	{
	
	// Region: send control
	if (Cursor==C4MC_Cursor_Region)
		{ SendControl(DownRegion.RightCom); return; }

	// Help: end
	if (Help)
		{ Help=false; KeepCaption=0; return; }

	// Selection: send selection (not exclusive)
	if (Cursor==C4MC_Cursor_Select)
		Game.Input.AddPlayerSelection(Player,Selection);

	// Check for any secondary context target objects
	DWORD ocf=OCF_All;
	if (!TargetObject) TargetObject=Game.AtObject(X,Y,ocf);
	// Avoid stinkin' Windrad - cheaper goes it not
	if (TargetObject && (TargetObject->id==C4Id("WWNG"))) TargetObject=Game.AtObject(X,Y,ocf,TargetObject);

	// Target object: context menu
	if (TargetObject)
		{
		SendCommand(C4CMD_Context,X-Viewport->ViewX,Y-Viewport->ViewY,NULL,TargetObject);
		return;
		}

	// Free click: select next clonk
	SendPlayerSelectNext();

	}

void C4MouseControl::UpdateFogOfWar()
	{
	// Assume no fog of war
	FogOfWar=false;
	// No fog of war block if on region or dragging menu
	if (TargetRegion) return;
	if ((Drag==C4MC_Drag_Menu) || (Drag==C4MC_Drag_MenuScroll)) return;
	// Check for fog of war
	if (pPlayer->FogOfWar)
		// On fog of war
		if (!pPlayer->FogOfWar->IsVisible(X,Y))
			{
			FogOfWar=true;
			if (!Scrolling) Cursor=C4MC_Cursor_Nothing;
			Drag=C4MC_Drag_None; DownCursor=C4MC_Cursor_Nothing;
			ShowPointX=ShowPointY=-1;
			DragImage.Clear(); DragImage.Default();
			}
	}

void C4MouseControl::SendPlayerSelectNext()
	{
	C4ObjectLink *cLnk;
	if (cLnk=pPlayer->Crew.GetLink(pPlayer->Cursor))
		for (cLnk=cLnk->Next; cLnk; cLnk=cLnk->Next)
			if (cLnk->Obj->Status) break;
	if (!cLnk)
		for (cLnk=pPlayer->Crew.First; cLnk; cLnk=cLnk->Next)
			if (cLnk->Obj->Status) break;
	if (cLnk) 
		{
		// Crew selection to control queue
		Selection.Clear(); Selection.Add(cLnk->Obj);
		Game.Input.AddPlayerSelection(Player,Selection);
		Selection.Clear();						
		}
	}

void C4MouseControl::ShowCursor()
	{
	Visible=true;
	}

void C4MouseControl::HideCursor()
	{
	Visible=false;
	}

const char *C4MouseControl::GetCaption()
	{
	return Caption;
	}
