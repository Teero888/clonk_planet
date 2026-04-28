/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* That which fills the world with life */

#include <C4Include.h>

#define offsC4O(x) offsetof(C4Object, x)

C4CompilerValue C4CR_Object[] = {

    {"Object", C4CV_Section, 0, 0},
    {"id", C4CV_Id, offsC4O(id), 1},
    {"Number", C4CV_Integer, offsC4O(Number), 1},
    {"Info", C4CV_String, offsC4O(nInfo), C4MaxName},
    {"Owner", C4CV_Integer, offsC4O(Owner), 1},
    {"Timer", C4CV_Integer, offsC4O(Timer), 1},
    {"Controller", C4CV_Integer, offsC4O(Controller), 1},
    {"Category", C4CV_Integer, offsC4O(Category), 1},
    {"X", C4CV_Integer, offsC4O(x), 1},
    {"Y", C4CV_Integer, offsC4O(y), 1},
    {"Rotation", C4CV_Integer, offsC4O(r), 1},
    {"MotionX", C4CV_Integer, offsC4O(motion_x), 1},
    {"MotionY", C4CV_Integer, offsC4O(motion_y), 1},
    {"NoCollectDelay", C4CV_Integer, offsC4O(NoCollectDelay), 1},
    {"MaskPutMotion", C4CV_Integer, offsC4O(MaskPutMotion), 1},
    {"Base", C4CV_Integer, offsC4O(Base), 1},
    {"Size", C4CV_Integer, offsC4O(Con), 1},
    {"Mass", C4CV_Integer, offsC4O(Mass), 1},
    {"Damage", C4CV_Integer, offsC4O(Damage), 1},
    {"Energy", C4CV_Integer, offsC4O(Energy), 1},
    {"MagicEnergy", C4CV_Integer, offsC4O(MagicEnergy), 1},
    {"Alive", C4CV_Integer, offsC4O(Alive), 1},
    {"Breath", C4CV_Integer, offsC4O(Breath), 1},
    {"FirePhase", C4CV_Integer, offsC4O(FirePhase), 1},
    {"Color", C4CV_Integer, offsC4O(Color), 1},
    {"Locals", C4CV_Integer, offsC4O(Local), C4MaxVariable},
    {"FixX", C4CV_Integer, offsC4O(fix_x), 1},
    {"FixY", C4CV_Integer, offsC4O(fix_y), 1},
    {"FixR", C4CV_Integer, offsC4O(fix_r), 1},
    {"XDir", C4CV_Integer, offsC4O(xdir), 1},
    {"YDir", C4CV_Integer, offsC4O(ydir), 1},
    {"RDir", C4CV_Integer, offsC4O(rdir), 1},
    {"Width", C4CV_Integer, offsC4O(Shape.Wdt), 1},
    {"Height", C4CV_Integer, offsC4O(Shape.Hgt), 1},
    {"Offset", C4CV_Integer, offsC4O(Shape.x), 2},
    {"Vertices", C4CV_Integer, offsC4O(Shape.VtxNum), 1},
    {"VertexX", C4CV_Integer, offsC4O(Shape.VtxX), C4D_MaxVertex},
    {"VertexY", C4CV_Integer, offsC4O(Shape.VtxY), C4D_MaxVertex},
    {"VertexCNAT", C4CV_Integer, offsC4O(Shape.VtxCNAT), C4D_MaxVertex},
    {"VertexFriction", C4CV_Integer, offsC4O(Shape.VtxFriction), C4D_MaxVertex},
    {"FireTop", C4CV_Integer, offsC4O(Shape.FireTop), 1},
    {"SolidMask", C4CV_Integer, offsC4O(SolidMask), 6},
    {"Mobile", C4CV_Integer, offsC4O(Mobile), 1},
    {"Selected", C4CV_Integer, offsC4O(Select), 1},
    {"OnFire", C4CV_Integer, offsC4O(OnFire), 1},
    {"InLiquid", C4CV_Integer, offsC4O(InLiquid), 1},
    {"EntranceStatus", C4CV_Integer, offsC4O(EntranceStatus), 1},
    {"PhysicalTemporary", C4CV_Integer, offsC4O(PhysicalTemporary), 1},
    {"NeedEnergy", C4CV_Integer, offsC4O(NeedEnergy), 1},
    {"OCF", C4CV_Integer, offsC4O(OCF), 1},
    {"Action", C4CV_String, offsC4O(Action.Name), C4D_MaxIDLen},
    {"Dir", C4CV_Integer, offsC4O(Action.Dir), 1},
    {"ComDir", C4CV_Integer, offsC4O(Action.ComDir), 1},
    {"ActionTime", C4CV_Integer, offsC4O(Action.Time), 1},
    {"ActionData", C4CV_Integer, offsC4O(Action.Data), 1},
    {"Phase", C4CV_Integer, offsC4O(Action.Phase), 1},
    {"PhaseDelay", C4CV_Integer, offsC4O(Action.PhaseDelay), 1},
    {"Contained", C4CV_Integer, offsC4O(nContained), 1},
    {"ActionTarget1", C4CV_Integer, offsC4O(nActionTarget1), 1},
    {"ActionTarget2", C4CV_Integer, offsC4O(nActionTarget2), 1},
    {"Component", C4CV_IdList, offsC4O(Component), 1},
    {"Contents", C4CV_ObjectList, offsC4O(Contents), 1},

    {"Physical", C4CV_Section, 0, 0},
    {"Energy", C4CV_Integer, offsC4O(TemporaryPhysical.Energy), 1},
    {"Breath", C4CV_Integer, offsC4O(TemporaryPhysical.Breath), 1},
    {"Walk", C4CV_Integer, offsC4O(TemporaryPhysical.Walk), 1},
    {"Jump", C4CV_Integer, offsC4O(TemporaryPhysical.Jump), 1},
    {"Scale", C4CV_Integer, offsC4O(TemporaryPhysical.Scale), 1},
    {"Hangle", C4CV_Integer, offsC4O(TemporaryPhysical.Hangle), 1},
    {"Dig", C4CV_Integer, offsC4O(TemporaryPhysical.Dig), 1},
    {"Swim", C4CV_Integer, offsC4O(TemporaryPhysical.Swim), 1},
    {"Throw", C4CV_Integer, offsC4O(TemporaryPhysical.Throw), 1},
    {"Push", C4CV_Integer, offsC4O(TemporaryPhysical.Push), 1},
    {"Fight", C4CV_Integer, offsC4O(TemporaryPhysical.Fight), 1},
    {"Magic", C4CV_Integer, offsC4O(TemporaryPhysical.Magic), 1},
    {"Float", C4CV_Integer, offsC4O(TemporaryPhysical.Float), 1},
    {"CanScale", C4CV_Integer, offsC4O(TemporaryPhysical.CanScale), 1},
    {"CanHangle", C4CV_Integer, offsC4O(TemporaryPhysical.CanHangle), 1},
    {"CanDig", C4CV_Integer, offsC4O(TemporaryPhysical.CanDig), 1},
    {"CanConstruct", C4CV_Integer, offsC4O(TemporaryPhysical.CanConstruct), 1},
    {"CanChop", C4CV_Integer, offsC4O(TemporaryPhysical.CanChop), 1},
    {"CanSwimDig", C4CV_Integer, offsC4O(TemporaryPhysical.CanSwimDig), 1},
    {"CorrosionResist", C4CV_Integer, offsC4O(TemporaryPhysical.CorrosionResist), 1},
    {"BreatheWater", C4CV_Integer, offsC4O(TemporaryPhysical.BreatheWater), 1},

    {NULL, C4CV_End, 0, 0}};

void DrawVertex(C4Facet &cgo, int tx, int ty, int col, int contact) {
  if (Inside(tx, 1, cgo.Wdt - 2) && Inside(ty, 1, cgo.Hgt - 2)) {
    Engine.DDraw.DrawHorizontalLine(cgo.Surface, cgo.X + tx - 1, cgo.X + tx + 1, cgo.Y + ty, col);
    Engine.DDraw.DrawVerticalLine(cgo.Surface, cgo.X + tx, cgo.Y + ty - 1, cgo.Y + ty + 1, col);
    if (contact)
      Engine.DDraw.DrawFrame(cgo.Surface, cgo.X + tx - 2, cgo.Y + ty - 2, cgo.X + tx + 2, cgo.Y + ty + 2, CWhite);
  }
}

C4Object::C4Object() { Default(); }

void C4Object::Default() {
  Magic = 0x434F424A;
  id = C4ID_None;
  Status = 1;
  RemovalDelay = 0;
  Owner = NO_OWNER;
  Controller = NO_OWNER;
  Category = 0;
  x = y = r = 0;
  motion_x = motion_y = 0;
  NoCollectDelay = 0;
  MaskPutMotion = 0;
  Base = NO_OWNER;
  Con = 0;
  Mass = 0;
  Damage = 0;
  Energy = 0;
  MagicEnergy = 0;
  Alive = 0;
  Breath = 0;
  FirePhase = 0;
  InMat = MNone;
  Color = 0;
  ViewEnergy = 0;
  for (int cnt = 0; cnt < C4MaxVariable; cnt++)
    Local[cnt] = 0;
  fix_x = fix_y = fix_r = 0;
  xdir = ydir = rdir = 0;
  Mobile = 0;
  Select = 0;
  MaskPut = 0;
  Unsorted = FALSE;
  OnFire = 0;
  InLiquid = 0;
  EntranceStatus = 0;
  Audible = LastAudible = 0;
  NeedEnergy = 0;
  Timer = 0;
  t_contact = 0;
  OCF = 0;
  Action.Default();
  Shape.Default();
  Contents.Default();
  Component.Default();
  SolidMask.Default();
  Def = NULL;
  Info = NULL;
  Command = NULL;
  Contained = NULL;
  BaseFace.Default();
  TopFace.Default();
  cBitmap = cSolidMask = NULL;
  nContained = nActionTarget1 = nActionTarget2 = 0;
  nInfo[0] = 0;
  Menu = NULL;
  PhysicalTemporary = FALSE;
  TemporaryPhysical.Default();
  MaterialContents = NULL;
}

BOOL C4Object::Init(C4Def *pDef, int iOwner, C4ObjectInfo *pInfo, int nx, int ny, int nr, FIXED nxdir, FIXED nydir, FIXED nrdir) {

  // Def & basics
  id = pDef->id;
  Owner = iOwner;
  Controller = iOwner;
  Info = pInfo;
  Def = pDef;
  Category = Def->Category;

  // Position
  if (!Def->Rotateable) {
    nr = 0;
    nrdir = 0;
  }
  x = nx;
  y = ny;
  r = nr;
  fix_x = itofix(x);
  fix_y = itofix(y);
  fix_r = itofix(r);
  xdir = nxdir;
  ydir = nydir;
  rdir = nrdir;

  // Initial mobility
  if (Category != C4D_StaticBack)
    if (xdir || ydir || rdir)
      Mobile = 1;

  // Mass
  Mass = Max(Def->Mass * Con / FullCon, 1);

  // Life, energy, breath
  if (Category & C4D_Living)
    Alive = 1;
  if (Alive)
    Energy = GetPhysical()->Energy;
  Breath = GetPhysical()->Breath;

  // Components
  Component = Def->Component;
  ComponentConCutoff();

  // Color
  if (Def->ColorByOwner)
    if (ValidPlr(Owner))
      Color = BoundBy(Game.Players.Get(Owner)->Color, 0, C4MaxColor - 1);

  // Shape & face
  Shape = Def->Shape;
  SolidMask = Def->SolidMask;
  UpdateFace();

  // Initial audibility
  Audible = LastAudible = Game.GraphicsSystem.GetAudibility(x, y);

  // Construction call
  Call(PSF_Construction);

  return TRUE;
}

C4Object::~C4Object() { Clear(); }

void C4Object::AssignRemoval() {
  // Destruction call
  Call(PSF_Destruction);
  // Extinguish
  Extinguish();
  // Action idle
  SetAction(ActIdle);
  // Object system operation
  Status = 0;
  // Kill contents
  C4Object *cobj;
  C4ObjectLink *clnk, *next;
  for (clnk = Contents.First; clnk && (cobj = clnk->Obj); clnk = next) {
    next = clnk->Next;
    Contents.Remove(cobj);
    cobj->AssignRemoval();
  }
  // Object info
  if (Info)
    Info->Retire();
  // Object system operation
  Game.ClearPointers(this);
  ClearCommands();
  RemoveSolidMask();
  DestroySolidMask();
  RemovalDelay = 2;
}

void C4Object::UpdateShape() {

  // Line shape independent
  if (Def->Line)
    return;

  // Copy shape from def
  Shape = Def->Shape;

  // Construction zoom
  if (Con != FullCon)
    if (Def->GrowthType)
      Shape.Stretch(Con * 100 / FullCon);
    else
      Shape.Jolt(Con * 100 / FullCon);

  // Rotation
  if (Def->Rotateable)
    if (r != 0)
      Shape.Rotate(r);
}

BYTE RecheckSurface(SURFACE *lpSfc, int wdt = -1, int hgt = -1) {
  // Set the surface to specified size.
  // Create or destroy surface if necessary.
  int swdt, shgt;
  // Exists, check size
  if (*lpSfc)
    if (!GetSurfaceSize(*lpSfc, swdt, shgt))
      return 0;
    else if ((wdt != swdt) || (hgt != shgt)) {
      DestroySurface(*lpSfc);
      *lpSfc = NULL;
    }
  // Create if does not exist
  if (!*lpSfc)
    if ((wdt > 0) && (hgt > 0))
      if (!(*lpSfc = CreateSurface(wdt, hgt)))
        return 0;
  // Success
  return 1;
}

void C4Object::RecheckSolidMask() {
  // If necessary, create cSolidMask from MainFace
  if (!cSolidMask) {
    if (!(cSolidMask = CreateSurface(SolidMask.Wdt, SolidMask.Hgt)))
      return;
    Engine.DDraw.WipeSurface(cSolidMask);
    Engine.DDraw.Blit(Def->Bitmap[Color], SolidMask.x, SolidMask.y, SolidMask.Wdt, SolidMask.Hgt, cSolidMask, 0, 0, SolidMask.Wdt, SolidMask.Hgt);
    // Solid mask target x/y is relative to def bitmap top-left, not object
    // center.
  }
}

void C4Object::DestroySolidMask() {
  // Destroy cSolidMask
  if (cSolidMask)
    DestroySurface(cSolidMask);
  cSolidMask = NULL;
}

void C4Object::PutSolidMask() {
  // If not put or motion has changed, put mask to background,
  // storing background pixels in cSolidMask.

  // Calculate motion
  int iMotion = BoundBy(motion_x, -1, +1);
  // No mask
  if (!cSolidMask)
    return;
  // Contained
  if (Contained)
    return;
  // Mask is put and up to date
  if (MaskPut && (MaskPutMotion == iMotion))
    return;
  // Lock mask surface
  int iPitch;
  BYTE *pSolidMask = LockSurface(cSolidMask, iPitch);
  if (!pSolidMask)
    return;
  // Put mask pixels
  int xcnt, ycnt, iTx, iTy;
  BYTE byPixel, iMaskColor;
  // Calculate mask color by motion
  iMaskColor = Mat2PixCol(MVehic) + 1 + iMotion;
  for (ycnt = 0; ycnt < SolidMask.Hgt; ycnt++)
    for (xcnt = 0; xcnt < SolidMask.Wdt; xcnt++)
      if (pSolidMask[ycnt * iPitch + xcnt]) {
        // Calucate pixel position
        iTx = x + Def->Shape.x + xcnt + SolidMask.tx;
        iTy = y + Def->Shape.y + ycnt + SolidMask.ty;
        // Get background pixel
        byPixel = GBackPix(iTx, iTy);
        // Store background if not put
        if (!MaskPut)
          pSolidMask[ycnt * iPitch + xcnt] = byPixel;
        // Set background pixel only if not put or background is still vehic
        if (!MaskPut || (GBackMat(iTx, iTy) == MVehic))
          SBackPix(iTx, iTy, iMaskColor);
      }
  // Unlock mask surface
  UnLockSurface(cSolidMask);
  // Store mask put status & motion
  MaskPut = TRUE;
  MaskPutMotion = iMotion;
}

void C4Object::RemoveSolidMask(BOOL fCauseInstability) {
  // If put, restore background pixels from cSolidMask if background
  // has not been changed.

  // Not put
  if (!MaskPut || !cSolidMask)
    return;
  // Lock solid mask surface
  int iPitch;
  BYTE *pSolidMask = LockSurface(cSolidMask, iPitch);
  if (!pSolidMask)
    return;
  // Reput background pixels
  int xcnt, ycnt, iTx, iTy;
  BYTE byPixel;
  for (ycnt = 0; ycnt < SolidMask.Hgt; ycnt++)
    for (xcnt = 0; xcnt < SolidMask.Wdt; xcnt++)
      if ((byPixel = pSolidMask[ycnt * iPitch + xcnt])) {
        // Calculate pixel position
        iTx = x + Def->Shape.x + xcnt + SolidMask.tx;
        iTy = y + Def->Shape.y + ycnt + SolidMask.ty;
        // Restore background only if still MVehic
        if (GBackMat(iTx, iTy) == MVehic) {
          // Set pixel
          SBackPix(iTx, iTy, byPixel);
          // Instability
          if (fCauseInstability)
            Game.Landscape.CheckInstabilityRange(iTx, iTy);
        }
      }
  // Unlock solid mask surface
  UnLockSurface(cSolidMask);
  // Mask not put flag
  MaskPut = FALSE;
}

void C4Object::UpdateSolidMask() {
  // Determine necessity, update cSolidMask, put or remove mask
  BOOL fMaskOn = FALSE;
  // Mask if enabled, fullcon, no rotation, not contained
  if (SolidMask.Wdt > 0)
    if (Con >= FullCon)
      if (r == 0)
        if (!Contained)
          fMaskOn = TRUE;
  // Recheck and put mask
  if (fMaskOn) {
    RecheckSolidMask();
    PutSolidMask();
  }
  // Remove and destroy mask
  else {
    RemoveSolidMask();
    DestroySolidMask();
  }
}

void C4Object::UpdateFace() {

  // Update shape
  UpdateShape();

  // SolidMask
  UpdateSolidMask();

  // Null defaults
  BaseFace.Default();
  TopFace.Default();

  // Object complete
  if (Con >= FullCon) {
    // Straight: Original gfx
    if (!Def->Rotateable || (r == 0)) {
      RecheckSurface(&(cBitmap));
      // Primary Face
      BaseFace = Def->MainFace[Color];
      // Secondary Face
      if (Def->TopFace.Wdt > 0) // Fullcon & no rotation
        TopFace.Set(Def->Bitmap[Color], Def->TopFace.x, Def->TopFace.y, Def->TopFace.Wdt, Def->TopFace.Hgt);
    }
    // Rotated
    else {
      RecheckSurface(&(cBitmap), Shape.Wdt, Shape.Hgt);
      Engine.DDraw.DrawBox(cBitmap, 0, 0, Shape.Wdt - 1, Shape.Hgt - 1, 0);
      Engine.DDraw.BlitRotate(Def->Bitmap[Color], 0, 0, Def->Shape.Wdt, Def->Shape.Hgt, cBitmap, (Shape.Wdt - Def->Shape.Wdt) / 2, (Shape.Hgt - Def->Shape.Hgt) / 2, Def->Shape.Wdt, Def->Shape.Hgt,
                              r * 100);
      BaseFace.Set(cBitmap, 0, 0, Shape.Wdt, Shape.Hgt);
    }
  }
  // Under construction
  else
    // Grow Type Display
    if (Def->GrowthType) {
      // Straight: Facet of stretched cBitmap
      if (!Def->Rotateable || (r == 0)) {
        RecheckSurface(&(cBitmap), Def->Shape.Wdt, Def->Shape.Hgt);
        Engine.DDraw.DrawBox(cBitmap, 0, 0, Def->Shape.Wdt - 1, Def->Shape.Hgt - 1, 0);
        Engine.DDraw.Blit(Def->Bitmap[Color], 0, 0, Def->Shape.Wdt, Def->Shape.Hgt, cBitmap, 0, 0, Shape.Wdt, Shape.Hgt);
        BaseFace.Set(cBitmap, 0, 0, Shape.Wdt, Shape.Hgt);
      }
      // Rotated: Rotated stretched cBitmap
      else {
        RecheckSurface(&(cBitmap), Shape.Wdt, Shape.Hgt);
        Engine.DDraw.DrawBox(cBitmap, 0, 0, Shape.Wdt - 1, Shape.Hgt - 1, 0);
        Engine.DDraw.BlitRotate(Def->Bitmap[Color], 0, 0, Def->Shape.Wdt, Def->Shape.Hgt, cBitmap, (Shape.Wdt - Def->Shape.Wdt * Con / FullCon) / 2, (Shape.Hgt - Def->Shape.Hgt * Con / FullCon) / 2,
                                Def->Shape.Wdt * Con / FullCon, Def->Shape.Hgt * Con / FullCon, r * 100);
        BaseFace.Set(cBitmap, 0, 0, Shape.Wdt, Shape.Hgt);
      }
    }
    // Construction Type Display
    else {
      // Straight: Jolted facet of original gfx
      if (!Def->Rotateable || (r == 0)) {
        RecheckSurface(&(cBitmap));
        BaseFace.Set(Def->Bitmap[Color], 0, Def->Shape.Hgt - Shape.Hgt, Def->Shape.Wdt, Shape.Hgt);
      }
      // Rotated: Rotated jolted cBitmap
      else {
        RecheckSurface(&(cBitmap), Shape.Wdt, Shape.Hgt);
        Engine.DDraw.DrawBox(cBitmap, 0, 0, Shape.Wdt - 1, Shape.Hgt - 1, 0);
        Engine.DDraw.BlitRotate(Def->Bitmap[Color], 0, Def->Shape.Hgt - Def->Shape.Hgt * Con / FullCon, Def->Shape.Wdt, Def->Shape.Hgt * Con / FullCon, cBitmap, (Shape.Wdt - Def->Shape.Wdt) / 2,
                                (Shape.Hgt - (Def->Shape.Hgt * Con / FullCon)) / 2, Def->Shape.Wdt, Def->Shape.Hgt * Con / FullCon, r * 100);
        BaseFace.Set(cBitmap, 0, 0, Shape.Wdt, Shape.Hgt);
      }
    }

  // Active face
  UpdateActionFace();
}

void C4Object::UpdateMass() {
  Mass = Max(Def->Mass * Con / FullCon, 1);
  Mass += Contents.Mass;
  if (Contained) {
    Contained->Contents.MassCount();
    Contained->UpdateMass();
  }
}

void C4Object::ComponentConCutoff() {
  int cnt;
  for (cnt = 0; Component.GetID(cnt); cnt++)
    Component.SetCount(cnt, Min(Component.GetCount(cnt), Def->Component.GetCount(cnt) * Con / FullCon));
}

void C4Object::ComponentConGain() {
  int cnt;
  for (cnt = 0; Component.GetID(cnt); cnt++)
    Component.SetCount(cnt, Max(Component.GetCount(cnt), Def->Component.GetCount(cnt) * Con / FullCon));
}

void C4Object::SetOCF() {
  // Update the object character flag according to the object's current
  // situation
  FIXED cspeed = GetSpeed();
  InMat = GBackMat(x, y);
  // OCF_Normal: The OCF is never zero
  OCF = OCF_Normal;
  // OCF_Construct: Can be built outside
  if (Def->Constructable && (Con < FullCon) && (r == 0) && !OnFire)
    OCF |= OCF_Construct;
  // OCF_Grab: Can be pushed
  if (Def->Pushable && !(Category & C4D_StaticBack))
    OCF |= OCF_Grab;
  // OCF_Carryable: Can be picked up
  if (Def->Carryable)
    OCF |= OCF_Carryable;
  // OCF_OnFire: Is burning
  if (OnFire)
    OCF |= OCF_OnFire;
  // OCF_Inflammable: Is not burning and is inflammable
  if (!OnFire && Def->ContactIncinerate > 0)
    // Is not a dead living
    if (!(Category & C4D_Living) || Alive)
      OCF |= OCF_Inflammable;
  // OCF_FullCon: Is fully completed/grown
  if (Con >= FullCon)
    OCF |= OCF_FullCon;
  // OCF_Chop: Can be chopped
  if (Def->Chopable)
    // StaticBack only (dumb restriction to exclude trees that are already
    // chopped)
    if (Category & C4D_StaticBack)
      OCF |= OCF_Chop;
  // OCF_Rotate: Can be rotated
  if (Def->Rotateable)
    // Don't rotate minimum (invisible) construction sites
    if (Con > 100)
      OCF |= OCF_Rotate;
  // OCF_Exclusive: No action through this, no construction in front of this
  if (Def->Exclusive)
    OCF |= OCF_Exclusive;
  // OCF_Entrance: Can currently be entered/activated
  if ((Def->Entrance.Wdt > 0) && (Def->Entrance.Hgt > 0))
    if ((OCF & OCF_FullCon) && !r)
      OCF |= OCF_Entrance;
  // HitSpeeds
  if (cspeed >= HitSpeed1)
    OCF |= OCF_HitSpeed1;
  if (cspeed >= HitSpeed2)
    OCF |= OCF_HitSpeed2;
  if (cspeed >= HitSpeed3)
    OCF |= OCF_HitSpeed3;
  if (cspeed >= HitSpeed4)
    OCF |= OCF_HitSpeed4;
  // OCF_Collection
  if ((OCF & OCF_FullCon) || Def->IncompleteActivity)
    if ((Def->Collection.Wdt > 0) && (Def->Collection.Hgt > 0))
      if (!Def->CollectionLimit || (Contents.ObjectCount() < Def->CollectionLimit))
        if ((Action.Act <= ActIdle) || (!Def->ActMap[Action.Act].Disabled))
          if (NoCollectDelay == 0)
            OCF |= OCF_Collection;
  // OCF_Living
  if (Category & C4D_Living)
    OCF |= OCF_Living;
  // OCF_FightReady
  if (OCF & OCF_Living)
    if ((Action.Act <= ActIdle) || (!Def->ActMap[Action.Act].Disabled))
      OCF |= OCF_FightReady;
  // OCF_LineConstruct
  if (OCF & OCF_FullCon)
    if (Def->LineConnect)
      OCF |= OCF_LineConstruct;
  // OCF_Prey
  if (Def->Prey)
    if (Alive)
      OCF |= OCF_Prey;
  // OCF_CrewMember
  if (Def->CrewMember)
    if (Alive)
      OCF |= OCF_CrewMember;
  // OCF_AttractLightning
  if (Def->AttractLightning)
    if (OCF & OCF_FullCon)
      OCF |= OCF_AttractLightning;
  // OCF_NotContained
  if (!Contained)
    OCF |= OCF_NotContained;
  // OCF_Edible
  if (Def->Edible)
    OCF |= OCF_Edible;
  // OCF_InLiquid
  if (InLiquid)
    if (!Contained)
      OCF |= OCF_InLiquid;
  // OCF_InSolid
  if (GBackSolid(x, y))
    if (!Contained)
      OCF |= OCF_InSolid;
  // OCF_InFree
  if (!GBackSemiSolid(x, y - 1))
    if (!Contained)
      OCF |= OCF_InFree;
  // OCF_Available
  if (!GBackSemiSolid(x, y - 1) || (!GBackSolid(x, y - 1) && !GBackSemiSolid(x, y - 8)))
    if (!Contained || (Contained->Def->GrabPutGet & C4D_Grab_Get) || (Contained->OCF & OCF_Entrance))
      OCF |= OCF_Available;
  // OCF_PowerConsumer
  if (Def->LineConnect & C4D_Power_Consumer)
    if (OCF & OCF_FullCon)
      OCF |= OCF_PowerConsumer;
  // OCF_PowerSupply
  if ((Def->LineConnect & C4D_Power_Generator) || ((Def->LineConnect & C4D_Power_Output) && (Energy > 0)))
    if (OCF & OCF_FullCon)
      OCF |= OCF_PowerSupply;
  // OCF_Container
  if ((Def->GrabPutGet & C4D_Grab_Put) || (Def->GrabPutGet & C4D_Grab_Get) || (OCF & OCF_Entrance))
    OCF |= OCF_Container;
}

const int MaxFirePhase = 15;

BOOL C4Object::ExecFire() {
  // OnFire check
  if (!OnFire)
    return FALSE;
  // Fire Phase
  FirePhase++;
  if (FirePhase >= MaxFirePhase)
    FirePhase = 0;
  // Extinguish in base
  if (!Tick5)
    if (Category & C4D_Living)
      if (Contained && ValidPlr(Contained->Base))
        Extinguish();
  // Decay
  if (!Def->NoBurnDecay)
    DoCon(-100);
  // Damage
  if (!Tick10)
    DoDamage(+2, NO_OWNER);
  // Energy
  if (!Tick5)
    DoEnergy(-1);
  // Effects
  int smoke_level = 2 * Shape.Wdt / 3;
  if (!(Tick255 % Max(smoke_level / 2, 3)) || (OCF & OCF_HitSpeed2))
    Smoke(x, y, smoke_level);
  // Background Effects
  if (!Tick5) {
    int mat;
    if (MatValid(mat = GBackMat(x, y))) {
      // Extinguish
      if (Game.Material.Map[mat].Extinguisher) {
        Extinguish();
        if (GBackLiquid(x, y))
          SoundEffect("Pshshsh", 0, 100, this);
      }
      // Inflame
      if (!Random(3))
        Game.Landscape.Incinerate(x, y);
    }
  }

  return TRUE;
}

BOOL C4Object::BuyEnergy() {
  C4Player *pPlr = Game.Players.Get(Base);
  if (!pPlr)
    return FALSE;
  if (!GetPhysical()->Energy)
    return FALSE;
  if (pPlr->Eliminated)
    return FALSE;
  if (pPlr->Wealth < 5)
    return FALSE;
  pPlr->DoWealth(-5);
  DoEnergy(+100);
  return TRUE;
}

BOOL C4Object::ExecLife() {

  // Growth
  if (!Tick35)
    // Growth specified by definition
    if (Def->Growth)
      // Alive livings && trees only
      if (((Category & C4D_Living) && Alive) || (Category & C4D_StaticBack))
        // Not burning
        if (!OnFire)
          // Not complete yet
          if (Con < FullCon)
            // Grow
            DoCon(Def->Growth * 100);

  // Energy reload in base
  int transfer;
  if (!Tick3)
    if (Alive)
      if (Contained && ValidPlr(Contained->Base))
        if (!Hostile(Owner, Contained->Base))
          if (Energy < GetPhysical()->Energy) {
            if (Contained->Energy <= 0)
              Contained->BuyEnergy();
            transfer = Min(Min(2 * C4MaxPhysical / 100, Contained->Energy), GetPhysical()->Energy - Energy);
            if (transfer) {
              Contained->Energy -= transfer;
              Energy += transfer;
              ViewEnergy = C4ViewDelay;
            }
          }

  // Magic reload
  if (!Tick3)
    if (Alive)
      if (Contained)
        if (!Hostile(Owner, Contained->Owner))
          if (MagicEnergy < GetPhysical()->Magic) {
            transfer = Min(Min(2 * C4MaxPhysical / 100, Contained->MagicEnergy), GetPhysical()->Magic - MagicEnergy);
            if (transfer) {
              Contained->MagicEnergy -= transfer;
              MagicEnergy += transfer;
              ViewEnergy = C4ViewDelay;
            }
          }

  // Breathing
  if (!Tick5)
    if (Alive) {
      // Supply check
      BOOL Breathe = FALSE;
      if (GetPhysical()->BreatheWater) {
        if (GBackMat(x, y) == MWater)
          Breathe = TRUE;
      } else {
        if (!GBackSemiSolid(x, y + Shape.y / 2))
          Breathe = TRUE;
      }
      if (Contained)
        Breathe = TRUE;
      // No supply
      if (!Breathe) {
        // Reduce breath, then energy, bubble
        if (Breath > 0)
          Breath = Max(Breath - 2 * C4MaxPhysical / 100, 0);
        else
          DoEnergy(-1);
        BubbleOut(x + Random(5) - 2, y + Shape.y / 2);
        ViewEnergy = C4ViewDelay;
        // Physical training
        if (Info)
          if (Info->Physical.Breath < C4MaxPhysical)
            Info->Physical.Breath += 2;
      }
      // Supply
      else {
        // Take breath
        int takebreath = GetPhysical()->Breath - Breath;
        if (takebreath > GetPhysical()->Breath / 2)
          Call(PSF_DeepBreath);
        Breath += takebreath;
      }
    }

  // Corrosion energy loss
  if (!Tick10)
    if (Alive)
      if (InMat != MNone)
        if (Game.Material.Map[InMat].Corrosive)
          if (!GetPhysical()->CorrosionResist)
            DoEnergy(-Game.Material.Map[InMat].Corrosive / 15);

  // InMat incineration
  if (!Tick10)
    if (InMat != MNone)
      if (Game.Material.Map[InMat].Incindiary)
        if (Def->ContactIncinerate)
          Incinerate(NO_OWNER);

  // Nonlife normal energy loss
  if (!Tick10)
    if (!(Category & C4D_Living))
      if (!ValidPlr(Base))
        DoEnergy(-1);

  return TRUE;
}

void C4Object::AutoSellContents() {
  C4ObjectLink *clnk;
  C4Object *cobj1, *cobj2;
  C4Player *pPlr = Game.Players.Get(Base);
  if (!pPlr)
    return;

  // Grab lorry contents
  for (clnk = Contents.First; clnk && (cobj1 = clnk->Obj); clnk = clnk->Next)
    if (cobj1->Status)
      if (cobj1->Def->id == C4ID_Lorry)
        while (cobj2 = cobj1->Contents.GetObject())
          cobj2->Enter(this);

  // Content's gold contents
  for (clnk = Contents.First; clnk && (cobj1 = clnk->Obj); clnk = clnk->Next)
    if (cobj1->Status)
      while (cobj2 = cobj1->Contents.Find(C4ID_Gold)) {
        cobj2->Exit();
        pPlr->Sell2Home(cobj2);
      }

  // Gold contents
  while (cobj1 = Contents.Find(C4ID_Gold)) {
    cobj1->Exit();
    pPlr->Sell2Home(cobj1);
  }
}

void C4Object::ExecBase() {
  C4Object *flag;
  ;

  // New base assignment by flag (no old base removal)
  if (!Tick10)
    if (Def->CanBeBase)
      if (!ValidPlr(Base))
        if (flag = Contents.Find(C4ID_Flag))
          if (ValidPlr(flag->Owner) && (flag->Owner != Base)) {
            // Attach new flag
            flag->Exit();
            flag->SetActionByName("FlyBase", this);
            // Assign new base
            Base = flag->Owner;
            Owner = flag->Owner;
            Contents.CloseMenus();
            SoundEffect("Trumpet", 0, 100, this);
            // Set color
            if (Def->ColorByOwner)
              if (ValidPlr(Owner)) {
                Color = BoundBy(Game.Players.Get(Owner)->Color, 0, C4MaxColor - 1);
                UpdateFace();
              }
          }

  // Base execution
  if (!Tick35)
    if (ValidPlr(Base)) {
      // Auto sell contents
      AutoSellContents();

      // Lost flag?
      if (!Game.FindObject(C4ID_Flag, 0, 0, 0, 0, OCF_All, "FlyBase", this)) {
        Base = NO_OWNER;
        Contents.CloseMenus();
      }
    }

  // Environmental action
  if (!Tick35) {
    // Structures dig free snow
    if (Category & C4D_Structure)
      if (r == 0) {
        Game.Landscape.DigFreeMat(x + Shape.x, y + Shape.y, Shape.Wdt, Shape.Hgt, MSnow);
        Game.Landscape.DigFreeMat(x + Shape.x, y + Shape.y, Shape.Wdt, Shape.Hgt, Game.Material.Get("FlyAshes"));
      }
  }
}

void C4Object::Execute() {
  // OCF
  SetOCF();
  // Command
  ExecuteCommand();
  // Action
  ExecAction();
  // Movement
  ExecMovement();
  // Fire
  ExecFire();
  // Life
  ExecLife();
  // Base
  ExecBase();
  // Timer
  Timer++;
  if (Timer >= Def->Timer) {
    Timer = 0;
    // TimerCall
    if (Def->TimerCall[0]) {
      C4Thread cthr;
      cthr.cObj = this;
      sprintf(cthr.Function, "TimerCall");
      Call(&cthr, Def->TimerCall);
    }
  }
  // Menu
  if (Menu)
    Menu->Execute();
  // View delays
  if (ViewEnergy > 0)
    ViewEnergy--;
  // Update audibility
  UpdateAudible();
}

BOOL C4Object::At(int ctx, int cty, DWORD &ocf) {
  DWORD rocf;

  // Minimum top action size for build check
  int addtop = Max(18 - Shape.Hgt, 0);

  if (Status)
    if (!Contained)
      if (Def)
        if (OCF & ocf)
          if (Inside(cty - (y + Shape.y - addtop), 0, Shape.Hgt - 1 + addtop))
            if (Inside(ctx - (x + Shape.x), 0, Shape.Wdt - 1)) {
              // Set ocf return value
              rocf = OCF;
              // Verify entrance area OCF return
              if (rocf & OCF_Entrance)
                if (!Inside(cty - (y + Def->Entrance.y), 0, Def->Entrance.Hgt - 1) || !Inside(ctx - (x + Def->Entrance.x), 0, Def->Entrance.Wdt - 1))
                  rocf &= (~OCF_Entrance);
              // Verify collection area OCF return
              if (rocf & OCF_Collection)
                if (!Inside(cty - (y + Def->Collection.y), 0, Def->Collection.Hgt - 1) || !Inside(ctx - (x + Def->Collection.x), 0, Def->Collection.Wdt - 1))
                  rocf &= (~OCF_Collection);
              ocf = rocf;
              return TRUE;
            }

  return FALSE;
}

void C4Object::AssignDeath() {
  C4Object *thing;
  // Alive objects only
  if (!Alive)
    return;
  // Action
  SetActionByName("Dead");
  // Extinguish
  Extinguish();
  // Values
  Select = 0;
  Alive = 0;
  ClearCommands();
  if (Info)
    Info->HasDied = TRUE;
  // Lose contents
  while (thing = Contents.GetObject())
    thing->Exit(thing->x, thing->y);
  // Remove from crew/cursor
  if (ValidPlr(Owner))
    Game.Players.Get(Owner)->ClearPointers(this);
  // Engine script call
  Call(PSF_Death);
}

BOOL C4Object::ChangeDef(C4ID idNew) {
  // Get new definitino
  C4Def *pDef = C4Id2Def(idNew);
  if (!pDef)
    return FALSE;
  // Containment storage
  C4Object *pContainer = Contained;
  // Exit container (no Ejection/Departure)
  if (Contained)
    Exit(0, 0, 0, 0, 0, 0, FALSE);
  // Pre change resets
  SetAction(ActIdle);
  Action.Act = ActIdle; // Enforce ActIdle because SetAction may have failed due
                        // to NoOtherAction
  SetDir(0);
  RemoveSolidMask();
  DestroySolidMask();
  // Def change
  Def = pDef;
  id = pDef->id;
  // Catch object settings that might be invalid for new def
  if (!Def->ColorByOwner)
    Color = 0;
  if (!Def->Rotateable)
    r = fix_r = rdir = 0;
  // Reset solid mask
  SolidMask = Def->SolidMask;
  // Post change updates
  UpdateMass();
  UpdateFace();
  // Containment (no Entrance)
  if (pContainer)
    Enter(pContainer, FALSE);
  // Done
  return TRUE;
}

BOOL C4Object::Incinerate(int iCausedBy) {
  // Already on fire
  if (OnFire)
    return FALSE;
  // Dead living don't burn
  if ((Category & C4D_Living) && !Alive)
    return FALSE;
  // In extinguishing material
  int iMat;
  if (MatValid(iMat = GBackMat(x, y)))
    if (Game.Material.Map[iMat].Extinguisher)
      return FALSE;
  // BurnTurnTo
  if (Def->BurnTurnTo != C4ID_None)
    ChangeDef(Def->BurnTurnTo);
  // Set values
  OnFire = 1;
  FirePhase = Random(MaxFirePhase);
  if (Shape.Wdt * Shape.Hgt > 500)
    SoundEffect("Inflame", 0, 100, this);
  if (Def->Mass >= 100)
    SoundEffect("Fire", +1, 100, this);
  // Engine script call
  Call(PSF_Incineration, iCausedBy);
  // Done
  return TRUE;
}

BOOL C4Object::Extinguish() {
  if (!OnFire)
    return FALSE;
  OnFire = 0;
  if (Def->Mass >= 100)
    SoundEffect("Fire", -1, 100, this);
  return TRUE;
}

void C4Object::DoDamage(int iChange, int iCausedBy) {
  // Change value
  Damage = Max(Damage + iChange, 0);
  // Engine script call
  Call(PSF_Damage, iChange, iCausedBy);
}

void C4Object::DoEnergy(int iChange) {
  // iChange 100% = Physical 100000
  iChange = iChange * C4MaxPhysical / 100;
  // Was zero?
  BOOL fWasZero = (Energy == 0);
  // Do change
  Energy = BoundBy(Energy + iChange, 0, GetPhysical()->Energy);
  // Alive and energy reduced to zero: death
  if (Alive)
    if (Energy == 0)
      if (!fWasZero)
        AssignDeath();
  // View change
  ViewEnergy = C4ViewDelay;
}

void C4Object::DoBreath(int iChange) {
  // iChange 100% = Physical 100000
  iChange = iChange * C4MaxPhysical / 100;
  // Do change
  Breath = BoundBy(Breath + iChange, 0, GetPhysical()->Breath);
  // View change
  ViewEnergy = C4ViewDelay;
}

void C4Object::Blast(int iLevel, int iCausedBy) {
  // Damage
  DoDamage(iLevel, iCausedBy);
  // Energy (alive objects)
  if (Alive)
    DoEnergy(-iLevel / 3);
  // Incinerate
  if (Def->BlastIncinerate)
    if (Damage >= Def->BlastIncinerate)
      Incinerate(iCausedBy);
}

void C4Object::DoCon(int iChange, BOOL fInitial) {
  int iStepSize = FullCon / 100;
  int lRHgt = Shape.Hgt, lRy = Shape.y;
  int iLastStep = Con / iStepSize;
  int strgt_con_b = y + Shape.y + Shape.Hgt;
  BOOL fWasFull = (Con >= FullCon);

  // Change con
  if (Def->Oversize)
    Con = Max(Con + iChange, 0);
  else
    Con = BoundBy(Con + iChange, 0, FullCon);

  // Update OCF
  SetOCF();

  // If step changed or limit reached or degraded from full: update mass, face,
  // components, etc.
  if ((Con / iStepSize != iLastStep) || (Con >= FullCon) || (Con == 0) || (fWasFull && (Con < FullCon))) {
    // Mass
    UpdateMass();
    // Decay from full remove mask before face is changed
    if (fWasFull && (Con < FullCon))
      RemoveSolidMask();
    // Face
    UpdateFace();
    // Decay: reduce components
    if (iChange < 0)
      ComponentConCutoff();
    // Growth: gain components
    else
      ComponentConGain();
    // Unfullcon
    if (Con < FullCon) {
      // Lose contents
      if (!Def->IncompleteActivity) {
        C4Object *cobj;
        while (cobj = Contents.GetObject())
          if (Contained)
            cobj->Enter(Contained);
          else
            cobj->Exit(cobj->x, cobj->y);
      }
      // No energy need
      NeedEnergy = 0;
    }
    // Decay from full stop action
    if (fWasFull && (Con < FullCon))
      if (!Def->IncompleteActivity)
        SetAction(ActIdle);
  }

  // Straight Con bottom y-adjust
  if (!r)
    if ((Shape.Hgt != lRHgt) || (Shape.y != lRy)) {
      RemoveSolidMask();
      y = strgt_con_b - Shape.Hgt - Shape.y;
      PutSolidMask();
    }

  // Completion (after bottom y-adjust for correct position)
  if (!fWasFull && (Con >= FullCon)) {
    Call(PSF_Completion);
    Call(PSF_Initialize);
  }

  // Con Zero Removal
  if (Con <= 0)
    AssignRemoval();
}

void C4Object::DoExperience(int change) {
  const int MaxExperience = 100000000;

  if (!Info)
    return;

  Info->Experience = BoundBy(Info->Experience + change, 0, MaxExperience);

  // Promotion check
  if (Info->Experience < MaxExperience)
    if (Info->Experience >= Game.Rank.Experience(Info->Rank + 1))
      Promote(Info->Rank + 1);
}

BOOL C4Object::Exit(int iX, int iY, int iR, FIXED iXDir, FIXED iYDir, FIXED iRDir, BOOL fCalls) {
  // 1. Exit the current container.
  // 2. Update Contents of container object and set Contained to NULL.
  // 3. Set offset position/motion if desired.
  // 4. Call Ejection for container and Departure for object.

  // Not contained
  C4Object *pContainer = Contained;
  if (!pContainer)
    return FALSE;
  // Remove object from container
  pContainer->Contents.Remove(this);
  pContainer->UpdateMass();
  pContainer->SetOCF();
  // No container
  Contained = NULL;
  // Position/motion
  x = iX;
  y = iY;
  r = iR;
  fix_x = itofix(x);
  fix_y = itofix(y);
  fix_r = itofix(r);
  xdir = iXDir;
  ydir = iYDir;
  rdir = iRDir;
  // Misc updates
  Mobile = 1;
  InLiquid = 0;
  CloseMenu();
  UpdateFace();
  SetOCF();
  // Engine calls
  if (fCalls)
    pContainer->Call(PSF_Ejection, (long)this);
  if (fCalls)
    Call(PSF_Departure, (long)pContainer);
  // Success
  return TRUE;
}

BOOL C4Object::Enter(C4Object *pTarget, BOOL fCalls) {
  // 1. Exit if contained.
  // 2. Set new container.
  // 3. Update Contents and mass of the new container.
  // 4. Call entrance for object.

  // No target or target is self
  if (!pTarget || (pTarget == this))
    return FALSE;
  // Exit if contained
  if (Contained)
    if (!Exit(x, y))
      return FALSE;
  // Failsafe updates
  CloseMenu();
  SetOCF();
  // Enter
  if (!pTarget->Contents.Add(this))
    return FALSE;
  // Set container
  Contained = pTarget;
  // Misc updates
  SetOCF();
  UpdateFace();
  // Update container
  Contained->UpdateMass();
  Contained->SetOCF();
  // Entrance call
  if (fCalls)
    Call(PSF_Entrance, (long)Contained);
  // Success
  return TRUE;
}

void C4Object::Fling(FIXED txdir, FIXED tydir) {
  if (!ObjectActionTumble(this, (txdir < 0), txdir, tydir))
    ObjectActionJump(this, txdir, tydir);
}

BOOL C4Object::ActivateEntrance(int by_plr, C4Object *by_obj) {
  // Hostile: no entrance to base
  if (Hostile(by_plr, Base)) {
    if (ValidPlr(Owner)) {
      sprintf(OSTR, LoadResStr(IDS_OBJ_HOSTILENOENTRANCE), Game.Players.Get(Owner)->Name);
      GameMsgObject(OSTR, this);
    }
    return FALSE;
  }
  // Try entrance activation
  if (OCF & OCF_Entrance)
    if (Call(PSF_ActivateEntrance, (long)by_obj))
      return TRUE;
  // Failure
  return FALSE;
}

void C4Object::Explode(int iLevel) {
  // Called by FnExplode only
  C4Object *pContainer = Contained;
  int iCausedBy = Owner;
  AssignRemoval();
  Explosion(x, y, iLevel, pContainer, iCausedBy, this);
}

BOOL C4Object::Build(int iLevel, C4Object *pBuilder) {
  int cnt;
  BOOL fNeeds = FALSE;
  char ostr[10];
  C4Def *pComponent;
  C4Object *pMaterial;

  // Invalid or complete: no build
  if (!Status || !Def || (Con >= FullCon))
    return FALSE;

  // Material check (if rule set or any other than structure)
  if ((Game.Rules & C4RULE_ConstructionNeedsMaterial) || !(Category & C4D_Structure)) {
    // Grab any needed components from builder
    for (cnt = 0; Component.GetID(cnt); cnt++)
      if (Component.GetCount(cnt) < Def->Component.GetCount(cnt))
        if ((pMaterial = pBuilder->Contents.Find(Def->Component.GetID(cnt))))
          if (!pMaterial->OnFire)
            if (pMaterial->OCF & OCF_FullCon) {
              Component.SetCount(cnt, Component.GetCount(cnt) + 1);
              pBuilder->Contents.Remove(pMaterial);
              pMaterial->AssignRemoval();
            }
    // Grab any needed components from container
    if (Contained)
      for (cnt = 0; Component.GetID(cnt); cnt++)
        if (Component.GetCount(cnt) < Def->Component.GetCount(cnt))
          if ((pMaterial = Contained->Contents.Find(Def->Component.GetID(cnt))))
            if (!pMaterial->OnFire)
              if (pMaterial->OCF & OCF_FullCon) {
                Component.SetCount(cnt, Component.GetCount(cnt) + 1);
                Contained->Contents.Remove(pMaterial);
                pMaterial->AssignRemoval();
              }
    // Check for needed components at current con
    for (cnt = 0; Def->Component.GetID(cnt); cnt++)
      if (Def->Component.GetCount(cnt) != 0)
        if ((100 * Component.GetCount(cnt) / Def->Component.GetCount(cnt)) < (100 * Con / FullCon))
          fNeeds = TRUE;
  }

  // Needs components
  if (fNeeds) {

    // Create needed component list & compose message
    C4IDList NeededMaterial;
    sprintf(OSTR, LoadResStr(IDS_OBJ_NEEDS), Def->Name);
    for (cnt = 0; Def->Component.GetID(cnt); cnt++)
      if (Def->Component.GetCount(cnt) != 0)
        if (Component.GetCount(cnt) < Def->Component.GetCount(cnt)) {
          // Append component text to message
          SAppend("|", OSTR);
          sprintf(ostr, "%dx ", Def->Component.GetCount(cnt) - Component.GetCount(cnt));
          SAppend(ostr, OSTR);
          // Needed component undefined: fail
          if (!(pComponent = C4Id2Def(Def->Component.GetID(cnt)))) {
            GetC4IdText(Def->Component.GetID(cnt), ostr);
            sprintf(OSTR, LoadResStr(IDS_OBJ_NEEDSUNDEF), ostr);
            GameMsgObject(OSTR, this);
            return FALSE;
          }
          SAppend(pComponent->Name, OSTR);
          NeededMaterial.SetIDCount(Def->Component.GetID(cnt), Def->Component.GetCount(cnt) - Component.GetCount(cnt), TRUE);
        }

    // Builder is a crew member...
    if (pBuilder->OCF & OCF_CrewMember)
      // ...tell builder to acquire the material
      if (pBuilder->AddCommand(C4CMD_Acquire, NULL, 0, 0, 50, NULL, TRUE, NeededMaterial.GetID(0), FALSE, 5))
        // Done/fail (no message)
        return FALSE;

    // BuildNeedsMaterial call to builder script...
    if (!pBuilder->Call(PSF_BuildNeedsMaterial, NeededMaterial.GetID(0), NeededMaterial.GetCount(0)))
      // ...game message if not overloaded
      GameMsgObject(OSTR, this);

    // Still in need: done/fail
    return FALSE;
  }

  // Do con (mass-relative)
  DoCon(iLevel * 10 * 1500 / Def->Mass);

  // TurnTo
  if (Def->BuildTurnTo != C4ID_None)
    ChangeDef(Def->BuildTurnTo);

  // Repair
  Damage = 0;

  // Done/success
  return TRUE;
}

BOOL C4Object::Chop(C4Object *pByObject) {
  // Valid check
  if (!Status || !Def || Contained || !(OCF & OCF_Chop))
    return FALSE;
  // Chop
  if (!Tick10)
    DoDamage(+10, pByObject ? pByObject->Owner : NO_OWNER);
  return TRUE;
}

BOOL C4Object::Push(FIXED txdir, FIXED dforce, BOOL fStraighten) {
  // Valid check
  if (!Status || !Def || Contained || !(OCF & OCF_Grab))
    return FALSE;
  // Grabbing okay, no pushing
  if (Def->Pushable == 2)
    return TRUE;
  // Mobilization check (pre-mobilization zero)
  if (!Mobile) {
    xdir = ydir = 0;
    fix_x = itofix(x);
    fix_y = itofix(y);
  }
  // General pushing force vs. object mass
  dforce = ftofix(fixtof(dforce) * 100.0 / Mass);
  // Set dir
  if (xdir < 0)
    SetDir(DIR_Left);
  if (xdir > 0)
    SetDir(DIR_Right);
  // Work towards txdir
  if (Abs(xdir - txdir) <= dforce) // Close-enough-set
  {
    xdir = txdir;
  } else // Work towards
  {
    if (xdir < txdir)
      xdir += dforce;
    if (xdir > txdir)
      xdir -= dforce;
  }
  // Straighten
  if (fStraighten)
    if (Inside(r, -StableRange, +StableRange)) {
      rdir = 0; // cheap way out
    } else {
      if (r > 0) {
        if (rdir > -RotateAccel)
          rdir -= dforce;
      } else {
        if (rdir < +RotateAccel)
          rdir += dforce;
      }
    }

  // Mobilization check
  if (xdir || ydir || rdir)
    Mobile = 1;

  // Stuck check
  if (!Tick35)
    if (txdir)
      if (!Def->NoHorizontalMove)
        if (ContactCheck(x, y)) // Resets t_contact
        {
          sprintf(OSTR, LoadResStr(IDS_OBJ_STUCK), GetName());
          GameMsgObject(OSTR, this);
          Call(PSF_Stuck);
        }

  return TRUE;
}

BOOL C4Object::Lift(FIXED tydir, FIXED dforce) {
  // Valid check
  if (!Status || !Def || Contained)
    return FALSE;
  // Mobilization check
  if (!Mobile) {
    xdir = ydir = 0;
    fix_x = itofix(x);
    fix_y = itofix(y);
    Mobile = 1;
  }
  // General pushing force vs. object mass
  dforce = ftofix(fixtof(dforce) * 100.0 / Mass);
  // If close enough, set tydir
  if (Abs(tydir - ydir) <= Abs(dforce))
    ydir = tydir;
  else // Work towards tydir
  {
    if (ydir < tydir)
      ydir += dforce;
    if (ydir > tydir)
      ydir -= dforce;
  }
  // Stuck check
  if (tydir != -GravAccel)
    if (ContactCheck(x, y)) // Resets t_contact
    {
      sprintf(OSTR, LoadResStr(IDS_OBJ_STUCK), GetName());
      GameMsgObject(OSTR, this);
      Call(PSF_Stuck);
    }
  return TRUE;
}

C4Object *C4Object::CreateContents(C4ID n_id) {
  C4Object *nobj;
  if (!(nobj = Game.CreateObject(n_id, Owner)))
    return NULL;
  if (!nobj->Enter(this)) {
    nobj->AssignRemoval();
    return NULL;
  }
  return nobj;
}

BOOL C4Object::CreateContentsByList(C4IDList &idlist) {
  int cnt, cnt2;
  for (cnt = 0; idlist.GetID(cnt); cnt++)
    for (cnt2 = 0; cnt2 < idlist.GetCount(cnt); cnt2++)
      if (!CreateContents(idlist.GetID(cnt)))
        return FALSE;
  return TRUE;
}

BOOL C4Object::ActivateMenu(int iMenu, int iMenuSelect, int iMenuData, int iMenuPosition, C4Object *pTarget) {
  // Variables
  C4FacetEx fctSymbol;
  char szCaption[256 + 1], szCommand[256 + 1];
  int cnt, iCount;
  C4Def *pDef;
  C4Player *pPlayer;
  C4IDList ListItems;
  // Close any other menu
  CloseMenu();
  // Create menu
  Menu = new C4Menu;
  // Open menu
  switch (iMenu) {
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Activate:
    // Target is container
    if (!(pTarget = Contained))
      break;
    // Create symbol
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol);
    sprintf(szCaption, LoadResStr(IDS_OBJ_EMPTY), pTarget->GetName());
    // Init
    Menu->Init(fctSymbol, szCaption, this, C4MN_Extra_None, 0, iMenu);
    fctSymbol.Default();
    Menu->SetPermanent(TRUE);
    Menu->SetRefillObject(pTarget);
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Buy:
    // No target specified: container is base
    if (!pTarget)
      if (!(pTarget = Contained))
        break;
    // Create symbol
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol);
    // Init menu
    Menu->Init(fctSymbol, LoadResStr(IDS_PLR_NOBUY), this, C4MN_Extra_Value, 0, iMenu);
    Menu->SetPermanent(TRUE);
    Menu->SetRefillObject(pTarget);
    // Default symbol to avoid destruction
    fctSymbol.Default();
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Sell:
    // No target specified: container is base
    if (!pTarget)
      if (!(pTarget = Contained))
        break;
    // Create symbol & init
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol);
    sprintf(szCaption, LoadResStr(IDS_OBJ_EMPTY), pTarget->GetName());
    Menu->Init(fctSymbol, szCaption, this, C4MN_Extra_Value, 0, iMenu);
    fctSymbol.Default();
    Menu->SetPermanent(TRUE);
    Menu->SetRefillObject(pTarget);
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Get:
    // Target by parameter
    if (!pTarget)
      break;
    // Create symbol & init
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol);
    sprintf(szCaption, LoadResStr(IDS_OBJ_EMPTY), pTarget->GetName());
    Menu->Init(fctSymbol, szCaption, this, C4MN_Extra_None, 0, iMenu);
    fctSymbol.Default();
    Menu->SetPermanent(TRUE);
    Menu->SetRefillObject(pTarget);
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Context:
    // Target by parameter
    if (!pTarget)
      break; // default to self...?
    // Create symbol & init menu
    pPlayer = Game.Players.Get(pTarget->Owner);
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol, FALSE, (pPlayer && pTarget->Def->ColorByOwner) ? pPlayer->Color : 0);
    Menu->Init(fctSymbol, pTarget->GetName(), this, C4MN_Extra_None, 0, iMenu, C4MN_Style_Context);
    fctSymbol.Default();

    // Contents (if container, friendly or self contained)
    if (pTarget->OCF & OCF_Container)
      if ((ValidPlr(pTarget->Owner) && !Hostile(pTarget->Owner, Owner)) || (pTarget == Contained)) {
        sprintf(szCommand, "SetCommand(this(),\"Activate\",0,0,0,Object(%d))", pTarget->Number);
        fctSymbol.Create(C4SymbolSize, C4SymbolSize);
        pTarget->Def->Draw(fctSymbol);
        Menu->Add(LoadResStr(IDS_CON_CONTENTS), fctSymbol, szCommand);
        fctSymbol.Default();
      }

    // Ought to be in advanced flag/homebase script...
    // Homebase buy/sell (if friendly base)
    if (ValidPlr(pTarget->Base) && !Hostile(pTarget->Base, Owner)) {
      // Buy
      sprintf(szCommand, "SetCommand(this(),\"Buy\",Object(%d))", pTarget->Number);
      fctSymbol.Create(C4SymbolSize, C4SymbolSize);
      DrawMenuSymbol(C4MN_Buy, fctSymbol, pTarget->Base, pTarget);
      Menu->Add(LoadResStr(IDS_CON_BUY), fctSymbol, szCommand);
      fctSymbol.Default();
      // Sell
      sprintf(szCommand, "SetCommand(this(),\"Sell\",Object(%d))", pTarget->Number);
      fctSymbol.Create(C4SymbolSize, C4SymbolSize);
      DrawMenuSymbol(C4MN_Sell, fctSymbol, pTarget->Base, pTarget);
      Menu->Add(LoadResStr(IDS_CON_SELL), fctSymbol, szCommand);
      fctSymbol.Default();
    }

    // Context functions (if crew member only on own)
    int iFunction;
    C4ScriptFnRef *pFunction;
    if (!(pTarget->OCF & OCF_CrewMember) || (pTarget->Owner == Owner))
      for (iFunction = 0; pFunction = pTarget->Def->Script.GetFunctionRef(iFunction); iFunction++)
        if (SEqual2(pFunction->Name, "Context"))
          if (!pFunction->Condition[0] || pTarget->Call(pFunction->Condition)) {
            sprintf(szCommand, "ObjectCall(Object(%d),\"%s\",this())", pTarget->Number, pFunction->Name);
            fctSymbol.Create(16, 16);
            if (pDef = C4Id2Def(pFunction->idImage))
              pDef->Draw(fctSymbol);
            Menu->Add(pFunction->Desc, fctSymbol, szCommand);
            fctSymbol.Default();
          }

    // Info (if desc available)
    if (pTarget->Def->Desc && pTarget->Def->Desc[0]) {
      sprintf(szCommand, "ShowInfo(Object(%d))", pTarget->Number);
      fctSymbol.Create(16, 16);
      GfxR->fctOKCancel.Draw(fctSymbol, TRUE, 0, 1);
      Menu->Add(LoadResStr(IDS_CON_INFO), fctSymbol, szCommand);
      fctSymbol.Default();
    }

    // Preselect
    Menu->SetSelection(iMenuSelect);
    Menu->SetPosition(iMenuPosition);
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Construction:
    // Check valid player
    if (!(pPlayer = Game.Players.Get(Owner)))
      break;
    // Create symbol
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    DrawMenuSymbol(C4MN_Construction, fctSymbol, -1, NULL);
    // Init menu
    sprintf(OSTR, LoadResStr(IDS_PLR_NOBKNOW), pPlayer->Name);
    Menu->Init(fctSymbol, OSTR, this, C4MN_Extra_Components, 0, iMenu);
    // Default symbol to avoid destruction
    fctSymbol.Default();
    // Add player's structure build knowledge
    for (cnt = 0; pDef = C4Id2Def(pPlayer->Knowledge.GetID(Game.Defs, C4D_Structure, cnt, &iCount)); cnt++) {
      // Caption
      sprintf(szCaption, LoadResStr(IDS_MENU_CONSTRUCT), pDef->Name);
      // Picture
      fctSymbol.Set(pDef->Bitmap[0], pDef->PictureRect.x, pDef->PictureRect.y, pDef->PictureRect.Wdt, pDef->PictureRect.Hgt);
      // Command
      sprintf(szCommand, "SetCommand(this(),\"Construct\",0,0,0,0,%s)", C4IdText(pDef->id));
      // Add menu item
      Menu->Add(szCaption, fctSymbol, szCommand, C4MN_Item_NoCount, NULL, 0, pDef->Desc, pDef->id);
    }
    // Preselect
    Menu->SetSelection(iMenuSelect);
    Menu->SetPosition(iMenuPosition);
    // Success
    return TRUE;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case C4MN_Info:
    // Target by parameter
    if (!pTarget)
      break;
    pPlayer = Game.Players.Get(pTarget->Owner);
    // Create symbol & init menu
    fctSymbol.Create(C4SymbolSize, C4SymbolSize);
    pTarget->Def->Draw(fctSymbol, FALSE, (pPlayer && pTarget->Def->ColorByOwner) ? pPlayer->Color : 0);
    Menu->Init(fctSymbol, pTarget->GetName(), this, C4MN_Extra_None, 0, iMenu, C4MN_Style_Info);
    Menu->SetPermanent(TRUE);
    Menu->SetAlignment(C4MN_Align_Free);
    C4Viewport *pViewport = Game.GraphicsSystem.GetViewport(Owner); // Hackhackhack!!!
    if (pViewport)
      Menu->SetLocation(pTarget->x + pTarget->Shape.x + pTarget->Shape.Wdt + 10 - pViewport->ViewX, pTarget->y + pTarget->Shape.y - pViewport->ViewY);
    fctSymbol.Default();
    // Add info item
    fctSymbol.Clear();
    Menu->Add(pTarget->GetName(), fctSymbol, "", C4MN_Item_NoCount, NULL, 0, pTarget->Def->Desc);
    // Success
    return TRUE;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - -
  }
  // Invalid menu identification
  CloseMenu();
  return FALSE;
}

void C4Object::CloseMenu() {
  if (Menu)
    delete Menu;
  Menu = NULL;
}

BYTE C4Object::GetArea(int &aX, int &aY, int &aWdt, int &aHgt) {
  if (!Status || !Def)
    return 0;
  aX = x + Shape.x;
  aY = y + Shape.y;
  aWdt = Shape.Wdt;
  aHgt = Shape.Hgt;
  return 1;
}

BYTE C4Object::GetEntranceArea(int &aX, int &aY, int &aWdt, int &aHgt) {
  if (!Status || !Def)
    return 0;
  // Return actual entrance
  if (OCF & OCF_Entrance) {
    aX = x + Def->Entrance.x;
    aY = y + Def->Entrance.y;
    aWdt = Def->Entrance.Wdt;
    aHgt = Def->Entrance.Hgt;
  }
  // Return object center
  else {
    aX = x;
    aY = y;
    aWdt = 0;
    aHgt = 0;
  }
  // Done
  return 1;
}

BYTE C4Object::GetMomentum(FIXED &rxdir, FIXED &rydir) {
  rxdir = rydir = 0;
  if (!Status || !Def)
    return 0;
  rxdir = xdir;
  rydir = ydir;
  return 1;
}

FIXED C4Object::GetSpeed() {
  FIXED cobjspd = 0;
  if (xdir < 0)
    cobjspd -= xdir;
  else
    cobjspd += xdir;
  if (ydir < 0)
    cobjspd -= ydir;
  else
    cobjspd += ydir;
  return cobjspd;
}

const char *C4Object::GetName() {
  if (Info)
    return Info->Name;
  return Def->Name;
}

#define BarrelFill Action.ComDir
#define MaxBarrelFill 200

int C4Object::GetValue() {
  // Value by definition
  int iValue = Def->Value;
  // Barrels: empty barrel iValue + full barrel difference according to fill
  // iLevel
  if ((Def->id == C4Id("WBRL")) || (Def->id == C4Id("ABRL")) || (Def->id == C4Id("OBRL")) || (Def->id == C4Id("LBRL"))) {
    // Hardcoded crap...
    C4Def *pDefBarrel = C4Id2Def(C4Id("BARL"));
    if (pDefBarrel)
      iValue = (Def->Value - pDefBarrel->Value) * BarrelFill / MaxBarrelFill + pDefBarrel->Value;
  }
  // Con percentage
  iValue = iValue * Con / FullCon;
  // Dead living
  if ((Category & C4D_Living) && !Alive)
    iValue = 0;
  // Return value
  return iValue;
}

C4PhysicalInfo *C4Object::GetPhysical() {
  // Temporary physical
  if (PhysicalTemporary)
    return &TemporaryPhysical;
  // Info physical
  if (Info)
    return &(Info->Physical);
  // Definition physical
  return &(Def->Physical);
}

BOOL C4Object::Promote(int torank, BOOL exception) {
  if (!Info)
    return FALSE;
  if (!Game.Rank.Name(torank))
    return FALSE;
  Info->Promote(torank, Game.Rank);
  sprintf(OSTR, LoadResStr(IDS_OBJ_PROMOTION), Info->Name, Info->RankName);
  GameMsgObject(OSTR, this);
  SoundEffect("Trumpet", 0, 100, this);
  return TRUE;
}

void C4Object::ClearPointers(C4Object *pObj) {
  // Container
  if (Contained == pObj)
    Contained = NULL;
  // Contents
  while (Contents.Remove(pObj))
    ;
  // Action targets
  if (Action.Target == pObj)
    Action.Target = NULL;
  if (Action.Target2 == pObj)
    Action.Target2 = NULL;
  // Commands
  C4Command *cCom;
  for (cCom = Command; cCom; cCom = cCom->Next)
    cCom->ClearPointers(pObj);
  // Local variable pointers
  for (int cnt = 0; cnt < C4MaxVariable; cnt++)
    if (Local[cnt] == (long)pObj)
      Local[cnt] = 0;
  // Menu
  if (Menu)
    Menu->ClearPointers(pObj);
}

long C4Object::Call(const char *szFunctionCall, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  return Call(NULL, szFunctionCall, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

long C4Object::Call(C4Thread *pCaller, const char *szFunctionCall, long par0, long par1, long par2, long par3, long par4, long par5, long par6, long par7, long par8, long par9) {
  if (!Status || !Def || !szFunctionCall || !szFunctionCall[0])
    return FALSE;
  return Def->Script.ObjectCall(pCaller, this, szFunctionCall, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

BOOL C4Object::SetPhase(int iPhase) {
  if (Action.Act <= ActIdle)
    return FALSE;
  C4ActionDef *actdef = &(Def->ActMap[Action.Act]);
  Action.Phase = BoundBy(iPhase, 0, actdef->Length);
  return TRUE;
}

void C4Object::Draw(C4FacetEx &cgo) {
  C4Facet ccgo;

  // Status
  if (!Status || !Def)
    return;
  // Line
  if (Def->Line) {
    DrawLine(cgo);
    return;
  }

  // Object output position
  int cox, coy;
  cox = x + Shape.x - cgo.TargetX;
  coy = y + Shape.y - cgo.TargetY;

  BOOL fYStretchObject = FALSE;
  if (Action.Act > ActIdle)
    if (Def->ActMap[Action.Act].FacetTargetStretch)
      fYStretchObject = TRUE;

  // Set audibility
  Audible = Max(Audible, BoundBy(100 - 100 * Distance(cgo.TargetX + cgo.Wdt / 2, cgo.TargetY + cgo.Hgt / 2, x, y) / 700, 0, 100));

  // Output boundary
  if (!Inside(cox, 1 - Shape.Wdt, cgo.Wdt) || (!Inside(coy, 1 - Shape.Hgt, cgo.Hgt) && !fYStretchObject))
    return;

  // Debug Display
  // //////////////////////////////////////////////////////////////////////
  if (Game.GraphicsSystem.ShowCommand) {
    C4Command *pCom;
    int ccx = x, ccy = y;
    int x1, y1, x2, y2;
    OSTR[0] = 0;
    char szCommand[200];
    int iMoveTos = 0;
    for (pCom = Command; pCom; pCom = pCom->Next) {
      switch (pCom->Command) {
      case C4CMD_MoveTo:
        // Angle
        int iAngle;
        iAngle = Angle(ccx, ccy, pCom->Tx, pCom->Ty);
        while (iAngle > 180)
          iAngle -= 360;
        // Path
        x1 = ccx - cgo.TargetX;
        y1 = ccy - cgo.TargetY;
        x2 = pCom->Tx - cgo.TargetX;
        y2 = pCom->Ty - cgo.TargetY;
        Engine.DDraw.DrawLine(cgo.Surface, cgo.X + x1, cgo.Y + y1, cgo.X + x2, cgo.Y + y2, CRed);
        Engine.DDraw.DrawFrame(cgo.Surface, cgo.X + x2 - 1, cgo.Y + y2 - 1, cgo.X + x2 + 1, cgo.Y + y2 + 1, CRed);
        if (x1 > x2)
          Swap(x1, x2);
        if (y1 > y2)
          Swap(y1, y2);
        ccx = pCom->Tx;
        ccy = pCom->Ty;
        // Message
        iMoveTos++;
        szCommand[0] = 0;
        // sprintf(szCommand,"%s
        // %d/%d",CommandName(pCom->Command),pCom->Tx,pCom->Ty,iAngle);
        break;
      case C4CMD_Put:
        sprintf(szCommand, "%s %s to %s", CommandName(pCom->Command),
                pCom->Target2 ? pCom->Target2->GetName()
                : pCom->Data  ? C4IdText(pCom->Data)
                              : "Content",
                pCom->Target ? pCom->Target->GetName() : "");
        break;
      case C4CMD_Buy:
      case C4CMD_Sell:
        sprintf(szCommand, "%s %s at %s", CommandName(pCom->Command), C4IdText(pCom->Data), pCom->Target ? pCom->Target->GetName() : "closest base");
        break;
      case C4CMD_Acquire:
        sprintf(szCommand, "%s %s", CommandName(pCom->Command), C4IdText(pCom->Data));
        break;
      case C4CMD_Call:
        sprintf(szCommand, "%s %s in %s", CommandName(pCom->Command), pCom->Text, pCom->Target ? pCom->Target->GetName() : "(null)");
        break;
      case C4CMD_Construct:
        C4Def *pDef;
        pDef = C4Id2Def(pCom->Data);
        sprintf(szCommand, "%s %s", CommandName(pCom->Command), pDef ? pDef->Name : "");
        break;
      case C4CMD_None:
        szCommand[0] = 0;
        break;
      case C4CMD_Transfer:
        // Path
        x1 = ccx - cgo.TargetX;
        y1 = ccy - cgo.TargetY;
        x2 = pCom->Tx - cgo.TargetX;
        y2 = pCom->Ty - cgo.TargetY;
        Engine.DDraw.DrawLine(cgo.Surface, cgo.X + x1, cgo.Y + y1, cgo.X + x2, cgo.Y + y2, CGreen);
        Engine.DDraw.DrawFrame(cgo.Surface, cgo.X + x2 - 1, cgo.Y + y2 - 1, cgo.X + x2 + 1, cgo.Y + y2 + 1, CGreen);
        if (x1 > x2)
          Swap(x1, x2);
        if (y1 > y2)
          Swap(y1, y2);
        ccx = pCom->Tx;
        ccy = pCom->Ty;
        // Message
        sprintf(szCommand, "%s %s", CommandName(pCom->Command), pCom->Target ? pCom->Target->GetName() : "");
        break;
      default:
        sprintf(szCommand, "%s %s", CommandName(pCom->Command), pCom->Target ? pCom->Target->GetName() : "");
        break;
      }
      // Compose command stack message
      if (szCommand[0]) {
        // End MoveTo stack first
        if (iMoveTos) {
          SNewSegment(OSTR, "|");
          sprintf(OSTR + SLen(OSTR), "%dx MoveTo", iMoveTos);
          iMoveTos = 0;
        }
        // Current message
        SNewSegment(OSTR, "|");
        SAppend(szCommand, OSTR);
      }
    }
    // Open MoveTo stack
    if (iMoveTos) {
      SNewSegment(OSTR, "|");
      sprintf(OSTR + SLen(OSTR), "%dx MoveTo", iMoveTos);
      iMoveTos = 0;
    }
    // Draw message
    int cmwdt, cmhgt;
    Engine.DDraw.TextExtent(OSTR, cmwdt, cmhgt);
    Engine.DDraw.TextOut(OSTR, cgo.Surface, cgo.X + cox - Shape.x, cgo.Y + coy - 10 - cmhgt, FWhite, FBlack, ACenter);
  }
  // Debug Display
  // ///////////////////////////////////////////////////////////////////////////////

  // Don't draw (show solidmask)
  if (Game.GraphicsSystem.ShowSolidMask)
    if (SolidMask.Wdt)
      return;

  // Contained check
  if (Contained)
    return;

  // Fire
  if (OnFire) {
    C4Facet fgo;
    // Straight: Full Shape.Rect on fire
    if (r == 0) {
      fgo.Set(cgo.Surface, cgo.X + cox, cgo.Y + coy, Shape.Wdt, Shape.Hgt - Shape.FireTop);
    }
    // Rotated: Reduced fire rect
    else {
      C4Rect fr;
      Shape.GetVertexOutline(fr);
      fgo.Set(cgo.Surface, cgo.X + cox - Shape.x + fr.x, cgo.Y + coy - Shape.y + fr.y, fr.Wdt, fr.Hgt);
    }
    Game.GraphicsResource.fctFire.Draw(fgo, FALSE, FirePhase);
  }

  // Not active or rotated: BaseFace only
  if ((Action.Act <= ActIdle) || (r != 0)) {
    BaseFace.Draw(cgo.Surface, cgo.X + cox, cgo.Y + coy);
  }

  // Active
  else {
    // FacetBase
    if (Def->ActMap[Action.Act].FacetBase)
      BaseFace.Draw(cgo.Surface, cgo.X + cox, cgo.Y + coy, 0, Action.Dir);
    // Facet
    if (Action.Facet.Surface) {
      // Special: stretched action facet
      if (Def->ActMap[Action.Act].FacetTargetStretch) {
        if (Action.Target)
          Action.Facet.DrawX(cgo.Surface, cgo.X + cox + Action.FacetX, cgo.Y + coy + Action.FacetY, Action.Facet.Wdt, (Action.Target->y + Action.Target->Shape.y) - (y + Shape.y + Action.FacetY));
      }
      // Regular action facet
      else {
        // Calculate graphics phase index
        int iPhase = Action.Phase;
        if (Def->ActMap[Action.Act].Reverse)
          iPhase = Def->ActMap[Action.Act].Length - 1 - Action.Phase;
        // Exact fullcon
        if (Con == FullCon)
          Action.Facet.Draw(cgo.Surface, cgo.X + cox + Action.FacetX, cgo.Y + coy + Action.FacetY, iPhase, Action.Dir);
        // Growth strechted
        else
          Action.Facet.DrawX(cgo.Surface, cgo.X + cox, cgo.Y + coy, Shape.Wdt, Shape.Hgt, iPhase, Action.Dir);
      }
    }
  }

  // Select Mark
  if (Select)
    if (ValidPlr(Owner))
      if (Game.Players.Get(Owner)->SelectFlash)
        if (Game.Players.Get(Owner)->LocalControl)
          DrawSelectMark(cgo);

  // Energy shortage
  if (NeedEnergy)
    if (Tick35 > 12) {
      C4Facet &fctEnergy = Game.GraphicsResource.fctEnergy;
      int tx = cox + BaseFace.Wdt / 2 - fctEnergy.Wdt / 2, ty = coy - fctEnergy.Hgt - 5;
      fctEnergy.Draw(cgo.Surface, cgo.X + tx, cgo.Y + ty);
    }

  // Debug Display
  // ////////////////////////////////////////////////////////////////////////
  if (Game.GraphicsSystem.ShowVertices) {
    int cnt;
    if (Shape.VtxNum > 1)
      for (cnt = 0; cnt < Shape.VtxNum; cnt++) {
        DrawVertex(cgo, cox - Shape.x + Shape.VtxX[cnt], coy - Shape.y + Shape.VtxY[cnt], Mobile ? CRed : CYellow, Shape.VtxContactCNAT[cnt]);
      }
  }

  if (Game.GraphicsSystem.ShowEntrance) {
    if (OCF & OCF_Entrance)
      Engine.DDraw.DrawFrame(cgo.Surface, cgo.X + cox - Shape.x + Def->Entrance.x, cgo.Y + coy - Shape.y + Def->Entrance.y, cgo.X + cox - Shape.x + Def->Entrance.x + Def->Entrance.Wdt - 1,
                             cgo.Y + coy - Shape.y + Def->Entrance.y + Def->Entrance.Hgt - 1, CBlue);
    if (OCF & OCF_Collection)
      Engine.DDraw.DrawFrame(cgo.Surface, cgo.X + cox - Shape.x + Def->Collection.x, cgo.Y + coy - Shape.y + Def->Collection.y, cgo.X + cox - Shape.x + Def->Collection.x + Def->Collection.Wdt - 1,
                             cgo.Y + coy - Shape.y + Def->Collection.y + Def->Collection.Hgt - 1, CRed);
  }

  if (Game.GraphicsSystem.ShowAction) {
    if (Action.Act > ActIdle) {
      sprintf(OSTR, "%s (%d)", Def->ActMap[Action.Act].Name, Action.Phase);
      int cmwdt, cmhgt;
      Engine.DDraw.TextExtent(OSTR, cmwdt, cmhgt);
      Engine.DDraw.TextOut(OSTR, cgo.Surface, cgo.X + cox - Shape.x, cgo.Y + coy - cmhgt, InLiquid ? FPlayer + 0 : FWhite, FBlack, ACenter);
    }
  }
  // Debug Display
  // ///////////////////////////////////////////////////////////////////////
}

void C4Object::DrawTopFace(C4FacetEx &cgo) {
  // Status
  if (!Status || !Def)
    return;
  // TopFace
  if (!(TopFace.Surface || (OCF & OCF_Construct)))
    return;
  // Output position
  int cox, coy;
  cox = x + Shape.x - cgo.TargetX;
  coy = y + Shape.y - cgo.TargetY;
  // Output bounds check
  if (!Inside(cox, 1 - Shape.Wdt, cgo.Wdt) || !Inside(coy, 1 - Shape.Hgt, cgo.Hgt))
    return;
  // Don't draw (show solidmask)
  if (Game.GraphicsSystem.ShowSolidMask)
    if (SolidMask.Wdt)
      return;
  // Contained
  if (Contained)
    return;
  // Construction sign
  if (OCF & OCF_Construct)
    if (r == 0) {
      C4Facet &fctConSign = Game.GraphicsResource.fctConSign;
      fctConSign.Draw(cgo.Surface, cgo.X + cox, cgo.Y + coy + Shape.Hgt - fctConSign.Hgt);
    }
  // FacetTopFace: Override TopFace.x/y
  if ((Action.Act > ActIdle) && Def->ActMap[Action.Act].FacetTopFace) {
    C4ActionDef *actdef = &Def->ActMap[Action.Act];
    TopFace.X = actdef->Facet.x + Def->TopFace.x + actdef->Facet.Wdt * Action.Phase;
    TopFace.Y = actdef->Facet.y + Def->TopFace.y + actdef->Facet.Hgt * Action.Dir;
  }
  // Draw top face bitmap
  TopFace.Draw(cgo.Surface, cgo.X + cox + Def->TopFace.tx, cgo.Y + coy + Def->TopFace.ty);
}

void C4Object::DrawLine(C4FacetEx &cgo) {
  // Lines are fully audible
  Audible = 100;
  // Draw line segments
  for (int vtx = 0; vtx + 1 < Shape.VtxNum; vtx++)
    switch (Def->Line) {
    case C4D_Line_Power:
      cgo.DrawLine(Shape.VtxX[vtx], Shape.VtxY[vtx], Shape.VtxX[vtx + 1], Shape.VtxY[vtx + 1], 68, 26);
      break;
    case C4D_Line_Source:
    case C4D_Line_Drain:
      cgo.DrawLine(Shape.VtxX[vtx], Shape.VtxY[vtx], Shape.VtxX[vtx + 1], Shape.VtxY[vtx + 1], 23, 26);
      break;
    case C4D_Line_Lightning:
      cgo.DrawBolt(Shape.VtxX[vtx], Shape.VtxY[vtx], Shape.VtxX[vtx + 1], Shape.VtxY[vtx + 1], CWhite);
      break;
    case C4D_Line_Rope:
      cgo.DrawLine(Shape.VtxX[vtx], Shape.VtxY[vtx], Shape.VtxX[vtx + 1], Shape.VtxY[vtx + 1], 65, 65);
      break;
    case C4D_Line_Vertex:
    case C4D_Line_Colored:
      cgo.DrawLine(Shape.VtxX[vtx], Shape.VtxY[vtx], Shape.VtxX[vtx + 1], Shape.VtxY[vtx + 1], Local[0], Local[1]);
      break;
    }
}

void C4Object::DrawEnergy(C4Facet &cgo) { cgo.DrawEnergyLevel(Energy, GetPhysical()->Energy); }

void C4Object::DrawMagicEnergy(C4Facet &cgo) { cgo.DrawEnergyLevel(MagicEnergy, GetPhysical()->Magic, 39); }

void C4Object::DrawBreath(C4Facet &cgo) { cgo.DrawEnergyLevel(Breath, GetPhysical()->Breath, 99); }

BOOL C4Object::Compile(const char *szSource) {
  // Compile source code
  C4Compiler Compiler;
  Default();

  // Detect-no-compile invalid default
  SolidMask.Set(-1, -1, -1, -1, -1, -1);

  // Compile
  Compiler.CompileStructure(C4CR_Object, szSource, this);

  // Read commands
  const char *cPos;
  C4Command *pCom, *pPrev = NULL;
  if (cPos = SSearch(szSource, "[Commands]"))
    for (cPos = SAdvancePast(cPos, 0x0A); SEqual2(cPos, "Command"); cPos = SAdvancePast(cPos, 0x0A)) {
      pCom = new C4Command;
      pCom->cObj = this;
      if (pPrev)
        pPrev->Next = pCom;
      else
        Command = pCom;
      pPrev = pCom;
      pCom->Read(cPos);
    }

  // Set def
  if (!(Def = Game.Defs.ID2Def(id))) {
    // Undefined: message
    if (Config.Graphics.VerboseObjectLoading >= 1) {
      sprintf(OSTR, LoadResStr(IDS_PRC_UNDEFINEDOBJECT), C4IdText(id));
      Log(OSTR);
    }
    // Compile failure
    return FALSE;
  }

  // Set no-compile defaults (previous versions)
  if ((SolidMask.x == -1) && (SolidMask.y == -1) && (SolidMask.Wdt == -1) && (SolidMask.Hgt == -1) && (SolidMask.tx == -1) && (SolidMask.ty == -1))
    SolidMask = Def->SolidMask;

  // Set action (override running data)
  int iTime = Action.Time;
  int iPhase = Action.Phase;
  int iPhaseDelay = Action.PhaseDelay;
  if (SetActionByName(Action.Name, 0, 0, FALSE)) {
    Action.Time = iTime;
    Action.Phase = iPhase; // No checking for valid phase
    Action.PhaseDelay = iPhaseDelay;
  }

  // Success
  return TRUE;
}

BOOL C4Object::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;
  C4Object dC4O;
  // Invalid default to enforce compilation
  dC4O.SolidMask.Set(-1, -1, -1, -1, -1, -1);
  // Write object core and physicals
  if (!Compiler.DecompileStructure(C4CR_Object, this, &dC4O, ppOutput, ipSize))
    return FALSE;
  // Append commands (assume no-own buffer, compiler did not append eof)
  if (Command) {
    SAppend(LineFeed, *ppOutput);
    SAppend("[Commands]", *ppOutput);
    SAppend(LineFeed, *ppOutput);
    char *cPos = *ppOutput + SLen(*ppOutput);
    int iCom = 1;
    for (C4Command *pCom = Command; pCom; pCom = pCom->Next) {
      pCom->Write(cPos, iCom++);
      SAppend(LineFeed, *ppOutput);
      cPos += SLen(cPos);
    }
  }
  // Success
  return TRUE;
}

void C4Object::EnumeratePointers() {

  // Standard enumerated pointers
  nContained = Game.Objects.ObjectNumber(Contained);
  nActionTarget1 = Game.Objects.ObjectNumber(Action.Target);
  nActionTarget2 = Game.Objects.ObjectNumber(Action.Target2);

  // Info by name
  if (Info)
    SCopy(Info->Name, nInfo, C4MaxName);

  // Local variable pointers
  int cnt, iPtrNum;
  for (cnt = 0; cnt < C4MaxVariable; cnt++)
    if (iPtrNum = Game.Objects.ObjectNumber((C4Object *)Local[cnt]))
      Local[cnt] = C4EnumPointer1 + iPtrNum;

  // Commands
  for (C4Command *pCom = Command; pCom; pCom = pCom->Next)
    pCom->EnumeratePointers();
}

void C4Object::DenumeratePointers() {

  // Standard enumerated pointers
  Contained = Game.Objects.ObjectPointer(nContained);
  Action.Target = Game.Objects.ObjectPointer(nActionTarget1);
  Action.Target2 = Game.Objects.ObjectPointer(nActionTarget2);

  // Post-compile object list
  Contents.DenumerateRead();

  // Local variable pointers
  for (int cnt = 0; cnt < C4MaxVariable; cnt++)
    // Looks like pointer
    if (Inside(Local[cnt], C4EnumPointer1, C4EnumPointer2)) {
      // Denumerate
      C4Object *pObj = Game.Objects.ObjectPointer(Local[cnt] - C4EnumPointer1);
      // Pointer ok, set it.
      if (pObj)
        Local[cnt] = (long)pObj;
      // Hm, it's not a valid pointer
      else
        // Could it be a C4ID? If yes, leave it. If no, zero it.
        if (!LooksLikeID(C4IdText(Local[cnt])))
          Local[cnt] = 0;
    }
  // Notice that id/pointer misinterpretation in variables will lead to
  // synchronization loss in network games.

  // Commands
  for (C4Command *pCom = Command; pCom; pCom = pCom->Next)
    pCom->DenumeratePointers();
}

void C4Object::DrawCommands(C4Facet &cgoBottom, C4Facet &cgoSide, C4RegionList *pRegions) {

  int cnt;
  C4Facet ccgo, ccgo2;
  C4Object *tObj;
  int iDFA = GetProcedure();
  BOOL fContainedDownOverride = FALSE;
  BOOL fContentsActivationOverride = FALSE;
  C4FacetEx fctImage;
  fctImage.Create(C4SymbolSize, C4SymbolSize);

  // Active menu (does not consider owner's active player menu)
  if (Menu && Menu->IsActive())
    return;

  // Grab target control (control flag)
  if (iDFA == DFA_PUSH)
    if (tObj = Action.Target)
      for (cnt = ComOrderNum - 1; cnt >= 0; cnt--)
        if (tObj->Def->ControlFlag & (1 << cnt))
          tObj->DrawCommand(cgoBottom, C4FCT_Right, PSF_Control, ComOrder(cnt), pRegions, Owner);

  // Contained control (contained control flag)
  if (tObj = Contained)
    for (cnt = ComOrderNum - 1; cnt >= 0; cnt--)
      if (tObj->Def->ContainedControlFlag & (1 << cnt)) {
        tObj->DrawCommand(cgoBottom, C4FCT_Right, PSF_ContainedControl, ComOrder(cnt), pRegions, Owner);
        // Contained control down overrides contained exit control
        if (Com2Control(ComOrder(cnt)) == CON_Down)
          fContainedDownOverride = TRUE;
      }

  // Contained exit
  if (Contained)
    if (!fContainedDownOverride) {
      SCopy(LoadResStr(IDS_CON_EXIT), OSTR);
      C4FacetEx fctE = Game.GraphicsResource.fctEntry.GetPhase(2);
      DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Down, pRegions, Owner, OSTR, &fctE);
    }

  // Contained base commands
  if (Contained && ValidPlr(Contained->Base)) {
    // Sell
    SCopy(LoadResStr(IDS_CON_SELL), OSTR);
    fctImage.Wipe();
    DrawMenuSymbol(C4MN_Sell, fctImage, Contained->Base, Contained);
    Contained->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Dig, pRegions, Owner, OSTR, &fctImage);
    // Buy
    SCopy(LoadResStr(IDS_CON_BUY), OSTR);
    fctImage.Wipe();
    DrawMenuSymbol(C4MN_Buy, fctImage, Contained->Base, Contained);
    Contained->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Up, pRegions, Owner, OSTR, &fctImage);
  }

  // Contained put & activate
  if (Contained)
    if (Contents.GetObject()) {
      // Put
      sprintf(OSTR, LoadResStr(IDS_CON_PUT), Contents.GetObject()->GetName(), Contained->GetName());
      fctImage.Wipe();
      Contents.GetObject()->Def->Draw(fctImage);
      Contained->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Throw, pRegions, Owner, OSTR, &fctImage);
    } else if (Contained->Contents.ListIDCount(C4D_Get)) {
      // Get
      sprintf(OSTR, LoadResStr(IDS_CON_ACTIVATEFROM), Contained->GetName());
      Contained->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Throw, pRegions, Owner, OSTR);
    }

  // Grab put & get
  if ((iDFA == DFA_PUSH) && Action.Target)
    if (Contents.GetObject() && (Action.Target->Def->GrabPutGet & C4D_Grab_Put)) {
      // Put
      sprintf(OSTR, LoadResStr(IDS_CON_PUT), Contents.GetObject()->GetName(), Action.Target->GetName());
      fctImage.Wipe();
      Contents.GetObject()->Def->Draw(fctImage);
      Action.Target->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Throw, pRegions, Owner, OSTR, &fctImage);
    } else if (Action.Target->Contents.ListIDCount(C4D_Get) && (Action.Target->Def->GrabPutGet & C4D_Grab_Get)) {
      // Get
      sprintf(OSTR, LoadResStr(IDS_CON_GET), Action.Target->GetName());
      Action.Target->DrawCommand(cgoBottom, C4FCT_Right, NULL, COM_Throw, pRegions, Owner, OSTR);
    }

  // Contents activation (activation control flag)
  if (!Contained)
    if ((iDFA == DFA_WALK) || (iDFA == DFA_SWIM) || (iDFA == DFA_DIG))
      if (tObj = Contents.GetObject())
        if (tObj->Def->ActivationControlFlag) {
          tObj->DrawCommand(cgoBottom, C4FCT_Right, PSF_Activate, COM_Dig_D, pRegions, Owner);
          // Flag override self-activation
          fContentsActivationOverride = TRUE;
        }

  // Self activation (activation control flag)
  if (!fContentsActivationOverride)
    if (!Contained)
      if ((iDFA == DFA_WALK) || (iDFA == DFA_SWIM))
        if (Def->ActivationControlFlag)
          DrawCommand(cgoSide, C4FCT_Bottom | C4FCT_Half, PSF_Activate, COM_Dig_D, pRegions, Owner);

  // Self special control (control flag)
  for (cnt = 6; cnt < ComOrderNum; cnt++) {
    // Hardcoded com order indexes for COM_Specials
    if (cnt == 8)
      cnt = 14;
    if (cnt == 16)
      cnt = 22;
    // Control in control flag?
    if (Def->ControlFlag & (1 << cnt))
      DrawCommand(cgoSide, C4FCT_Bottom | C4FCT_Half, PSF_Control, ComOrder(cnt), pRegions, Owner);
  }
}

void C4Object::DrawPicture(C4Facet &cgo, BOOL fSelected, C4RegionList *pRegions) {
  // Draw def picture with object color
  Def->Draw(cgo, fSelected, Color);
  // Region
  if (pRegions)
    pRegions->Add(cgo.X, cgo.Y, cgo.Wdt, cgo.Hgt, GetName(), COM_None, this);
}

BOOL C4Object::ValidateOwner() {
  // Check owner and controller
  if (!ValidPlr(Owner))
    Owner = NO_OWNER;
  if (!ValidPlr(Base))
    Base = NO_OWNER;
  if (!ValidPlr(Controller))
    Controller = NO_OWNER;
  // Check color
  if (!Def->ColorByOwner)
    Color = 0;
  else if (ValidPlr(Owner))
    Color = Game.Players.Get(Owner)->Color;
  return TRUE;
}

BOOL C4Object::AssignInfo() {
  if (Info)
    return FALSE;
  // Info object: make player crew
  if (nInfo[0])
    if (ValidPlr(Owner))
      Game.Players.Get(Owner)->MakeCrewMember(this);
  // Info updates
  if (Info) {
    // Dead and gone (info flags, remove from crew/cursor)
    if (!Alive) {
      Info->HasDied = TRUE;
      if (ValidPlr(Owner))
        Game.Players.Get(Owner)->ClearPointers(this);
    }
  }
  return TRUE;
}

void C4Object::ClearInfo(C4ObjectInfo *pInfo) {
  if (Info == pInfo)
    Info = NULL;
}

void C4Object::Clear() {
  if (cBitmap)
    DestroySurface(cBitmap);
  cBitmap = NULL;
  if (cSolidMask)
    DestroySurface(cSolidMask);
  cSolidMask = NULL;
  if (Menu)
    delete Menu;
  Menu = NULL;
  if (MaterialContents)
    delete MaterialContents;
  MaterialContents = NULL;
}

BOOL C4Object::ContainedControl(BYTE byCom) {
  // Check
  if (!Contained)
    return FALSE;
  // Exit, activate, buy, sell
  switch (byCom) {
  case COM_Down:
    PlayerObjectCommand(Owner, C4CMD_Exit);
    break;
  case COM_Throw:
    PlayerObjectCommand(Owner, C4CMD_Throw);
    break;
  case COM_Up:
    if (ValidPlr(Contained->Base))
      if (!Hostile(Owner, Contained->Base))
        ActivateMenu(C4MN_Buy);
    break;
  case COM_Dig:
    if (ValidPlr(Contained->Base))
      if (!Hostile(Owner, Contained->Base))
        ActivateMenu(C4MN_Sell);
    break;
  }
  // Call container script
  sprintf(OSTR, PSF_ContainedControl, ComName(byCom));
  Contained->Call(OSTR, (long)this);
  // Success
  return TRUE;
}

int C4Object::CallControl(BYTE byCom, int par0, int par1, int par2, int par3, int par4, int par5, int par6, int par7, int par8, int par9) {
  sprintf(OSTR, PSF_Control, ComName(byCom));
  return Call(OSTR, par0, par1, par2, par3, par4, par5, par6, par7, par8, par9);
}

void C4Object::DirectCom(BYTE byCom, int iData) // By player ObjectCom
{

  // ObjectMenu control
  if (Menu && Menu->Control(byCom, iData))
    return;

  // Ignore any menu com leftover in control queue from closed menu
  if (Inside(byCom, COM_MenuNavigation1, COM_MenuNavigation2))
    return;

  // Control count / experience
  if (Info) {
    Info->ControlCount++;
    if ((Info->ControlCount % 5) == 0)
      DoExperience(+1);
  }

  // Decrease NoCollectDelay
  if (!(byCom & COM_Single) && !(byCom & COM_Double))
    if (NoCollectDelay > 0)
      NoCollectDelay--;

  // COM_Contents contents shift (data is target id)
  if (byCom == COM_Contents) {
    // No object in contents
    if (!Contents.GetObject())
      return;
    // Desired id not in contents
    if (!Contents.Find(iData))
      return;
    // Desired id already at front
    if (Contents.GetObject()->id == (C4ID)iData)
      return;
    // Move first object to tail until desired id is at front
    while (Contents.GetObject()->id != (C4ID)iData) {
      C4Object *pMove = Contents.GetObject();
      if (!pMove)
        return;
      Contents.Remove(pMove);
      Contents.Add(pMove, FALSE);
    }
    // Selection sound
    if (!Contents.GetObject()->Call("~Selection"))
      SoundEffect("Grab", 0, 100, this);
    // Done
    return;
  }

  // Contained control (except specials - hey, doesn't catch singles or doubles)
  if (Contained)
    if ((byCom != COM_Special) && (byCom != COM_Special2)) {
      ContainedControl(byCom);
      return;
    }

  // Regular DirectCom clears commands
  if (!(byCom & COM_Single) && !(byCom & COM_Double))
    ClearCommands();

  // Object script override
  if (CallControl(byCom))
    return;

  // Control by procedure
  switch (GetProcedure()) {
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_WALK:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Down:
      ObjectComMovement(this, COMD_Stop);
      break;
    case COM_Up:
      ObjectComUp(this);
      break;
    case COM_Down_D:
      ObjectComDownDouble(this);
      break;
    case COM_Dig_S:
      ObjectComDig(this);
      break;
    case COM_Dig_D:
      ObjectComDigDouble(this);
      break;
    case COM_Throw:
      PlayerObjectCommand(Owner, C4CMD_Throw);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_FLIGHT:
  case DFA_KNEEL:
  case DFA_THROW:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Down:
      ObjectComMovement(this, COMD_Stop);
      break;
    case COM_Throw:
      PlayerObjectCommand(Owner, C4CMD_Throw);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_SCALE:
    switch (byCom) {
    case COM_Left:
      if (Action.Dir == DIR_Left)
        ObjectComMovement(this, COMD_Stop);
      else
        ObjectComLetGo(this, -1);
      break;
    case COM_Right:
      if (Action.Dir == DIR_Left)
        ObjectComLetGo(this, +1);
      else
        ObjectComMovement(this, COMD_Stop);
      break;
    case COM_Up:
      ObjectComMovement(this, COMD_Up);
      break;
    case COM_Down:
      ObjectComMovement(this, COMD_Down);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_HANGLE:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Up:
      ObjectComMovement(this, COMD_Stop);
      break;
    case COM_Down:
      ObjectComLetGo(this, 0);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_DIG:
    switch (byCom) {
    case COM_Left:
      if (Inside(Action.ComDir, COMD_UpRight, COMD_Left))
        Action.ComDir++;
      break;
    case COM_Right:
      if (Inside(Action.ComDir, COMD_Right, COMD_UpLeft))
        Action.ComDir--;
      break;
    case COM_Down:
      ObjectComStop(this);
      break;
    case COM_Dig_D:
      ObjectComDigDouble(this);
      break;
    case COM_Dig_S:
      Action.Data = (!Action.Data);
      break; // Dig mat 2 object request
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_SWIM:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Up:
      ObjectComMovement(this, COMD_Up);
      ObjectComUp(this);
      break;
    case COM_Down:
      ObjectComMovement(this, COMD_Down);
      break;
    case COM_Throw:
      PlayerObjectCommand(Owner, C4CMD_Throw);
      break;
    case COM_Dig_D:
      ObjectComDigDouble(this);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_BRIDGE:
    switch (byCom) {
    case COM_Down:
      ObjectComStop(this);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_BUILD:
    switch (byCom) {
    case COM_Down:
      ObjectComStop(this);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_FIGHT:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Down:
      ObjectComStop(this);
      break;
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_PUSH:
    switch (byCom) {
    case COM_Left:
      ObjectComMovement(this, COMD_Left);
      break;
    case COM_Right:
      ObjectComMovement(this, COMD_Right);
      break;
    case COM_Up:
      // Target -> enter
      if (ObjectComEnter(Action.Target))
        ObjectComMovement(this, COMD_Stop);
      // Else, comdir up for target straightening
      else
        ObjectComMovement(this, COMD_Up);
      break;
    case COM_Down:
      ObjectComMovement(this, COMD_Stop);
      break;
    case COM_Down_D:
      PlayerObjectCommand(Owner, C4CMD_UnGrab);
      break;
    case COM_Throw:
      PlayerObjectCommand(Owner, C4CMD_Throw);
      break;
    }
    // Action target call control
    if (Action.Target)
      Action.Target->CallControl(byCom, (long)this);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - - - - -
  case DFA_CHOP:
    switch (byCom) {
    case COM_Down:
      ObjectComStop(this);
      break;
    }
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - - - -
  }
}

BOOL C4Object::MenuCommand(const char *szCommand) {
  // Native script execution
  char szScript[1000];
  // Create valid statement
  sprintf(szScript, "return(%s);", szCommand);
  // Execute
  C4Thread cthr;
  return cthr.Execute(NULL, szScript, NULL, NULL, this);
}

C4Object *C4Object::ComposeContents(C4ID id) {
  int cnt, cnt2;
  C4ID c_id;
  BOOL fInsufficient = FALSE;
  C4Object *pObj;
  C4ID idNeeded = C4ID_None;
  int iNeeded = 0;
  // Get def
  C4Def *pDef = C4Id2Def(id);
  if (!pDef)
    return NULL;
  // Check for sufficient components
  sprintf(OSTR, LoadResStr(IDS_OBJ_NEEDS), pDef->Name);
  for (cnt = 0; c_id = pDef->Component.GetID(cnt); cnt++)
    if (pDef->Component.GetCount(cnt) > Contents.ObjectCount(c_id)) {
      sprintf(OSTR + SLen(OSTR), "|%ix %s", pDef->Component.GetCount(cnt) - Contents.ObjectCount(c_id), C4Id2Def(c_id) ? C4Id2Def(c_id)->Name : C4IdText(c_id));
      if (!idNeeded) {
        idNeeded = c_id;
        iNeeded = pDef->Component.GetCount(cnt) - Contents.ObjectCount(c_id);
      }
      fInsufficient = TRUE;
    }
  // Insufficient
  if (fInsufficient) {
    // BuildNeedsMaterial call to object...
    if (!Call(PSF_BuildNeedsMaterial, idNeeded, iNeeded))
      // ...game message if not overloaded
      GameMsgObject(OSTR, this);
    // Return
    return NULL;
  }
  // Remove components
  for (cnt = 0; c_id = pDef->Component.GetID(cnt); cnt++)
    for (cnt2 = 0; cnt2 < pDef->Component.GetCount(cnt); cnt2++)
      if (!(pObj = Contents.Find(c_id)))
        return NULL;
      else
        pObj->AssignRemoval();
  // Create composed object
  return CreateContents(id);
}

void C4Object::SetSolidMask(int iX, int iY, int iWdt, int iHgt, int iTX, int iTY) {
  RemoveSolidMask();
  DestroySolidMask();
  SolidMask.Set(iX, iY, iWdt, iHgt, iTX, iTY);
  UpdateSolidMask();
}

void C4Object::SyncClearance() {
  // Misc. no-save safeties
  Action.t_attach = CNAT_None;
  InMat = MNone;
  t_contact = 0;
  // Fixed point values - precision reduction
  fix_x = itofix(x);
  fix_y = itofix(y);
  fix_r = itofix(r);
  // Update OCF
  SetOCF();
  // Menu
  CloseMenu();
  // Material contents
  if (MaterialContents)
    delete MaterialContents;
  MaterialContents = NULL;
}

void C4Object::DrawSelectMark(C4FacetEx &cgo) {
  // Status
  if (!Status)
    return;
  // Output boundary
  if (!Inside(x - cgo.TargetX, 0, cgo.Wdt - 1) || !Inside(y - cgo.TargetY, 0, cgo.Hgt - 1))
    return;
  // Draw select marks
  int cox = x + Shape.x - cgo.TargetX + cgo.X - 2;
  int coy = y + Shape.y - cgo.TargetY + cgo.Y - 2;
  GfxR->fctSelectMark.Draw(cgo.Surface, cox, coy, 0);
  GfxR->fctSelectMark.Draw(cgo.Surface, cox + BaseFace.Wdt, coy, 1);
  GfxR->fctSelectMark.Draw(cgo.Surface, cox, coy + BaseFace.Hgt, 2);
  GfxR->fctSelectMark.Draw(cgo.Surface, cox + BaseFace.Wdt, coy + BaseFace.Hgt, 3);
}

void C4Object::ClearCommands() {
  C4Command *pNext;
  while (Command) {
    pNext = Command->Next;
    delete Command;
    Command = pNext;
  }
}

void C4Object::ClearCommand(C4Command *pUntil) {
  C4Command *pCom, *pNext;
  for (pCom = Command; pCom; pCom = pNext) {
    // Last one to clear
    if (pCom == pUntil)
      pNext = NULL;
    // Next one to clear after this
    else
      pNext = pCom->Next;
    Command = pCom->Next;
    delete pCom;
  }
}

BOOL C4Object::AddCommand(int iCommand, C4Object *pTarget, int iTx, int iTy, int iUpdateInterval, C4Object *pTarget2, BOOL fInitEvaluation, int iData, BOOL fAppend, int iRetries, const char *szText) {
  // Command stack size safety
  const int MaxCommandStack = 35;
  C4Command *pCom, *pLast;
  int iCommands;
  for (pCom = Command, iCommands = 0; pCom; pCom = pCom->Next, iCommands++)
    ;
  if (iCommands >= MaxCommandStack)
    return FALSE;
  // Valid command safety
  if (!Inside(iCommand, C4CMD_First, C4CMD_Last))
    return FALSE;
  // Allocate and set new command
  if (!(pCom = new C4Command))
    return FALSE;
  pCom->Set(iCommand, this, pTarget, iTx, iTy, pTarget2, iData, iUpdateInterval, !fInitEvaluation, iRetries, szText);
  // Append to bottom of stack
  if (fAppend) {
    for (pLast = Command; pLast && pLast->Next; pLast = pLast->Next)
      ;
    if (pLast)
      pLast->Next = pCom;
    else
      Command = pCom;
  }
  // Add to top of command stack
  else {
    pCom->Next = Command;
    Command = pCom;
  }
  // Success
  // sprintf(OSTR,"%s command %s added: %i/%i %s %s
  // (%i)",GetName(),CommandName(iCommand),iTx,iTy,pTarget ? pTarget->GetName()
  // : "O",pTarget2 ? pTarget2->GetName() : "O",iUpdateInterval); Log(OSTR);
  return TRUE;
}

void C4Object::SetCommand(int iCommand, C4Object *pTarget, int iTx, int iTy, C4Object *pTarget2, BOOL fControl, int iData, int iRetries, const char *szText) {
  // Decrease NoCollectDelay
  if (NoCollectDelay > 0)
    NoCollectDelay--;
  // Clear stack
  ClearCommands();
  // Close menu
  if (fControl)
    CloseMenu();
  // Script overload
  if (fControl)
    if (Call(PSF_ControlCommand, (long)CommandName(iCommand), (long)pTarget, iTx, iTy, (long)pTarget2, iData))
      return;
  // Inside vehicle control overload
  if (Contained)
    if (Contained->Def->VehicleControl & C4D_VehicleControl_Inside) {
      Contained->Controller = Controller;
      if (Contained->Call(PSF_ControlCommand, (long)CommandName(iCommand), (long)pTarget, iTx, iTy, (long)pTarget2, iData))
        return;
    }
  // Outside vehicle control overload
  if (GetProcedure() == DFA_PUSH)
    if (Action.Target)
      if (Action.Target->Def->VehicleControl & C4D_VehicleControl_Outside) {
        Action.Target->Controller = Controller;
        if (Action.Target->Call(PSF_ControlCommand, (long)CommandName(iCommand), (long)pTarget, iTx, iTy, (long)pTarget2, iData))
          return;
      }
  // Add new command
  AddCommand(iCommand, pTarget, iTx, iTy, 0, pTarget2, TRUE, iData, FALSE, iRetries, szText);
}

BOOL C4Object::ExecuteCommand() {
  // Execute first command
  if (Command)
    Command->Execute();
  // Command finished: engine call
  if (Command && Command->Finished)
    Call(PSF_ControlCommandFinished, (long)CommandName(Command->Command), (long)Command->Target, Command->Tx, Command->Ty, (long)Command->Target2, Command->Data);
  // Clear finished commands
  while (Command && Command->Finished)
    ClearCommand(Command);
  // Done
  return TRUE;
}

void C4Object::AddMaterialContents(int iMaterial, int iAmount) {
  // Create contents list if necessary
  if (!MaterialContents)
    MaterialContents = new C4MaterialList;
  // Add amount
  MaterialContents->Add(iMaterial, iAmount);
}

void C4Object::DigOutMaterialCast(BOOL fRequest) {
  // Check material contents for sufficient object cast amounts
  if (!MaterialContents)
    return;
  for (int iMaterial = 0; iMaterial < Game.Material.Num; iMaterial++)
    if (MaterialContents->Amount[iMaterial])
      if (Game.Material.Map[iMaterial].Dig2Object != C4ID_None)
        if (Game.Material.Map[iMaterial].Dig2ObjectRatio != 0)
          if (fRequest || !Game.Material.Map[iMaterial].Dig2ObjectOnRequestOnly)
            if (MaterialContents->Amount[iMaterial] >= Game.Material.Map[iMaterial].Dig2ObjectRatio) {
              Game.CreateObject(Game.Material.Map[iMaterial].Dig2Object, NO_OWNER, x, y + Shape.y + Shape.Hgt, Random(360));
              MaterialContents->Amount[iMaterial] = 0;
            }
}

void C4Object::UpdateAudible() {
  // Timing
  if (Tick5)
    return;
  // Determine audible change
  int iAudibleChange;
  iAudibleChange = Audible - LastAudible;
  LastAudible = Audible;
  // Update looping sounds
  if (iAudibleChange) {
    // Act sound
    if (Action.Act > ActIdle)
      if (Def->ActMap[Action.Act].Sound[0])
        SoundEffect(Def->ActMap[Action.Act].Sound, 2, iAudibleChange, NULL);
    // Fire sound
    if (OnFire)
      if (Def->Mass >= 100)
        SoundEffect("Fire", 2, iAudibleChange, NULL);
  }
}

void C4Object::DrawCommand(C4Facet &cgoBar, int iAlign, const char *szFunctionFormat, int iCom, C4RegionList *pRegions, int iPlayer, const char *szDesc, C4Facet *pfctImage) {
  const char *cpDesc = szDesc;
  C4ID idDescImage = id;
  C4Def *pDescImageDef;
  C4Facet cgoLeft, cgoRight;
  BOOL fFlash = FALSE;

  // Flash
  C4Player *pPlr;
  if (pPlr = Game.Players.Get(Owner))
    if (iCom == pPlr->FlashCom)
      fFlash = TRUE;

  // Get desc from def script function desc
  if (szFunctionFormat)
    cpDesc = Def->Script.GetControlDesc(szFunctionFormat, iCom, &idDescImage);

  // Image def by id
  pDescImageDef = C4Id2Def(idDescImage);

  // Symbol sections
  cgoRight = cgoBar.TruncateSection(iAlign);
  if (!cgoRight.Wdt)
    return;
  if (iAlign & C4FCT_Bottom)
    cgoLeft = cgoRight.TruncateSection(C4FCT_Left);
  else
    cgoLeft = cgoBar.TruncateSection(iAlign);
  if (!cgoLeft.Wdt)
    return;

  // Flash background
  // if (fFlash)
  //	Engine.DDraw.DrawBox(cgoLeft.Surface,cgoLeft.X-1,cgoLeft.Y-1,cgoLeft.X+cgoLeft.Wdt*2,cgoLeft.Y+cgoLeft.Hgt,CRed);

  // Image
  if (pfctImage)
    pfctImage->Draw(cgoRight);
  else if (pDescImageDef)
    pDescImageDef->Draw(cgoRight);
  else
    DrawPicture(cgoRight);

  // Command
  if (!fFlash || Tick35 > 15)
    DrawCommandKey(cgoLeft, iCom, FALSE, Config.Graphics.ShowCommandKeys ? PlrControlKeyName(iPlayer, Com2Control(iCom)) : NULL);

  // Region (both symbols)
  if (pRegions)
    pRegions->Add(cgoLeft.X, cgoLeft.Y, cgoLeft.Wdt * 2, cgoLeft.Hgt, cpDesc ? cpDesc : GetName(), iCom);
}

void C4Object::Resort() {
  // Flag resort
  Unsorted = TRUE;
  // Must not immediately resort - link change/removal would crash
  // Game::ExecObjects
}

BOOL C4Object::SetAction(int iAct, C4Object *pTarget, C4Object *pTarget2, BOOL fStartCall) {
  int iLastAction = Action.Act;
  C4ActionDef *pAction;

  // Def lost actmap: idle (safety)
  if (!Def->ActMap)
    iLastAction = ActIdle;

  // No other action
  if (iLastAction > ActIdle)
    if (Def->ActMap[iLastAction].NoOtherAction)
      if (iAct != iLastAction)
        return FALSE;

  // Invalid action
  if (Def && !Inside(iAct, ActIdle, Def->ActNum - 1))
    return FALSE;

  // Stop previous act sound
  if (iLastAction > ActIdle)
    if (iAct != iLastAction)
      if (Def->ActMap[iLastAction].Sound[0])
        SoundEffect(Def->ActMap[iLastAction].Sound, -1, 100, this);

  // Unfullcon objects no action
  if (Con < FullCon)
    if (!Def->IncompleteActivity)
      return FALSE;

  // Reset action time on change
  if (iAct != iLastAction)
    Action.Time = 0;

  // Set new action
  Action.Act = iAct;
  ZeroMem(Action.Name, C4D_MaxIDLen + 1);
  if (Action.Act > ActIdle)
    SCopy(Def->ActMap[Action.Act].Name, Action.Name);
  Action.Phase = Action.PhaseDelay = 0;

  // Set target if specified
  if (pTarget)
    Action.Target = pTarget;
  if (pTarget2)
    Action.Target2 = pTarget2;

  // Set Action Facet
  UpdateActionFace();

  // Start act sound
  if (Action.Act > ActIdle)
    if (Action.Act != iLastAction)
      if (Def->ActMap[Action.Act].Sound[0])
        SoundEffect(Def->ActMap[Action.Act].Sound, +1, 100, this);

  // Reset OCF
  SetOCF();

  // Reset fixed position...
  fix_x = itofix(x);
  fix_y = itofix(y);

  // Execute StartCall
  if (fStartCall)
    if (Action.Act > ActIdle) {
      pAction = &(Def->ActMap[Action.Act]);
      if (pAction->StartCall[0]) {
        C4Thread cthr;
        cthr.cObj = this;
        sprintf(cthr.Function, "%s StartCall", pAction->Name);
        Call(&cthr, pAction->StartCall);
      }
    }

  return TRUE;
}

void C4Object::UpdateActionFace() {
  // Default: no action face
  Action.Facet.Default();
  // Active: get action facet from action definition
  if (Action.Act > ActIdle) {
    C4ActionDef *pAction = &(Def->ActMap[Action.Act]);
    if (pAction->Facet.Wdt > 0) {
      Action.Facet.Set(Def->Bitmap[Color], pAction->Facet.x, pAction->Facet.y, pAction->Facet.Wdt, pAction->Facet.Hgt);
      Action.FacetX = pAction->Facet.tx;
      Action.FacetY = pAction->Facet.ty;
    }
  }
}

BOOL C4Object::SetActionByName(const char *szActName, C4Object *pTarget, C4Object *pTarget2, BOOL fStartCall) {
  int cnt;
  // Check for ActIdle passed by name
  if (SEqual(szActName, "ActIdle") || SEqual(szActName, "Idle"))
    return SetAction(ActIdle);
  // Find act in ActMap of object
  for (cnt = 0; cnt < Def->ActNum; cnt++)
    if (SEqual(szActName, Def->ActMap[cnt].Name))
      return SetAction(cnt, pTarget, pTarget2, fStartCall);
  return FALSE;
}

void C4Object::SetDir(int iDir) {
  // Not active
  if (Action.Act <= ActIdle)
    return;
  // Invalid direction
  if (!Inside(iDir, 0, Def->ActMap[Action.Act].Directions - 1))
    return;
  // Set dir
  Action.Dir = iDir;
}

int C4Object::GetProcedure() {
  if (Action.Act <= ActIdle)
    return DFA_NONE;
  return Def->ActMap[Action.Act].Procedure;
}

void GrabLost(C4Object *cObj) {
  // Grab lost script call on target (quite hacky stuff...)
  cObj->Action.Target->Call(PSF_GrabLost);
  // Clear commands down to first PushTo (if any) in command stack
  C4Command *pCom;
  for (pCom = cObj->Command; pCom; pCom = pCom->Next)
    if (pCom->Next && pCom->Next->Command == C4CMD_PushTo)
      break;
  if (pCom)
    cObj->ClearCommand(pCom);
}

void DoGravity(C4Object *cobj, BOOL fFloatFriction = TRUE);

void NoAttachAction(C4Object *cObj) {
  // Active objects
  if (cObj->Action.Act > ActIdle) {
    int iProcedure = cObj->GetProcedure();
    ;
    // Scaling upwards: corner scale
    if ((iProcedure == DFA_SCALE) && (cObj->Action.ComDir == COMD_Up))
      if (ObjectActionCornerScale(cObj))
        return;
    // Scaling and stopped: fall off to side (avoid zuppel)
    if ((iProcedure == DFA_SCALE) && (cObj->Action.ComDir == COMD_Stop))
      if (cObj->Action.Dir == DIR_Left) {
        if (ObjectActionJump(cObj, ftofix(+1.0), 0))
          return;
      } else {
        if (ObjectActionJump(cObj, ftofix(-1.0), 0))
          return;
      }
    // Pushing: grab loss
    if (iProcedure == DFA_PUSH)
      GrabLost(cObj);
    // Else jump
    ObjectActionJump(cObj, cObj->xdir, 0);
  }
  // Inactive objects, simple mobile natural gravity
  else {
    DoGravity(cObj);
    cObj->Mobile = 1;
  }
}

BOOL ContactVtxCNAT(C4Object *cobj, BYTE cnat_dir);

void C4Object::ContactAction() {
  // Take certain action on contact. Evaluate t_contact-CNAT and Procedure.

  int iDir, iComDir;
  C4PhysicalInfo *pPhysical = GetPhysical();

  // Determine Procedure
  if (Action.Act <= ActIdle)
    return;
  int iProcedure = Def->ActMap[Action.Act].Procedure;
  int fDisabled = Def->ActMap[Action.Act].Disabled;

  //------------------------------- Hit Bottom
  //---------------------------------------------
  if (t_contact & CNAT_Bottom)
    switch (iProcedure) {

    case DFA_FLIGHT:
      // Jump: FlatHit / HardHit / Walk
      if ((OCF & OCF_HitSpeed4) || fDisabled)
        if (ObjectActionFlat(this, Action.Dir))
          return;
      if (OCF & OCF_HitSpeed3)
        if (ObjectActionKneel(this))
          return;
      // Walk, but keep horizontal momentum (momentum is reset
      // by ObjectActionWalk) to avoid walk-jump-flipflop on
      // sideways corner hit if initial walk acceleration is
      // not enough to reach the next pixel for attachment.
      // Urks, all those special cases...
      FIXED last_xdir;
      last_xdir = xdir;
      ObjectActionWalk(this);
      xdir = last_xdir;
      return;

    case DFA_SCALE:
      // Scale up: try corner scale
      if (Action.ComDir == COMD_Up) {
        if (ObjectActionCornerScale(this))
          return;
        return;
      }
      // Any other: Stand
      ObjectActionStand(this);
      return;

    case DFA_DIG:
      // Redirect downleft/downright
      if (Action.ComDir == COMD_DownLeft) {
        Action.ComDir = COMD_Left;
        break;
      }
      if (Action.ComDir == COMD_DownRight) {
        Action.ComDir = COMD_Right;
        break;
      }
      // Else stop
      ObjectComStopDig(this);
      return;

    case DFA_SWIM:
      // Try corner scale out
      if (!GBackLiquid(x, y - 1))
        if (ObjectActionCornerScale(this))
          return;
      return;
    }

  //------------------------------- Hit Ceiling
  //-----------------------------------------
  if (t_contact & CNAT_Top)
    switch (iProcedure) {

    case DFA_WALK:
      // Walk: Stop
      ObjectActionStand(this);
      return;

    case DFA_SCALE:
      // Scale: Try hangle, else stop
      if (pPhysical->CanHangle)
        if (Action.ComDir == COMD_Up) {
          iDir = DIR_Left;
          iComDir = COMD_Left;
          if (Action.Dir == DIR_Left) {
            iDir = DIR_Right;
            iComDir = COMD_Right;
          }
          ObjectActionHangle(this, iDir, iComDir);
          return;
        }
      // Else stop if going upward
      if (Action.ComDir == COMD_Up)
        Action.ComDir = COMD_Stop;
      break;

    case DFA_FLIGHT:
      // Jump: Try hangle, else bounce off
      // High Speed Flight: Tumble
      if ((OCF & OCF_HitSpeed3) || fDisabled) {
        ObjectActionTumble(this, Action.Dir, 0, 0);
      }
      if (pPhysical->CanHangle) {
        ObjectActionHangle(this, Action.Dir, COMD_Stop);
        return;
      }
      break;

    case DFA_DIG:
      // Dig: Stop
      ObjectComStopDig(this);
      return;

    case DFA_HANGLE:
      Action.ComDir = COMD_Stop;
      break;
    }

  //---------------------------- Hit Left Wall
  //----------------------------------------
  if (t_contact & CNAT_Left) {
    switch (iProcedure) {
    case DFA_FLIGHT:
      // High Speed Flight: Tumble
      if ((OCF & OCF_HitSpeed3) || fDisabled) {
        ObjectActionTumble(this, DIR_Left, ftofix(+1.5), ftofix(+0.0));
      }
      // Else
      else if (pPhysical->CanScale) {
        ObjectActionScale(this, DIR_Left, COMD_Stop);
        return;
      }
      break;
    case DFA_WALK:
      // Walk: Try scale, else stop
      if (Action.ComDir == COMD_Left) {
        if (pPhysical->CanScale) {
          ObjectActionScale(this, DIR_Left, COMD_Up);
          return;
        }
        // Else stop
        Action.ComDir = COMD_Stop;
      }
      // Heading away from solid
      if (Action.ComDir == COMD_Right) {
        // Slide off
        ObjectActionJump(this, xdir / 2, ydir);
      }
      return;

    case DFA_SWIM:
      // Try scale
      if (Action.ComDir == COMD_Left)
        if (pPhysical->CanScale) {
          ObjectActionScale(this, DIR_Left, COMD_Up);
          return;
        }
      // Try corner scale out
      if (ObjectActionCornerScale(this))
        return;
      return;

    case DFA_HANGLE:
      // Hangle: Try scale, else stop
      if (pPhysical->CanScale)
        if (ObjectActionScale(this, DIR_Left, COMD_Down))
          return;
      Action.ComDir = COMD_Stop;
      return;

    case DFA_DIG:
      // Dig: Stop
      ObjectComStopDig(this);
      return;
    }
  }

  //------------------------------ Hit Right Wall
  //--------------------------------------
  if (t_contact & CNAT_Right) {
    switch (iProcedure) {

    case DFA_FLIGHT:
      // High Speed Flight: Tumble
      if ((OCF & OCF_HitSpeed3) || fDisabled) {
        ObjectActionTumble(this, DIR_Right, ftofix(-1.5), ftofix(+0.0));
      }
      // Else
      else if (pPhysical->CanScale) {
        ObjectActionScale(this, DIR_Right, COMD_Stop);
        return;
      }
      break;

    case DFA_WALK:
      // Walk: Try scale, else stop
      if (Action.ComDir == COMD_Right) {
        if (pPhysical->CanScale) {
          ObjectActionScale(this, DIR_Right, COMD_Up);
          return;
        }
        // Else stop
        Action.ComDir = COMD_Stop;
      }
      // Heading away from solid
      if (Action.ComDir == COMD_Left) {
        // Slide off
        ObjectActionJump(this, xdir / 2, ydir);
      }
      return;

    case DFA_SWIM:
      // Try scale
      if (Action.ComDir == COMD_Right)
        if (pPhysical->CanScale) {
          ObjectActionScale(this, DIR_Right, COMD_Up);
          return;
        }
      // Try corner scale out
      if (ObjectActionCornerScale(this))
        return;
      // Skid to enable walk out
      return;

    case DFA_HANGLE:
      // Hangle: Try scale, else stop
      if (pPhysical->CanScale)
        if (ObjectActionScale(this, DIR_Right, COMD_Down))
          return;
      Action.ComDir = COMD_Stop;
      return;

    case DFA_DIG:
      // Dig: Stop
      ObjectComStopDig(this);
      return;
    }
  }

  //---------------------------- Unresolved Cases
  //---------------------------------------

  // Flight stuck
  if (iProcedure == DFA_FLIGHT) {
    // Enforce slide free (might slide through tiny holes this way)
    if (t_contact & CNAT_Right) {
      ForcePosition(x - 1, y + 1);
      xdir = ydir = 0;
    }
    if (t_contact & CNAT_Left) {
      ForcePosition(x + 1, y + 1);
      xdir = ydir = 0;
    }
    if (t_contact & CNAT_Top) {
      ForcePosition(x, y + 1);
      xdir = ydir = 0;
    }
  }
}

void Towards(FIXED &val, FIXED target, FIXED step) {
  if (val == target)
    return;
  if (Abs(val - target) <= step) {
    val = target;
    return;
  }
  if (val < target)
    val += step;
  else
    val -= step;
}

void DoBridge(C4Object *clk) {
  int tx, ty;
  switch (clk->Action.ComDir) {
  case COMD_Left:
    tx = clk->x - 8 + 5 - clk->Action.Time / 5;
    ty = clk->y - 10 + 20;
    break;
  case COMD_Right:
    tx = clk->x - 8 + 10 + clk->Action.Time / 5;
    ty = clk->y - 10 + 20;
    break;
  case COMD_Up:
    tx = clk->x - 8 + 15 * (clk->Action.Dir == DIR_Right);
    ty = clk->y - 10 + 20 - clk->Action.Time / 4;
    break;
  case COMD_UpLeft:
    tx = clk->x - 8 + 3 - clk->Action.Time / 6;
    ty = clk->y - 10 + 22 - clk->Action.Time / 6;
    break;
  case COMD_UpRight:
    tx = clk->x - 8 + 13 + clk->Action.Time / 6;
    ty = clk->y - 10 + 22 - clk->Action.Time / 6;
    break;
  default:
    return;
  }
  Game.Landscape.DrawMaterialRect(clk->Action.Data, tx - 2, ty, 4, 3);
}

void DoGravity(C4Object *cobj, BOOL fFloatFriction) {
  // Floatation in liquids
  if (cobj->InLiquid && cobj->Def->Float) {
    cobj->ydir -= FloatAccel;
    if (cobj->ydir < -10 * FloatAccel)
      cobj->ydir = -10 * FloatAccel;
    if (fFloatFriction) {
      if (cobj->xdir < -FloatFriction)
        cobj->xdir += FloatFriction;
      if (cobj->xdir > +FloatFriction)
        cobj->xdir -= FloatFriction;
      if (cobj->rdir < -FloatFriction)
        cobj->rdir += FloatFriction;
      if (cobj->rdir > +FloatFriction)
        cobj->rdir -= FloatFriction;
    }
    if (!GBackLiquid(cobj->x, cobj->y - 1 + cobj->Def->Float * cobj->Con / FullCon - 1))
      if (cobj->ydir < 0)
        cobj->ydir = 0;
  }
  // Free fall gravity
  else
    cobj->ydir += GravAccel;
}

void StopActionDelayCommand(C4Object *cobj) {
  ObjectComStop(cobj);
  cobj->AddCommand(C4CMD_Wait, NULL, 0, 0, 50);
}

BOOL ReduceLineSegments(C4Shape &rShape) {
  for (int cnt = 0; cnt + 2 < rShape.VtxNum; cnt++)
    if (PathFree(rShape.VtxX[cnt], rShape.VtxY[cnt], rShape.VtxX[cnt + 2], rShape.VtxY[cnt + 2])) {
      rShape.RemoveVertex(cnt + 1);
      return TRUE;
    }
  return FALSE;
}

void C4Object::ExecAction() {
  Action.t_attach = CNAT_None;
  DWORD ocf;
  int iTargetX;
  int iPushRange, iPushDistance;

  // Standard phase advance
  int iPhaseAdvance = 1;

  // Upright attachment check
  if (!Mobile)
    if (Def->UprightAttach)
      if (Inside(r, -StableRange, +StableRange)) {
        Action.t_attach |= Def->UprightAttach;
        Mobile = 1;
      }

  // Idle objects do natural gravity only
  if (Action.Act <= ActIdle) {
    if (Mobile)
      DoGravity(this);
    return;
  }

  // Determine ActDef & Physical Info
  C4ActionDef *pAction = &(Def->ActMap[Action.Act]);
  C4PhysicalInfo *pPhysical = GetPhysical();
  FIXED lLimit;
  int smpx, smpy;

  // Energy usage
  if (Game.Rules & C4RULE_StructuresNeedEnergy)
    if (pAction->EnergyUsage)
      if (pAction->EnergyUsage <= Energy) {
        Energy -= pAction->EnergyUsage;
        // No general DoEnergy-Process
        NeedEnergy = 0;
      }
      // Insufficient energy for action: same as idle
      else {
        NeedEnergy = 1;
        if (Mobile)
          DoGravity(this);
        return;
      }

  // Action time advance
  Action.Time++;

  // InLiquidAction check
  if (InLiquid)
    if (pAction->InLiquidAction[0]) {
      SetActionByName(pAction->InLiquidAction);
      return;
    }

  // Handle Default Action Procedure: evaluates Procedure and Action.ComDir
  // Update xdir,ydir,Action.Dir,attachment,iPhaseAdvance
  switch (pAction->Procedure) {
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_WALK:
    lLimit = ftofix(2.8 * (float)pPhysical->Walk / (float)C4MaxPhysical);
    switch (Action.ComDir) {
    case COMD_Left:
      xdir -= WalkAccel;
      if (xdir < -lLimit)
        xdir = -lLimit;
      break;
    case COMD_Right:
      xdir += WalkAccel;
      if (xdir > +lLimit)
        xdir = +lLimit;
      break;
    case COMD_Stop:
      if (xdir < 0)
        xdir += WalkAccel;
      if (xdir > 0)
        xdir -= WalkAccel;
      if ((xdir > -WalkAccel) && (xdir < +WalkAccel))
        xdir = 0;
      break;
    }
    iPhaseAdvance = 0;
    if (xdir < 0) {
      iPhaseAdvance = -fixtoi(10 * xdir);
      SetDir(DIR_Left);
    }
    if (xdir > 0) {
      iPhaseAdvance = +fixtoi(10 * xdir);
      SetDir(DIR_Right);
    }
    ydir = 0;
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_KNEEL:
    ydir = 0;
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_SCALE:

    lLimit = ftofix(2.0 * (float)pPhysical->Scale / (float)C4MaxPhysical);

    // Physical training
    if (!Tick5)
      if (Info)
        if (Abs(ydir) == lLimit)
          Info->Physical.Scale = Min(Info->Physical.Scale + 1, C4MaxPhysical);

    switch (Action.ComDir) {
    case COMD_Up:
      ydir -= WalkAccel;
      if (ydir < -lLimit)
        ydir = -lLimit;
      break;
    case COMD_Down:
      ydir += WalkAccel;
      if (ydir > +lLimit)
        ydir = +lLimit;
      break;
    case COMD_Stop:
      if (ydir < 0)
        ydir += WalkAccel;
      if (ydir > 0)
        ydir -= WalkAccel;
      if ((ydir > -WalkAccel) && (ydir < +WalkAccel))
        ydir = 0;
      break;
    }
    iPhaseAdvance = 0;
    if (ydir < 0)
      iPhaseAdvance = -fixtoi(14 * ydir);
    if (ydir > 0)
      iPhaseAdvance = +fixtoi(14 * ydir);
    xdir = 0;
    if (Action.Dir == DIR_Left)
      Action.t_attach |= CNAT_Left;
    if (Action.Dir == DIR_Right)
      Action.t_attach |= CNAT_Right;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_HANGLE:

    lLimit = ftofix(1.6 * (float)pPhysical->Hangle / (float)C4MaxPhysical);

    // Physical training
    if (!Tick5)
      if (Info)
        if (Abs(xdir) == lLimit)
          Info->Physical.Hangle = Min(Info->Physical.Hangle + 1, C4MaxPhysical);

    switch (Action.ComDir) {
    case COMD_Left:
      xdir -= WalkAccel;
      if (xdir < -lLimit)
        xdir = -lLimit;
      break;
    case COMD_Right:
      xdir += WalkAccel;
      if (xdir > +lLimit)
        xdir = +lLimit;
      break;
    case COMD_Stop:
      if (xdir < 0)
        xdir += WalkAccel;
      if (xdir > 0)
        xdir -= WalkAccel;
      if ((xdir > -WalkAccel) && (xdir < +WalkAccel))
        xdir = 0;
      break;
    }
    iPhaseAdvance = 0;
    if (xdir < 0) {
      iPhaseAdvance = -fixtoi(10 * xdir);
      SetDir(DIR_Left);
    }
    if (xdir > 0) {
      iPhaseAdvance = +fixtoi(10 * xdir);
      SetDir(DIR_Right);
    }
    ydir = 0;
    Action.t_attach |= CNAT_Top;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_FLIGHT:
    // Contained: fall out (one try only)
    if (!Tick10)
      if (Contained) {
        StopActionDelayCommand(this);
        SetCommand(C4CMD_Exit);
      }
    // Gravity/mobile
    DoGravity(this);
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_DIG:
    smpx = x;
    smpy = y;
    if (!Shape.Attach(smpx, smpy, CNAT_Bottom)) {
      ObjectComStopDig(this);
      return;
    }
    lLimit = ftofix(1.25 * (float)pPhysical->Dig / (float)C4MaxPhysical);
    switch (Action.ComDir) {
    case COMD_UpLeft:
      xdir = -lLimit;
      ydir = -lLimit / 2;
      break;
    case COMD_Left:
      xdir = -lLimit;
      ydir = 0;
      break;
    case COMD_DownLeft:
      xdir = -lLimit;
      ydir = +lLimit;
      break;
    case COMD_Down:
      xdir = 0;
      ydir = +lLimit;
      break;
    case COMD_DownRight:
      xdir = +lLimit;
      ydir = +lLimit;
      break;
    case COMD_Right:
      xdir = +lLimit;
      ydir = 0;
      break;
    case COMD_UpRight:
      xdir = +lLimit;
      ydir = -lLimit / 2;
      break;
    case COMD_Stop:
      ObjectActionStand(this);
      return;
    }
    SetDir((xdir < 0) ? DIR_Left : DIR_Right);
    iPhaseAdvance = fixtoi(40 * lLimit);
    Action.t_attach = CNAT_None;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_SWIM:

    lLimit = ftofix(1.6 * (float)pPhysical->Swim / (float)C4MaxPhysical);

    // Physical training
    if (!Tick10)
      if (Info)
        if (Abs(xdir) == lLimit)
          Info->Physical.Swim = Min(Info->Physical.Swim + 1, C4MaxPhysical);

    // ComDir changes xdir/ydir
    switch (Action.ComDir) {
    case COMD_Up:
      ydir -= SwimAccel;
      break;
    case COMD_Down:
      ydir += SwimAccel;
      break;
    case COMD_Right:
      xdir += SwimAccel;
      break;
    case COMD_Left:
      xdir -= SwimAccel;
      break;
    }

    // Out of liquid check
    if (!InLiquid) {
      // Just above liquid: move down
      if (GBackLiquid(x, y + 1 + Def->Float * Con / FullCon - 1))
        ydir = +SwimAccel;
      // Free fall: walk
      else {
        ObjectActionWalk(this);
        return;
      }
    }

    // xdir/ydir bounds
    if (ydir < -lLimit)
      ydir = -lLimit;
    if (ydir > +lLimit)
      ydir = +lLimit;
    if (xdir > +lLimit)
      xdir = +lLimit;
    if (xdir < -lLimit)
      xdir = -lLimit;
    // Surface dir bound
    if (!GBackLiquid(x, y - 1 + Def->Float * Con / FullCon - 1))
      if (ydir < 0)
        ydir = 0;
    // Dir, Phase, Attach
    if (xdir < 0)
      SetDir(DIR_Left);
    if (xdir > 0)
      SetDir(DIR_Right);
    iPhaseAdvance = fixtoi(10 * lLimit);
    Action.t_attach = CNAT_None;
    Mobile = 1;

    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_THROW:
    ydir = 0;
    xdir = 0;
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_BRIDGE:
    if (Action.Time >= 100) {
      ObjectActionStand(this);
      return;
    }
    if (!Tick3)
      DoBridge(this);
    switch (Action.ComDir) {
    case COMD_Left:
    case COMD_UpLeft:
      SetDir(DIR_Left);
      break;
    case COMD_Right:
    case COMD_UpRight:
      SetDir(DIR_Right);
      break;
    }
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_BUILD:
    // Woa, structures can build without target
    if ((Category & C4D_Structure) || (Category & C4D_StaticBack))
      if (!Action.Target)
        break;
    // No target
    if (!Action.Target) {
      ObjectComStop(this);
      return;
    }
    // Target internal: container needs to support by own DFA_BUILD
    if (Action.Target->Contained)
      if ((Action.Target->Contained->GetProcedure() != DFA_BUILD) || (Action.Target->Contained->NeedEnergy))
        return;
    // Build speed
    int iLevel;
    // Clonk-standard
    iLevel = 10;
    // Internal builds slower
    if (Action.Target->Contained)
      iLevel = 1;
    // Out of target area: stop
    if (!Inside(x - (Action.Target->x + Action.Target->Shape.x), 0, Action.Target->Shape.Wdt) || !Inside(y - (Action.Target->y + Action.Target->Shape.y), -16, Action.Target->Shape.Hgt + 16)) {
      ObjectComStop(this);
      return;
    }
    // Build target
    if (!Action.Target->Build(iLevel, this)) {
      // Cannot build because target is complete: we're done
      if (Action.Target->Con >= FullCon) {
        // Stop
        ObjectComStop(this);
        // Exit target if internal
        if (Action.Target->Contained == this)
          Action.Target->SetCommand(C4CMD_Exit);
      }
      // Cannot build because target needs material (assumeably)
      else {
        // Wait (for material)
        StopActionDelayCommand(this);
      }
      return;
    }
    xdir = ydir = 0;
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_PUSH:
    // No target
    if (!Action.Target) {
      StopActionDelayCommand(this);
      return;
    }
    // Inside target
    if (Contained == Action.Target) {
      StopActionDelayCommand(this);
      return;
    }
    // Target pushing force
    FIXED iTXDir;
    BOOL fStraighten;
    iTXDir = 0;
    fStraighten = FALSE;
    lLimit = ftofix(2.8 * (float)pPhysical->Walk / (float)C4MaxPhysical);
    switch (Action.ComDir) {
    case COMD_Left:
      iTXDir = -lLimit;
      break;
    case COMD_Right:
      iTXDir = +lLimit;
      break;
    case COMD_Up:
      fStraighten = 1;
      break;
    case COMD_Stop:
      iTXDir = 0;
      break;
    }
    // Push object
    if (!Action.Target->Push(iTXDir, ftofix(2.5 * (float)pPhysical->Push / (float)C4MaxPhysical), fStraighten)) {
      StopActionDelayCommand(this);
      return;
    }
    // Set target controller
    Action.Target->Controller = Controller;
    // ObjectAction got hold check
    iPushDistance = Max(Shape.Wdt / 2 - 8, 0);
    iPushRange = iPushDistance + 10;
    int sax, say, sawdt, sahgt;
    Action.Target->GetArea(sax, say, sawdt, sahgt);
    // Object lost
    if (!Inside(x - sax, -iPushRange, sawdt - 1 + iPushRange) || !Inside(y - say, -iPushRange, sahgt - 1 + iPushRange)) {
      // Wait command (why, anyway?)
      StopActionDelayCommand(this);
      // Grab lost action
      GrabLost(this);
      // Done
      return;
    }
    // Follow object (full xdir reset)
    iTargetX = BoundBy(x, sax - iPushDistance, sax + sawdt - 1 + iPushDistance);
    if (x == iTargetX)
      xdir = 0;
    else {
      if (x < iTargetX)
        xdir = +lLimit;
      if (x > iTargetX)
        xdir = -lLimit;
    }
    // Phase by XDir
    if (xdir < 0) {
      iPhaseAdvance = -fixtoi(10 * xdir);
      SetDir(DIR_Left);
    }
    if (xdir > 0) {
      iPhaseAdvance = +fixtoi(10 * xdir);
      SetDir(DIR_Right);
    }
    // No YDir
    ydir = 0;
    // Attachment
    Action.t_attach |= CNAT_Bottom;
    // Mobile
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - -
  case DFA_PULL:
    // No target
    if (!Action.Target) {
      StopActionDelayCommand(this);
      return;
    }
    // Inside target
    if (Contained == Action.Target) {
      StopActionDelayCommand(this);
      return;
    }
    // Target contained
    if (Action.Target->Contained) {
      StopActionDelayCommand(this);
      return;
    }

    int iPullDistance;
    int iPullX;
    double fWalk, fMove;

    iPullDistance = Action.Target->Shape.Wdt / 2 + Shape.Wdt / 2;

    iTargetX = x;
    if (Action.ComDir == COMD_Right)
      iTargetX = Action.Target->x + iPullDistance;
    if (Action.ComDir == COMD_Left)
      iTargetX = Action.Target->x - iPullDistance;

    iPullX = Action.Target->x;
    if (Action.ComDir == COMD_Right)
      iPullX = x - iPullDistance;
    if (Action.ComDir == COMD_Left)
      iPullX = x + iPullDistance;

    fWalk = 2.8 * (float)pPhysical->Walk / (float)C4MaxPhysical;

    fMove = 0.0;
    if (Action.ComDir == COMD_Right)
      fMove = +fWalk;
    if (Action.ComDir == COMD_Left)
      fMove = -fWalk;

    iTXDir = ftofix(fMove + fWalk * ((float)BoundBy(iPullX - Action.Target->x, -10, +10) / 10.0));

    // Push object
    if (!Action.Target->Push(iTXDir, ftofix(2.5 * (float)pPhysical->Push / (float)C4MaxPhysical), FALSE)) {
      StopActionDelayCommand(this);
      return;
    }
    // Set target controller
    Action.Target->Controller = Controller;

    // Train pulling: com dir transfer
    if ((Action.Target->GetProcedure() == DFA_WALK) || (Action.Target->GetProcedure() == DFA_PULL)) {
      Action.Target->Action.ComDir = COMD_Stop;
      if (iTXDir < 0)
        Action.Target->Action.ComDir = COMD_Left;
      if (iTXDir > 0)
        Action.Target->Action.ComDir = COMD_Right;
    }

    // Pulling range
    iPushDistance = Max(Shape.Wdt / 2 - 8, 0);
    iPushRange = iPushDistance + 20;
    Action.Target->GetArea(sax, say, sawdt, sahgt);
    // Object lost
    if (!Inside(x - sax, -iPushRange, sawdt - 1 + iPushRange) || !Inside(y - say, -iPushRange, sahgt - 1 + iPushRange)) {
      // Wait command (why, anyway?)
      StopActionDelayCommand(this);
      // Grab lost action
      GrabLost(this);
      // Lose target
      Action.Target = NULL;
      // Done
      return;
    }

    // Move to pulling position
    xdir = ftofix(fMove + fWalk * ((float)BoundBy(iTargetX - x, -10, +10) / 10.0));

    // Phase by XDir
    iPhaseAdvance = 0;
    if (xdir < 0) {
      iPhaseAdvance = -fixtoi(10 * xdir);
      SetDir(DIR_Left);
    }
    if (xdir > 0) {
      iPhaseAdvance = +fixtoi(10 * xdir);
      SetDir(DIR_Right);
    }
    // No YDir
    ydir = 0;
    // Attachment
    Action.t_attach |= CNAT_Bottom;
    // Mobile
    Mobile = 1;

    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - -
  case DFA_CHOP:
    // Valid check
    if (!Action.Target) {
      ObjectActionStand(this);
      return;
    }
    // Chop
    if (!Tick3)
      if (!Action.Target->Chop(this)) {
        ObjectActionStand(this);
        return;
      }
    // Valid check (again, target might have been destroyed)
    if (!Action.Target) {
      ObjectActionStand(this);
      return;
    }
    // AtObject check
    ocf = OCF_Chop;
    if (!Action.Target->At(x, y, ocf)) {
      ObjectActionStand(this);
      return;
    }
    // Position
    SetDir((x > Action.Target->x) ? DIR_Left : DIR_Right);
    xdir = ydir = 0;
    Action.t_attach |= CNAT_Bottom;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_FIGHT:
    // Valid check
    if (!Action.Target || (Action.Target->GetProcedure() != DFA_FIGHT)) {
      ObjectActionStand(this);
      return;
    }

    // Physical training
    if (!Tick5)
      if (Info)
        Info->Physical.Fight = Min(Info->Physical.Fight + 1, C4MaxPhysical);

    // Direction
    if (Action.Target->x > x)
      SetDir(DIR_Right);
    if (Action.Target->x < x)
      SetDir(DIR_Left);
    // Position
    iTargetX = x;
    if (Action.Dir == DIR_Left)
      iTargetX = Action.Target->x + Action.Target->Shape.Wdt / 2 + 2;
    if (Action.Dir == DIR_Right)
      iTargetX = Action.Target->x - Action.Target->Shape.Wdt / 2 - 2;
    lLimit = ftofix(0.95 * (float)pPhysical->Walk / (float)C4MaxPhysical);
    if (x == iTargetX)
      Towards(xdir, 0, lLimit);
    if (x < iTargetX)
      Towards(xdir, +lLimit, lLimit);
    if (x > iTargetX)
      Towards(xdir, -lLimit, lLimit);
    // Distance check
    if ((Abs(x - Action.Target->x) > Shape.Wdt) || (Abs(y - Action.Target->y) > Shape.Wdt)) {
      ObjectActionStand(this);
      return;
    }
    // Other
    Action.t_attach |= CNAT_Bottom;
    ydir = 0;
    Mobile = 1;
    // Experience
    if (!Tick35)
      DoExperience(+2);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_LIFT:
    // Valid check
    if (!Action.Target) {
      SetAction(ActIdle);
      return;
    }
    // Target lifting force
    FIXED lftspeed, tydir;
    lftspeed = itofix(2);
    tydir = 0;
    switch (Action.ComDir) {
    case COMD_Up:
      tydir = -lftspeed;
      break;
    case COMD_Stop:
      tydir = -GravAccel;
      break;
    case COMD_Down:
      tydir = +lftspeed;
      break;
    }
    // Lift object
    if (!Action.Target->Lift(tydir, ftofix(0.5))) {
      SetAction(ActIdle);
      return;
    }
    // Check LiftTop
    if (Def->LiftTop)
      if (Action.Target->y <= (y + Def->LiftTop))
        if (Action.ComDir == COMD_Up)
          Call(PSF_LiftTop);
    // General
    DoGravity(this);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  case DFA_FLOAT:
    // Float speed
    lLimit = ftofix((float)pPhysical->Float / 100.0);
    // ComDir changes xdir/ydir
    switch (Action.ComDir) {
    case COMD_Up:
      ydir -= FloatAccel;
      break;
    case COMD_Down:
      ydir += FloatAccel;
      break;
    case COMD_Right:
      xdir += FloatAccel;
      break;
    case COMD_Left:
      xdir -= FloatAccel;
      break;
    case COMD_UpRight:
      ydir -= FloatAccel;
      xdir += FloatAccel;
      break;
    case COMD_DownRight:
      ydir += FloatAccel;
      xdir += FloatAccel;
      break;
    case COMD_DownLeft:
      ydir += FloatAccel;
      xdir -= FloatAccel;
      break;
    case COMD_UpLeft:
      ydir -= FloatAccel;
      xdir -= FloatAccel;
      break;
    }
    // xdir/ydir bounds
    if (ydir < -lLimit)
      ydir = -lLimit;
    if (ydir > +lLimit)
      ydir = +lLimit;
    if (xdir > +lLimit)
      xdir = +lLimit;
    if (xdir < -lLimit)
      xdir = -lLimit;
    Mobile = 1;
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - ATTACH: Force position to target object, vertex 0 to vertex 0
  case DFA_ATTACH:

    // No target
    if (!Action.Target) {
      SetAction(ActIdle);
      return;
    }

    // Target incomplete and no incomplete activity
    if (!(Action.Target->OCF & OCF_FullCon))
      if (!Action.Target->Def->IncompleteActivity) {
        SetAction(ActIdle);
        return;
      }

    // Force containment
    if (Action.Target->Contained != Contained)
      if (Action.Target->Contained)
        Enter(Action.Target->Contained);
      else
        Exit(x, y, r);

    // Force position
    ForcePosition(Action.Target->x + Action.Target->Shape.VtxX[0] - Shape.VtxX[0], Action.Target->y + Action.Target->Shape.VtxY[0] - Shape.VtxY[0]);
    xdir = ydir = 0;

    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
  case DFA_CONNECT:

    BOOL fBroke;
    fBroke = FALSE;
    int iConnectX, iConnectY;

    // Line destruction check: Target missing or incomplete
    if (!Action.Target || (Action.Target->Con < FullCon))
      fBroke = TRUE;
    if (!Action.Target2 || (Action.Target2->Con < FullCon))
      fBroke = TRUE;
    if (fBroke) {
      Call(PSF_LineBreak, 1);
      AssignRemoval();
      return;
    }

    // Movement by Target
    if (Action.Target) {
      // Connect to vertex
      if (Def->Line == C4D_Line_Vertex) {
        iConnectX = Action.Target->x + Action.Target->Shape.GetVertexX(Local[2]);
        iConnectY = Action.Target->y + Action.Target->Shape.GetVertexY(Local[2]);
      }
      // Connect to bottom center
      else {
        iConnectX = Action.Target->x;
        iConnectY = Action.Target->y + Action.Target->Shape.Hgt / 4;
      }
      if ((iConnectX != Shape.VtxX[0]) || (iConnectY != Shape.VtxY[0])) {
        // Regular wrapping line
        if (Def->LineIntersect == 0)
          if (!Shape.LineConnect(iConnectX, iConnectY, 0, +1))
            fBroke = TRUE;
        // No-intersection line
        if (Def->LineIntersect == 1) {
          Shape.VtxX[0] = iConnectX;
          Shape.VtxY[0] = iConnectY;
        }
      }
    }
    // Movement by Target2
    if (Action.Target2) {
      // Connect to vertex
      if (Def->Line == C4D_Line_Vertex) {
        iConnectX = Action.Target2->x + Action.Target2->Shape.GetVertexX(Local[3]);
        iConnectY = Action.Target2->y + Action.Target2->Shape.GetVertexY(Local[3]);
      }
      // Connect to bottom center
      else {
        iConnectX = Action.Target2->x;
        iConnectY = Action.Target2->y + Action.Target2->Shape.Hgt / 4;
      }
      if ((iConnectX != Shape.VtxX[Shape.VtxNum - 1]) || (iConnectY != Shape.VtxY[Shape.VtxNum - 1])) {
        // Regular wrapping line
        if (Def->LineIntersect == 0)
          if (!Shape.LineConnect(iConnectX, iConnectY, Shape.VtxNum - 1, -1))
            fBroke = TRUE;
        // No-intersection line
        if (Def->LineIntersect == 1) {
          Shape.VtxX[Shape.VtxNum - 1] = iConnectX;
          Shape.VtxY[Shape.VtxNum - 1] = iConnectY;
        }
      }
    }

    // Line fBroke
    if (fBroke) {
      Call(PSF_LineBreak, 0);
      AssignRemoval();
      return;
    }

    // Reduce line segments
    if (!Tick35)
      ReduceLineSegments(Shape);

    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - -
  default:
    // Attach
    if (pAction->Attach) {
      Action.t_attach |= pAction->Attach;
      xdir = ydir = 0;
      Mobile = 1;
    }
    // Free gravity
    else
      DoGravity(this);
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - -
  }

  // Phase Advance (zero delay means no phase advance)
  if (pAction->Delay) {
    Action.PhaseDelay += iPhaseAdvance;
    if (Action.PhaseDelay >= pAction->Delay) {
      // Advance Phase
      Action.PhaseDelay = 0;
      Action.Phase++;
      // Phase call
      if (pAction->PhaseCall[0]) {
        C4Thread cthr;
        cthr.cObj = this;
        sprintf(cthr.Function, "%s PhaseCall", pAction->Name);
        Call(&cthr, pAction->PhaseCall);
      }
      // Phase end
      if (Action.Phase >= pAction->Length) {
        // Hold
        if (pAction->NextAction == ActHold) {
          Action.Phase--;
        }
        // Next action
        else {
          SetAction(pAction->NextAction);
          // End call
          if (pAction->EndCall[0]) {
            C4Thread cthr;
            cthr.cObj = this;
            sprintf(cthr.Function, "%s EndCall", pAction->Name);
            Call(&cthr, pAction->EndCall);
          }
        }
      }
    }
  }

  return;
}
