#include <Standard.h>
#include <StdRegistry.h>

#ifndef _WIN32
BOOL DeleteRegistryValue(const char *szSubKey, const char *szValueName) { return FALSE; }
BOOL DeleteRegistryValue(HKEY hKey, const char *szSubKey, const char *szValueName) { return FALSE; }
BOOL SetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD dwValue) { return FALSE; }
BOOL GetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD *lpdwValue) { return FALSE; }
BOOL GetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD *lpdwValue) { return FALSE; }
BOOL SetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD dwValue) { return FALSE; }
BOOL GetRegistryString(const char *szSubKey, const char *szValueName, char *szValue, DWORD dwValSize) { return FALSE; }
BOOL SetRegistryString(const char *szSubKey, const char *szValueName, const char *szValue) { return FALSE; }
BOOL SetRegClassesRoot(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL SetRegClassesRootString(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL StoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey, BOOL fStoreSize) { return FALSE; }
BOOL RestoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey) { return FALSE; }
BOOL SetRegFileClass(const char *szClassRoot, const char *szExtension, const char *szClassName, const char *szIconPath, int iIconIndex, const char *szContentType) { return FALSE; }
#endif
