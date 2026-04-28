/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Loads all standard graphics from Graphics.c4g */

#include <C4Include.h>

C4GraphicsResource::C4GraphicsResource() { Default(); }

C4GraphicsResource::~C4GraphicsResource() { Clear(); }

void C4GraphicsResource::Default() {
  sfcGfxResource.Default();
  sfcControl.Default();

  fctPlayer.Default();
  fctSurrender.Default();
  fctFlag.Default();
  fctCrew.Default();
  fctRank.Default();
  fctFire.Default();
  fctBackground.Default();
  fctEntry.Default();
  fctCaptain.Default();
  fctMouseCursor.Default();
  fctSelectMark.Default();
  fctMenuSymbol.Default();
  fctMenu.Default();
  fctFogOfWar.Default();

  ZeroMem(GamePalette, 3 * 256);

  fctCursor.Default();
  fctDropTarget.Default();
  fctInsideSymbol.Default();
  fctEnergy.Default();
  fctConSign.Default();
  fctWealthSymbol.Default();
  fctValueSymbol.Default();
  fctKeyboard.Default();
  fctGamepad.Default();
  fctCommand.Default();
  fctKey.Default();
  fctMagicSymbol.Default();
  fctOKCancel.Default();
  fctMenuScroll.Default();
  fctMouse.Default();
}

void C4GraphicsResource::Clear() {
  File.Close();

  sfcGfxResource.Clear();
  sfcControl.Clear();

  fctPlayer.Clear();
  fctSurrender.Clear();
  fctFlag.Clear();
  fctCrew.Clear();
  fctRank.Clear();
  fctFire.Clear();
  fctBackground.Clear();
  fctEntry.Clear();
  fctCaptain.Clear();
  fctMouseCursor.Clear();
  fctSelectMark.Clear();
  fctMenuSymbol.Clear();
  fctMenu.Clear();
  fctFogOfWar.Clear();
}

BOOL C4GraphicsResource::Init() {
  int cnt;

  // Open file
  if (!File.Open(Config.AtExePath(C4CFN_Graphics))) {
    sprintf(OSTR, LoadResStr(IDS_PRC_NOGFXFILE), C4CFN_Graphics,
            File.GetError());
    Log(OSTR);
    return FALSE;
  }

  // Resource
  if (!sfcGfxResource.Load(File, "Resource.bmp"))
    return FALSE;
  fctConSign.Set(sfcGfxResource.Surface, 0, 50, 16, 16);
  fctCursor.Set(sfcGfxResource.Surface, 32, 40, 9, 13);
  fctDropTarget.Set(sfcGfxResource.Surface, 255, 0, 11, 16);
  fctEnergy.Set(sfcGfxResource.Surface, 68, 41, 11, 17);
  fctInsideSymbol.Set(sfcGfxResource.Surface, 50, 40, 17, 20);
  fctMagicSymbol.Set(sfcGfxResource.Surface, 230, 0, 25, 34);
  fctWealthSymbol.Set(sfcGfxResource.Surface, 296, 18, 24, 18);
  fctValueSymbol.Set(sfcGfxResource.Surface, 281, 0, 39, 18);

  // Control
  if (!sfcControl.Load(File, "Control.bmp"))
    return FALSE;
  fctKeyboard.Set(sfcControl.Surface, 0, 0, 80, 36);
  fctGamepad.Set(sfcControl.Surface, 0, 164, 80, 36);
  fctCommand.Set(sfcControl.Surface, 0, 36, 32, 32);
  fctKey.Set(sfcControl.Surface, 0, 100, 64, 64);
  fctOKCancel.Set(sfcControl.Surface, 128, 100, 32, 32);
  fctMenuScroll.Set(sfcControl.Surface, 192, 100, 32, 64);
  fctMouse.Set(sfcControl.Surface, 198, 100, 42, 25);

  // Facet bitmap resources
  if (!fctFire.Load(File, "Fire", C4FCT_Height))
    return FALSE;
  if (!fctBackground.Load(File, "Background"))
    return FALSE;
  if (!fctFlag.Load(File, "Flag", 50, 40))
    return FALSE;
  fctSurrender = fctFlag.GetPhase(C4MaxColor);
  if (!fctCrew.Load(File, "Crew", 24))
    return FALSE;
  if (!fctPlayer.Load(File, "Player", C4FCT_Height))
    return FALSE;
  if (!fctRank.Load(File, "Rank", C4FCT_Height))
    return FALSE;
  if (!fctEntry.Load(File, "Entry", 17, 25))
    return FALSE;
  if (!fctCaptain.Load(File, "Captain"))
    return FALSE;
  if (!fctMouseCursor.Load(File, "Cursor", C4FCT_Height))
    return FALSE;
  if (!fctSelectMark.Load(File, "SelectMark", C4FCT_Height))
    return FALSE;
  if (!fctMenuSymbol.Load(File, "MenuSymbol", C4FCT_Height))
    return FALSE;
  if (!fctMenu.Load(File, "Menu", C4FCT_Height))
    return FALSE;
  if (!fctFogOfWar.Load(File, "FogOfWar"))
    return FALSE;

  // Game palette
  if (!File.AccessEntry("C4.pal") || (!File.Read(GamePalette, 256 * 3)))
    return FALSE;
  for (cnt = 0; cnt < 256 * 3; cnt++)
    GamePalette[cnt] <<= 2;

  // Set default force field color
  GamePalette[191 * 3 + 0] = 0;
  GamePalette[191 * 3 + 1] = 0;
  GamePalette[191 * 3 + 2] = 255;

  return TRUE;
}