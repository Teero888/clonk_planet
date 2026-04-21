/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some wrappers for easier access to the Windows registry */

BOOL DeleteRegistryValue(HKEY hKey, const char *szSubKey, 
												 const char *szValueName);

BOOL DeleteRegistryValue(const char *szSubKey, 
												 const char *szValueName);

BOOL GetRegistryDWord(HKEY hKey, const char *szSubKey, 
                      const char *szValueName, DWORD *lpdwValue);

BOOL GetRegistryDWord(const char *szSubKey, 
                      const char *szValueName, DWORD *lpdwValue);

BOOL SetRegistryDWord(HKEY hKey, const char *szSubKey, 
                      const char *szValueName, DWORD dwValue);

BOOL SetRegistryDWord(const char *szSubKey, 
                      const char *szValueName, DWORD dwValue);

BOOL GetRegistryString(const char *szSubKey, 
                       const char *szValueName, 
                       char *sValue, DWORD dwValSize);

BOOL SetRegistryString(const char *szSubKey, 
                       const char *szValueName, 
                       const char *szValue);

BOOL SetRegClassesRoot(const char *szSubKey,
                       const char *szValueName,
                       const char *szStringValue);

BOOL SetRegShell(const char *szClassName, 
                 const char *szShellName,
                 const char *szShellCaption,
                 const char *szCommand);

BOOL SetRegFileClass(const char *szClassRoot,
                     const char *szExtension,
                     const char *szClassName,
                     const char *szIconPath, int iIconNum,
										 const char *szContentType);

BOOL StoreWindowPosition(HWND hwnd, 
												 const char *szWindowName, 
												 const char *szSubKey,
												 BOOL fStoreSize = TRUE);

BOOL RestoreWindowPosition(HWND hwnd, 
													 const char *szWindowName, 
													 const char *szSubKey);
