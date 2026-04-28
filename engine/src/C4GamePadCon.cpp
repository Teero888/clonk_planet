/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Gamepad control */

#include <C4Include.h>

C4GamePadControl::C4GamePadControl() {
  Status = FALSE;
  Plr = -1;
  GamePad = 0;
  Pos = Buttons = 0;
  LastPos = LastButtons = 0;
  CallOkay = FALSE;
  Extended = TRUE;
}

BOOL C4GamePadControl::Initialize(int iPlr, int iGamepad) {
  // Set data
  Status = TRUE;
  Plr = iPlr;
  GamePad = iGamepad;
  // Set calibration
  StdSetGamepadCalibration(Config.Gamepad.MinX, Config.Gamepad.MaxX, Config.Gamepad.MinY, Config.Gamepad.MaxY);
  // Done
  return TRUE;
}

const int MaxGamePadButton = 10;

void C4GamePadControl::Execute() {
  // Not active
  if (!Status)
    return;
  // Get gamepad input
  CallOkay = StdGetGamepad(GamePad, Extended, Pos, Buttons);
  // Evaluate changes and pass single controls
  if (CallOkay) {
    if (Pos != LastPos) {
      if (Pos & PAD_Left)
        Game.LocalPlayerControl(Plr, COM_Left);
      if (Pos & PAD_Right)
        Game.LocalPlayerControl(Plr, COM_Right);
      if (Pos & PAD_Up)
        Game.LocalPlayerControl(Plr, COM_Up);
      if (Pos & PAD_Down)
        Game.LocalPlayerControl(Plr, COM_Down);
    }
    for (int iButton = 0; iButton < MaxGamePadButton; iButton++)
      if ((Buttons & (1 << iButton)) && !(LastButtons & (1 << iButton)))
        for (int iCtrl = 0; iCtrl < C4MaxKey; iCtrl++)
          if (Config.Gamepad.Button[iCtrl] - 1 == iButton)
            Game.LocalPlayerControl(Plr, Control2Com(iCtrl));
    LastPos = Pos;
    LastButtons = Buttons;
  }
}

BOOL C4GamePadControl::AnyButtonDown() {
  if (!Status)
    FALSE;

  if (StdGetGamepad(GamePad, Extended, Pos, Buttons))
    if (Buttons)
      return TRUE;

  return FALSE;
}
