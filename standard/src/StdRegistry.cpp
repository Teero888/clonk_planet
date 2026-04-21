/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some wrappers for easier access to the Windows registry */

#include <Windows.h>
#include <Stdio.h>

#include <Standard.h>
#include <StdRegistry.h>

BOOL DeleteRegistryValue(const char *szSubKey, const char *szValueName)
	{
	return DeleteRegistryValue(HKEY_CURRENT_USER,szSubKey,szValueName);
	}

BOOL DeleteRegistryValue(HKEY hKey, const char *szSubKey, const char *szValueName)
	{
  long qerr;
  HKEY ckey;  
  // Open the key
  if ((qerr=RegOpenKeyEx(hKey,
                         szSubKey,
                         0,
                         KEY_ALL_ACCESS,
                         &ckey
                        ))!=ERROR_SUCCESS) return FALSE;
	// Delete the key
  if ((qerr=RegDeleteValue(ckey,
													 szValueName
													 ))!=ERROR_SUCCESS) return FALSE;
  // Close the key
  RegCloseKey(ckey);
	// Success
	return TRUE;
	}

BOOL SetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD dwValue)
	{
	return SetRegistryDWord(HKEY_CURRENT_USER,szSubKey,szValueName,dwValue);
	}

BOOL GetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD *lpdwValue)
	{
	return GetRegistryDWord(HKEY_CURRENT_USER,szSubKey,szValueName,lpdwValue);
	}

BOOL GetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD *lpdwValue)
  {
  long qerr;
  HKEY ckey;
  DWORD valtype;
  DWORD valsize=sizeof(DWORD);
  
  // Open the key
  if ((qerr=RegOpenKeyEx(hKey,
                         szSubKey,
                         0,
                         KEY_READ,
                         &ckey
                        ))!=ERROR_SUCCESS) return FALSE;

  // Get the value
  if ((qerr=RegQueryValueEx(ckey,
                            szValueName,
                            NULL,
                            &valtype,
                            (BYTE*) lpdwValue,
                            &valsize
                           ))!=ERROR_SUCCESS)  { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);
  
  if (valtype!=REG_DWORD) return FALSE;

  return TRUE;
  }

BOOL SetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD dwValue)
  {
  long qerr;
  HKEY ckey;
  DWORD disposition;
  // Open the key
  if ((qerr=RegCreateKeyEx(hKey,
                           szSubKey,
                           0,
                           "",
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 
                           &ckey,
                           &disposition
                          ))!=ERROR_SUCCESS) return FALSE;
  // Set the value
  if ((qerr=RegSetValueEx(ckey,
                          szValueName,
                          0,
                          REG_DWORD,
                          (BYTE*) &dwValue,
                          sizeof(dwValue)
                         ))!=ERROR_SUCCESS) { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);
	// Success
	return TRUE;
  }

BOOL GetRegistryString(const char *szSubKey, 
                       const char *szValueName, 
                       char *sValue, DWORD dwValSize)
  {
  long qerr;
  HKEY ckey;
  DWORD valtype;
  
  // Open the key
  if ((qerr=RegOpenKeyEx(HKEY_CURRENT_USER,
                         szSubKey,
                         0,
                         KEY_READ,
                         &ckey
                        ))!=ERROR_SUCCESS) return FALSE;

  // Get the value
  if ((qerr=RegQueryValueEx(ckey,
                            szValueName,
                            NULL,
                            &valtype,
                            (BYTE*) sValue,
                            &dwValSize
                           ))!=ERROR_SUCCESS)  { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);
  
  if (valtype!=REG_SZ) return FALSE;

  return TRUE;
  }

BOOL SetRegistryString(const char *szSubKey, 
                       const char *szValueName, 
                       const char *szValue)
  {
  
  long qerr;
  HKEY ckey;
  DWORD disposition;
  
  // Open the key
  if ((qerr=RegCreateKeyEx(HKEY_CURRENT_USER,
                           szSubKey,
                           0,
                           "",
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 
                           &ckey,
                           &disposition
                          ))!=ERROR_SUCCESS) return FALSE;

  // Set the value
  if ((qerr=RegSetValueEx(ckey,
                          szValueName,
                          0,
                          REG_SZ,
                          (BYTE*) szValue,
                          SLen(szValue)+1
                         ))!=ERROR_SUCCESS) { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);

  return TRUE;
  }

BOOL SetRegClassesRoot(const char *szSubKey,
                       const char *szValueName,
                       const char *szStringValue)
  {
  
  long qerr;
  HKEY ckey;
  DWORD disposition;
  
  // Open the key
  if ((qerr=RegCreateKeyEx(HKEY_CLASSES_ROOT,
                           szSubKey,
                           0,
                           "",
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 
                           &ckey,
                           &disposition
                          ))!=ERROR_SUCCESS) return FALSE;

  // Set the value
  if ((qerr=RegSetValueEx(ckey,
                          szValueName,
                          0,
                          REG_SZ,
                          (BYTE*) szStringValue,
                          SLen(szStringValue)+1
                         ))!=ERROR_SUCCESS) { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);

  return TRUE;
  }

BOOL SetRegClassesRootString(const char *szSubKey, 
														 const char *szValueName, 
														 const char *szValue)
  {
  
  long qerr;
  HKEY ckey;
  DWORD disposition;
  
  // Open the key
  if ((qerr=RegCreateKeyEx(HKEY_CLASSES_ROOT,
                           szSubKey,
                           0,
                           "",
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           NULL,                 
                           &ckey,
                           &disposition
                          ))!=ERROR_SUCCESS) return FALSE;

  // Set the value
  if ((qerr=RegSetValueEx(ckey,
                          szValueName,
                          0,
                          REG_SZ,
                          (BYTE*) szValue,
                          SLen(szValue)+1
                         ))!=ERROR_SUCCESS) { RegCloseKey(ckey); return FALSE; }

  // Close the key
  RegCloseKey(ckey);

  return TRUE;
  }

BOOL SetRegShell(const char *szClassName, 
                 const char *szShellName,
                 const char *szShellCaption,
                 const char *szCommand)
  {
  char szKeyName[256+1];
  // Set shell caption
  sprintf(szKeyName,"%s\\Shell\\%s",szClassName,szShellName);
  if (!SetRegClassesRoot(szKeyName, NULL, szShellCaption)) return FALSE;
  // Set shell command
  sprintf(szKeyName,"%s\\Shell\\%s\\Command",szClassName,szShellName);
  if (!SetRegClassesRoot(szKeyName, NULL, szCommand)) return FALSE;
  return TRUE;  
  }

BOOL SetRegFileClass(const char *szClassRoot,
                     const char *szExtension,
                     const char *szClassName,
                     const char *szIconPath, int iIconNum,
										 const char *szContentType)
  { 
  char keyname[100];
  char iconpath[512];
	// Create root class entry
  if (!SetRegClassesRoot(szClassRoot,NULL,szClassName)) return FALSE; 
	// Set root class icon
  sprintf(keyname,"%s\\DefaultIcon",szClassRoot);
  sprintf(iconpath,"%s,%d",szIconPath,iIconNum);
  if (!SetRegClassesRoot(keyname,NULL,iconpath)) return FALSE;
	// Set extension map entry
  sprintf(keyname,".%s",szExtension);
  if (!SetRegClassesRoot(keyname,NULL,szClassRoot)) return FALSE;  
	// Set extension content type
  sprintf(keyname,".%s",szExtension);
	if (!SetRegClassesRootString(keyname,"Content Type",szContentType)) return FALSE;  
	// Success
  return TRUE;
  }


//------------------------------ Window Position ------------------------------------------

BOOL StoreWindowPosition(HWND hwnd, 
												 const char *szWindowName, 
												 const char *szSubKey,
												 BOOL fStoreSize)
	{
	RECT winpos;
	char regstr[100];
	if (IsZoomed(hwnd)) 
		return SetRegistryString(szSubKey,szWindowName,"Maximized");
	if (IsIconic(hwnd))
		return SetRegistryString(szSubKey,szWindowName,"Minimized");
	if (!GetWindowRect(hwnd,&winpos)) return FALSE;
	if (fStoreSize) sprintf(regstr,"%i,%i,%i,%i",winpos.left,winpos.top,winpos.right-winpos.left,winpos.bottom-winpos.top);
	else sprintf(regstr,"%i,%i",winpos.left,winpos.top);
	return SetRegistryString(szSubKey,szWindowName,regstr);
	}

BOOL RestoreWindowPosition(HWND hwnd, 
													 const char *szWindowName, 
													 const char *szSubKey)
	{
	char regstr[100],buffer2[5];
	int x,y,wdt,hgt;
	BOOL fSetSize=TRUE;
	if (!GetRegistryString(szSubKey,szWindowName,regstr,100)) return FALSE;
	if (SEqual(regstr,"Maximized"))
		return ShowWindow(hwnd,SW_MAXIMIZE | SW_NORMAL);
	if (SEqual(regstr,"Minimized"))
		return ShowWindow(hwnd,SW_MINIMIZE | SW_NORMAL);
	SCopySegment(regstr,0,buffer2,',',4); sscanf(buffer2,"%i",&x);
	SCopySegment(regstr,1,buffer2,',',4); sscanf(buffer2,"%i",&y);
	if (SCopySegment(regstr,2,buffer2,',',4)) sscanf(buffer2,"%i",&wdt); else fSetSize=FALSE;
	if (SCopySegment(regstr,3,buffer2,',',4)) sscanf(buffer2,"%i",&hgt); else fSetSize=FALSE;
	if (!fSetSize)
		{
		RECT winpos; if (!GetWindowRect(hwnd,&winpos)) return FALSE;
		wdt=winpos.right-winpos.left; hgt=winpos.bottom-winpos.top; 
		}
	if (!MoveWindow(hwnd,x,y,wdt,hgt,TRUE)) return FALSE;
	return ShowWindow(hwnd,SW_NORMAL);
	}