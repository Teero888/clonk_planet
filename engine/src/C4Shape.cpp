/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Basic classes for rectangles and vertex outlines */

#include <C4Include.h>

BOOL C4Shape::AddVertex(int iX, int iY) {
  if (VtxNum >= C4D_MaxVertex)
    return FALSE;
  VtxX[VtxNum] = iX;
  VtxY[VtxNum] = iY;
  VtxNum++;
  return TRUE;
}

void C4Shape::Default() {
  ZeroMem(this, sizeof(C4Shape));
  AttachMat = MNone;
}

C4Shape::C4Shape() { Default(); }

void C4Rect::Default() { x = y = Wdt = Hgt = 0; }

void C4TargetRect::Default() {
  C4Rect::Default();
  tx = ty = 0;
}

void C4TargetRect::Set(int iX, int iY, int iWdt, int iHgt, int iTX, int iTY) {
  C4Rect::Set(iX, iY, iWdt, iHgt);
  tx = iTX;
  ty = iTY;
}

void C4Rect::Set(int iX, int iY, int iWdt, int iHgt) {
  x = iX;
  y = iY;
  Wdt = iWdt;
  Hgt = iHgt;
}

BOOL C4Rect::Overlap(C4Rect &rTarget) {
  if (x + Wdt - 1 < rTarget.x)
    return FALSE;
  if (x > rTarget.x + rTarget.Wdt - 1)
    return FALSE;
  if (y + Hgt - 1 < rTarget.y)
    return FALSE;
  if (y > rTarget.y + rTarget.Hgt - 1)
    return FALSE;
  return TRUE;
}

void C4Shape::Clear() { ZeroMem(this, sizeof(C4Shape)); }

void C4Shape::Rotate(int iAngle) {

  const double pi = 3.1415926535;
  int cnt, nvtx, nvty, rdia;

  int *vtx = VtxX;
  int *vty = VtxY;
  double mtx[4], dang;
  dang = pi * (double)iAngle / 180.0;

  // Calculate rotation matrix
  mtx[0] = cos(dang);
  mtx[1] = -sin(dang);
  mtx[2] = sin(dang);
  mtx[3] = cos(dang);
  // Rotate vertices
  for (int cnt = 0; cnt < VtxNum; cnt++) {
    // nvtx= (int) ( mtx[0]*vtx[cnt] + mtx[1]*vty[cnt] );
    // nvty= (int) ( mtx[2]*vtx[cnt] + mtx[3]*vty[cnt] );
    nvtx = (int)(mtx[0] * VtxX[cnt] + mtx[1] * VtxY[cnt]);
    nvty = (int)(mtx[2] * VtxX[cnt] + mtx[3] * VtxY[cnt]);
    VtxX[cnt] = nvtx;
    VtxY[cnt] = nvty;
  }

  /* This is freaking nuts. I used the int* to shortcut the
           two int arrays Shape.Vtx_[]. Without modifications to
           this code, after rotation the x-values of vertex 2 and 4
           are screwed to that of vertex 0. Direct use of the array
           variables instead of the pointers helped. Later in
           development, again without modification to this code, the
           same error occured again. I moved back to pointer array
           shortcut and it worked again. ?!

           The error occurs after the C4DefCore structure has
           changed. It must have something to do with struct
           member alignment. But why does pointer usage vs. array
           index make a difference?
  */

  // Enlarge Rect
  rdia = (int)sqrt(x * x + y * y) + 2;
  x = -rdia;
  y = -rdia;
  Wdt = 2 * rdia;
  Hgt = 2 * rdia;
}

void C4Shape::Stretch(int iPercent) {
  int cnt;
  x = x * iPercent / 100;
  y = y * iPercent / 100;
  Wdt = Wdt * iPercent / 100;
  Hgt = Hgt * iPercent / 100;
  FireTop = FireTop * iPercent / 100;
  for (int cnt = 0; cnt < VtxNum; cnt++) {
    VtxX[cnt] = VtxX[cnt] * iPercent / 100;
    VtxY[cnt] = VtxY[cnt] * iPercent / 100;
  }
}

void C4Shape::Jolt(int iPercent) {
  int cnt;
  y = y * iPercent / 100;
  Hgt = Hgt * iPercent / 100;
  FireTop = FireTop * iPercent / 100;
  for (int cnt = 0; cnt < VtxNum; cnt++)
    VtxY[cnt] = VtxY[cnt] * iPercent / 100;
}

void C4Shape::GetVertexOutline(C4Rect &rRect) {
  int cnt;
  rRect.x = rRect.y = rRect.Wdt = rRect.Hgt = 0;
  for (int cnt = 0; cnt < VtxNum; cnt++) {
    // Extend left
    if (VtxX[cnt] < rRect.x) {
      rRect.Wdt += rRect.x - VtxX[cnt];
      rRect.x = VtxX[cnt];
    }
    // Extend right
    else if (VtxX[cnt] > rRect.x + rRect.Wdt) {
      rRect.Wdt = VtxX[cnt] - rRect.x;
    }

    // Extend up
    if (VtxY[cnt] < rRect.y) {
      rRect.Hgt += rRect.y - VtxY[cnt];
      rRect.y = VtxY[cnt];
    }
    // Extend down
    else if (VtxY[cnt] > rRect.y + rRect.Hgt) {
      rRect.Hgt = VtxY[cnt] - rRect.y;
    }
  }

  rRect.Hgt += rRect.y - y;
  rRect.y = y;
}

BOOL C4Shape::Attach(int &cx, int &cy, BYTE cnat_pos) {
  // Adjust given position to one pixel before contact
  // at vertices matching CNAT request.

  BOOL fAttached = FALSE;

#ifdef C4ENGINE

  int vtx, xcnt, ycnt, xcrng, ycrng, xcd, ycd;
  int motion_x = 0;
  BYTE cpix;

  for (vtx = 0; vtx < VtxNum; vtx++)
    if (VtxCNAT[vtx] & cnat_pos) {
      xcd = ycd = 0;
      switch (cnat_pos) {
      case CNAT_Top:
        ycd = -1;
        break;
      case CNAT_Bottom:
        ycd = +1;
        break;
      case CNAT_Left:
        xcd = -1;
        break;
      case CNAT_Right:
        xcd = +1;
        break;
      }
      xcrng = AttachRange * xcd * (-1);
      ycrng = AttachRange * ycd * (-1);
      for (xcnt = xcrng, ycnt = ycrng; (xcnt != -xcrng) || (ycnt != -ycrng);
           xcnt += xcd, ycnt += ycd)
        if (GBackSolid(cx + VtxX[vtx] + xcnt + xcd,
                       cy + VtxY[vtx] + ycnt + ycd)) {

          cpix = GBackPix(cx + VtxX[vtx] + xcnt + xcd,
                          cy + VtxY[vtx] + ycnt + ycd);
          AttachMat = PixCol2Mat(cpix);
          if (AttachMat == MVehic)
            motion_x = PixColMatIndex(cpix) - 1;

          cx += xcnt;
          cy += ycnt;
          fAttached = 1;
          break;
        }
    }

  cx += motion_x;

#endif

  return fAttached;
}

BOOL C4Shape::LineConnect(int tx, int ty, int cvtx, int ld) {
#ifdef C4ENGINE

  if (VtxNum < 2)
    return FALSE;

  // No modification
  if ((VtxX[cvtx] == tx) && (VtxY[cvtx] == ty))
    return TRUE;

  // Check new path
  int ix, iy;
  if (PathFree(tx, ty, VtxX[cvtx + ld], VtxY[cvtx + ld], &ix, &iy)) {
    // Okay, set vertex
    VtxX[cvtx] = tx;
    VtxY[cvtx] = ty;
    return TRUE;
  } else {
    // Intersected, find bend vertex
    int cix, ciy, irange = 8;
    int cnt;
    for (cnt = 0; cnt < 4; cnt++) {
      cix = ix - irange / 2 + irange * (cnt % 2);
      ciy = iy - irange / 2 + irange * (cnt / 2);
      if (PathFree(cix, ciy, tx, ty) &&
          PathFree(cix, ciy, VtxX[cvtx + ld], VtxY[cvtx + ld]))
        break;
    }
    if (cnt >= 4)
      return FALSE; // Found no bend vertex
    // Insert bend vertex
    if (ld > 0) {
      if (!InsertVertex(cvtx + 1, cix, ciy))
        return FALSE;
    } else {
      if (!InsertVertex(cvtx, cix, ciy))
        return FALSE;
      cvtx++;
    }
    // Okay, set vertex
    VtxX[cvtx] = tx;
    VtxY[cvtx] = ty;
    return TRUE;
  }

#endif

  return FALSE;
}

BOOL C4Shape::InsertVertex(int iPos, int tx, int ty) {
  if (VtxNum + 1 > C4D_MaxVertex)
    return FALSE;
  // Insert vertex before iPos
  for (int cnt = VtxNum; cnt > iPos; cnt--) {
    VtxX[cnt] = VtxX[cnt - 1];
    VtxY[cnt] = VtxY[cnt - 1];
  }
  VtxX[iPos] = tx;
  VtxY[iPos] = ty;
  VtxNum++;
  return TRUE;
}

BOOL C4Shape::RemoveVertex(int iPos) {
  if (!Inside(iPos, 0, VtxNum - 1))
    return FALSE;
  for (int cnt = iPos; cnt + 1 < VtxNum; cnt++) {
    VtxX[cnt] = VtxX[cnt + 1];
    VtxY[cnt] = VtxY[cnt + 1];
  }
  VtxNum--;
  return TRUE;
}

BOOL C4Shape::CheckContact(int cx, int cy) {
  // Check all vertices at given object position.
  // Return TRUE on any contact.

#ifdef C4ENGINE

  for (int cvtx = 0; cvtx < VtxNum; cvtx++)
    if (GBackSolid(cx + VtxX[cvtx], cy + VtxY[cvtx]))
      return TRUE;

#endif

  return FALSE;
}

BOOL C4Shape::ContactCheck(int cx, int cy) {
  // Check all vertices at given object position.
  // Set ContactCNAT and ContactCount.
  // Set VtxContactCNAT and VtxContactMat.
  // Return TRUE on any contact.

#ifdef C4ENGINE

  ContactCNAT = CNAT_None;
  ContactCount = 0;

  for (int cvtx = 0; cvtx < VtxNum; cvtx++) {

    VtxContactCNAT[cvtx] = CNAT_None;
    VtxContactMat[cvtx] = GBackMat(cx + VtxX[cvtx], cy + VtxY[cvtx]);

    if (GBackSolid(cx + VtxX[cvtx], cy + VtxY[cvtx])) {
      ContactCNAT |= VtxCNAT[cvtx];
      VtxContactCNAT[cvtx] |= CNAT_Center;
      ContactCount++;
      // Vertex center contact, now check top,bottom,left,right
      if (GBackSolid(cx + VtxX[cvtx], cy + VtxY[cvtx] - 1))
        VtxContactCNAT[cvtx] |= CNAT_Top;
      if (GBackSolid(cx + VtxX[cvtx], cy + VtxY[cvtx] + 1))
        VtxContactCNAT[cvtx] |= CNAT_Bottom;
      if (GBackSolid(cx + VtxX[cvtx] - 1, cy + VtxY[cvtx]))
        VtxContactCNAT[cvtx] |= CNAT_Left;
      if (GBackSolid(cx + VtxX[cvtx] + 1, cy + VtxY[cvtx]))
        VtxContactCNAT[cvtx] |= CNAT_Right;
    }
  }

#endif

  return ContactCount;
}

int C4Shape::GetVertexX(int iVertex) {
  if (!Inside(iVertex, 0, VtxNum - 1))
    return 0;
  return VtxX[iVertex];
}

int C4Shape::GetVertexY(int iVertex) {
  if (!Inside(iVertex, 0, VtxNum - 1))
    return 0;
  return VtxY[iVertex];
}
