/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Small member of the landscape class to handle the sky background */

#include <C4Include.h>

void C4Sky::SetFadePalette(int *ipColors) {
  int cnt, iStart, iEnd;
  iStart = 0;
  iEnd = 19;
  // If colors all zero, use game palette default blue
  if (ipColors[0] + ipColors[1] + ipColors[2] + ipColors[3] + ipColors[4] +
          ipColors[5] ==
      0) {
    MemCopy(Game.GraphicsResource.GamePalette + 3 * CSkyDef1, Palette, 3 * 20);
    return;
  }
  // Set start/end colors
  Palette[3 * iStart + 0] = ipColors[0];
  Palette[3 * iStart + 1] = ipColors[1];
  Palette[3 * iStart + 2] = ipColors[2];
  Palette[3 * iEnd + 0] = ipColors[3];
  Palette[3 * iEnd + 1] = ipColors[4];
  Palette[3 * iEnd + 2] = ipColors[5];
  // Fade mean colors
  for (cnt = iStart + 1; cnt < iEnd; cnt++) {
    Palette[3 * cnt + 0] = Palette[3 * iStart + 0] +
                           (Palette[3 * iEnd + 0] - Palette[3 * iStart + 0]) *
                               (cnt - iStart) / (iEnd - iStart + 1);
    Palette[3 * cnt + 1] = Palette[3 * iStart + 1] +
                           (Palette[3 * iEnd + 1] - Palette[3 * iStart + 1]) *
                               (cnt - iStart) / (iEnd - iStart + 1);
    Palette[3 * cnt + 2] = Palette[3 * iStart + 2] +
                           (Palette[3 * iEnd + 2] - Palette[3 * iStart + 2]) *
                               (cnt - iStart) / (iEnd - iStart + 1);
  }
}

SURFACE C4Sky::CreateColorFadeSky() {
  SURFACE nsfc;
  BYTE *sfcbuf, *cpx;
  int sfcpitch;
  int tx, ty, skoln, clayh, ladv, lbcol, stlwdt, skyhgt, horizon;

  skyhgt = GBackHgt;
  stlwdt = 64;
  horizon = GBackHgt;

  // Create SkyDef Fade Tile (stdwdt x skyhgt)
  if (!(nsfc = CreateSurface(stlwdt, skyhgt)))
    return NULL;
  if (!(sfcbuf = LockSurface(nsfc, sfcpitch))) {
    DestroySurface(nsfc);
    return NULL;
  }

  // Fade to horizon
  skoln = CSkyDef2 - CSkyDef1 - 1;
  clayh = horizon / skoln;
  for (ty = 0; ty < horizon; ty++) {
    ladv = ty % clayh;
    lbcol = Min(CSkyDef1 + ty / clayh, CSkyDef2 - 1);
    cpx = sfcbuf + sfcpitch * ty;
    for (tx = 0; tx < stlwdt; tx++, cpx++)
      *cpx = lbcol + (SafeRandom(clayh) < ladv);
  }

  // Below horizon
  if (horizon < GBackHgt)
    FillMem(sfcbuf + sfcpitch * horizon, sfcpitch * (GBackHgt - horizon),
            CSkyDef2);

  UnLockSurface(nsfc);

  // Set fade colors by scenario core
  SetFadePalette(Game.C4S.Landscape.SkyDefFade);

  return nsfc;
}

BOOL C4Sky::Init() {
  int skylistn, binlen;

  // Check for sky bitmap in scenario file
  if (!Surface)
    if (Game.ScenarioFile.AccessEntry(C4CFN_Sky, &binlen))
      if (Surface = GroupReadSurface(Game.ScenarioFile, Palette)) {
        // Surface loaded, adjust color index
        Engine.DDraw.SurfaceAllowColor(Surface, 0, 19);
        Engine.DDraw.SurfaceShiftColor(Surface, CSkyDef1);
      }

  // Else, evaluate scenario core landscape sky default list
  if (!Surface) {
    // Scan list sections
    SReplaceChar(Game.C4S.Landscape.SkyDef, ',',
                 ';'); // modifying the C4S here...!
    skylistn = SCharCount(';', Game.C4S.Landscape.SkyDef) + 1;
    SCopySegment(Game.C4S.Landscape.SkyDef, Random(skylistn), OSTR, ';');
    SClearFrontBack(OSTR);
    // Sky tile specified, try load
    if (OSTR[0] && !SEqual(OSTR, "Default")) {
      SAppend(".bmp", OSTR);
      // Check for sky tile in scenario file
      if (Game.ScenarioFile.AccessEntry(OSTR, &binlen))
        Surface = GroupReadSurface(Game.ScenarioFile, Palette);
      // Else, check for sky tile in graphics file
      if (!Surface)
        if (Game.GraphicsResource.File.AccessEntry(OSTR, &binlen))
          Surface = GroupReadSurface(Game.GraphicsResource.File, Palette);
      // Surface loaded, adjust color index
      if (Surface) {
        Engine.DDraw.SurfaceAllowColor(Surface, 0, 19);
        Engine.DDraw.SurfaceShiftColor(Surface, CSkyDef1);
      }
    }
  }

  // Else, try creating default Surface
  if (!Surface)
    Surface = CreateColorFadeSky();

  // Failure
  if (!Surface)
    return FALSE;

  // Copy sky palette to game palette
  MemCopy(Palette, Game.GraphicsResource.GamePalette + 3 * CSkyDef1, 3 * 20);

  // Store size
  GetSurfaceSize(Surface, Width, Height);

  // Success
  return TRUE;
}

C4Sky::C4Sky() {
  Width = Height = 0;
  Surface = NULL;
}

C4Sky::~C4Sky() { Clear(); }

void C4Sky::Clear() {
  if (Surface)
    DestroySurface(Surface);
  Surface = NULL;
}

BOOL C4Sky::Save(C4Group &hGroup) {
  // Sky-saving disabled by scenario core
  // (With this option enabled, script-defined changes to sky palette will not
  // be saved!)
  if (Game.C4S.Landscape.NoSky) {
    hGroup.Delete(C4CFN_Sky);
    return TRUE;
  }
  // Shift sky bitmap back to original palette index
  Engine.DDraw.SurfaceShiftColorRange(Surface, CSkyDef1, CSkyDef1 + 19,
                                      -CSkyDef1);
  // Save surface to temp file with sky palette
  if (!SaveSurface(Config.AtTempPath(C4CFN_TempSky), Surface, Palette))
    return FALSE;
  // Shift sky bitmap back to game palette index
  Engine.DDraw.SurfaceShiftColorRange(Surface, 0, 19, CSkyDef1);
  // Move temp file to target group
  if (!hGroup.Move(Config.AtTempPath(C4CFN_TempSky), C4CFN_Sky))
    return FALSE;
  // Success
  return TRUE;
}

void C4Sky::SetColor(int iIndex, int iRed, int iGreen, int iBlue) {
  // Index out of bounds
  if (!Inside(iIndex, 0, 19))
    return;
  // Set color
  Palette[iIndex * 3 + 0] = BoundBy(iRed, 0, 255);
  Palette[iIndex * 3 + 1] = BoundBy(iGreen, 0, 255);
  Palette[iIndex * 3 + 2] = BoundBy(iBlue, 0, 255);
}
