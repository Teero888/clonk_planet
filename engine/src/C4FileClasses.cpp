/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some class entries in the Windows registry */

#include <C4Include.h>

#define C4FileClassContentType "application/clonk4"

BOOL SetProtocol(const char *szProtocol, const char *szCommand,
                 const char *szModule) {

  if (!SetRegClassesRoot(szProtocol, NULL, "URL: Protocol"))
    return FALSE;
  if (!SetRegClassesRoot(szProtocol, "URL Protocol", ""))
    return FALSE;

  char szCmd[_MAX_PATH + 1], szKey[_MAX_PATH + 1];
  sprintf(szCmd, szCommand, szModule);
  sprintf(szKey, "%s\\shell\\open\\command", szProtocol);
  if (!SetRegClassesRoot(szKey, "", szCmd))
    return FALSE;

  char szIconpath[_MAX_PATH + 1];
  sprintf(szIconpath, "%s,1", szModule);
  sprintf(szKey, "%s\\DefaultIcon", szProtocol);
  if (!SetRegClassesRoot(szKey, "", szIconpath))
    return FALSE;

  return TRUE;
}

BOOL SetC4FileClasses(const char *szEnginePath, const char *szFrontendPath) {

  if (!SetRegFileClass("Clonk4.Scenario", "c4s", "Clonk 4 Scenario",
                       szEnginePath, 1, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Group", "c4g", "Clonk 4 Group", szEnginePath, 2,
                       C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Folder", "c4f", "Clonk 4 Folder", szEnginePath,
                       3, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Player", "c4p", "Clonk 4 Player", szEnginePath,
                       4, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Executable", "c4x", "Clonk 4 Engine",
                       szEnginePath, 5, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Definition", "c4d", "Clonk 4 Object Definition",
                       szEnginePath, 6, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Object", "c4i", "Clonk 4 Object Info",
                       szEnginePath, 7, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Material", "c4m", "Clonk 4 Material",
                       szEnginePath, 8, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Binary", "c4b", "Clonk 4 Binary", szEnginePath,
                       9, C4FileClassContentType))
    return FALSE;
  if (!SetRegFileClass("Clonk4.Video", "c4v", "Clonk 4 Video", szEnginePath, 10,
                       "video/avi"))
    return FALSE;

  if (!SetProtocol("clonk", "%s %%1 /Fullscreen", szEnginePath))
    return FALSE;
  if (!SetProtocol("clonkplanet", "%s /Net", szFrontendPath))
    return FALSE;

  return TRUE;
}
