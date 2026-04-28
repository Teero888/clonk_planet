/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Play midis using mci */

#include <Compat.h>
// #include <mmsystem.h>
#include <Midi.h>
#include <stdio.h>

#ifdef _WIN32

BOOL PlayMidi(const char *sFileName, HWND appWnd)
	{
  char buf[256];
  sprintf(buf, "open %s type sequencer alias ITSMYMUSIC", sFileName);  
	if (mciSendString("close all", NULL, 0, NULL) != 0)
    return FALSE;  
  if (mciSendString(buf, NULL, 0, NULL) != 0)
    return FALSE;
  if (mciSendString("play ITSMYMUSIC from 0 notify", NULL, 0, appWnd) != 0)
    return FALSE; 
  return TRUE;
	}

BOOL PauseMidi()
	{
	if (mciSendString("stop ITSMYMUSIC", NULL, 0, NULL) != 0) return FALSE;
	return TRUE;
	}

BOOL ResumeMidi(HWND appWnd)
	{       
	if (mciSendString("play ITSMYMUSIC notify", NULL, 0, appWnd) != 0) return FALSE;
	return TRUE;
	}

BOOL StopMidi()
	{
	if (mciSendString("close all", NULL, 0, NULL) != 0) return FALSE;
	return TRUE;
	}

BOOL ReplayMidi(HWND appWnd)
	{
	if (mciSendString("play ITSMYMUSIC from 0 notify", NULL, 0, appWnd) != 0) return FALSE;
	return TRUE;
	}


#endif

#ifndef _WIN32
BOOL StopMidi() { return FALSE; }
BOOL PlayMidi(const char *szConfig, HWND hWnd) { return FALSE; }
#endif
