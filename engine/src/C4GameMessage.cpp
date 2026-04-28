/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Text messages drawn inside the viewport */

#include <C4Include.h>

C4GameMessage::C4GameMessage() {}

C4GameMessage::~C4GameMessage() {}

void C4GameMessage::Init(const char *szText, C4Object *pTarget, int iPlayer, int iX, int iY, BYTE bCol) {
  // Set data
  SCopy(szText, Text, C4GM_MaxText);
  Target = pTarget;
  X = iX;
  Y = iY;
  Player = iPlayer;
  Color = bCol;
  Delay = SLen(szText) * 2;
  // Permanent message
  if (Text[0] == '@') {
    Delay = -1;
    SCopy(Text + 1, Text);
  }
  int dummy;
  Engine.DDraw.TextExtent(Text, dummy, Hgt);
}

BOOL C4GameMessage::Execute() {
  // Position by target
  if (Target) {
    X = Target->x;
    Y = Target->y - Target->Def->Shape.Hgt / 2 - 5;
  }
  // Delay / removal
  if (Delay > 0)
    Delay--;
  if (Delay == 0)
    return FALSE;
  // Done
  return TRUE;
}

void C4GameMessage::Draw(C4FacetEx &cgo, int iPlayer) {
  // Globals or player
  if ((Player == ANY_OWNER) || ((Player != NO_OWNER) && (Player == iPlayer)))
    Engine.DDraw.TextOut(Text, cgo.Surface, cgo.X + cgo.Wdt / 2, cgo.Y + 2 * cgo.Hgt / 3, Color, FBlack, ACenter);
  // Positioned
  else if (Inside(X - cgo.TargetX, 0, cgo.Wdt - 1))
    if (Inside(Y - cgo.TargetY, 0, cgo.Hgt - 1)) {
      // Word wrap to cgo width
      char szText[C4GM_MaxText + 1];
      SCopy(Text, szText);
      int iMaxLine = Max(cgo.Wdt / Engine.DDraw.TextWidth("m"), 20);
      SWordWrap(szText, ' ', '|', iMaxLine);
      // Adjust position by output boundaries
      int iTX, iTY, iTWdt, iTHgt;
      Engine.DDraw.TextExtent(szText, iTWdt, iTHgt);
      iTX = BoundBy(X - cgo.TargetX, iTWdt / 2, cgo.Wdt - iTWdt / 2);
      iTY = BoundBy(Y - cgo.TargetY - iTHgt, 0, cgo.Hgt - iTHgt);
      // Draw
      Engine.DDraw.TextOut(szText, cgo.Surface, cgo.X + iTX, cgo.Y + iTY, Color, FBlack, ACenter);
    }
}

C4GameMessageList::C4GameMessageList() { Default(); }

C4GameMessageList::~C4GameMessageList() { Clear(); }

void C4GameMessageList::Default() { First = NULL; }

void C4GameMessageList::ClearPointers(C4Object *pObj) {
  C4GameMessage *cmsg, *next, *prev = NULL;
  for (cmsg = First; cmsg; cmsg = next) {
    next = cmsg->Next;
    if (cmsg->Target == pObj) {
      delete cmsg;
      if (prev)
        prev->Next = next;
      else
        First = next;
    } else
      prev = cmsg;
  }
}

void C4GameMessageList::Clear() {
  C4GameMessage *cmsg, *next;
  for (cmsg = First; cmsg; cmsg = next) {
    next = cmsg->Next;
    delete cmsg;
  }
  First = NULL;
}

void C4GameMessageList::Execute() {
  C4GameMessage *cmsg, *next, *prev = NULL;
  for (cmsg = First; cmsg; cmsg = next) {
    next = cmsg->Next;
    if (!cmsg->Execute()) {
      delete cmsg;
      if (prev)
        prev->Next = next;
      else
        First = next;
    } else
      prev = cmsg;
  }
}

BOOL C4GameMessageList::New(const char *szText, C4Object *pTarget, int iPlayer, int iX, int iY, BYTE bCol) {

  // Clear messages with same target
  if (pTarget)
    ClearPointers(pTarget);

  // Clear other player messages
  if (iPlayer != NO_OWNER)
    ClearPlayers(iPlayer);

  // Add new message
  C4GameMessage *msgNew = new C4GameMessage;
  msgNew->Init(szText, pTarget, iPlayer, iX, iY, bCol);
  msgNew->Next = First;
  First = msgNew;

  return TRUE;
}

void C4GameMessageList::ClearPlayers(int iPlayer) {
  C4GameMessage *cmsg, *next, *prev = NULL;
  for (cmsg = First; cmsg; cmsg = next) {
    next = cmsg->Next;
    if (cmsg->Player == iPlayer) {
      delete cmsg;
      if (prev)
        prev->Next = next;
      else
        First = next;
    } else
      prev = cmsg;
  }
}

void C4GameMessageList::Draw(C4FacetEx &cgo, int iPlayer) {
  C4GameMessage *cmsg;
  for (cmsg = First; cmsg; cmsg = cmsg->Next)
    cmsg->Draw(cgo, iPlayer);
}
