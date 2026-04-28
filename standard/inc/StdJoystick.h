/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Simple joystick handling with DirectInput 1 */

const DWORD PAD_None = 0,
						PAD_Up = 1,
						PAD_Down = 2,
						PAD_Left = 4,
						PAD_Right = 8;

BOOL StdSetGamepadCalibration(int iMinX, int iMaxX, int iMinY, int iMaxY);
BOOL StdGetGamepadCalibration(int &iMinX, int &iMaxX, int &iMinY, int &iMaxY);
BOOL StdGetGamepad(int id, BOOL fExtended, DWORD &dwPos, DWORD &dwButtons);

BOOL StdGetJoyPos(int id, BOOL fExtended, DWORD &dwXPos, DWORD &dwYPos, DWORD &dwButtons);

int GetFirstSetBit(DWORD dwBitArray); // Returns -1 if no bit is set
