/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Log file handling */

BOOL Log(const char *szMessage);
BOOL NetLog(const char *szMessage);
void CloseLog(const char *szFileName);
void LogProcess(int iValue);
BOOL LogProcessCallback(const char *szFormat, int iValue);
