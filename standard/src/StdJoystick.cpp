/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Simple joystick handling with DirectInput 1 */

#include <Windows.h>
#include <WindowsX.h>
#include <MMSystem.h>

#include <Standard.h>
#include <StdJoystick.h>

DWORD dwStdGamepadMaxX = 0;
DWORD dwStdGamepadMinX = 0;
DWORD dwStdGamepadMaxY = 0;
DWORD dwStdGamepadMinY = 0;

BOOL StdSetGamepadCalibration(int iMinX, int iMaxX, int iMinY, int iMaxY)
	{
	dwStdGamepadMaxX = iMaxX;
	dwStdGamepadMinX = iMinX;
	dwStdGamepadMaxY = iMaxY;
	dwStdGamepadMinY = iMinY;
	return TRUE;
	}

BOOL StdGetGamepadCalibration(int &iMinX, int &iMaxX, int &iMinY, int &iMaxY)
	{
	iMaxX = dwStdGamepadMaxX;
	iMinX = dwStdGamepadMinX;
	iMaxY = dwStdGamepadMaxY;
	iMinY = dwStdGamepadMinY;
	return TRUE;
	}

BOOL StdGetGamepad(int id, BOOL fExtended, DWORD &dwPos, DWORD &dwButtons)
	{
	DWORD dwXPos,dwYPos;

	if (!StdGetJoyPos(id,fExtended,dwXPos,dwYPos,dwButtons)) return FALSE;

	// Calibration range
	dwStdGamepadMaxX = Max(dwStdGamepadMaxX,dwXPos);
	dwStdGamepadMinX = Min(dwStdGamepadMinX,dwXPos);
	dwStdGamepadMaxY = Max(dwStdGamepadMaxY,dwYPos);
	dwStdGamepadMinY = Min(dwStdGamepadMinY,dwYPos);

	DWORD dwCenterX = (dwStdGamepadMaxX + dwStdGamepadMinX) / 2;
	DWORD dwCenterY = (dwStdGamepadMaxY + dwStdGamepadMinY) / 2;
	DWORD dwRangeX = (dwStdGamepadMaxX - dwCenterX) / 3;
	DWORD dwRangeY = (dwStdGamepadMaxY - dwCenterY) / 3;

	// Evaluate
	dwPos = PAD_None;
	if ( dwXPos < dwCenterX - dwRangeX )  dwPos |= PAD_Left;
	if ( dwXPos > dwCenterX + dwRangeX )  dwPos |= PAD_Right;
	if ( dwYPos < dwCenterY - dwRangeY )  dwPos |= PAD_Up;
	if ( dwYPos > dwCenterY + dwRangeY )  dwPos |= PAD_Down;

	return TRUE;

	}

BOOL StdGetJoyPos(int id, BOOL fExtended, DWORD &dwXPos, DWORD &dwYPos, DWORD &dwButtons)
  {
	MMRESULT lResult;
  
	if (fExtended)
		{
		JOYINFOEX joyInfoEx;
		joyInfoEx.dwSize=sizeof(joyInfoEx);
		joyInfoEx.dwFlags=JOY_RETURNBUTTONS | JOY_RETURNRAWDATA | JOY_RETURNX | JOY_RETURNY; 
		if ((lResult=joyGetPosEx(JOYSTICKID1,&joyInfoEx)) == JOYERR_NOERROR) 
			{
			dwXPos=joyInfoEx.dwXpos;
			dwYPos=joyInfoEx.dwYpos;
			dwButtons=joyInfoEx.dwButtons;
			return TRUE;
			}
		}

	else
		{
		JOYINFO joyInfo;
		if ((lResult=joyGetPos(JOYSTICKID1,&joyInfo)) == JOYERR_NOERROR) 
			{
			dwXPos=joyInfo.wXpos;
			dwYPos=joyInfo.wYpos;
			dwButtons=joyInfo.wButtons;
			return TRUE;
			}
		}

	return FALSE;
  }

int GetFirstSetBit(DWORD dwBitArray)
	{
	for (int bitn=0; bitn<32; bitn++)
		if ( dwBitArray & ((DWORD) 1 << bitn) )
			return bitn;
	
	return -1;
	}

