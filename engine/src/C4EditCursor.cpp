/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles viewport editing in console mode */

#include <C4Include.h>

C4EditCursor::C4EditCursor()
	{
	Default();
	}

C4EditCursor::~C4EditCursor()
	{
	Clear();
	}

void C4EditCursor::Execute()
	{
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			// Hold selection
			C4Object *cobj; C4ObjectLink *clnk;
			if (Hold)
				for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
					{
					cobj->ForcePosition(cobj->x,cobj->y);			
					cobj->xdir=cobj->ydir=0;
					}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			switch (Console.ToolsDlg.Tool)
				{
				case C4TLS_Fill: 
					if (Hold) if (!Game.Halt) if (Console.Editing) ApplyToolFill();
					break;
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	}

BOOL C4EditCursor::Init(HINSTANCE hInst)
	{
	
	if (!(hMenu = LoadMenu(hInst,MAKEINTRESOURCE(IDR_CONTEXTMENUS)))) 
		return FALSE;

	Console.UpdateModeCtrls(Mode);

	return TRUE;
	}

void C4EditCursor::ClearPointers(C4Object *pObj)
	{
	if (Target==pObj) Target=NULL;
	if (Selection.ClearPointers(pObj))
		Console.PropertyDlg.Update(Selection);
	UpdateStatusBar();
	}

BOOL C4EditCursor::Move(int iX, int iY, WORD wKeyFlags)
	{
	// Offset movement
	int xoff = iX-X; int yoff = iY-Y;	X=iX; Y=iY;
	
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			// Drag frame
			if (DragFrame) UpdateGraphicsSystem(); 
			// Hold
			else if (Hold) 
				{
				MoveSelection(xoff,yoff); 
				UpdateDropTarget(wKeyFlags); 
				UpdateGraphicsSystem(); 
				}
			// Update target
			else Target = Game.FindObject(0,X,Y,0,0,OCF_NotContained);
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			switch (Console.ToolsDlg.Tool)
				{
				case C4TLS_Brush:
					if (Hold) ApplyToolBrush();
					break;
				case C4TLS_Line: case C4TLS_Rect:
					if (DragLine || DragFrame) UpdateGraphicsSystem();
					break;
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}

	// Update
	UpdateStatusBar();	
	return TRUE;
	}

BOOL C4EditCursor::UpdateStatusBar()
	{
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModePlay:
			SCopyUntil(Game.MouseControl.GetCaption(),OSTR,'|');
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			sprintf(OSTR,"%i/%i (%s)",X,Y,Target ? Target->GetName() : LoadResStr(IDS_CNS_NOTHING) );
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			sprintf(OSTR,"%i/%i (%s)",X,Y,MatValid(GBackMat(X,Y)) ? Game.Material.Map[GBackMat(X,Y)].Name : LoadResStr(IDS_CNS_NOTHING) );
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	return Console.UpdateCursorBar(OSTR);
	}

BOOL C4EditCursor::LeftButtonDown(BOOL fControl)
	{
	
	// Hold
	Hold=TRUE;
	
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			if (fControl)
				{
				// Toggle target
				if (Target)
					if (!Selection.Remove(Target))
						Selection.Add(Target);
				}
			else
				{
				// Click on unselected: select single
				if (Target && !Selection.GetLink(Target))
					{ Selection.Clear(); Selection.Add(Target); }
				// Click on nothing: drag frame
				if (!Target) 
					{ Selection.Clear(); DragFrame=TRUE; X2=X; Y2=Y; }
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			if (fControl)
				ApplyToolPicker();
			else
				switch (Console.ToolsDlg.Tool)
					{
					case C4TLS_Brush: ApplyToolBrush(); break;
					case C4TLS_Line: DragLine=TRUE; X2=X; Y2=Y; break;
					case C4TLS_Rect: DragFrame=TRUE; X2=X; Y2=Y; break;
					case C4TLS_Fill:
						if (Game.Halt) 
							{ Hold=FALSE; Console.Message(LoadResStr(IDS_CNS_FILLNOHALT)); return FALSE; }
						break;
					case C4TLS_Picker: ApplyToolPicker(); break;
					}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}

	DropTarget=NULL;
	
	UpdateStatusBar();
	UpdateGraphicsSystem();
	Console.PropertyDlg.Update(Selection);
	return TRUE;	
	}

BOOL C4EditCursor::RightButtonDown(BOOL fControl)
	{
	
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			if (!fControl)
				{
				// Click on unselected
				if (Target && !Selection.GetLink(Target))
						{ Selection.Clear(); Selection.Add(Target); }
				// Click on nothing
				if (!Target) Selection.Clear();
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}

	UpdateStatusBar();
	UpdateGraphicsSystem();
	Console.PropertyDlg.Update(Selection);
	return TRUE;	
	}

BOOL C4EditCursor::LeftButtonUp()
	{	
	// Finish edit/tool
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit:
			if (DragFrame) FrameSelection(); 
			if (DropTarget) PutContents();
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			switch (Console.ToolsDlg.Tool)
				{
				case C4TLS_Line:
					if (DragLine) ApplyToolLine();
					break;
				case C4TLS_Rect:
					if (DragFrame) ApplyToolRect();
					break;
				}
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	
	// Release
	Hold=FALSE;
	DragFrame=FALSE;
	DragLine=FALSE;
	DropTarget=NULL;
	// Update
	UpdateStatusBar();
	UpdateGraphicsSystem();
	return TRUE;	
	}

BOOL SetMenuItemEnable(HMENU hMenu, WORD id, BOOL fEnable)
	{
	return EnableMenuItem(hMenu,id,MF_BYCOMMAND | MF_ENABLED | ( fEnable ? 0 : MF_GRAYED));
	}

BOOL SetMenuItemText(HMENU hMenu, WORD id, const char *szText)
	{
	MENUITEMINFO minfo;
	ZeroMem(&minfo,sizeof(minfo));
	minfo.cbSize = sizeof(minfo);
	minfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
	minfo.fType = MFT_STRING;
	minfo.wID = id;
	minfo.dwTypeData = (char*) szText;
	minfo.cch = SLen(szText);
	return SetMenuItemInfo(hMenu,id,FALSE,&minfo);
	}

BOOL C4EditCursor::RightButtonUp()
	{
	Target=NULL;

	DoContextMenu();

	// Update
	UpdateStatusBar();
	UpdateGraphicsSystem();
	return TRUE;	
	}

BOOL C4EditCursor::Delete()
	{
	if (!EditingOK()) return FALSE;
	while (Selection.First && Selection.First->Obj)
		Selection.First->Obj->AssignRemoval();
	UpdateStatusBar();
	UpdateGraphicsSystem();
	Console.PropertyDlg.Update(Selection);
	return TRUE;
	}

BOOL C4EditCursor::OpenPropTools()
	{
	switch (Mode)
		{
		case C4CNS_ModeEdit: case C4CNS_ModePlay:
			Console.PropertyDlg.Open();
			Console.PropertyDlg.Update(Selection);
			break;
		case C4CNS_ModeDraw:
			Console.ToolsDlg.Open();
			break;
		}
	return TRUE;
	}

BOOL C4EditCursor::Duplicate()
	{
  C4Object *cobj,*nobj; C4ObjectLink *clnk;
	C4ObjectList newsel;
  for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		if (nobj=Game.CreateObject(cobj->Def->id,cobj->Owner,cobj->x,cobj->y))
			newsel.Add(nobj);
	Selection.Clear();
	for (clnk=newsel.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		Selection.Add(cobj);
	Hold=TRUE;
	Console.PropertyDlg.Update(Selection);
	return TRUE;	
	}

void C4EditCursor::Draw(C4FacetEx &cgo)
	{
	// Draw selection marks
	C4Object *cobj; C4ObjectLink *clnk; C4Facet frame;
  for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		{
		frame.Set(cgo.Surface,
							cobj->x+cobj->Shape.x + cgo.X-cgo.TargetX,
							cobj->y+cobj->Shape.y + cgo.Y-cgo.TargetY,
							cobj->Shape.Wdt,
							cobj->Shape.Hgt);						
		DrawSelectMark(frame);
		}
	// Draw drag frame
	if (DragFrame)
		Engine.DDraw.DrawFrame(cgo.Surface,Min(X,X2)+cgo.X-cgo.TargetX,Min(Y,Y2)+cgo.Y-cgo.TargetY,Max(X,X2)+cgo.X-cgo.TargetX,Max(Y,Y2)+cgo.Y-cgo.TargetY,CWhite);
	// Draw drag line
	if (DragLine)
		Engine.DDraw.DrawLine(cgo.Surface,X+cgo.X-cgo.TargetX,Y+cgo.Y-cgo.TargetY,X2+cgo.X-cgo.TargetX,Y2+cgo.Y-cgo.TargetY,CWhite);
	// Draw drop target
	if (DropTarget)
		Game.GraphicsResource.fctDropTarget.Draw(cgo.Surface,
											 DropTarget->x+cgo.X-cgo.TargetX-5,
											 DropTarget->y+DropTarget->Shape.y+cgo.Y-cgo.TargetY-17);
	}
	

void C4EditCursor::DrawSelectMark(C4Facet &cgo)
	{
	if ((cgo.Wdt<1) || (cgo.Hgt<1)) return;

	Engine.DDraw.SetPixel(cgo.Surface,cgo.X,cgo.Y,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+1,cgo.Y,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X,cgo.Y+1,CWhite);

	Engine.DDraw.SetPixel(cgo.Surface,cgo.X,cgo.Y+cgo.Hgt-1,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+1,cgo.Y+cgo.Hgt-1,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X,cgo.Y+cgo.Hgt-2,CWhite);

	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-2,cgo.Y,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y+1,CWhite);

	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-2,cgo.Y+cgo.Hgt-1,CWhite);
	Engine.DDraw.SetPixel(cgo.Surface,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-2,CWhite);
	}


void C4EditCursor::MoveSelection(int iXOff, int iYOff)
	{
  C4Object *cobj; C4ObjectLink *clnk;
	for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		{
		cobj->ForcePosition(cobj->x+iXOff,cobj->y+iYOff);
		cobj->xdir=cobj->ydir=0;
		}
	}

void C4EditCursor::FrameSelection()
	{
	Selection.Clear();
  C4Object *cobj; C4ObjectLink *clnk;
	for (clnk=Game.Objects.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		if (cobj->Status) if (cobj->OCF & OCF_NotContained)
			if (Inside(cobj->x,Min(X,X2),Max(X,X2)) && Inside(cobj->y,Min(Y,Y2),Max(Y,Y2)))
				Selection.Add(cobj);
	Console.PropertyDlg.Update(Selection);
	}

BOOL C4EditCursor::In(const char *szText)
	{
	// Create valid single statement
	int rval;
	char buffer[1000];
	char objname[C4D_MaxName+1];
	buffer[0]=0;
	SAppend("return(",buffer); SAppend(szText,buffer); SAppend(");",buffer);
	// Pass command line to objects
	C4Object *cobj; C4ObjectLink *clnk,*next;
	for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=next)
		{
		next=clnk->Next;
		SCopy(cobj->GetName(),objname);
		
		C4Thread cthr;
		rval=cthr.Execute(NULL,buffer,NULL,NULL,cobj);
		
		sprintf(OSTR,"%s::%s = %i",objname,szText,rval);
		Console.Out(OSTR);
		}
	Console.PropertyDlg.Update(Selection);
	UpdateGraphicsSystem();
	return TRUE;
	}

void C4EditCursor::Default()
	{
	Mode=C4CNS_ModePlay;
	X=Y=X2=Y2=0;
	Target=DropTarget=NULL;
	hMenu=NULL;
	Hold=DragFrame=DragLine=FALSE;
	Selection.Default();
	}

void C4EditCursor::Clear()
	{
	if (hMenu) DestroyMenu(hMenu); hMenu=NULL;
	Selection.Clear();
	}

void C4EditCursor::UpdateGraphicsSystem()
	{
	if (Game.Halt) Game.GraphicsSystem.Execute();
	}

BOOL C4EditCursor::SetMode(int iMode)
	{
	// Store focus
	HWND hFocus=GetFocus();
	// Update console buttons (always)
	Console.UpdateModeCtrls(iMode);
	// No change
	if (iMode==Mode) return TRUE;
	// Set mode
	Mode = iMode;
	// Update prop tools by mode
	BOOL fOpenPropTools = FALSE;
	switch (Mode)
		{
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeEdit: case C4CNS_ModePlay:
			if (Console.ToolsDlg.hDialog || Console.PropertyDlg.hDialog) fOpenPropTools=TRUE;
			Console.ToolsDlg.Clear();
			if (fOpenPropTools) OpenPropTools(); 
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		case C4CNS_ModeDraw:
			if (Console.ToolsDlg.hDialog || Console.PropertyDlg.hDialog) fOpenPropTools=TRUE;
			Console.PropertyDlg.Clear();
			if (fOpenPropTools) OpenPropTools(); 
			break;
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		}
	// Update cursor
	if (Mode==C4CNS_ModePlay) Game.MouseControl.ShowCursor();
	else Game.MouseControl.HideCursor();
	// Restore focus
	SetFocus(hFocus);
	// Done
	return TRUE;
	}

void C4EditCursor::ToggleMode()
	{
	
	// Step through modes
	int iNewMode;
	switch (Mode)
		{
		case C4CNS_ModePlay: iNewMode=C4CNS_ModeEdit; break;
		case C4CNS_ModeEdit: iNewMode=C4CNS_ModeDraw; break;
		case C4CNS_ModeDraw: iNewMode=C4CNS_ModePlay; break;
		}
	
	// Set new mode
	SetMode(iNewMode);
	
	}

void C4EditCursor::ApplyToolBrush()
	{
	if (!EditingOK()) return;
	C4ToolsDlg *pTools=&Console.ToolsDlg;
	if (!Game.Landscape.DrawBrush(X,Y,pTools->Grade,pTools->Material,pTools->Texture,pTools->ModeIFT)) { ToolFailure(); return; }
	UpdateGraphicsSystem();
	}

void C4EditCursor::ApplyToolLine()
	{
	if (!EditingOK()) return;
	C4ToolsDlg *pTools=&Console.ToolsDlg;
	if (!Game.Landscape.DrawLine(X,Y,X2,Y2,pTools->Grade,pTools->Material,pTools->Texture,pTools->ModeIFT)) { ToolFailure(); return; }
	UpdateGraphicsSystem();
	}

void C4EditCursor::ApplyToolRect()
	{
	if (!EditingOK()) return;
	C4ToolsDlg *pTools=&Console.ToolsDlg;
	if (!Game.Landscape.DrawBox(X,Y,X2,Y2,pTools->Grade,pTools->Material,pTools->Texture,pTools->ModeIFT)) { ToolFailure(); return; }
	UpdateGraphicsSystem();
	}

void C4EditCursor::ApplyToolFill()
	{
	if (!EditingOK()) return;
	int iMat = Game.Material.Get(Console.ToolsDlg.Material);
	if (!MatValid(iMat)) return;
	int grade=Console.ToolsDlg.Grade;
	for (int cnt=0; cnt<grade; cnt++)
		Game.Landscape.InsertMaterial(iMat,X+Random(grade)-grade/2,Y+Random(grade)-grade/2);
	}

BOOL C4EditCursor::SetToolPatCol(BYTE &bCol)
	{
	if (SEqual(Console.ToolsDlg.Material,C4TLS_MatSky))
		{
		bCol=0;
		Engine.DDraw.DefinePattern(Game.Landscape.Sky.Surface);	
		}
	else
		{
		int iMat = Game.Material.Get(Console.ToolsDlg.Material);
		if (!MatValid(iMat)) return FALSE;
		bCol = Mat2PixCol(iMat);
		if (Console.ToolsDlg.ModeIFT) bCol+=IFT;
		Engine.DDraw.DefinePattern(Game.TextureMap.GetTexture(Console.ToolsDlg.Texture));
		}
	return TRUE;
	}

BOOL C4EditCursor::DoContextMenu()
	{
	POINT point; GetCursorPos(&point);
	HMENU hContext = GetSubMenu(hMenu,0);
	BOOL fObjectSelected = Selection.ObjectCount();
	SetMenuItemEnable( hContext, IDM_VIEWPORT_DELETE, fObjectSelected && Console.Editing);
	SetMenuItemEnable( hContext, IDM_VIEWPORT_DUPLICATE, fObjectSelected && Console.Editing);
	SetMenuItemEnable( hContext, IDM_VIEWPORT_CONTENTS, fObjectSelected && Selection.GetObject()->Contents.ObjectCount() && Console.Editing);
	SetMenuItemEnable( hContext, IDM_VIEWPORT_PROPERTIES, Mode!=C4CNS_ModePlay);
	SetMenuItemText(hContext,IDM_VIEWPORT_DELETE,LoadResStr(IDS_MNU_DELETE));
	SetMenuItemText(hContext,IDM_VIEWPORT_DUPLICATE,LoadResStr(IDS_MNU_DUPLICATE));
	SetMenuItemText(hContext,IDM_VIEWPORT_CONTENTS,LoadResStr(IDS_MNU_CONTENTS));
	SetMenuItemText(hContext,IDM_VIEWPORT_PROPERTIES,LoadResStr((Mode==C4CNS_ModeEdit) ? IDS_CNS_PROPERTIES : IDS_CNS_TOOLS));
	int iItem = TrackPopupMenu(
								 hContext,
								 TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NONOTIFY,
								 point.x,point.y, 0,
								 Application.hWindow,
								 NULL);
	switch (iItem)
		{
		case IDM_VIEWPORT_DELETE:	Delete();	break;
		case IDM_VIEWPORT_DUPLICATE: Duplicate();	break;
		case IDM_VIEWPORT_CONTENTS: GrabContents(); break;
		case IDM_VIEWPORT_PROPERTIES:	OpenPropTools();	break;
		}
	return TRUE;
	}

void C4EditCursor::GrabContents()
	{
	C4Object *pFrom,*pObj;
	if (!( pFrom = Selection.GetObject() )) return;
	Selection.Clear();
	while (pObj = pFrom->Contents.GetObject())
		{
		pObj->Exit(X,Y);
		Selection.Add(pObj);
		}
	Hold=TRUE;
	Console.PropertyDlg.Update(Selection);
	}

void C4EditCursor::UpdateDropTarget(WORD wKeyFlags)
	{
  C4Object *cobj; C4ObjectLink *clnk;
	
	DropTarget=NULL;

	if (wKeyFlags & MK_CONTROL)
		if (Selection.GetObject())
			for (clnk=Game.Objects.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
				if (cobj->Status)
					if (!cobj->Contained)
						if (Inside(X-(cobj->x+cobj->Shape.x),0,cobj->Shape.Wdt-1))
							if (Inside(Y-(cobj->y+cobj->Shape.y),0,cobj->Shape.Hgt-1))
								if (!Selection.GetLink(cobj))
									{ DropTarget=cobj; break; }
	
	}

void C4EditCursor::PutContents()
	{
	if (!DropTarget) return;
	C4Object *cobj; C4ObjectLink *clnk;
	for (clnk=Selection.First; clnk && (cobj=clnk->Obj); clnk=clnk->Next)
		cobj->Enter(DropTarget);
	UpdateGraphicsSystem();
	}

C4Object *C4EditCursor::GetTarget()
	{
	return Target;
	}

BOOL C4EditCursor::EditingOK()
	{
	if (!Console.Editing) 
		{ 
		Hold=FALSE;
		Console.Message(LoadResStr(IDS_CNS_NONETEDIT)); 
		return FALSE; 
		}
	return TRUE;
	}

int C4EditCursor::GetMode()
	{
	return Mode;
	}

void C4EditCursor::ToolFailure()
	{
	C4ToolsDlg *pTools=&Console.ToolsDlg;
	Hold=FALSE;
	sprintf(OSTR,LoadResStr(IDS_CNS_NOMATDEF),pTools->Material,pTools->Texture); 
	Console.Message(OSTR); 
	}

void C4EditCursor::ApplyToolPicker()
	{
	int iMaterial;
	BYTE byIndex;
	const char *szMaterialTexture;
	switch (Game.Landscape.Mode)
		{
		case C4LSC_Static:
			// Material-texture from map
			if (byIndex=Game.Landscape.GetMapIndex(X/Game.Landscape.MapZoom,Y/Game.Landscape.MapZoom))
				{
				szMaterialTexture=Game.TextureMap.GetMaterialTexture(byIndex & (C4M_MaxTexIndex-1));
				SCopySegment(szMaterialTexture,0,OSTR,'-');				
				Console.ToolsDlg.SelectMaterial(OSTR);
				SCopySegment(szMaterialTexture,1,OSTR,'-');
				Console.ToolsDlg.SelectTexture(OSTR);
				Console.ToolsDlg.SetIFT(byIndex & ~(C4M_MaxTexIndex-1));
				}
			else
				Console.ToolsDlg.SelectMaterial(C4TLS_MatSky);
			break;
		case C4LSC_Exact:
			// Material only from landscape
			if (MatValid(iMaterial=GBackMat(X,Y)))
				{
				Console.ToolsDlg.SelectMaterial(Game.Material.Map[iMaterial].Name);
				Console.ToolsDlg.SetIFT(GBackIFT(X,Y));
				}
			else
				Console.ToolsDlg.SelectMaterial(C4TLS_MatSky);
			break;
		}
	Hold=FALSE;
	}
