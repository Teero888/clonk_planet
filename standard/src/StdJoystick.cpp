/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Simple joystick handling with DirectInput 1 */

#include <Compat.h>
#include <Compat.h>
// #include <MMSystem.h>

#include <Standard.h>
#include <StdJoystick.h>

#include <GLFW/glfw3.h>

DWORD dwStdGamepadMaxX = 65535;
DWORD dwStdGamepadMinX = 0;
DWORD dwStdGamepadMaxY = 65535;
DWORD dwStdGamepadMinY = 0;

BOOL StdSetGamepadCalibration(int iMinX, int iMaxX, int iMinY, int iMaxY) {
  dwStdGamepadMaxX = iMaxX;
  dwStdGamepadMinX = iMinX;
  dwStdGamepadMaxY = iMaxY;
  dwStdGamepadMinY = iMinY;
  return TRUE;
}

BOOL StdGetGamepadCalibration(int &iMinX, int &iMaxX, int &iMinY, int &iMaxY) {
  iMaxX = dwStdGamepadMaxX;
  iMinX = dwStdGamepadMinX;
  iMaxY = dwStdGamepadMaxY;
  iMinY = dwStdGamepadMinY;
  return TRUE;
}

BOOL StdGetGamepad(int id, BOOL fExtended, DWORD &dwPos, DWORD &dwButtons) {
  int jid = (id == 0) ? GLFW_JOYSTICK_1 : GLFW_JOYSTICK_2;
  if (!glfwJoystickPresent(jid))
    return FALSE;

  int count;
  const float *axes = glfwGetJoystickAxes(jid, &count);
  const unsigned char *buttons = glfwGetJoystickButtons(jid, &count);

  dwButtons = 0;
  if (buttons) {
    for (int i = 0; i < count && i < 32; i++) {
      if (buttons[i] == GLFW_PRESS)
        dwButtons |= (1 << i);
    }
  }

  dwPos = PAD_None;
  if (axes && count >= 2) {
    if (axes[0] < -0.3f)
      dwPos |= PAD_Left;
    if (axes[0] > 0.3f)
      dwPos |= PAD_Right;
    if (axes[1] < -0.3f)
      dwPos |= PAD_Up;
    if (axes[1] > 0.3f)
      dwPos |= PAD_Down;
  }

  return TRUE;
}

BOOL StdGetJoyPos(int id, BOOL fExtended, DWORD &dwXPos, DWORD &dwYPos, DWORD &dwButtons) {
  int jid = (id == 0) ? GLFW_JOYSTICK_1 : GLFW_JOYSTICK_2;
  if (!glfwJoystickPresent(jid))
    return FALSE;

  int count;
  const float *axes = glfwGetJoystickAxes(jid, &count);
  const unsigned char *buttons = glfwGetJoystickButtons(jid, &count);

  dwButtons = 0;
  if (buttons) {
    for (int i = 0; i < count && i < 32; i++) {
      if (buttons[i] == GLFW_PRESS)
        dwButtons |= (1 << i);
    }
  }

  dwXPos = 32768;
  dwYPos = 32768;
  if (axes && count >= 2) {
    dwXPos = (DWORD)((axes[0] + 1.0f) * 32767.5f);
    dwYPos = (DWORD)((axes[1] + 1.0f) * 32767.5f);
  }

  return TRUE;
}

int GetFirstSetBit(DWORD dwBitArray) {
  for (int bitn = 0; bitn < 32; bitn++)
    if (dwBitArray & ((DWORD)1 << bitn))
      return bitn;

  return -1;
}
