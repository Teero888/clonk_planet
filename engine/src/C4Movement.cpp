/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Object motion, collision, friction */

#include <C4Include.h>

/* Some physical constants */

FIXED FRedirect = ftofix(0.5);
FIXED FFriction = ftofix(0.3);
FIXED FixFullCircle = ftofix(360.0), FixHalfCircle = FixFullCircle / 2;
FIXED FloatFriction = ftofix(0.02);
FIXED RotateAccel = ftofix(0.2);
FIXED FloatAccel = ftofix(0.1);
FIXED WalkAccel = ftofix(0.5), SwimAccel = ftofix(0.2);
FIXED HitSpeed1 = ftofix(1.5); // Hit Event
FIXED HitSpeed2 = ftofix(2.0); // Cross Check Hit
FIXED HitSpeed3 = ftofix(6.0); // Scale disable, kneel
FIXED HitSpeed4 = ftofix(8.0); // Flat

/* Some helper functions */

void RedirectForce(FIXED &from, FIXED &to, int tdir) {
  FIXED fred;
  fred = Min(Abs(from), FRedirect);
  from -= fred * Sign(from);
  to += fred * tdir;
}

void ApplyFriction(FIXED &tval, int percent) {
  FIXED ffric = FFriction * percent / 100;
  if (tval > +ffric) {
    tval -= ffric;
    return;
  }
  if (tval < -ffric) {
    tval += ffric;
    return;
  }
  tval = 0;
}

// Compares all Shape.VtxContactCNAT[] CNAT flags to search flag.
// Returns true if CNAT match has been found.

BOOL ContactVtxCNAT(C4Object *cobj, BYTE cnat_dir) {
  C4Def *dfc = cobj->Def;
  int cnt;
  BOOL fcontact = FALSE;
  for (cnt = 0; cnt < cobj->Shape.VtxNum; cnt++)
    if (cobj->Shape.VtxContactCNAT[cnt] & cnat_dir)
      fcontact = TRUE;
  return fcontact;
}

// Finds first vertex with contact flag set.
// Returns -1/0/+1 for relation on vertex to object center.

int ContactVtxWeight(C4Object *cobj) {
  int cnt;
  for (cnt = 0; cnt < cobj->Shape.VtxNum; cnt++)
    if (cobj->Shape.VtxContactCNAT[cnt]) {
      if (cobj->Shape.VtxX[cnt] < 0)
        return -1;
      if (cobj->Shape.VtxX[cnt] > 0)
        return +1;
    }
  return 0;
}

// ContactVtxFriction: Returns 0-100 friction value of first
//                     contacted vertex;

int ContactVtxFriction(C4Object *cobj) {
  int cnt;
  for (cnt = 0; cnt < cobj->Shape.VtxNum; cnt++)
    if (cobj->Shape.VtxContactCNAT[cnt])
      return cobj->Shape.VtxFriction[cnt];
  return 0;
}

void NoAttachAction(C4Object *clk);

const char *CNATName(int cnat) {
  switch (cnat) {
  case CNAT_None:
    return "None";
  case CNAT_Left:
    return "Left";
  case CNAT_Right:
    return "Right";
  case CNAT_Top:
    return "Top";
  case CNAT_Bottom:
    return "Bottom";
  case CNAT_Center:
    return "Center";
  }
  return "Undefined";
}

BOOL C4Object::Contact(int iCNAT) {
  if (Def->ContactFunctionCalls) {
    sprintf(OSTR, PSF_Contact, CNATName(iCNAT));
    return Call(OSTR);
  }
  return FALSE;
}

void C4Object::DoMotion(int mx, int my) {
  RemoveSolidMask();
  x += mx;
  y += my;
  motion_x += mx;
  motion_y += my;
}

void C4Object::TargetBounds(int &ctco, int limit_low, int limit_hi, int cnat_low, int cnat_hi) {
  switch (ForceLimits(ctco, limit_low, limit_hi)) {
  case -1:
    Contact(cnat_low);
    break;
  case +1:
    Contact(cnat_hi);
    break;
  }
}

int C4Object::ContactCheck(int iAtX, int iAtY) {
  // Check shape contact at given position
  Shape.ContactCheck(iAtX, iAtY);

  // Store shape contact values in object t_contact
  t_contact = Shape.ContactCNAT;

  // Contact script call for the first contacted cnat
  if (Shape.ContactCNAT)
    for (int ccnat = 0; ccnat < 4; ccnat++) // Left, right, top bottom
      if (Shape.ContactCNAT & (1 << ccnat))
        if (Contact(1 << ccnat))
          break; // Will stop on first positive return contact call!

  // Return shape contact count
  return Shape.ContactCount;
}

void C4Object::DoMovement() {
  int ctcox, ctcoy, ctcor, ctx, cty, iContact = 0;
  BOOL fAnyContact = FALSE;
  BYTE fTurned = 0, fRedirectYR = 0, fNoAttach = 0;

  // Reset motion for this frame
  motion_x = motion_y = 0;

  // Restrictions
  if (Def->NoHorizontalMove)
    xdir = 0;

  // Dig free target area
  if (Action.Act > ActIdle)
    if (Def->ActMap[Action.Act].DigFree) {
      // Shape size square
      if (Def->ActMap[Action.Act].DigFree == 1) {
        ctcox = fixtoi(fix_x + xdir);
        ctcoy = fixtoi(fix_y + ydir);
        Game.Landscape.DigFreeRect(ctcox + Shape.x, ctcoy + Shape.y, Shape.Wdt, Shape.Hgt, Action.Data, this);
      }
      // Free size round (variable size)
      else {
        ctcox = fixtoi(fix_x + xdir);
        ctcoy = fixtoi(fix_y + ydir);
        int rad = Def->ActMap[Action.Act].DigFree;
        if (Con < FullCon)
          rad = rad * 6 * Con / 5 / FullCon;
        Game.Landscape.DigFree(ctcox, ctcoy - 1, rad, Action.Data, this);
      }
    }

  if (!Action.t_attach) // Unattached movement  = = = = = = = = = = = = = = = =
                        // = = = = = = = = = = = = = = = =

  {

    // Horizontal movement - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -

    // Movement target
    fix_x += xdir;
    ctcox = fixtoi(fix_x);

    // Movement bounds (horiz)
    if (Def->BorderBound & C4D_Border_Sides)
      TargetBounds(ctcox, 0 - Shape.x, GBackWdt + Shape.x, CNAT_Left, CNAT_Right);

    // Move to target
    while (x != ctcox) {
      // Next step
      ctx = x + Sign(ctcox - x);

      if (iContact = ContactCheck(ctx, y)) {
        fAnyContact = TRUE;
        // Abort horizontal movement
        ctcox = x;
        fix_x = itofix(x);
        // Vertical redirection (always)
        RedirectForce(xdir, ydir, -1);
        ApplyFriction(ydir, ContactVtxFriction(this));
      } else // Free horizontal movement
        DoMotion(ctx - x, 0);
    }

    // Vertical movement  - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -

    // Movement target
    fix_y += ydir;
    ctcoy = fixtoi(fix_y);

    // Movement bounds (verti)
    if (Def->BorderBound & C4D_Border_Top)
      TargetBounds(ctcoy, 0 - Shape.y, +1000000, CNAT_Top, CNAT_Bottom);
    if (Def->BorderBound & C4D_Border_Bottom)
      TargetBounds(ctcoy, -1000000, GBackHgt + Shape.y, CNAT_Top, CNAT_Bottom);

    // Move to target
    while (y != ctcoy) {
      // Next step
      cty = y + Sign(ctcoy - y);
      if (iContact = ContactCheck(x, cty)) {
        fAnyContact = TRUE;
        ctcoy = y;
        fix_y = itofix(y);
        // Vertical contact horizontal friction
        ApplyFriction(xdir, ContactVtxFriction(this));
        // Redirection slide or rotate
        if (!ContactVtxCNAT(this, CNAT_Left))
          RedirectForce(ydir, xdir, -1);
        else if (!ContactVtxCNAT(this, CNAT_Right))
          RedirectForce(ydir, xdir, +1);
        else {
          if (OCF & OCF_Rotate)
            if (iContact == 1) {
              RedirectForce(ydir, rdir, -ContactVtxWeight(this));
              fRedirectYR = 1;
            }
          ydir = 0;
        }
      } else // Free vertical movement
        DoMotion(0, cty - y);
    }

    // Rotation  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - -
    if (OCF & OCF_Rotate) {
      if (rdir) {
        // Set target
        fix_r += rdir * 5;
        // Rotation limit
        if (Def->Rotateable > 1) {
          if (fix_r > itofix(Def->Rotateable)) {
            fix_r = itofix(Def->Rotateable);
            rdir = 0;
          }
          if (fix_r < itofix(-Def->Rotateable)) {
            fix_r = itofix(-Def->Rotateable);
            rdir = 0;
          }
        }
        ctcor = fixtoi(fix_r);
        // Move to target
        while (r != ctcor) {
          // Save step undos
          int lcobjr = r;
          C4Shape lshape = Shape;
          // Try next step
          r += Sign(ctcor - r);
          UpdateShape();
          if (iContact = ContactCheck(x, y)) // Contact
          {
            fAnyContact = TRUE;
            // Undo step and abort movement
            Shape = lshape;
            r = lcobjr;
            ctcor = r;
            fix_r = itofix(r);
            // Redirect to y
            if (iContact == 1)
              if (!fRedirectYR)
                RedirectForce(rdir, ydir, -1);
            // Stop rotation
            rdir = 0;
          } else
            fTurned = 1;
        }
        // Circle bounds
        if (fix_r < -FixHalfCircle) {
          fix_r += FixFullCircle;
          r = fixtoi(fix_r);
        }
        if (fix_r > +FixHalfCircle) {
          fix_r -= FixFullCircle;
          r = fixtoi(fix_r);
        }
      }
    }
  }

  if (Action.t_attach) // Attached movement = = = = = = = = = = = = = = = = = =
                       // = = = = = = = = = = = = = = =
  {

    BYTE at_xovr, at_yovr;

    // Set target position by momentum
    fix_x += xdir;
    fix_y += ydir;

    // Movement target
    ctcox = fixtoi(fix_x);
    ctcoy = fixtoi(fix_y);

    // Movement bounds (horiz + verti)
    if (Def->BorderBound & C4D_Border_Sides)
      TargetBounds(ctcox, 0 - Shape.x, GBackWdt + Shape.x, CNAT_Left, CNAT_Right);
    if (Def->BorderBound & C4D_Border_Top)
      TargetBounds(ctcoy, 0 - Shape.y, +1000000, CNAT_Top, CNAT_Bottom);
    if (Def->BorderBound & C4D_Border_Bottom)
      TargetBounds(ctcoy, -1000000, GBackHgt + Shape.y, CNAT_Top, CNAT_Bottom);

    // Move to target
    do {
      at_xovr = at_yovr = 0; // ctco Attachment override flags

      // Set next step target
      ctx = x + Sign(ctcox - x);
      cty = y + Sign(ctcoy - y);

      // Attachment check
      if (!Shape.Attach(ctx, cty, Action.t_attach))
        fNoAttach = 1;
      else {
        // Attachment change to ctx/cty overrides ctco target
        if (cty != y + Sign(ctcoy - y))
          at_yovr = 1;
        if (ctx != x + Sign(ctcox - x))
          at_xovr = 1;
      }

      // Contact check & evaluation
      if (iContact = ContactCheck(ctx, cty)) {
        fAnyContact = TRUE;
        // Abort movement
        ctcox = x;
        fix_x = itofix(x);
        ctcoy = y;
        fix_y = itofix(y);
      } else // Continue free movement
        DoMotion(ctx - x, cty - y);

      // ctco Attachment overrides
      if (at_xovr) {
        ctcox = x;
        xdir = 0;
        fix_x = itofix(x);
      }
      if (at_yovr) {
        ctcoy = y;
        ydir = 0;
        fix_y = itofix(y);
      }
    } while ((x != ctcox) || (y != ctcoy));
  }

  // Reput solid mask: Might have been removed by motion or
  // motion might be out of date from last frame.
  PutSolidMask();

  // Misc checks
  // ===========================================================================================

  // InLiquid check
  if (GBackLiquid(x, y + Def->Float * Con / FullCon - 1)) // In Liquid
  {
    if (!InLiquid) // Enter liquid
    {
      if (OCF & OCF_HitSpeed2)
        if (Mass > 3)
          Splash(x, y + 1, Min(Shape.Wdt * Shape.Hgt / 10, 20), this);
      fNoAttach = FALSE;
      InLiquid = 1;
    }
  } else // Out of liquid
  {
    if (InLiquid) // Leave liquid
      InLiquid = 0;
  }

  // Contact Action
  if (fAnyContact)
    ContactAction();

  // Attachment Loss Action
  if (fNoAttach)
    NoAttachAction(this);

  // Movement Script Execution
  if (fAnyContact) {
    if (OCF & OCF_HitSpeed1)
      Call(PSF_Hit);
    if (OCF & OCF_HitSpeed2)
      Call(PSF_Hit2);
    if (OCF & OCF_HitSpeed3)
      Call(PSF_Hit3);
  }

  // Rotation gfx
  if (fTurned)
    UpdateFace();
}

void C4Object::Stabilize() {
  if (r != 0)
    if (Inside(r, -StableRange, +StableRange)) {
      // Save step undos
      int lcobjr = r;
      C4Shape lshape = Shape;
      // Try rotation
      r = 0;
      UpdateShape();
      if (ContactCheck(x, y)) { // Undo rotation
        Shape = lshape;
        r = lcobjr;
      } else { // Stabilization okay
        fix_r = itofix(r);
        UpdateFace();
      }
    }
}

void C4Object::CopyMotion(C4Object *from) {
  // Designed for contained objects, no static
  x = from->x;
  y = from->y;
  fix_x = itofix(x);
  fix_y = itofix(y);
  xdir = from->xdir;
  ydir = from->ydir;
}

void C4Object::ForcePosition(int tx, int ty) {
  if (!this)
    return;
  if ((x != tx) || (y != ty))
    RemoveSolidMask();
  x = tx;
  y = ty;
  fix_x = itofix(x);
  fix_y = itofix(y);
  UpdateSolidMask();
  // Crew member: update fog of war
  /*C4Player *pPlr;
  if (pPlr=Game.Players.Get(Owner))
          if (pPlr->ObjectInCrew(this))
                  pPlr->RedrawFogOfWar=TRUE;*/
}

BOOL C4Object::ExecMovement() // Every Tick1 by Execute
{

  // Containment check
  if (Contained) {
    CopyMotion(Contained);
    return TRUE;
  }

  // General mobility check
  if (Category & C4D_StaticBack)
    return FALSE;

  // Movement execution
  if (Mobile) // Object is moving
  {
    // Move object
    DoMovement();
    // Demobilization check
    if ((xdir == 0) && (ydir == 0) && (rdir == 0))
      Mobile = 0;
    // Check for stabilization
    if (rdir == 0)
      Stabilize();
  } else // Object is static
  {
    // Check for stabilization
    Stabilize();
    // Check for mobilization
    if (!Tick10) {
      // Gravity mobilization
      xdir = ydir = rdir = 0;
      fix_x = itofix(x);
      fix_y = itofix(y);
      fix_r = itofix(r);
      Mobile = 1;
    }
  }

  // Enforce zero rotation
  if (!Def->Rotateable)
    r = 0;

  // Out of bounds check
  if (!Inside(x, 0, GBackWdt) || (y > GBackHgt)) {
    AssignDeath();
    AssignRemoval();
  }

  return TRUE;
}

BYTE SimFlightHitsLiquid(FIXED fcx, FIXED fcy, FIXED xdir, FIXED ydir) {
  // Returns pixel first contacted
  int ctcox, ctcoy, cx, cy, tdens;
  BYTE abort_sim;
  cx = fixtoi(fcx);
  cy = fixtoi(fcy);
  abort_sim = 0;
  do {
    // Set target position by momentum
    fcx += xdir;
    fcy += ydir;
    // Movement to target
    ctcox = fixtoi(fcx);
    ctcoy = fixtoi(fcy);
    // Bounds
    if (!Inside(ctcox, 0, GBackWdt) || (ctcoy >= GBackHgt))
      return 0;
    // Move to target
    do {
      // Set next step target
      cx += Sign(ctcox - cx);
      cy += Sign(ctcoy - cy);
      // Contact check
      tdens = GBackDensity(cx, cy);
      if (DensitySemiSolid(tdens)) // Hit something
      {
        if (DensityLiquid(tdens))       // Hit Liquid
          if (GBackLiquid(cx, cy + 10)) // Is deep?
            return 1;
        return 0;
      }
    } while ((cx != ctcox) || (cy != ctcoy));
    // Adjust GravAccel once per frame
    ydir += GravAccel;
  } while (!abort_sim);
  return 0;
}
