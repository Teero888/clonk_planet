/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* The command stack controls an object's complex and independent behavior */

#include <C4Include.h>

const int MoveToRange = 5, LetGoRange1 = 7, LetGoRange2 = 30, DigRange = 1;
const int FollowRange = 6, PushToRange = 10, DigOutPositionRange = 15;
const int PathRange = 20, MaxPathRange = 1000;
const int JumpAngle = 35, JumpLowAngle = 80, JumpAngleRange = 10, JumpHighAngle = 0;
const int DigOutDirectRange = 130;
const int LetGoHangleAngle = 110;

const char *CommandName(int iCommand) {
  static const char *szCommandName[] = {"None", "Follow",   "MoveTo", "Enter",     "Exit",     "Grab",   "Build",   "Throw", "Chop", "UnGrab",  "Jump",   "Wait",  "Get",  "Put", "Drop",
                                        "Dig",  "Activate", "PushTo", "Construct", "Transfer", "Attack", "Context", "Buy",   "Sell", "Acquire", "Energy", "Retry", "Home", "Call"};

  if (!Inside(iCommand, C4CMD_First, C4CMD_Last))
    return "None";

  return szCommandName[iCommand];
}

WORD CommandNameID(int iCommand) {
  static WORD dwCommandNameID[] = {IDS_COMM_NONE,     IDS_COMM_FOLLOW, IDS_COMM_MOVETO,    IDS_COMM_ENTER,    IDS_COMM_EXIT,   IDS_COMM_GRAB,    IDS_COMM_BUILD, IDS_COMM_THROW,
                                   IDS_COMM_CHOP,     IDS_COMM_UNGRAB, IDS_COMM_JUMP,      IDS_COMM_WAIT,     IDS_COMM_GET,    IDS_COMM_PUT,     IDS_COMM_DROP,  IDS_COMM_DIG,
                                   IDS_COMM_ACTIVATE, IDS_COMM_PUSHTO, IDS_COMM_CONSTRUCT, IDS_COMM_TRANSFER, IDS_COMM_ATTACK, IDS_COMM_CONTEXT, IDS_COMM_BUY,   IDS_COMM_SELL,
                                   IDS_COMM_ACQUIRE,  IDS_COMM_ENERGY, IDS_COMM_RETRY,     IDS_CON_HOME,      IDS_COMM_CALL};

  if (!Inside(iCommand, C4CMD_First, C4CMD_Last))
    return IDS_COMM_NONE;

  return dwCommandNameID[iCommand];
}

int CommandByName(const char *szCommand) {
  for (int cnt = C4CMD_First; cnt <= C4CMD_Last; cnt++)
    if (SEqual(szCommand, CommandName(cnt)))
      return cnt;
  return C4CMD_None;
}

void AdjustMoveToTarget(int &rX, int &rY, BOOL fFreeMove, int iShapeHgt) {
  int iY;
  // Above solid (always)
  for (iY = rY; (iY >= 0) && GBackSolid(rX, iY); iY--)
    ;
  if (iY >= 0)
    rY = iY;
  // No-free-move adjustments (i.e. if walking)
  if (!fFreeMove) {
    // Drop down to bottom of free space
    if (!GBackSemiSolid(rX, rY)) {
      for (iY = rY; (iY < GBackHgt) && !GBackSemiSolid(rX, iY + 1); iY++)
        ;
      if (iY < GBackHgt)
        rY = iY;
    }
    // Vertical shape offset above solid
    if (GBackSolid(rX, rY + 1) || GBackSolid(rX, rY + 5))
      if (!GBackSemiSolid(rX, rY - iShapeHgt / 2))
        rY -= iShapeHgt / 2;
  }
}

BOOL FreeMoveTo(C4Object *cObj) {
  // Floating: we accept any move-to target
  if (cObj->GetProcedure() == DFA_FLOAT)
    return TRUE;
  // Assume we're walking: move-to targets are adjusted
  return FALSE;
}

BOOL AdjustSolidOffset(int &rX, int &rY, int iXOff, int iYOff) {
  int cnt;
  // In solid: fail
  if (GBackSolid(rX, rY))
    return FALSE;
  // Y Offset
  for (cnt = 1; cnt < iYOff; cnt++) {
    if (GBackSolid(rX, rY + cnt) && !GBackSolid(rX, rY - cnt))
      rY--;
    if (GBackSolid(rX, rY - cnt) && !GBackSolid(rX, rY + cnt))
      rY++;
  }
  // X Offset
  for (cnt = 1; cnt < iXOff; cnt++) {
    if (GBackSolid(rX + cnt, rY) && !GBackSolid(rX - cnt, rY))
      rX--;
    if (GBackSolid(rX - cnt, rY) && !GBackSolid(rX + cnt, rY))
      rX++;
  }
  // Done
  return TRUE;
}

int SolidOnWhichSide(int iX, int iY) {
  for (int cx = 1; cx < 10; cx++)
    for (int cy = 0; cy < 10; cy++) {
      if (GBackSolid(iX - cx, iY - cy) || GBackSolid(iX - cx, iY + cy))
        return -1;
      if (GBackSolid(iX + cx, iY - cy) || GBackSolid(iX + cx, iY + cy))
        return +1;
    }
  return 0;
}

C4Command::C4Command() { Default(); }

C4Command::~C4Command() { Clear(); }

void C4Command::Default() {
  Command = C4CMD_None;
  cObj = NULL;
  Evaluated = FALSE;
  PathChecked = FALSE;
  Finished = FALSE;
  Tx = Ty = 0;
  Target = Target2 = NULL;
  Data = 0;
  UpdateInterval = 0;
  Failures = 0;
  Retries = 0;
  Permit = 0;
  Text = NULL;
  Next = NULL;
}

BOOL ObjectAddWaypoint(int iX, int iY, int iTransferTarget, int ipObject) {
  C4Object *cObj = (C4Object *)(intptr_t)ipObject;
  if (!cObj)
    return FALSE;

  // Transfer waypoint
  if (iTransferTarget)
    return cObj->AddCommand(C4CMD_Transfer, (C4Object *)(intptr_t)iTransferTarget, iX, iY, 0, NULL, FALSE);

  // Solid offset
  AdjustSolidOffset(iX, iY, cObj->Shape.Wdt / 2, cObj->Shape.Hgt / 2);

  // Standard movement waypoint update interval
  int iUpdate = 25;
  // Waypoints before transfer zones are not updated (enforce move to that
  // waypoint)
  if (cObj->Command && (cObj->Command->Command == C4CMD_Transfer))
    iUpdate = 0;
  // Add waypoint
  if (!cObj->AddCommand(C4CMD_MoveTo, NULL, iX, iY, 25, NULL, FALSE))
    return FALSE;

  return TRUE;
}

void C4Command::MoveTo() {
  // Current object position
  int cx, cy;
  cx = cObj->x;
  cy = cObj->y;
  BOOL fWaypoint = FALSE;
  if (Next && (Next->Command == C4CMD_MoveTo))
    fWaypoint = TRUE;

  // Contained: exit
  if (cObj->Contained) {
    cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    return;
  }

  // Check path (crew members or specific only)
  if ((cObj->OCF & OCF_CrewMember) || cObj->Def->Pathfinder)
    if (!PathChecked)
      // Not too far
      if (Distance(cx, cy, Tx, Ty) < MaxPathRange)
        // Not too close
        if (!(Inside(cx - Tx, -PathRange, +PathRange) && Inside(cy - Ty, -PathRange, +PathRange))) {
          // Path not free: find path
          if (!PathFree(cx, cy, Tx, Ty)) {
            if (!Game.PathFinder.Find(cObj->x, cObj->y, Tx, Ty, &ObjectAddWaypoint, (intptr_t)cObj)) { /* Path not found: react? */
              PathChecked = TRUE;                                                                      /* recheck delay */
            }
            return;
          }
          // Path free: recheck delay
          else
            PathChecked = TRUE;
        }
  // Path recheck
  if (!Tick35)
    PathChecked = FALSE;

  // Pushing grab only: let go (pulling, too?)
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target)
      if (cObj->Action.Target->Def->Pushable == 2) {
        // Re-evaluate this command because vehicle control might have blocked
        // evaluation
        Evaluated = FALSE;
        cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
        return;
      }

  // Special by procedure
  switch (cObj->GetProcedure()) {
  // Push/pull
  case DFA_PUSH:
  case DFA_PULL:
    // Use target object's position if on final waypoint
    if (!fWaypoint)
      if (cObj->Action.Target) {
        cx = cObj->Action.Target->x;
        cy = cObj->Action.Target->y;
      }
    break;
  // Chop, build, dig, bridge: stop
  case DFA_CHOP:
  case DFA_BUILD:
  case DFA_DIG:
  case DFA_BRIDGE:
    ObjectComStop(cObj);
    break;
  }

  // Target range
  int iTargetRange = MoveToRange;
  int iRangeFactorTop = 1, iRangeFactorBottom = 1, iRangeFactorSide = 1;

  // Crew members/pathfinder specific target range
  if (cObj->OCF & OCF_CrewMember) {
    // Range by size
    iTargetRange = cObj->Shape.Wdt / 5;
    // Easier range if waypoint
    if (fWaypoint)
      if (cObj->GetProcedure() != DFA_SCALE) {
        iRangeFactorTop = 3;
        iRangeFactorSide = 3;
        iRangeFactorBottom = 2;
      }
  }

  // Target reached (success)
  if (Inside(cx - Tx, -iRangeFactorSide * iTargetRange, +iRangeFactorSide * iTargetRange) && Inside(cy - Ty, -iRangeFactorBottom * iTargetRange, +iRangeFactorTop * iTargetRange)) {
    cObj->Action.ComDir = COMD_Stop;
    Finish(TRUE);
    return;
  }

  // Idles can't move to
  if (cObj->Action.Act <= ActIdle) {
    Finish();
    return;
  }

  // Action
  switch (cObj->GetProcedure()) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_WALK:
    // Head to target
    if (cx < Tx - iTargetRange)
      cObj->Action.ComDir = COMD_Right;
    if (cx > Tx + iTargetRange)
      cObj->Action.ComDir = COMD_Left;
    // Jump control
    if (JumpControl())
      return;
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_PUSH:
  case DFA_PULL:
    // Head to target
    if (cx < Tx - iTargetRange)
      cObj->Action.ComDir = COMD_Right;
    if (cx > Tx + iTargetRange)
      cObj->Action.ComDir = COMD_Left;
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_SCALE:
    // Head to target
    if (cy > Ty + iTargetRange)
      cObj->Action.ComDir = COMD_Up;
    if (cy < Ty - iTargetRange)
      cObj->Action.ComDir = COMD_Down;
    // Let-Go Control
    if (cObj->Action.Dir == DIR_Left) {
      // Target direction
      if ((Tx > cx + LetGoRange1) && (Inside(cy - Ty, -LetGoRange2, +LetGoRange2))) {
        ObjectComLetGo(cObj, +1);
        return;
      }
      // Contact (not if just started)
      if (cObj->Action.Time > 2)
        if (cObj->t_contact /* & CNAT_Left*/) {
          ObjectComLetGo(cObj, +1);
          return;
        }
    }
    if (cObj->Action.Dir == DIR_Right) {
      // Target direction
      if ((Tx < cx - LetGoRange1) && (Inside(cy - Ty, -LetGoRange2, +LetGoRange2))) {
        ObjectComLetGo(cObj, -1);
        return;
      }
      // Contact (not if just started)
      if (cObj->Action.Time > 2)
        if (cObj->t_contact /* & CNAT_Right*/) {
          ObjectComLetGo(cObj, -1);
          return;
        }
    }
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_SWIM:
    // Head to target
    if (Tick2) {
      if (cx < Tx - iTargetRange)
        cObj->Action.ComDir = COMD_Right;
      if (cx > Tx + iTargetRange)
        cObj->Action.ComDir = COMD_Left;
    } else {
      if (cy < Ty)
        cObj->Action.ComDir = COMD_Down;
      if (cy > Ty)
        cObj->Action.ComDir = COMD_Up;
    }
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_HANGLE:
    // Head to target
    if (cx < Tx - iTargetRange)
      cObj->Action.ComDir = COMD_Right;
    if (cx > Tx + iTargetRange)
      cObj->Action.ComDir = COMD_Left;
    // Let-Go Control
    if (Abs(Angle(cx, cy, Tx, Ty)) > LetGoHangleAngle)
      ObjectComLetGo(cObj, 0);
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  case DFA_FLOAT:
    // Head to target
    if (cx < Tx - iTargetRange)
      cObj->Action.ComDir = COMD_Right;
    if (cx > Tx + iTargetRange)
      cObj->Action.ComDir = COMD_Left;
    if (cy < Ty - iTargetRange)
      cObj->Action.ComDir = COMD_Down;
    if (cy > Ty + iTargetRange)
      cObj->Action.ComDir = COMD_Up;
    if (cx < Tx - iTargetRange)
      if (cy < Ty - iTargetRange)
        cObj->Action.ComDir = COMD_DownRight;
    if (cx > Tx - iTargetRange)
      if (cy < Ty - iTargetRange)
        cObj->Action.ComDir = COMD_DownLeft;
    if (cx < Tx - iTargetRange)
      if (cy > Ty + iTargetRange)
        cObj->Action.ComDir = COMD_UpRight;
    if (cx > Tx + iTargetRange)
      if (cy > Ty + iTargetRange)
        cObj->Action.ComDir = COMD_UpLeft;
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - -
  }
}

void C4Command::Dig() {
  // Current object and target coordinates
  int cx, cy, tx, ty;
  cx = cObj->x;
  cy = cObj->y;
  tx = Tx;
  ty = Ty + cObj->Shape.y + 3; // Target coordinates are bottom center
  BOOL fDigOutMaterial = Data;

  // Grabbing: let go
  if (cObj->GetProcedure() == DFA_PUSH) {
    cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
    return;
  }

  // If contained: exit
  if (cObj->Contained) {
    cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    return;
  }

  // Building or chopping: stop
  if ((cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD))
    ObjectComStop(cObj);

  // Scaling or hangling: let go
  if ((cObj->GetProcedure() == DFA_SCALE) || (cObj->GetProcedure() == DFA_HANGLE))
    ObjectComLetGo(cObj, (cObj->Action.Dir == DIR_Left) ? +1 : -1);

  // Target reached: success
  if (Inside(cx - tx, -MoveToRange, +MoveToRange) && Inside(cy - ty, -MoveToRange, +MoveToRange)) {
    cObj->Action.ComDir = COMD_Stop;
    Finish(TRUE);
    return;
  }

  // Can start digging only from walk
  if (cObj->GetProcedure() != DFA_DIG)
    if (cObj->GetProcedure() != DFA_WALK)
      // Continue trying (assume currently in flight)
      return;

  // Start digging
  if (cObj->GetProcedure() != DFA_DIG)
    if (!ObjectComDig(cObj)) {
      Finish();
      return;
    }

  // Dig2Object
  if (fDigOutMaterial)
    cObj->Action.Data = 1;

  // Head to target
  if (cx < tx - DigRange)
    cObj->Action.ComDir = COMD_Right;
  if (cx > tx + DigRange)
    cObj->Action.ComDir = COMD_Left;
  if (cy < ty - DigRange)
    cObj->Action.ComDir = COMD_Down;
  if (cx < tx - DigRange)
    if (cy < ty - DigRange)
      cObj->Action.ComDir = COMD_DownRight;
  if (cx > tx - DigRange)
    if (cy < ty - DigRange)
      cObj->Action.ComDir = COMD_DownLeft;
  if (cx < tx - DigRange)
    if (cy > ty + DigRange)
      cObj->Action.ComDir = COMD_UpRight;
  if (cx > tx + DigRange)
    if (cy > ty + DigRange)
      cObj->Action.ComDir = COMD_UpLeft;
}

void C4Command::Follow() {

  // If crew member, only selected objects can follow
  if (cObj->Def->CrewMember)
    // Finish successfully to avoid fail message
    if (!cObj->Select) {
      Finish(TRUE);
      return;
    }

  // No-one to follow
  if (!Target) {
    Finish();
    return;
  }

  // Follow containment
  if (cObj->Contained != Target->Contained) {
    // Only crew members can follow containment
    if (!cObj->Def->CrewMember) {
      Finish();
      return;
    }
    // Exit/enter
    if (cObj->Contained)
      cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    else
      cObj->AddCommand(C4CMD_Enter, Target->Contained, 0, 0, 50);
    return;
  }

  // Follow target grabbing
  if (Target->GetProcedure() == DFA_PUSH) {
    // Grabbing same: copy ComDir
    if (cObj->GetProcedure() == DFA_PUSH)
      if (cObj->Action.Target == Target->Action.Target) {
        cObj->Action.ComDir = Target->Action.ComDir;
        return;
      }
    // Else, grab target's grab
    cObj->AddCommand(C4CMD_Grab, Target->Action.Target);
    return;
  }

  // If in following range
  if (Inside(cObj->x - Target->x, -FollowRange, +FollowRange) && Inside(cObj->y - Target->y, -FollowRange, +FollowRange)) {
    // Copy target's Action.ComDir
    cObj->Action.ComDir = Target->Action.ComDir;
  } else // Else, move to target
  {
    cObj->AddCommand(C4CMD_MoveTo, NULL, Target->x, Target->y, 10);
  }
}

void C4Command::Enter() {
  DWORD ocf;

  // No object to enter
  if (!Target) {
    Finish();
    return;
  }

  // Already in target object
  if (cObj->Contained == Target) {
    Finish(TRUE);
    return;
  }

  // Building or chopping: stop
  if ((cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD))
    ObjectComStop(cObj);

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Pushing grab only: let go
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target)
      if (cObj->Action.Target->Def->Pushable == 2) {
        cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
        return;
      }

  // Pushing target: let go
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target == Target) {
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
      return;
    }

  // Grabbing overrides position for target
  int cx, cy;
  cx = cObj->x;
  cy = cObj->y;
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target) {
      cx = cObj->Action.Target->x;
      cy = cObj->Action.Target->y;
    }

  // If in entrance range
  ocf = OCF_Entrance;
  if (!cObj->Contained && Target->At(cx, cy, ocf) && (ocf & OCF_Entrance)) {
    // Stop
    cObj->Action.ComDir = COMD_Stop;
    // Pushing: push target into entrance, then stop
    if ((cObj->GetProcedure() == DFA_PUSH) && cObj->Action.Target) {
      cObj->Action.Target->SetCommand(C4CMD_Enter, Target);
      Finish(TRUE);
      return;
    }
    // Else, enter self
    else {
      // If entrance open, enter object
      if (Target->EntranceStatus != 0) {
        cObj->Enter(Target);
        Finish(TRUE);
        return;
      } else // Else, activate entrance
        Target->ActivateEntrance(cObj->Controller, cObj);
    }

  } else // Else, move to object's entrance
  {
    int ex, ey, ewdt, ehgt;
    if (Target->GetEntranceArea(ex, ey, ewdt, ehgt))
      cObj->AddCommand(C4CMD_MoveTo, NULL, ex + ewdt / 2, ey + ehgt / 2, 50);
  }
}

void C4Command::Exit() {

  // Outside: done
  if (!cObj->Contained) {
    Finish(TRUE);
    return;
  }

  // Building: stop
  if (cObj->GetProcedure() == DFA_BUILD)
    ObjectComStop(cObj);

  // Entrance open, leave object
  if (cObj->Contained->EntranceStatus) {
    // Exit to container's container
    if (cObj->Contained->Contained) {
      cObj->Enter(cObj->Contained->Contained);
      Finish(TRUE);
      return;
    }
    // Exit to entrance area
    int ex, ey, ewdt, ehgt;
    if (cObj->Contained->OCF & OCF_Entrance)
      if (cObj->Contained->GetEntranceArea(ex, ey, ewdt, ehgt)) {
        cObj->Exit(ex + ewdt / 2, ey + ehgt + cObj->Shape.y - 1);
        Finish(TRUE);
        return;
      }
    // Exit jump out of collection area
    if (cObj->Def->Carryable)
      if (cObj->Contained->Def->Collection.Wdt) {
        cObj->Exit(cObj->Contained->x, cObj->Contained->y + cObj->Contained->Def->Collection.y - 1);
        ObjectComJump(cObj);
        Finish(TRUE);
        return;
      }
    // Plain exit
    cObj->Exit(cObj->x, cObj->y);
    Finish(TRUE);
    return;
  }

  // Entrance closed, activate entrance
  else {
    if (!cObj->Contained->ActivateEntrance(cObj->Controller, cObj))
    // Entrance activation failed: fail
    {
      Finish();
      return;
    }
  }
}

void C4Command::Grab() {
  DWORD ocf;
  // Command fulfilled
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target == Target) {
      Finish(TRUE);
      return;
    }
  // Building or chopping: stop
  if ((cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD))
    ObjectComStop(cObj);
  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);
  // Grabbing: let go
  if (cObj->GetProcedure() == DFA_PUSH) {
    cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
    return;
  }
  // No target
  if (!Target) {
    Finish();
    return;
  }
  // At target object: grab
  ocf = OCF_All;
  if (!cObj->Contained && Target->At(cObj->x, cObj->y, ocf)) {
    // Scaling or hangling: let go
    if ((cObj->GetProcedure() == DFA_SCALE) || (cObj->GetProcedure() == DFA_HANGLE))
      ObjectComLetGo(cObj, (cObj->Action.Dir == DIR_Left) ? +1 : -1);
    // Grab
    cObj->Action.ComDir = COMD_Stop;
    ObjectComGrab(cObj, Target);
  }
  // Else, move to object
  else {
    cObj->AddCommand(C4CMD_MoveTo, NULL, Target->x, Target->y, 50);
  }
}

void C4Command::PushTo() {
  // Target check
  if (!Target) {
    Finish();
    return;
  }

  // Target is target self: fail
  if (Target == Target2) {
    Finish();
    return;
  }

  // Command fulfilled
  if (Target2) {
    // Object in correct target container: success
    if (Target->Contained == Target2) {
      Finish(TRUE);
      return;
    }
  } else {
    // Object at target position: success
    if (Inside(Target->x - Tx, -PushToRange, +PushToRange))
      if (Inside(Target->y - Ty, -PushToRange, +PushToRange)) {
        cObj->Action.ComDir = COMD_Stop;
        cObj->AddCommand(C4CMD_UnGrab);
        cObj->AddCommand(C4CMD_Wait, NULL, 0, 0, 10);
        Finish(TRUE);
        return;
      }
  }

  // Building or chopping: stop
  if ((cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD))
    ObjectComStop(cObj);

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Target contained: activate
  if (Target->Contained) {
    cObj->AddCommand(C4CMD_Activate, Target, 0, 0, 40);
    return;
  }

  // Grab target
  if (!((cObj->GetProcedure() == DFA_PUSH) && (cObj->Action.Target == Target))) {
    cObj->AddCommand(C4CMD_Grab, Target, 0, 0, 40);
    return;
  }

  // Move to target position / enter target object
  if (Target2) {
    cObj->AddCommand(C4CMD_Enter, Target2, 0, 0, 40);
    return;
  } else {
    cObj->AddCommand(C4CMD_MoveTo, NULL, Tx, Ty, 40);
    return;
  }
}

void C4Command::Chop() {
  DWORD ocf;
  // No target: fail
  if (!Target) {
    Finish();
    return;
  }
  // Can not chop: fail
  if (!cObj->GetPhysical()->CanChop) {
    Finish();
    return;
  }
  // Target not chopable: done (assume was successfully chopped)
  if (!(Target->OCF & OCF_Chop)) {
    Finish(TRUE);
    return;
  }
  // Chopping target: wait
  if ((cObj->GetProcedure() == DFA_CHOP) && (cObj->Action.Target == Target))
    return;
  // Grabbing: let go
  if (cObj->GetProcedure() == DFA_PUSH) {
    cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
    return;
  }
  // Building, digging or chopping other: stop
  if ((cObj->GetProcedure() == DFA_DIG) || (cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD)) {
    ObjectComStop(cObj);
    return;
  }
  // At target object
  ocf = OCF_All;
  if (!cObj->Contained && Target->At(cObj->x, cObj->y, ocf)) {
    cObj->Action.ComDir = COMD_Stop;
    ObjectComChop(cObj, Target);
  }
  // Else, move to object
  else {
    cObj->AddCommand(C4CMD_MoveTo, NULL, (cObj->x > Target->x) ? Target->x + 6 : Target->x - 6, Target->y, 50);
  }
}

void C4Command::Build() {
  DWORD ocf;
  // No target: cancel
  if (!Target) {
    Finish();
    return;
  }
  // Target complete: Command fulfilled
  if (Target->Con >= FullCon) {
    // Activate internal vehicles
    if (Target->Contained && (Target->Category & C4D_Vehicle))
      cObj->AddCommand(C4CMD_Activate, Target);
    // Energy supply (if necessary and nobody else is doing so already)
    if (Game.Rules & C4RULE_StructuresNeedEnergy)
      if (Target->Def->LineConnect & C4D_Power_Input)
        if (!Game.FindObjectByCommand(C4CMD_Energy, Target))
          cObj->AddCommand(C4CMD_Energy, Target);
    // Done
    cObj->Action.ComDir = COMD_Stop;
    Finish(TRUE);
    return;
  }
  // Currently working on target: continue
  if (cObj->GetProcedure() == DFA_BUILD)
    if (cObj->Action.Target == Target)
      return;
  // Grabbing: let go
  if (cObj->GetProcedure() == DFA_PUSH) {
    cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
    return;
  }
  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);
  // Worker ist structure or static back: internal target build only (old stuff)
  if ((cObj->Category & C4D_Structure) || (cObj->Category & C4D_StaticBack)) {
    // Target is internal
    if (Target->Contained == cObj) {
      ObjectComBuild(cObj, Target);
      return;
    }
    // Target is not internal: cancel
    Finish();
    return;
  }
  // At target check
  ocf = OCF_All;
  if ((Target->Contained && (cObj->Contained == Target->Contained)) || (Target->At(cObj->x, cObj->y, ocf) && (cObj->GetProcedure() == DFA_WALK))) {
    ObjectComStop(cObj);
    ObjectComBuild(cObj, Target);
    return;
  }
  // Else, move to object
  else {
    if (Target->Contained)
      cObj->AddCommand(C4CMD_Enter, Target->Contained, 0, 0, 50);
    else
      cObj->AddCommand(C4CMD_MoveTo, NULL, Target->x, Target->y, 50);
    return;
  }
}

void C4Command::UnGrab() {
  ObjectComUnGrab(cObj);
  cObj->Action.ComDir = COMD_Stop;
  Finish(TRUE);
}

void C4Command::Throw() {

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Throw specific object not in contents: get object
  if (Target)
    if (!cObj->Contents.GetLink(Target)) {
      cObj->AddCommand(C4CMD_Get, Target, 0, 0, 40);
      return;
    }

  // Target coordinates are not default 0/0: targeted throw
  if ((Tx != 0) || (Ty != 0)) {

    // Grabbing: let go
    if (cObj->GetProcedure() == DFA_PUSH) {
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
      return;
    }

    // Preferred throwing direction
    int iDir = +1;
    if (cObj->x > Tx)
      iDir = -1;

    // Find throwing position
    long lx = 0, ly = 0;
    FIXED pthrow = ftofix(4.0 * (float)cObj->GetPhysical()->Throw / (float)C4MaxPhysical);
    int iHeight = cObj->Shape.Hgt;
    if (!FindThrowingPosition(Tx, Ty, pthrow * iDir, -pthrow, iHeight, lx, ly))
      if (!FindThrowingPosition(Tx, Ty, pthrow * iDir * -1, -pthrow, iHeight, lx, ly))
      // No throwing position: fail
      {
        Finish();
        return;
      }
    int iTx = lx, iTy = ly;

    // At throwing position: face target and throw
    if (Inside(cObj->x - iTx, -MoveToRange, +MoveToRange) && Inside(cObj->y - iTy, -15, +15)) {
      if (cObj->x < Tx)
        cObj->SetDir(DIR_Right);
      else
        cObj->SetDir(DIR_Left);
      cObj->Action.ComDir = COMD_Stop;
      if (ObjectComThrow(cObj, Target))
        Finish(TRUE); // Throw successfull: done, else continue
      return;
    }

    // Move to target position
    cObj->AddCommand(C4CMD_MoveTo, NULL, iTx, iTy, 20);

    return;
  }

  // Contained: put or take
  if (cObj->Contained) {
    ObjectComPutTake(cObj, cObj->Contained, Target);
    Finish(TRUE);
    return;
  }

  // Pushing: put or take
  if (cObj->GetProcedure() == DFA_PUSH) {
    if (cObj->Action.Target)
      ObjectComPutTake(cObj, cObj->Action.Target, Target);
    Finish(TRUE);
    return;
  }

  // Outside: Throw
  ObjectComThrow(cObj, Target);
  Finish(TRUE);
}

void C4Command::Drop() {

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Drop specific object not in contents: get object
  if (Target)
    if (!cObj->Contents.GetLink(Target)) {
      cObj->AddCommand(C4CMD_Get, Target, 0, 0, 40);
      return;
    }

  // Target coordinates are not default 0/0: targeted drop
  if ((Tx != 0) || (Ty != 0)) {
    // Grabbing: let go
    if (cObj->GetProcedure() == DFA_PUSH) {
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
      return;
    }
    // At target position: drop
    if (Inside(cObj->x - Tx, -MoveToRange, +MoveToRange) && Inside(cObj->y - Ty, -15, +15)) {
      cObj->Action.ComDir = COMD_Stop;
      ObjectComDrop(cObj, Target);
      Finish(TRUE);
      return;
    }
    // Move to target position
    cObj->AddCommand(C4CMD_MoveTo, NULL, Tx, Ty, 20);
    return;
  }

  // Contained: put
  if (cObj->Contained) {
    ObjectComPutTake(cObj, cObj->Contained, Target);
    Finish(TRUE);
    return;
  }

  // Pushing: put
  if (cObj->GetProcedure() == DFA_PUSH) {
    if (cObj->Action.Target)
      ObjectComPutTake(cObj, cObj->Action.Target, Target);
    Finish(TRUE);
    return;
  }

  // Outside: Drop
  ObjectComDrop(cObj, Target);
  Finish(TRUE);
}

void C4Command::Jump() {
  // Tx not default 0: adjust jump direction
  if (Tx) {
    if (Tx < cObj->x)
      cObj->SetDir(DIR_Left);
    if (Tx > cObj->x)
      cObj->SetDir(DIR_Right);
  }
  // Jump
  ObjectComJump(cObj);
  // Done
  Finish(TRUE);
}

void C4Command::Wait() {
  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);
}

void C4Command::Context() {
  // Not context object specified (in Target2): fail
  if (!Target2) {
    Finish();
    return;
  }
  // Open context menu for target
  cObj->ActivateMenu(C4MN_Context, 0, 0, 0, Target2);
  if (Tx != 0 && Ty != 0)
    if (cObj->Menu) {
      cObj->Menu->SetAlignment(C4MN_Align_Free);
      cObj->Menu->SetLocation(Tx, Ty);
    }
  // Done
  Finish(TRUE);
}

void C4Command::Get() {

  // Data set and target specified: open get menu & done (old style)
  if ((Data == 1) && Target) {
    cObj->ActivateMenu(C4MN_Get, 0, 0, 0, Target);
    Finish(TRUE);
    return;
  }

  // No target: failure
  if (!Target) {
    Finish();
    return;
  }

  // Target not carryable: failure
  if (!(Target->OCF & OCF_Carryable)) {
    Finish();
    return;
  }

  // Target collected: success
  if (Target->Contained == cObj) {
    cObj->Action.ComDir = COMD_Stop;
    Finish(TRUE);
    return;
  }

  // Grabbing other than target container: let go
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target != Target->Contained) {
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
      return;
    }

  // Target in solid: dig out
  if (!Target->Contained && (Target->OCF & OCF_InSolid)) {
    // Check for closest free position
    long lx = Target->x, ly = Target->y;
    // Find all-closest dig-out position
    if (!FindClosestFree(lx, ly, -120, +120, -1, -1))
    // None found
    {
      Finish();
      return;
    }
    int iX = lx, iY = ly;
    // Check good-angle left/right dig-out position
    long lx2 = Target->x, ly2 = Target->y;
    if (FindClosestFree(lx2, ly2, -140, +140, -40, +40))
      // Use good-angle position if it's not way worse
      if (Distance(Target->x, Target->y, lx2, ly2) < 10 * Distance(Target->x, Target->y, iX, iY)) {
        iX = lx2;
        iY = ly2;
      }
    // Move to closest free position (if not in dig-direct range)
    if (!Inside(cObj->x - iX, -DigOutPositionRange, +DigOutPositionRange) || !Inside(cObj->y - iY, -DigOutPositionRange, +DigOutPositionRange)) {
      cObj->AddCommand(C4CMD_MoveTo, NULL, iX, iY, 50);
      return;
    }
    // DigTo
    cObj->AddCommand(C4CMD_Dig, NULL, Target->x, Target->y + 4, 50);
    return;
  }

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Target contained
  if (Target->Contained) {

    // In same container: grab target
    if (cObj->Contained == Target->Contained) {
      // Collection limit: put other object to container
      if (cObj->Def->CollectionLimit && (cObj->Contents.ObjectCount() >= cObj->Def->CollectionLimit))
        ObjectComPut(cObj, cObj->Contained);
      // Check RejectCollect
      if (cObj->Call(PSF_RejectCollection, (int)(long)Target->Def->id, (intptr_t)Target)) {
        // Can't get due to RejectCollect: fail
        Finish();
        return;
      }
      // No minimum con knowledge vehicles/items (hack): fail
      if ((Target->Category & C4D_Vehicle) || (Target->Category & C4D_Object))
        if (Target->Category & C4D_SelectKnowledge)
          if (Target->Con < FullCon * 75 / 100) {
            Finish();
            return;
          }
      // Get target object
      Target->Enter(cObj);
      // Get call to object script
      cObj->Call(PSF_Get);
      // Done
      return;
    }

    // Leave own container
    if (cObj->Contained) {
      cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
      return;
    }

    // Target container has grab get: grab target container
    if (Target->Contained->Def->GrabPutGet & C4D_Grab_Get) {
      // Grabbing target container
      if ((cObj->GetProcedure() == DFA_PUSH) && (cObj->Action.Target == Target->Contained)) {
        // Collection limit: put other object to container
        if (cObj->Def->CollectionLimit && (cObj->Contents.ObjectCount() >= cObj->Def->CollectionLimit))
          ObjectComPut(cObj, Target->Contained);
        // Check RejectCollect
        if (cObj->Call(PSF_RejectCollection, (int)(long)Target->Def->id, (intptr_t)Target)) {
          // Can't get due to RejectCollect: fail
          Finish();
          return;
        } // Get target object
        Target->Enter(cObj);
        // Get call to object script
        cObj->Call(PSF_Get);
        // Done
        return;
      }
      // Grab target container
      cObj->AddCommand(C4CMD_Grab, Target->Contained, 0, 0, 50);
      return;
    }

    // Target container has entrance: enter target container
    if (Target->Contained->OCF & OCF_Entrance) {
      cObj->AddCommand(C4CMD_Enter, Target->Contained, 0, 0, 50);
      return;
    }

    // Can't get to target due to target container: fail
    Finish();
    return;
  }

  // Target outside

  // Leave own container
  if (cObj->Contained) {
    cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    return;
  }

  // Outside
  if (!cObj->Contained) {

    // Target in collection range
    DWORD ocf = OCF_Normal | OCF_Collection;
    if (cObj->At(Target->x, Target->y, ocf)) {
      // Collection limit: drop other object
      if (cObj->Def->CollectionLimit && (cObj->Contents.ObjectCount() >= cObj->Def->CollectionLimit)) {
        cObj->Action.ComDir = COMD_Stop;
        ObjectComDrop(cObj);
      }
      // Check RejectCollect
      if (cObj->Call(PSF_RejectCollection, (int)(long)Target->Def->id, (int)(long)Target)) {
        // Can't get due to RejectCollect: fail
        Finish();
        cObj->Action.ComDir = COMD_Stop;
        return;
      }
      // Grab target object
      Target->Enter(cObj);
    }

    // Target not in range
    else {
      // Target in jumping range above clonk: try side-move jump
      if (Inside(cObj->x - Target->x, -10, +10))
        if (Inside(cObj->y - Target->y, 30, 50)) {
          int iSideX = 1;
          if (Random(2))
            iSideX = -1;
          iSideX = cObj->x + iSideX * (cObj->y - Target->y);
          if (PathFree(iSideX, cObj->y, Target->x, Target->y)) {
            // Side-move jump
            cObj->AddCommand(C4CMD_Jump, NULL, Tx, Ty);
            if (cObj->Def->CollectionLimit && (cObj->Contents.ObjectCount() >= cObj->Def->CollectionLimit))
              cObj->AddCommand(C4CMD_Drop); // Drop object if necessary due to
                                            // collection limit
            // Need to kill NoCollectDelay after drop...!
            cObj->AddCommand(C4CMD_MoveTo, NULL, iSideX, cObj->y, 50);
          }
        }
      // Move to target (random offset for difficult pickups)
      // ...avoid offsets into solid which would lead to high above surface
      // locations!
      cObj->AddCommand(C4CMD_MoveTo, NULL, Target->x + Random(15) - 7, Target->y, 25, NULL);
    }
  }
}

void C4Command::Activate() {

  // Container specified, but no Target & no type: open get menu for container
  if (Target2 && !Target && !Data) {
    cObj->ActivateMenu(C4MN_Get, 0, 0, 0, Target2);
    Finish(TRUE);
    return;
  }

  // Target object specified & outside: success
  if (Target)
    if (!Target->Contained) {
      Finish(TRUE);
      return;
    }

  // No container specified: determine container by target object
  if (!Target2)
    if (Target)
      Target2 = Target->Contained;

  // No container specified: fail
  if (!Target2) {
    Finish();
    return;
  }

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // In container
  if (cObj->Contained == Target2) {
    // Target object specified: activate & done
    if (Target) {
      if (!ObjectComActivate(cObj, C4ID_None, Target)) {
        Finish();
        return;
      }
      Finish(TRUE);
      return;
    }
    // Type specified: activate & done
    if (Data) {
      for (Tx = Max(Tx, 1); Tx; Tx--)
        if (!ObjectComActivate(cObj, Data)) {
          Finish();
          return;
        }
      Finish(TRUE);
      return;
    }
    // No target object & no type specified: fail
    Finish();
    return;
  }

  // Leave own container
  if (cObj->Contained) {
    cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    return;
  }

  // Target container has entrance: enter
  if (Target2->OCF & OCF_Entrance) {
    cObj->AddCommand(C4CMD_Enter, Target2, 0, 0, 50);
    return;
  }

  // Can't get to target due to target container: fail
  Finish();
}

void C4Command::Put() {
  // No target: failure
  if (!Target) {
    Finish();
    return;
  }

  // Thing to put specified by type
  if (!Target2)
    if (Data)
      Target2 = cObj->Contents.Find(Data);
  // Thing to put is first contents object
  if (!Target2)
    Target2 = cObj->Contents.GetObject();

  // No thing to put: nothing to do
  if (!Target2) {
    Finish();
    return;
  }

  // Thing is in target: done
  if (Target2->Contained == Target) {
    Finish(TRUE);
    return;
  }

  // Thing to put not in contents: get object
  if (!cObj->Contents.GetLink(Target2)) {
    // Object is nearby and traveling: wait
    if (!Target2->Contained)
      if (Distance(cObj->x, cObj->y, Target2->x, Target2->y) < 80)
        if (Target2->OCF & OCF_HitSpeed1)
          return;
    // Go get it
    cObj->AddCommand(C4CMD_Get, Target2, 0, 0, 40);
    return;
  }

  // Target is contained: can't do it
  if (Target->Contained) {
    Finish();
    return;
  }

  // Digging: stop
  if (cObj->GetProcedure() == DFA_DIG)
    ObjectComStop(cObj);

  // Grabbing other than target: let go
  C4Object *pGrabbing = NULL;
  if (cObj->GetProcedure() == DFA_PUSH)
    pGrabbing = cObj->Action.Target;
  if (pGrabbing && (pGrabbing != Target)) {
    cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
    return;
  }

  // In target: put & done
  if (cObj->Contained == Target) {
    ObjectComPut(cObj, Target, Target2);
    Finish(TRUE);
    return;
  }

  // Leave own container
  if (cObj->Contained) {
    cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 50);
    return;
  }

  // Target has collection: throw in if not fragile, not grabbing target and
  // throwing position found
  if (Target->OCF & OCF_Collection)
    if (!Target2->Def->Fragile)
      if (pGrabbing != Target) {
        int iTx = Target->x + Target->Def->Collection.x + Target->Def->Collection.Wdt / 2;
        int iTy = Target->y + Target->Def->Collection.y + Target->Def->Collection.Hgt / 2;
        FIXED pthrow = ftofix(4.0 * (float)cObj->GetPhysical()->Throw / (float)C4MaxPhysical);
        int iHeight = cObj->Shape.Hgt;
        long lx = 0, ly = 0;
        int iObjDist = Distance(cObj->x, cObj->y, Target->x, Target->y);
        if ((FindThrowingPosition(iTx, iTy, pthrow, -pthrow, iHeight, lx, ly) && (Distance((int)lx, (int)ly, cObj->x, cObj->y) < iObjDist)) ||
            (FindThrowingPosition(iTx, iTy, pthrow * -1, -pthrow, iHeight, lx, ly) && (Distance((int)lx, (int)ly, cObj->x, cObj->y) < iObjDist))) {
          // Throw
          cObj->AddCommand(C4CMD_Throw, Target2, (int)lx, (int)ly, 5);
          return;
        }
      }

  // Target has C4D_Grab_Put: grab target and put
  if (Target->Def->GrabPutGet & C4D_Grab_Put) {
    // Grabbing target container
    if (pGrabbing == Target) {
      // Put to target
      ObjectComPut(cObj, Target, Target2);
      // Ungrab & done
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0);
      Finish(TRUE);
      return;
    }
    // Grab target
    cObj->AddCommand(C4CMD_Grab, Target, 0, 0, 50);
    return;
  }

  // Target can be entered: enter target
  if (Target->OCF & OCF_Entrance) {
    cObj->AddCommand(C4CMD_Enter, Target, 0, 0, 50);
    return;
  }
}

void C4Command::ClearPointers(C4Object *pObj) {
  if (cObj == pObj)
    cObj = NULL;
  if (Target == pObj)
    Target = NULL;
  if (Target2 == pObj)
    Target2 = NULL;
}

void C4Command::Execute() {

  // Finished?!
  if (Finished)
    return;

  // Parent object safety
  if (!cObj) {
    Finish();
    return;
  }

  // Delegated command failed
  if (Failures) {
    // Retry
    if (Retries > 0) {
      Failures = 0;
      Retries--;
      cObj->AddCommand(C4CMD_Retry, NULL, 0, 0, 10);
      return;
    }
    // Too many failures
    else {
      Finish();
      return;
    }
  }

  // Command update
  if (UpdateInterval > 0) {
    UpdateInterval--;
    if (UpdateInterval == 0) {
      Finish(TRUE);
      return;
    }
  }

  // Initial command evaluation
  if (InitEvaluation())
    return;

  // Execute
  switch (Command) {
  case C4CMD_Follow:
    Follow();
    break;
  case C4CMD_MoveTo:
    MoveTo();
    break;
  case C4CMD_Enter:
    Enter();
    break;
  case C4CMD_Exit:
    Exit();
    break;
  case C4CMD_Grab:
    Grab();
    break;
  case C4CMD_UnGrab:
    UnGrab();
    break;
  case C4CMD_Throw:
    Throw();
    break;
  case C4CMD_Chop:
    Chop();
    break;
  case C4CMD_Build:
    Build();
    break;
  case C4CMD_Jump:
    Jump();
    break;
  case C4CMD_Wait:
    Wait();
    break;
  case C4CMD_Get:
    Get();
    break;
  case C4CMD_Put:
    Put();
    break;
  case C4CMD_Drop:
    Drop();
    break;
  case C4CMD_Dig:
    Dig();
    break;
  case C4CMD_Activate:
    Activate();
    break;
  case C4CMD_PushTo:
    PushTo();
    break;
  case C4CMD_Construct:
    Construct();
    break;
  case C4CMD_Transfer:
    Transfer();
    break;
  case C4CMD_Attack:
    Attack();
    break;
  case C4CMD_Context:
    Context();
    break;
  case C4CMD_Buy:
    Buy();
    break;
  case C4CMD_Sell:
    Sell();
    break;
  case C4CMD_Acquire:
    Acquire();
    break;
  case C4CMD_Energy:
    Energy();
    break;
  case C4CMD_Retry:
    Retry();
    break;
  case C4CMD_Home:
    Home();
    break;
  case C4CMD_Call:
    Call();
    break;
  default:
    Finish();
    break;
  }

  // Remember this command might have already been deleted through calls
  // made during execution. You must not do anything here...
}

void C4Command::Finish(BOOL fSuccess) {
  // Mark finished
  Finished = TRUE;
  // Failed
  if (!fSuccess)
    Fail();
}

BOOL C4Command::InitEvaluation() {
  // Already evaluated
  if (Evaluated)
    return FALSE;
  // Set evaluation flag
  Evaluated = TRUE;
  // Evaluate
  switch (Command) {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - -
  case C4CMD_MoveTo:
    // Target coordinates by Target
    if (Target) {
      Tx = Target->x;
      Ty = Target->y;
      Target = NULL;
    }
    // Adjust coordinates
    AdjustMoveToTarget(Tx, Ty, FreeMoveTo(cObj), cObj->Shape.Hgt);
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4CMD_PushTo:
    // Adjust coordinates
    AdjustMoveToTarget(Tx, Ty, FreeMoveTo(cObj), cObj->Shape.Hgt);
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4CMD_Exit:
    // Cancel attach
    ObjectComCancelAttach(cObj);
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4CMD_Wait:
    // Update interval by Data
    if (Data)
      UpdateInterval = Data;
    // Else update interval by Tx
    else if (Tx)
      UpdateInterval = Tx;
    return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - -
  }
  // Need not be evaluated
  return FALSE;
}

void C4Command::Clear() {
  Command = C4CMD_None;
  cObj = NULL;
  Evaluated = FALSE;
  PathChecked = FALSE;
  Tx = Ty = 0;
  Target = Target2 = NULL;
  UpdateInterval = 0;
  if (Text)
    delete[] Text;
  Text = NULL;
}

void C4Command::Construct() {

  // No target type to construct: open menu & done
  if (!Data) {
    cObj->ActivateMenu(C4MN_Construction);
    Finish(TRUE);
    return;
  }

  // No valid target type: fail
  C4Def *pDef;
  if (!(pDef = C4Id2Def(Data))) {
    Finish();
    return;
  }

  // Building, chopping, digging: stop
  if ((cObj->GetProcedure() == DFA_CHOP) || (cObj->GetProcedure() == DFA_BUILD) || (cObj->GetProcedure() == DFA_DIG))
    ObjectComStop(cObj);

  // Pushing: let go
  if (cObj->GetProcedure() == DFA_PUSH)
    if (cObj->Action.Target) {
      cObj->AddCommand(C4CMD_UnGrab, NULL, 0, 0, 50);
      return;
    }

  // No construction site specified: find one
  if ((Tx == 0) && (Ty == 0)) {
    long lx = cObj->x, ly = cObj->y;
    if (!FindConSiteSpot(lx, ly, pDef->Shape.Wdt, pDef->Shape.Hgt, pDef->Category, 20))
    // No site found: fail
    {
      Finish();
      return;
    }
    Tx = lx;
    Ty = ly;
  }

  // Has no construction kit: acquire one
  C4Object *pKit;
  if (!(pKit = cObj->Contents.Find(C4ID_Conkit))) {
    cObj->AddCommand(C4CMD_Acquire, 0, 0, 0, 50, 0, TRUE, C4ID_Conkit, FALSE, 5);
    return;
  }

  // Move to construction site
  if (!Inside(cObj->x - Tx, -MoveToRange, +MoveToRange) || !Inside(cObj->y - Ty, -20, +20)) {
    cObj->AddCommand(C4CMD_MoveTo, NULL, Tx, Ty, 50);
    return;
  }

  // Check construction site
  if (!ConstructionCheck(Data, Tx, Ty))
  // Site no good: fail
  {
    Finish();
    return;
  }

  // Create construction
  C4Object *pConstruction = Game.CreateObjectConstruction(Data, cObj->Owner, Tx, Ty, 1, TRUE);

  // Remove conkit
  pKit->AssignRemoval();

  // Finish, start building
  Finish(TRUE);
  cObj->AddCommand(C4CMD_Build, pConstruction);
}

BOOL C4Command::JumpControl() {

  // Crew members only
  if (!((cObj->OCF & OCF_CrewMember) || cObj->Def->Pathfinder))
    return FALSE;

  // Target angle
  int cx = cObj->x, cy = cObj->y;
  int iAngle = Angle(cx, cy, Tx, Ty);
  while (iAngle > 180)
    iAngle -= 360;

  // Diagonal free jump (if in angle range, minimum distance, and top free)
  if (Inside(iAngle - JumpAngle, -JumpAngleRange, +JumpAngleRange) || Inside(iAngle + JumpAngle, -JumpAngleRange, +JumpAngleRange))
    if (PathFree(cx, cy, Tx, Ty))
      if (Distance(cx, cy, Tx, Ty) > 30) {
        int iTopFree;
        for (iTopFree = 0; (iTopFree < 50) && !GBackSolid(cx, cy + cObj->Shape.y - iTopFree); iTopFree++)
          ;
        if (iTopFree >= 15) {
          // sprintf(OSTR,"Diagonal %d (%d)",iAngle,Distance(cx,cy,Tx,Ty));
          // GameMsgObject(OSTR,cObj);
          cObj->AddCommand(C4CMD_Jump, NULL, Tx, Ty);
          return TRUE;
        }
      }

  // High angle side move - jump (2x range)
  if (Inside(iAngle - JumpHighAngle, -3 * JumpAngleRange, +3 * JumpAngleRange))
    // Vertical range
    if (Inside(cy - Ty, 10, 40)) {
      int iSide = SolidOnWhichSide(Tx,
                                   Ty); // take jump height of side move position into consideration...!
      int iDist = 5 * Abs(cy - Ty) / 6;
      int iSideX = cx - iDist * iSide, iSideY = cy;
      AdjustMoveToTarget(iSideX, iSideY, FALSE, 0);
      // Side move target in range
      if (Inside(iSideY - cy, -20, +20)) {
        // Path free from side move target to jump target
        if (PathFree(iSideX, iSideY, Tx, Ty)) {
          // sprintf(OSTR,"High side move %d
          // (%d,%d)",iAngle,iSideX-cx,iSideY-cy); GameMsgObject(OSTR,cObj);
          cObj->AddCommand(C4CMD_Jump, NULL, Tx, Ty);
          cObj->AddCommand(C4CMD_MoveTo, NULL, iSideX, iSideY, 50);
          return TRUE;
        }
        /*else
                { sprintf(OSTR,"Side move %d/%d path not free",iSideX,iSideY);
           GameMsgObject(OSTR,cObj); }*/
      }
      /*else
              { sprintf(OSTR,"Side move %d out of range",iSideX-cx);
         GameMsgObject(OSTR,cObj); }*/
    }
  /*else
          { sprintf(OSTR,"No high range %d",cy-Ty); GameMsgObject(OSTR,cObj);
     }*/

  // Low side contact jump
  int iLowSideRange = 5;
  if (cObj->t_contact & CNAT_Right)
    if (Inside(iAngle - JumpLowAngle, -iLowSideRange * JumpAngleRange, +iLowSideRange * JumpAngleRange)) {
      // sprintf(OSTR,"Low contact right %d",iAngle); GameMsgObject(OSTR,cObj);
      cObj->AddCommand(C4CMD_Jump, NULL, Tx, Ty);
      return TRUE;
    }
  if (cObj->t_contact & CNAT_Left)
    if (Inside(iAngle + JumpLowAngle, -iLowSideRange * JumpAngleRange, +iLowSideRange * JumpAngleRange)) {
      // sprintf(OSTR,"Low contact left %d",iAngle); GameMsgObject(OSTR,cObj);
      cObj->AddCommand(C4CMD_Jump, NULL, Tx, Ty);
      return TRUE;
    }

  // No jump control
  return FALSE;
}

void C4Command::Transfer() {

  // No target: failure
  if (!Target) {
    Finish();
    return;
  }

  // Find transfer zone
  C4TransferZone *pZone;
  int iEntryX, iEntryY;
  if (!(pZone = Game.TransferZones.Find(Target))) {
    Finish();
    return;
  }

  // Not at or in transfer zone: move to entry point
  if (!Inside(cObj->x - pZone->X, -5, pZone->Wdt - 1 + 5)) {
    if (!pZone->GetEntryPoint(iEntryX, iEntryY, cObj->x, cObj->y)) {
      Finish();
      return;
    }
    cObj->AddCommand(C4CMD_MoveTo, NULL, iEntryX, iEntryY, 25);
    return;
  }

  // Call target transfer script
  if (!Tick5)
    if (!Target->Call(PSF_ControlTransfer, (intptr_t)cObj, Tx, Ty))
    // Transfer not handled by target: done
    {
      Finish(TRUE);
      return;
    }
}

void C4Command::Attack() {

  // No target: failure
  if (!Target) {
    Finish();
    return;
  }

  // Target is crew member
  if (Target->OCF & OCF_CrewMember) {

    C4Object *pProjectile = NULL;
    // Throw projectile at target
    for (C4ObjectLink *pLnk = cObj->Contents.First; pLnk && (pProjectile = pLnk->Obj); pLnk = pLnk->Next)
      if (pProjectile->Def->Projectile) {
        // Add throw command
        cObj->AddCommand(C4CMD_Throw, pProjectile, Target->x, Target->y, 2);
        return;
      }

    // Follow containment
    if (cObj->Contained != Target->Contained) {
      // Exit
      if (cObj->Contained)
        cObj->AddCommand(C4CMD_Exit, NULL, 0, 0, 10);
      // Enter
      else
        cObj->AddCommand(C4CMD_Enter, Target->Contained, 0, 0, 10);
      return;
    }

    // Move to target
    cObj->AddCommand(C4CMD_MoveTo, NULL, Target->x, Target->y, 10);
    return;

  }

  // For now, attack crew members only
  else {
    // Success, target might be no crew member due to that is has been killed
    Finish(TRUE);
    return;
  }
}

void C4Command::Buy() {
  // No target (base) object specified: find closest base
  int cnt;
  C4Object *pBase;
  if (!Target)
    for (cnt = 0; pBase = Game.FindBase(cObj->Owner, cnt); cnt++)
      if (!Target || Distance(cObj->x, cObj->y, pBase->x, pBase->y) < Distance(cObj->x, cObj->y, Target->x, Target->y))
        Target = pBase;
  // No target (base) object: fail
  if (!Target) {
    Finish();
    return;
  }
  // No type to buy specified: open buy menu for base
  if (!Data) {
    cObj->ActivateMenu(C4MN_Buy, 0, 0, 0, Target);
    Finish(TRUE);
    return;
  }
  // Target object is no base or hostile: fail
  if (!ValidPlr(Target->Base) || Hostile(Target->Base, cObj->Owner)) {
    Finish();
    return;
  }
  // Material not available for purchase at base: fail
  if (!Game.Players.Get(Target->Base)->HomeBaseMaterial.GetIDCount(Data)) {
    Finish();
    return;
  }
  // Target material undefined: fail
  C4Def *pDef = C4Id2Def(Data);
  if (!pDef) {
    Finish();
    return;
  }
  // Base owner has not enough funds: fail
  if (Game.Players.Get(Target->Base)->Wealth < pDef->Value) {
    Finish();
    return;
  }
  // Not within target object: enter
  if (cObj->Contained != Target) {
    cObj->AddCommand(C4CMD_Enter, Target, 0, 0, 50);
    return;
  }
  // Buy object(s)
  for (Tx = Max(Tx, 1); Tx; Tx--)
    if (!Buy2Base(cObj->Owner, Target, Data))
    // Failed (with ugly message)
    {
      Finish();
      return;
    }
  // Done: success
  Finish(TRUE);
}

void C4Command::Sell() {
  // No target (base) object specified: find closest base
  int cnt;
  C4Object *pBase;
  if (!Target)
    for (cnt = 0; pBase = Game.FindBase(cObj->Owner, cnt); cnt++)
      if (!Target || Distance(cObj->x, cObj->y, pBase->x, pBase->y) < Distance(cObj->x, cObj->y, Target->x, Target->y))
        Target = pBase;
  // No target (base) object: fail
  if (!Target) {
    Finish();
    return;
  }
  // No type to sell specified: open sell menu for base
  if (!Data) {
    cObj->ActivateMenu(C4MN_Sell, 0, 0, 0, Target);
    Finish(TRUE);
    return;
  }
  // Target object is no base or hostile: fail
  if (!ValidPlr(Target->Base) || Hostile(Target->Base, cObj->Owner)) {
    Finish();
    return;
  }
  // Not within target object: enter
  if (cObj->Contained != Target) {
    cObj->AddCommand(C4CMD_Enter, Target, 0, 0, 50);
    return;
  }
  // Sell object(s)
  for (Tx = Max(Tx, 1); Tx; Tx--)
    if (!SellFromBase(cObj->Owner, Target, Data))
    // Failed
    {
      Finish();
      return;
    }
  // Done
  Finish(TRUE);
}

void C4Command::Acquire() {

  // No type to acquire specified: fail
  if (!Data) {
    Finish();
    return;
  }

  // Target material in inventory: done
  if (cObj->Contents.Find(Data)) {
    Finish(TRUE);
    return;
  }

  // Find available material
  C4Object *pMaterial = NULL;
  // Next closest
  while (pMaterial = Game.FindObject(Data, cObj->x, cObj->y, -1, -1, OCF_Available, NULL, NULL, NULL, NULL, ANY_OWNER, pMaterial))
    // Object is not in container to be ignored
    if (!Target2 || pMaterial->Contained != Target2)
      // Object is near enough
      if (Inside(cObj->x - pMaterial->x, -500, +500))
        if (Inside(cObj->y - pMaterial->y, -250, +250))
          // Object is not connected to a pipe (for line construction kits)
          if (!Game.FindObject(C4Id("SPIP"), 0, 0, 0, 0, 0, "Connect", pMaterial))
            if (!Game.FindObject(C4Id("DPIP"), 0, 0, 0, 0, 0, "Connect", pMaterial))
              // Must be complete
              if (pMaterial->OCF & OCF_FullCon)
                // Doesn't burn
                if (!pMaterial->OnFire)
                  // We found one
                  break;

  // Available material found: get material
  if (pMaterial) {
    cObj->AddCommand(C4CMD_Get, pMaterial, 0, 0, 40);
    return;
  }

  // No available material found: buy material
  cObj->AddCommand(C4CMD_Buy, NULL, 0, 0, 100, NULL, TRUE, Data);
}

void C4Command::Fail() {
  // Check for base command (next unfinished)
  C4Command *pBase;
  for (pBase = Next; pBase; pBase = pBase->Next)
    if (!pBase->Finished)
      break;

  // Fail notice if was base command
  if (!pBase)
    if (cObj->OCF & OCF_CrewMember) {
      // Fail message
      OSTR[0] = 0;
      switch (Command) {
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
      case C4CMD_Build:
        // Needed components
        if (!Target)
          break;
        C4Def *pComponent;
        int cnt;
        sprintf(OSTR, LoadResStr(IDS_OBJ_NEEDS), Target->Def->Name);
        for (cnt = 0; Target->Def->Component.GetID(cnt); cnt++)
          if (Target->Def->Component.GetCount(cnt) != 0)
            if (Target->Component.GetCount(cnt) < Target->Def->Component.GetCount(cnt)) {
              SAppend("|", OSTR);
              sprintf(OSTR + SLen(OSTR), "%dx ", Target->Def->Component.GetCount(cnt) - Target->Component.GetCount(cnt));
              if ((pComponent = C4Id2Def(Target->Def->Component.GetID(cnt))))
                SAppend(pComponent->Name, OSTR);
            }
        break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
      case C4CMD_Call:
        // Call fail-function in target object (no message if non-zero)
        if (!CallFailed()) {
          // Fail-function not available or returned zero: standard message
          char szCommandName[24 + 1];
          SCopy(LoadResStr(CommandNameID(Command)), szCommandName);
          sprintf(OSTR, LoadResStr(IDS_CON_FAILURE), szCommandName);
        }
        break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
      case C4CMD_Exit:
        // No message
        break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
      case C4CMD_Dig:
        // No message
        break;
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // - - - - - - - - - - - - - -
      default:
        // Standard no-can-do message
        char szCommandName[24 + 1];
        SCopy(LoadResStr(CommandNameID(Command)), szCommandName);
        sprintf(OSTR, LoadResStr(IDS_CON_FAILURE), szCommandName);
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - -
      }
      // Message (if not empty)
      if (OSTR[0])
        GameMsgObject(OSTR, cObj, FWhite);
      // Fail sound
      SoundEffect("CommandFailure*", 0, 100, cObj);
    }

  // Count failures in base command
  if (pBase)
    pBase->Failures++;
}

C4Object *CreateLine(C4ID idType, int iOwner, C4Object *pFrom, C4Object *pTo);

void C4Command::Energy() {
  DWORD ocf = OCF_All;
  // No target: fail
  if (!Target) {
    Finish();
    return;
  }
  // Target can't be supplied: fail
  if (!(Target->Def->LineConnect & C4D_Power_Input)) {
    Finish();
    return;
  }
  // Target supplied
  if (!(Game.Rules & C4RULE_StructuresNeedEnergy) || (Game.FindObject(C4ID_PowerLine, 0, 0, 0, 0, OCF_All, "Connect", Target) && !Target->NeedEnergy)) {
    Finish(TRUE);
    return;
  }
  // No energy supply specified: find one
  if (!Target2)
    Target2 = Game.FindObject(0, Target->x, Target->y, -1, -1, OCF_PowerSupply, NULL, NULL, Target);
  // No energy supply: fail
  if (!Target2) {
    Finish();
    return;
  }
  // Energy supply too far away: fail
  if (Distance(cObj->x, cObj->y, Target2->x, Target2->y) > 650) {
    Finish();
    return;
  }
  // Not a valid energy supply: fail
  if (!(Target2->Def->LineConnect & C4D_Power_Output)) {
    Finish();
    return;
  }
  // No linekit: get one
  C4Object *pKit, *pLine, *pKitWithLine;
  if (!(pKit = cObj->Contents.Find(C4ID_Linekit))) {
    cObj->AddCommand(C4CMD_Acquire, NULL, 0, 0, 50, NULL, TRUE, C4ID_Linekit);
    return;
  }
  // Find line constructing kit
  for (int cnt = 0; pKitWithLine = cObj->Contents.GetObject(cnt); cnt++)
    if ((pKitWithLine->id == C4ID_Linekit) && (pLine = Game.FindObject(C4ID_PowerLine, 0, 0, 0, 0, OCF_All, "Connect", pKitWithLine)))
      break;
  // No line constructed yet
  if (!pLine) {
    // Move to supply
    if (!Target2->At(cObj->x, cObj->y, ocf)) {
      cObj->AddCommand(C4CMD_MoveTo, Target2, 0, 0, 50);
      return;
    }
    // At power supply: connect
    pLine = CreateLine(C4ID_PowerLine, cObj->Owner, Target2, pKit);
    if (!pLine) {
      Finish();
      return;
    }
    SoundEffect("Connect", 0, 100, cObj);
    return;
  }
  // Move to target
  if (!Target->At(cObj->x, cObj->y, ocf)) {
    cObj->AddCommand(C4CMD_MoveTo, Target, 0, 0, 50);
    return;
  }
  // Connect
  pLine->SetActionByName("Connect", Target2, Target);
  pKitWithLine->AssignRemoval();
  SoundEffect("Connect", 0, 100, cObj);
  // Done
  cObj->Action.ComDir = COMD_Stop;
  // Success
  Finish(TRUE);
}

void C4Command::Retry() {}

void C4Command::Home() {
  // At home base: done
  if (cObj->Contained && (cObj->Contained->Base == cObj->Owner)) {
    Finish(TRUE);
    return;
  }
  // No target (base) object specified: find closest base
  int cnt;
  C4Object *pBase;
  if (!Target)
    for (cnt = 0; pBase = Game.FindBase(cObj->Owner, cnt); cnt++)
      if (!Target || Distance(cObj->x, cObj->y, pBase->x, pBase->y) < Distance(cObj->x, cObj->y, Target->x, Target->y))
        Target = pBase;
  // No base: fail
  if (!Target) {
    Finish();
    return;
  }
  // Enter base
  cObj->AddCommand(C4CMD_Enter, Target);
}

void C4Command::Set(int iCommand, C4Object *pObj, C4Object *pTarget, int iTx, int iTy, C4Object *pTarget2, int iData, int iUpdateInterval, BOOL fEvaluated, int iRetries, const char *szText) {
  // Reset
  Clear();
  Default();
  // Set
  Command = iCommand;
  cObj = pObj;
  Target = pTarget;
  Tx = iTx;
  Ty = iTy;
  Target2 = pTarget2;
  Data = iData;
  UpdateInterval = iUpdateInterval;
  Evaluated = fEvaluated;
  Retries = iRetries;
  if (szText && szText[0]) {
    Text = new char[SLen(szText) + 1];
    SCopy(szText, Text);
  }
}

void C4Command::Call() {
  // No function name: fail
  if (!Text || !Text[0]) {
    Finish();
    return;
  }
  // No target object: fail
  if (!Target) {
    Finish();
    return;
  }
  // Done: success
  Finish(TRUE);
  // Object call
  Target->Call(Text, (intptr_t)cObj, Tx, Ty, (intptr_t)Target2);
  // Extreme caution notice: the script call might do just about anything
  // including clearing all commands (including this) i.e. through a call
  // to SetCommand. Thus, we must not do anything in this command anymore
  // after the script call (even the Finish has to be called before).
  // The Finish call being misled to the freshly created Build command (by
  // chance, the this-pointer was simply crap at the time) was reason for
  // the latest sync losses in 4.62.
}

BOOL C4Command::Write(char *sTarget, int iIndex) {
  sprintf(sTarget, "Command%d=%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s", iIndex, CommandName(Command), Tx, Ty, Target, Target2, Data, UpdateInterval, Evaluated, PathChecked, Finished, Failures,
          Retries, Permit, Text ? Text : "0");
  // Success
  return TRUE;
}

BOOL C4Command::Read(const char *szSource) {
  const char *cPos = SAdvancePast(szSource, '=');
  SCopyUntil(cPos, OSTR, ',');
  Command = CommandByName(OSTR);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Tx);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Ty);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Target);
  cPos = SAdvancePast(cPos, ','); // (enumerated pointer)
  sscanf(cPos, "%i", &Target2);
  cPos = SAdvancePast(cPos, ','); // (enumerated pointer)
  sscanf(cPos, "%i", &Data);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &UpdateInterval);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Evaluated);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &PathChecked);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Finished);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Failures);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Retries);
  cPos = SAdvancePast(cPos, ',');
  sscanf(cPos, "%i", &Permit);
  cPos = SAdvancePast(cPos, ',');
  SCopyUntil(cPos, OSTR, 0x0D);
  if (!SEqual(OSTR, "0")) {
    Text = new char[SLen(OSTR) + 1];
    SCopy(OSTR, Text);
  }
  cPos = SAdvancePast(cPos, ',');
  // Success
  return TRUE;
}

void C4Command::DenumeratePointers() {
  Target = Game.Objects.ObjectPointer((intptr_t)Target);
  Target2 = Game.Objects.ObjectPointer((intptr_t)Target2);
}

void C4Command::EnumeratePointers() {
  Target = (C4Object *)(intptr_t)Game.Objects.ObjectNumber(Target);
  Target2 = (C4Object *)(intptr_t)Game.Objects.ObjectNumber(Target2);
}

int C4Command::CallFailed() {
  // No function name or no target object: cannot call fail-function
  if (!Text || !Text[0])
    return 0;
  // Compose fail-function name
  char szFunctionFailed[1024 + 1];
  sprintf(szFunctionFailed, "%sFailed", Text);
  // Call failed-function
  return (int)Target->Call(szFunctionFailed, (long)cObj, Tx, Ty, (long)Target2);
  // Extreme caution notice: the script call might do just about anything
  // including clearing all commands (including this) i.e. through a call
  // to SetCommand. Thus, we must not do anything in this command anymore
  // after the script call.
}
