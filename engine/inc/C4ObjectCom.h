/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Lots of handler functions for object action */

void DrawCommandKey(C4Facet &cgo, int iCom, BOOL fPressed = FALSE, const char *szText = NULL);

void DrawControlKey(C4Facet &cgo, int iControl, BOOL fPressed = FALSE, const char *szText = NULL);

int Control2Com(int iControl);
int Com2Control(int iCom);
int Com2MenuCom(int iCom, BOOL fMultiline);
const char *ComName(int iCom);
int ComOrder(int iCom);
int VkCode2FullScreenCom(WORD dwCode, BOOL fMultiline);
const char *PlrControlKeyName(int iPlayer, int iControl);

const int ComOrderNum = 24;

BOOL PlayerObjectCommand(int plr, int cmdf, C4Object *pTarget = NULL, int tx = 0, int ty = 0);

BOOL ObjectActionWalk(C4Object *cObj);
BOOL ObjectActionStand(C4Object *cObj);
BOOL ObjectActionJump(C4Object *cObj, FIXED xdir, FIXED ydir);
BOOL ObjectActionDive(C4Object *cObj, FIXED xdir, FIXED ydir);
BOOL ObjectActionTumble(C4Object *cObj, int dir, FIXED xdir, FIXED ydir);
BOOL ObjectActionGetPunched(C4Object *cObj, FIXED xdir, FIXED ydir);
BOOL ObjectActionKneel(C4Object *cObj);
BOOL ObjectActionFlat(C4Object *cObj, int dir);
BOOL ObjectActionScale(C4Object *cObj, int dir, int comdir);
BOOL ObjectActionHangle(C4Object *cObj, int dir, int comdir);
BOOL ObjectActionThrow(C4Object *cObj, C4Object *pThing = NULL);
BOOL ObjectActionDig(C4Object *cObj, int iComDir);
BOOL ObjectActionBuild(C4Object *cObj, C4Object *pTarget);
BOOL ObjectActionPush(C4Object *cObj, C4Object *pTarget);
BOOL ObjectActionChop(C4Object *cObj, C4Object *pTarget);
BOOL ObjectActionCornerScale(C4Object *cObj);
BOOL ObjectActionFight(C4Object *cObj, C4Object *pTarget);

BOOL ObjectComMovement(C4Object *cObj, int iComDir);
BOOL ObjectComStop(C4Object *cObj);
BOOL ObjectComGrab(C4Object *cObj, C4Object *pTarget);
BOOL ObjectComPut(C4Object *cObj, C4Object *pTarget, C4Object *pThing = NULL);
BOOL ObjectComThrow(C4Object *cObj, C4Object *pThing = NULL);
BOOL ObjectComDrop(C4Object *cObj, C4Object *pThing = NULL);
BOOL ObjectComUnGrab(C4Object *cObj);
BOOL ObjectComJump(C4Object *cObj);
BOOL ObjectComLetGo(C4Object *cObj, int xdirf);
BOOL ObjectComUp(C4Object *cObj);
BOOL ObjectComDig(C4Object *cObj);
BOOL ObjectComChop(C4Object *cObj, C4Object *pTarget);
BOOL ObjectComBuild(C4Object *cObj, C4Object *pTarget);
BOOL ObjectComEnter(C4Object *cObj);
BOOL ObjectComDownDouble(C4Object *cObj);
BOOL ObjectComPutTake(C4Object *cObj, C4Object *pTarget, C4Object *pThing = NULL);
BOOL ObjectComTake(C4Object *cObj, C4ID id);
BOOL ObjectComPunch(C4Object *cObj, C4Object *pTarget, int iPunch = 0);
BOOL ObjectComActivate(C4Object *cObj, C4ID id, C4Object *pThing = NULL);
BOOL ObjectComCancelAttach(C4Object *cObj);
void ObjectComDigDouble(C4Object *cObj);
void ObjectComStopDig(C4Object *cObj);

BOOL Buy2Base(int iPlr, C4Object *pBase, C4ID id);
BOOL SellFromBase(int iPlr, C4Object *pBase, C4ID id);
