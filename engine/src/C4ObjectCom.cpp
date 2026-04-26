/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Lots of handler functions for object action */

#include <C4Include.h>

BYTE SimFlightHitsLiquid(FIXED fcx, FIXED fcy, FIXED xdir, FIXED ydir);
BOOL CreateConstructionSite(int ctx, int bty, C4ID strid, int owner, C4Object *pByObj);

BOOL ObjectActionWalk(C4Object *cObj)
  {
  if (!cObj->SetActionByName("Walk")) return FALSE;
  cObj->xdir=cObj->ydir=0;
  return TRUE;
  }

BOOL ObjectActionStand(C4Object *cObj)
  {
  cObj->Action.ComDir=COMD_Stop;
  if (!ObjectActionWalk(cObj)) return FALSE;
  return TRUE;
  }

BOOL ObjectActionJump(C4Object *cObj, FIXED xdir, FIXED ydir)
  {
  if (!cObj->SetActionByName("Jump")) return FALSE;
  cObj->xdir=xdir; cObj->ydir=ydir;
  cObj->Mobile=1;
  return TRUE;
  }

BOOL ObjectActionDive(C4Object *cObj, FIXED xdir, FIXED ydir)
  {
  if (!cObj->SetActionByName("Dive")) return FALSE;
  cObj->xdir=xdir; cObj->ydir=ydir;
  cObj->Mobile=1;
  return TRUE;
  }

BOOL ObjectActionTumble(C4Object *cObj, int dir, FIXED xdir, FIXED ydir)
  {
  if (!cObj->SetActionByName("Tumble")) return FALSE;
  cObj->SetDir(dir);
  cObj->xdir=xdir; cObj->ydir=ydir;
  return TRUE;
  }

BOOL ObjectActionGetPunched(C4Object *cObj, FIXED xdir, FIXED ydir)
  {
  if (!cObj->SetActionByName("GetPunched")) return FALSE;
  cObj->xdir=xdir; cObj->ydir=ydir;
  return TRUE;
  }

BOOL ObjectActionKneel(C4Object *cObj)
  {
  if (!cObj->SetActionByName("KneelDown")) return FALSE;
  cObj->xdir=cObj->ydir=0;
  return TRUE;
  }

BOOL ObjectActionFlat(C4Object *cObj, int dir)
  {
  if (!cObj->SetActionByName("FlatUp")) return FALSE;
  cObj->xdir=cObj->ydir=0;
  cObj->SetDir(dir);
  return TRUE;
  }

BOOL ObjectActionScale(C4Object *cObj, int dir, int comdir)
  {
  if (!cObj->SetActionByName("Scale")) return FALSE;
  cObj->xdir=cObj->ydir=0;
  cObj->SetDir(dir);
  cObj->Action.ComDir=comdir;
  return TRUE;
  }

BOOL ObjectActionHangle(C4Object *cObj, int dir, int comdir)
  {
  if (!cObj->SetActionByName("Hangle")) return FALSE;
  cObj->xdir=cObj->ydir=0;
  cObj->SetDir(dir);
  cObj->Action.ComDir=comdir;
  return TRUE;
  }

BOOL ObjectActionThrow(C4Object *cObj, C4Object *pThing)
  {
  // No object specified, first from contents
	if (!pThing) pThing = cObj->Contents.GetObject();
	// Nothing to throw
	if (!pThing) return FALSE;
	// Force and direction
  FIXED pthrow=ftofix(4.0*(float)cObj->GetPhysical()->Throw/(float)C4MaxPhysical);
  int iDir=1; if (cObj->Action.Dir==DIR_Left) iDir=-1;
	// Set action
  if (!cObj->SetActionByName("Throw")) return FALSE;
	// Exit object
  pThing->Exit(cObj->x,
              cObj->y+cObj->Shape.y-1,
              Random(360),
              pthrow*iDir,-pthrow,pthrow*iDir);       
	// Success
  return TRUE;
  }

BOOL ObjectActionDig(C4Object *cObj, int tcomdir)
  {
  if (!cObj->SetActionByName("Dig")) return FALSE;
  cObj->Action.ComDir=tcomdir;
  cObj->Action.Data=0; // Material Dig2Object request
  return TRUE;
  }

BOOL ObjectActionBuild(C4Object *cObj, C4Object *target)
  {
  return cObj->SetActionByName("Build",target);
  }
  
BOOL ObjectActionPush(C4Object *cObj, C4Object *target)
  {
  return cObj->SetActionByName("Push",target);
  }

BOOL ObjectActionFight(C4Object *cObj, C4Object *target)
  {
  return cObj->SetActionByName("Fight",target);
  }

BOOL ObjectActionChop(C4Object *cObj, C4Object *target)
  {
  return cObj->SetActionByName("Chop",target);
  }

BOOL CornerScaleOkay(C4Object *cObj, int iRangeX, int iRangeY)
  {
  int ctx,cty;
  cty=cObj->y-iRangeY;
  if (cObj->Action.Dir==DIR_Left) ctx=cObj->x-iRangeX; 
  else ctx=cObj->x+iRangeX;
  cObj->ContactCheck(ctx,cty); // (resets VtxContact & t_contact)
  if (!(cObj->t_contact & CNAT_Top))
    if (!(cObj->t_contact & CNAT_Left))
      if (!(cObj->t_contact & CNAT_Right))
				if (!(cObj->t_contact & CNAT_Bottom))
					return TRUE;
  return FALSE;
  }

BOOL CheckCornerScale(C4Object *cObj, int &iRangeX, int &iRangeY)
	{
	for (iRangeX=CornerRange; iRangeX>=1; iRangeX--)
		for (iRangeY=CornerRange; iRangeY>=1; iRangeY--)
			if (CornerScaleOkay(cObj,iRangeX,iRangeY))
				return TRUE;
	return FALSE;
	}

BOOL ObjectActionCornerScale(C4Object *cObj)
  {
	int iRangeX,iRangeY;
	// Scaling: check range max to min
	if (cObj->GetProcedure()==DFA_SCALE)
		{
		if (!CheckCornerScale(cObj,iRangeX,iRangeY)) return FALSE;
		}
	// Swimming: check range min to max
	else
		{
		iRangeY=2;
		while (!CornerScaleOkay(cObj,iRangeY,iRangeY))
			{ iRangeY++; if (iRangeY>CornerRange) return FALSE; }
		iRangeX=iRangeY;
		}
  // Do corner scale
  cObj->SetActionByName("KneelUp");
  cObj->xdir=cObj->ydir=0;
  if (cObj->Action.Dir==DIR_Left) cObj->Action.ComDir=COMD_Left; else cObj->Action.ComDir=COMD_Right;
  if (cObj->Action.Dir==DIR_Left) cObj->fix_x-=itofix(iRangeX); 
	else cObj->fix_x+=itofix(iRangeX);
  cObj->fix_y-=itofix(iRangeY);
  cObj->x=fixtoi(cObj->fix_x); cObj->y=fixtoi(cObj->fix_y);
  return TRUE;
  }

BOOL ObjectComMovement(C4Object *cObj, int comdir)
  {
  cObj->Action.ComDir=comdir;
  PlayerObjectCommand(cObj->Owner,C4CMD_Follow,cObj);
  return TRUE;
  }

BOOL ObjectComStop(C4Object *cObj)
  {
  // Cease current action
  cObj->SetActionByName("Idle");
  // Action walk if possible
  return ObjectActionStand(cObj);
  }

BOOL ObjectComGrab(C4Object *cObj, C4Object *pTarget)
  {
  if (!pTarget) return FALSE;
  if (cObj->GetProcedure()!=DFA_WALK) return FALSE;
  if (!ObjectActionPush(cObj,pTarget)) return FALSE;
  cObj->Call(PSF_Grab);
  return TRUE;
  }

BOOL ObjectComUnGrab(C4Object *cObj)
  {
  // Only if pushing, -> stand
  if (cObj->GetProcedure()==DFA_PUSH)
    if (ObjectActionStand(cObj))
      {
      cObj->CloseMenu();
      cObj->Call(PSF_Grab);

			// Special: UnGrab command clears LastComDownDouble,
			// so a quick throw after ungrab does not lead to a drop
			if (ValidPlr(cObj->Owner))
				Game.Players.Get(cObj->Owner)->LastComDownDouble=0;

      return TRUE;
      }

  return FALSE;  
  }

BOOL ObjectComJump(C4Object *cObj) // by ObjectComUp, ExecCMDFMoveTo, FnJump
  {
	// Only if walking
	if (cObj->GetProcedure()!=DFA_WALK) return FALSE;
	// Calculate direction & forces
  FIXED TXDir=0;
  C4PhysicalInfo *pPhysical=cObj->GetPhysical();
  FIXED iPhysicalWalk = ftofix(2.8*(float)pPhysical->Walk/(float)C4MaxPhysical);
  FIXED iPhysicalJump = ftofix(10.0*(float)pPhysical->Jump/(float)C4MaxPhysical);
  if (cObj->Action.ComDir==COMD_Left)  TXDir=-iPhysicalWalk;
  if (cObj->Action.ComDir==COMD_Right) TXDir=+iPhysicalWalk;
  if (cObj->Action.ComDir==COMD_Stop)
    {
    if (cObj->Action.Dir==DIR_Left)  TXDir=-iPhysicalWalk;
    if (cObj->Action.Dir==DIR_Right) TXDir=+iPhysicalWalk;
    }
	// Try dive
  if (SimFlightHitsLiquid(cObj->fix_x,cObj->fix_y,TXDir,-iPhysicalJump))
    if (ObjectActionDive(cObj,TXDir,-iPhysicalJump))
			return TRUE;
  // Regular jump
  return ObjectActionJump(cObj,TXDir,-iPhysicalJump);
  }

BOOL ObjectComLetGo(C4Object *cObj, int xdirf) 
  { // by ACTSCALE, ACTHANGLE or ExecCMDFMoveTo
  return ObjectActionJump(cObj,ftofix(1.0)*xdirf,0);
  }

BOOL ObjectComEnter(C4Object *cObj) // by pusher
  {
  if (!cObj) return FALSE;
  
	// NoPushEnter
	if (cObj->Def->NoPushEnter) return FALSE;

  // Check object entrance, try command enter
  C4Object *pTarget;
  DWORD ocf=OCF_Entrance; 
  if ((pTarget=Game.AtObject(cObj->x,cObj->y,ocf,cObj)))
    if (ocf & OCF_Entrance)
      { cObj->SetCommand(C4CMD_Enter,pTarget); return TRUE; }

  return FALSE;
  }


BOOL ObjectComUp(C4Object *cObj) // by DFA_WALK or DFA_SWIM
  {
  if (!cObj) return FALSE;
  
  // Check object entrance, try command enter
  C4Object *pTarget;
  DWORD ocf=OCF_Entrance; 
  if ((pTarget=Game.AtObject(cObj->x,cObj->y,ocf,cObj)))
    if (ocf & OCF_Entrance)
      return PlayerObjectCommand(cObj->Owner,C4CMD_Enter,pTarget);
  
  // Try jump
	if (cObj->GetProcedure()==DFA_WALK)
		return PlayerObjectCommand(cObj->Owner,C4CMD_Jump);

	return FALSE;
  }

BOOL ObjectComDig(C4Object *cObj) // by DFA_WALK
  {
  int iComDir;
  C4PhysicalInfo *phys=cObj->GetPhysical();
  iComDir=COMD_DownLeft; if (cObj->Action.Dir==DIR_Right) iComDir=COMD_DownRight;
  if (!phys->CanDig || !ObjectActionDig(cObj,iComDir))
    { 
    sprintf(OSTR,LoadResStr(IDS_OBJ_NODIG),cObj->GetName());
    GameMsgObject(OSTR,cObj);
    return FALSE;
    }
  return TRUE;
  }

C4Object *CreateLine(C4ID idType, int iOwner, C4Object *pFrom, C4Object *pTo)
	{
	C4Object *pLine;
	if (!pFrom || !pTo) return NULL;
	if (!(pLine=Game.CreateObject(idType,iOwner,0,0))) return NULL;
	pLine->Shape.VtxNum=2;
	pLine->Shape.VtxX[0]=pFrom->x;
	pLine->Shape.VtxY[0]=pFrom->y+pFrom->Shape.Hgt/4;
	pLine->Shape.VtxX[1]=pTo->x;
	pLine->Shape.VtxY[1]=pTo->y+pTo->Shape.Hgt/4;
	pLine->Action.Target=pFrom;
	pLine->Action.Target2=pTo;
	return pLine;
	}

BOOL ObjectComLineConstruction(C4Object *cObj)
	{
	C4Object *linekit,*tstruct,*cline;
	DWORD ocf;

  ObjectActionStand(cObj);  

  // Check physical
  if (!cObj->GetPhysical()->CanConstruct)
		{
		sprintf(OSTR,LoadResStr(IDS_OBJ_NOLINECONSTRUCT),cObj->GetName());
		GameMsgObject(OSTR,cObj); return FALSE;
		} 

	// - - - - - - - - - - - - - - - - - - Line pickup - - - - - - - - - - - - - - - - -

	// Check for linekit
	if (!(linekit=cObj->Contents.Find(C4ID_Linekit)))
		{
		// Check for collection limit
		if (cObj->Def->CollectionLimit && (cObj->Contents.ObjectCount()>=cObj->Def->CollectionLimit) ) return FALSE;
    // Check line pickup
		ocf=OCF_LineConstruct;
		tstruct=Game.AtObject(cObj->x,cObj->y,ocf);
		if (!tstruct || !(ocf & OCF_LineConstruct)) return FALSE;
		if (!(cline=Game.FindObject(C4ID_None,0,0,0,0,OCF_All,"Connect",tstruct))) return FALSE;
		// Check line connected to linekit at other end
		if ( (cline->Action.Target && (cline->Action.Target->Def->id==C4ID_Linekit))
			|| (cline->Action.Target2 && (cline->Action.Target2->Def->id==C4ID_Linekit)) )
				{
				SoundEffect("Error",0,100,cObj);
				sprintf(OSTR,LoadResStr(IDS_OBJ_NODOUBLEKIT),cline->GetName());
				GameMsgObject(OSTR,cObj);	return FALSE;
				}
		// Pick up line and new linekit
    if (!(linekit=Game.CreateObject(C4ID_Linekit,cline->Owner))) return FALSE;
		linekit->Enter(cObj); cObj->SetOCF();
		SoundEffect("Connect",0,100,cObj);
		if (cline->Action.Target==tstruct) cline->Action.Target=linekit;
		if (cline->Action.Target2==tstruct) cline->Action.Target2=linekit;

		sprintf(OSTR,LoadResStr(IDS_OBJ_DISCONNECT),cline->GetName(),tstruct->GetName());
		GameMsgObject(OSTR,tstruct);

		return TRUE;
		}

	// - - - - - - - - - -  - - - - - Active construction - - - - - - - - - - - - - - - - -

	// Active line construction
	if (cline=Game.FindObject(C4ID_None,0,0,0,0,OCF_All,"Connect",linekit))
		{
		
		// Check for structure connection
		ocf=OCF_LineConstruct;
		tstruct=Game.AtObject(cObj->x,cObj->y,ocf);
		// No structure
		if (!tstruct || !(ocf & OCF_LineConstruct)) 
			{
			// No connect
			SoundEffect("Error",0,100,cObj);
			sprintf(OSTR,LoadResStr(IDS_OBJ_NOCONNECT));
			GameMsgObject(OSTR,cObj);	return FALSE;
			}
		
		// Check short circuit -> removal
		if ((cline->Action.Target==tstruct)
		 || (cline->Action.Target2==tstruct))
			{
			SoundEffect("Connect",0,100,cObj);
			sprintf(OSTR,LoadResStr(IDS_OBJ_LINEREMOVAL),cline->GetName());
			GameMsgObject(OSTR,tstruct); 
			cline->AssignRemoval();
			return TRUE;
			}

		// Check for correct connection type
		BOOL connect_okay=FALSE;
		switch (cline->Def->Line) 
			{
			case C4D_Line_Power:
				if (tstruct->Def->LineConnect & C4D_Power_Input) connect_okay=TRUE;	
				if (tstruct->Def->LineConnect & C4D_Power_Output) connect_okay=TRUE;	
				break;
			case C4D_Line_Source:
				if (tstruct->Def->LineConnect & C4D_Liquid_Output) connect_okay=TRUE;	break;
			case C4D_Line_Drain:
				if (tstruct->Def->LineConnect & C4D_Liquid_Input) connect_okay=TRUE;	break;
			default: return FALSE; // Undefined line type
			}
	  if (!connect_okay)
			{
			SoundEffect("Error",0,100,cObj);
			sprintf(OSTR,LoadResStr(IDS_OBJ_NOCONNECTTYPE),cline->GetName(),tstruct->GetName());
			GameMsgObject(OSTR,tstruct); return FALSE;
			}
		
		// Connect line to structure
		SoundEffect("Connect",0,100,cObj);
		if (cline->Action.Target==linekit) cline->Action.Target=tstruct;
		if (cline->Action.Target2==linekit) cline->Action.Target2=tstruct;
		linekit->Exit();
		linekit->AssignRemoval();

		sprintf(OSTR,LoadResStr(IDS_OBJ_CONNECT),cline->GetName(),tstruct->GetName());
		GameMsgObject(OSTR,tstruct);

		return TRUE;
		}

	// - - - - - - - - - - - - - - - - New line - - - - - - - - - - - - - - - - - - - - -

	// Check for new structure connection
	ocf=OCF_LineConstruct;
	tstruct=Game.AtObject(cObj->x,cObj->y,ocf);
	if (!tstruct || !(ocf & OCF_LineConstruct)) 
		{
		SoundEffect("Error",0,100,cObj);
		sprintf(OSTR,LoadResStr(IDS_OBJ_NONEWLINE));
		GameMsgObject(OSTR,cObj);	return FALSE;
		}

	// Determine new line type
	C4ID linetype=C4ID_None;	
	// Check source pipe
	if (linetype==C4ID_None)
		if (tstruct->Def->LineConnect & C4D_Liquid_Pump)
			if (!Game.FindObject(C4ID_SourcePipe,0,0,0,0,OCF_All,"Connect",tstruct))
				linetype = C4ID_SourcePipe;
	// Check drain pipe
	if (linetype==C4ID_None)
		if (tstruct->Def->LineConnect & C4D_Liquid_Output)
			if (!Game.FindObject(C4ID_DrainPipe,0,0,0,0,OCF_All,"Connect",tstruct))
				linetype = C4ID_DrainPipe;
	// Check power
	if (linetype==C4ID_None)
		if (tstruct->Def->LineConnect & C4D_Power_Output)
			linetype = C4ID_PowerLine;
	// No good
	if (linetype==C4ID_None)
		{
		SoundEffect("Error",0,100,cObj);
		sprintf(OSTR,LoadResStr(IDS_OBJ_NONEWLINE));
		GameMsgObject(OSTR,cObj);	return FALSE;
		}

	// Create new line
	C4Object *newline=CreateLine(linetype,cObj->Owner,
															 tstruct,linekit);
	if (!newline) return FALSE;
	SoundEffect("Connect",0,100,cObj);		
	sprintf(OSTR,LoadResStr(IDS_OBJ_NEWLINE),newline->GetName());
	GameMsgObject(OSTR,tstruct);

	return TRUE;
	}

void ObjectComDigDouble(C4Object *cObj) // "Activation" by DFA_WALK, DFA_DIG, DFA_SWIM
  {  
	C4Object *pTarget;
  DWORD ocf;
  C4PhysicalInfo *phys=cObj->GetPhysical();

	// Contents activation (first contents object only)
	if (cObj->Contents.GetObject())
		if (cObj->Contents.GetObject()->Call(PSF_Activate,(long)cObj))
			return;
  
	// Linekit: Line construction (move to linekit script...)
  if (cObj->Contents.GetObject() && (cObj->Contents.GetObject()->id==C4ID_Linekit)) 
    {
		ObjectComLineConstruction(cObj);
		return;
    }
  
	// Chop
  ocf=OCF_Chop;
  if (phys->CanChop)
		if (cObj->GetProcedure()!=DFA_SWIM)
	    if ((pTarget=Game.AtObject(cObj->x,cObj->y,ocf,cObj)))
		    if (ocf & OCF_Chop)
			    { 
				  PlayerObjectCommand(cObj->Owner,C4CMD_Chop,pTarget);
					return; 
					}

	// Line construction pick up
  ocf=OCF_LineConstruct;
  if (phys->CanConstruct)
	  if (!cObj->Contents.GetObject())
	    if ((pTarget=Game.AtObject(cObj->x,cObj->y,ocf,cObj)))
		    if (ocf & OCF_LineConstruct)
			    if (ObjectComLineConstruction(cObj))
						return; 

  // Own activation call
  if (cObj->Call(PSF_Activate, (long)cObj) ) return;
	 
  }

BOOL ObjectComDownDouble(C4Object *cObj) // by DFA_WALK
  {
  C4Object *pTarget;
  DWORD ocf= OCF_Construct | OCF_Grab;
  if ((pTarget=Game.AtObject(cObj->x,cObj->y,ocf,cObj)))
    {
    if (ocf & OCF_Construct)  
      { PlayerObjectCommand(cObj->Owner,C4CMD_Build,pTarget); return TRUE; }
    if (ocf & OCF_Grab)
      { PlayerObjectCommand(cObj->Owner,C4CMD_Grab,pTarget); return TRUE; }
    } 
  return FALSE; 
  }

BOOL ObjectComPut(C4Object *cObj, C4Object *pTarget, C4Object *pThing)
  {
  // No object specified, first from contents
	if (!pThing) pThing = cObj->Contents.GetObject();
	// Nothing to put
	if (!pThing) return FALSE;
	// No target
  if (!pTarget) return FALSE;
  // Grabbing: check C4D_Grab_Put
  if (pTarget!=cObj->Contained)
    if (!(pTarget->Def->GrabPutGet & C4D_Grab_Put)) 
      {
      // Was meant to be a drop anyway
      if (ValidPlr(cObj->Owner))
        if (Game.Players.Get(cObj->Owner)->LastComDownDouble)
          return ObjectComDrop(cObj);
      // No grab put: fail
      return FALSE;
      }
  // Target no fullcon
  if (!(pTarget->OCF & OCF_FullCon)) return FALSE;
	// Check target collection limit
	if (pTarget->Def->CollectionLimit && (pTarget->Contents.ObjectCount()>=pTarget->Def->CollectionLimit)) return FALSE;
	// Check for collect rejection
	if (pTarget->Call(PSF_RejectCollection,(long)pThing->Def->id,(long)pThing)) return FALSE;
  // Transfer thing
  pThing->Enter(pTarget);
	// Put call to object script
  cObj->Call(PSF_Put);
	// Target collection call
  pTarget->Call(PSF_Collection,(long)pThing,TRUE);
	// Success
  return TRUE;
  }

BOOL ObjectComThrow(C4Object *cObj, C4Object *pThing)
  {
  // No object specified, first from contents
	if (!pThing) pThing = cObj->Contents.GetObject();
	// Nothing to throw
	if (!pThing) return FALSE;
  // Drop check
  if (ValidPlr(cObj->Owner))
    if (Game.Players.Get(cObj->Owner)->LastComDownDouble)
      return ObjectComDrop(cObj,pThing);
  // Throw com
  switch (cObj->GetProcedure())
    {
    case DFA_WALK: return ObjectActionThrow(cObj,pThing);
    case DFA_SWIM: return ObjectComDrop(cObj,pThing);
    }
	// Failure
  return FALSE;
  }

BOOL ObjectComDrop(C4Object *cObj, C4Object *pThing)
  {
  // No object specified, first from contents
	if (!pThing) pThing = cObj->Contents.GetObject();
	// Nothing to throw
	if (!pThing) return FALSE;
	// Force and direction
  FIXED pthrow=ftofix(4.0*(float)cObj->GetPhysical()->Throw/(float)C4MaxPhysical);
  int tdir=0;
  if (cObj->Action.ComDir==COMD_Left) tdir=-1;
  if (cObj->Action.ComDir==COMD_Right) tdir=+1;
	// Exit object
  pThing->Exit(cObj->x,cObj->y+cObj->Shape.y+cObj->Shape.Hgt+pThing->Shape.y*(cObj->GetProcedure()!=DFA_SWIM),0,pthrow*tdir,0,0);
	// NoCollectDelay
  cObj->NoCollectDelay=2; 
	// Update OCF
	cObj->SetOCF();
	// Ungrab
  ObjectComUnGrab(cObj);
	// Done
  return TRUE;
  }

BOOL ObjectComChop(C4Object *cObj, C4Object *pTarget)
  {
  if (!pTarget) return FALSE;
  if (!cObj->GetPhysical()->CanChop) 
		{
    sprintf(OSTR,LoadResStr(IDS_OBJ_NOCHOP),cObj->GetName());
    GameMsgObject(OSTR,cObj);
		return FALSE;
		}
  if (cObj->GetProcedure()!=DFA_WALK) return FALSE;
  return ObjectActionChop(cObj,pTarget);
  }

BOOL ObjectComBuild(C4Object *cObj, C4Object *pTarget)
  {
  if (!pTarget) return FALSE;
  // Needs to be idle or walking 
  if (cObj->Action.Act!=ActIdle)
    if (cObj->GetProcedure()!=DFA_WALK) 
      return FALSE;
  return ObjectActionBuild(cObj,pTarget);
  }

BOOL ObjectComPutTake(C4Object *cObj, C4Object *pTarget, C4Object *pThing) // by C4CMD_Throw
  {																																				 // by C4CMD_Drop
  // Valid checks
  if (!pTarget) return FALSE;
  // No object specified, first from contents
	if (!pThing) pThing = cObj->Contents.GetObject();
  // Has thing, put to target
  if (pThing)
    return ObjectComPut(cObj,pTarget,pThing);
  // If target is own container, activate activation menu
  if (pTarget==cObj->Contained)
    return cObj->ActivateMenu(C4MN_Activate);
  // Assuming target is grabbed, check for grab get
  if (pTarget->Def->GrabPutGet & C4D_Grab_Get)
    {
    // Activate get menu
    return cObj->ActivateMenu(C4MN_Get,0,0,0,pTarget);
    }
  // Failure
  return FALSE;
  }

BOOL ObjectComActivate(C4Object *cObj, C4ID c_id, C4Object *pThing) // by C4MN_Activate
  {																																	// or C4CMD_Activate
  
	// Valid target (own container)
  C4Object *pTarget=NULL; 
  if (cObj->Contained) pTarget=cObj->Contained;
  if (!pTarget) return FALSE;
  
	// If not specified get object from target contents by type
	// Find first object requested id that has no command exit yet
	C4Object *pObj; C4ObjectLink *cLnk;
	if (!pThing)
		for (cLnk=pTarget->Contents.First; cLnk && (pObj=cLnk->Obj); cLnk=cLnk->Next)
			if (pObj->Status && (pObj->Def->id==c_id))
				if (!pObj->Command || (pObj->Command->Command!=C4CMD_Exit))
					{ pThing=pObj; break; }

	// No target
	if (!pThing) return FALSE;
  
	// Thing in own container (target)
	if (pThing->Contained!=pTarget) return FALSE;

  // No minimum con knowledge vehicles/items
	if ((pThing->Category & C4D_Vehicle) || (pThing->Category & C4D_Object))
		if (pThing->Category & C4D_SelectKnowledge)
			if (pThing->Con<FullCon*75/100) 
				{
				sprintf(OSTR,LoadResStr(IDS_OBJ_NOCONACTIV),pThing->GetName());
				SoundEffect("Error",0,100,cObj);
				GameMsgObject(OSTR,cObj);
				return FALSE;
				}

  // Activate object to exit
  pThing->SetCommand(C4CMD_Exit);
  
  return TRUE;
  }

BOOL ObjectComPunch(C4Object *cObj, C4Object *pTarget, int punch)
  {
  if (!cObj || !pTarget) return FALSE;
	if (!punch)
		if (pTarget->GetPhysical()->Fight) 
			punch=BoundBy(5*cObj->GetPhysical()->Fight/pTarget->GetPhysical()->Fight,0,10);
	if (!punch) return TRUE;
	pTarget->DoEnergy(-punch);
  int tdir=+1; if (cObj->Action.Dir==DIR_Left) tdir=-1;
  pTarget->Call(PSF_CatchBlow,punch,(long)cObj);
  pTarget->Action.ComDir=COMD_Stop;
  // Hard punch
  if (punch>=10) 
    if (ObjectActionTumble(pTarget,pTarget->Action.Dir,ftofix(1.5)*tdir,ftofix(-2.0)))
			{ pTarget->Call(PSF_CatchBlow,punch,(long)cObj); return TRUE; }
  // Regular punch
  if (ObjectActionGetPunched(pTarget,ftofix(2.5)*tdir,0))
		{ pTarget->Call(PSF_CatchBlow,punch,(long)cObj); return TRUE; }
	return FALSE;
  }

BOOL ObjectComCancelAttach(C4Object *cObj)
  { 
  if (cObj->GetProcedure()==DFA_ATTACH)
    return cObj->SetAction(ActIdle);
  return FALSE;
  }

void ObjectComStopDig(C4Object *cObj)
	{
	// Stand
	ObjectActionStand(cObj);
	// Clear digging command
	if (cObj->Command)
		if (cObj->Command->Command == C4CMD_Dig)
			cObj->ClearCommand(cObj->Command);
	}

int ComOrder(int iIndex)
	{
	static BYTE bComOrder[ComOrderNum] =	
		{	
		COM_Left,	COM_Right, COM_Up, COM_Down, COM_Throw, COM_Dig, COM_Special, COM_Special2,
		COM_Left_S,	COM_Right_S, COM_Up_S, COM_Down_S, COM_Throw_S, COM_Dig_S, COM_Special_S, COM_Special2_S,
		COM_Left_D,	COM_Right_D, COM_Up_D, COM_Down_D, COM_Throw_D, COM_Dig_D, COM_Special_D, COM_Special2_D
		};
	
	if (Inside(iIndex,0,ComOrderNum-1)) return bComOrder[iIndex];

	return COM_None;
	}

const char *ComName(int iCom)
  {
  switch (iCom)
    {
    case COM_Up:					return "Up";
    case COM_Up_S:				return "UpSingle";
    case COM_Up_D:				return "UpDouble";
    case COM_Down:				return "Down";
    case COM_Down_S:			return "DownSingle";
    case COM_Down_D:			return "DownDouble";
    case COM_Left:				return "Left";
    case COM_Left_S:			return "LeftSingle";
    case COM_Left_D:			return "LeftDouble";
    case COM_Right:				return "Right";
    case COM_Right_S:			return "RightSingle";
    case COM_Right_D:			return "RightDouble";
    case COM_Dig:					return "Dig";
    case COM_Dig_S:				return "DigSingle";
    case COM_Dig_D:				return "DigDouble";
    case COM_Throw:				return "Throw";
    case COM_Throw_S:			return "ThrowSingle";
    case COM_Throw_D:			return "ThrowDouble";
    case COM_Special:			return "Special";
    case COM_Special_S:		return "SpecialSingle";
    case COM_Special_D:		return "SpecialDouble";
    case COM_Special2:		return "Special2";
    case COM_Special2_S:	return "Special2Single";
    case COM_Special2_D:	return "Special2Double";
    }
  return "Undefined";
  }

int Com2Control(int iCom)
  {	
	iCom = iCom & ~(COM_Double | COM_Single);
  switch (iCom)
    {
    case COM_CursorLeft:		return CON_CursorLeft;
    case COM_CursorToggle:	return CON_CursorToggle;
    case COM_CursorRight:		return CON_CursorRight;
    case COM_Throw:					return CON_Throw;
    case COM_Up:						return CON_Up;
    case COM_Dig:						return CON_Dig;
    case COM_Left:					return CON_Left;
    case COM_Down:					return CON_Down;
    case COM_Right:					return CON_Right;
    case COM_Special:				return CON_Special;
    case COM_Special2:			return CON_Special2;
    }
	return CON_Menu;
  }

int VkCode2FullScreenCom(WORD dwCode, BOOL fMultiline)
	{
	switch (dwCode)
		{
		case VK_LEFT: return COM_MenuLeft;
		case VK_RIGHT: return COM_MenuRight;
		case VK_UP: return (fMultiline ? COM_MenuUp : COM_MenuEnter);
		case VK_DOWN: return (fMultiline ? COM_MenuDown : COM_MenuClose);
		case VK_RETURN: case VK_DELETE: case VK_SPACE: return COM_MenuEnter;
		}
	return COM_None;
	}

int Control2Com(int iControl)
	{
	static BYTE con2com[C4MaxKey]=
		{ 
		COM_CursorLeft,	COM_CursorToggle,	COM_CursorRight,
		COM_Throw,			COM_Up,						COM_Dig,
		COM_Left,				COM_Down,					COM_Right,
		COM_PlayerMenu,	COM_Special,			COM_Special2
		};
	
	if (Inside(iControl,0,C4MaxKey-1))
		return con2com[iControl];
	
	return COM_None;
	}

int Com2MenuCom(int iCom, BOOL fMultiline)
  {
  switch (iCom)
    {
    case COM_Throw: return COM_MenuEnter;
    case COM_Up:    return (fMultiline ? COM_MenuUp : COM_MenuEnter);
    case COM_Dig:   return COM_MenuClose;
    case COM_Left:  return COM_MenuLeft;
    case COM_Down:  return (fMultiline ? COM_MenuDown : COM_MenuClose);
    case COM_Right: return COM_MenuRight;
    }
  return iCom;
  }

void DrawCommandKey(C4Facet &cgo, int iCom, BOOL fPressed, const char *szText)
	{
	Game.GraphicsResource.fctKey.Draw(cgo,FALSE,fPressed);
	Game.GraphicsResource.fctCommand.Draw(cgo,TRUE,Com2Control(iCom),((iCom & COM_Double)!=0));
	if (szText)
		Engine.DDraw.TextOut(szText,cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt-Engine.DDraw.TextHeight()-2,FWhite,FBlack,ACenter);
	}

void DrawControlKey(C4Facet &cgo, int iControl, BOOL fPressed, const char *szText)
	{
	Game.GraphicsResource.fctKey.Draw(cgo,FALSE,fPressed);
	Game.GraphicsResource.fctCommand.Draw(cgo,TRUE,iControl);
	if (szText)
		Engine.DDraw.TextOut(szText,cgo.Surface,cgo.X+cgo.Wdt/2,cgo.Y+cgo.Hgt-Engine.DDraw.TextHeight()-2,FWhite,FBlack,ACenter);
	}

BOOL SellFromBase(int iPlr, C4Object *pBaseObj, C4ID id)
  {
  C4Object *pThing;
  // Valid checks
  if (!ValidPlr(iPlr)) return FALSE;
  if (!pBaseObj || !ValidPlr(pBaseObj->Base)) return FALSE;
  // Base owner eliminated
  if (Game.Players.Get(pBaseObj->Base)->Eliminated)
    {
    SoundEffect("Error",0,100,pBaseObj); 
    sprintf(OSTR,LoadResStr(IDS_PLR_ELIMINATED),Game.Players.Get(pBaseObj->Base)->Name);
    GameMsgPlayer(OSTR,iPlr); return FALSE;
    }
  // Base owner hostile
  if (Hostile(iPlr,pBaseObj->Base))
    {
    SoundEffect("Error",0,100,pBaseObj); 
    sprintf(OSTR,LoadResStr(IDS_PLR_HOSTILE),Game.Players.Get(pBaseObj->Base)->Name);
    GameMsgPlayer(OSTR,iPlr); return FALSE;
    }
  // Get object from home pBaseObj via selected id
  if (!(pThing=pBaseObj->Contents.Find(id))) return FALSE;
  // Sell object (pBaseObj owner gets the money)
  return Game.Players.Get(pBaseObj->Base)->Sell2Home(pThing); 
  }

BOOL Buy2Base(int iPlr, C4Object *pBase, C4ID id)
  {
  int iAvailable; C4Def *pDef; C4Object *pThing;
  // Validity
  if (!ValidPlr(iPlr)) return FALSE;
  if (!pBase || !ValidPlr(pBase->Base)) return FALSE;
  // Base owner eliminated
  if (Game.Players.Get(pBase->Base)->Eliminated)
    {
    SoundEffect("Error",0,100,pBase); 
    sprintf(OSTR,LoadResStr(IDS_PLR_ELIMINATED),Game.Players.Get(pBase->Base)->Name);
    GameMsgPlayer(OSTR,iPlr); return FALSE;
    }
  // Base owner hostile
  if (Hostile(iPlr,pBase->Base))
    {
    SoundEffect("Error",0,100,pBase); 
    sprintf(OSTR,LoadResStr(IDS_PLR_HOSTILE),Game.Players.Get(pBase->Base)->Name);
    GameMsgPlayer(OSTR,iPlr); return FALSE;
    }
  // Get def (base owner's homebase material)
	iAvailable = Game.Players.Get(pBase->Base)->HomeBaseMaterial.GetIDCount(id);
  if (!(pDef=C4Id2Def(id))) return FALSE;
	// Object not available
  if (iAvailable<=0) return FALSE;
	// Not enough wealth (base owner's wealth)
  if (pDef->Value>Game.Players.Get(pBase->Base)->Wealth) 
    {                                       
    GameMsgPlayer(LoadResStr(IDS_PLR_NOWEALTH),iPlr); 
    SoundEffect("Error",0,100,pBase); return FALSE;    
    }
  // Decrease homebase material count
  if (!Game.Players.Get(pBase->Base)->HomeBaseMaterial.DecreaseIDCount(id,FALSE)) return FALSE;
  // Reduce wealth
  Game.Players.Get(pBase->Base)->DoWealth(-pDef->Value);
  // Create object (for player)
  if (!(pThing=Game.CreateObject(id,iPlr))) return FALSE;
	// Make crew member
	if (pDef->CrewMember) if (ValidPlr(iPlr))
		Game.Players.Get(iPlr)->MakeCrewMember(pThing);
  // Object enter target object
  pThing->Enter(pBase);
	// Success
  return TRUE;
  }

BOOL PlayerObjectCommand(int plr, int cmdf, C4Object *pTarget, int tx, int ty)
	{
	int iAddMode = C4P_Command_Set;
	// Adjust for old-style keyboard throw control: add & in range 
	if (cmdf==C4CMD_Throw) iAddMode = C4P_Command_Add | C4P_Command_Range;
	// Route to player
	C4Player *pPlr=Game.Players.Get(plr);
	if (pPlr) return pPlr->ObjectCommand(cmdf,pTarget,tx,ty,NULL,0,iAddMode);
	return FALSE;
	}
