#include <Standard.h>
#include <StdRegistry.h>
#include <StdIniRegistry.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>

// The registry is emulated by the user's clonk.ini (see StdIniRegistry.h), created from the
// defaults next to the executable.
static CStdIniRegistry *g_registry = nullptr;
static std::string g_registryPath;

static std::string GetDefaultsPath() {
#ifdef _WIN32
  char buf[1024];
  DWORD len = GetModuleFileNameA(NULL, buf, sizeof(buf));
  if (len > 0) {
    char *lastSlash = strrchr(buf, '\\');
    if (!lastSlash) lastSlash = strrchr(buf, '/');
    if (lastSlash) {
      strcpy(lastSlash + 1, "clonk.ini");
      return std::string(buf);
    }
  }
  return "clonk.ini";
#else
  char buf[1024];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (len != -1) {
    buf[len] = '\0';
    char *lastSlash = strrchr(buf, '/');
    if (lastSlash) {
      strcpy(lastSlash + 1, "clonk.ini");
      return std::string(buf);
    }
  }
  return "clonk.ini";
#endif
}

static CStdIniRegistry &Registry() {
  if (!g_registry) {
    g_registry = new CStdIniRegistry();
    g_registryPath = CStdIniRegistry::PrepareUserConfig(GetDefaultsPath());
    g_registry->Load(g_registryPath);
    // a file of the old flat [Software] format is written in the new format right away
    if (g_registry->WasLegacy()) g_registry->Save(g_registryPath);
  }
  return *g_registry;
}

static void SaveRegistry() {
  Registry().Save(g_registryPath);
}

// Only HKEY_CURRENT_USER is emulated (other keys like HKLM\Software\Microsoft\DirectDraw have no
// meaning outside of Windows)
static bool IsUserKey(HKEY hKey) { return hKey == HKEY_CURRENT_USER; }

BOOL DeleteRegistryValue(const char *szSubKey, const char *szValueName) {
  if (!Registry().Delete(CStdIniRegistry::SectionOfSubKey(szSubKey), szValueName)) return FALSE;
  SaveRegistry();
  return TRUE;
}
BOOL DeleteRegistryValue(HKEY hKey, const char *szSubKey, const char *szValueName) {
  return IsUserKey(hKey) && DeleteRegistryValue(szSubKey, szValueName);
}

BOOL SetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD dwValue) {
  Registry().Set(CStdIniRegistry::SectionOfSubKey(szSubKey), szValueName, std::to_string(dwValue));
  SaveRegistry();
  return TRUE;
}
BOOL GetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD *lpdwValue) {
  std::string value;
  if (!Registry().Get(CStdIniRegistry::SectionOfSubKey(szSubKey), szValueName, value)) return FALSE;
  // DWORDs are written unsigned (-1 = 4294967295); accept signed numbers written by hand too
  char *end = nullptr;
  long long number = strtoll(value.c_str(), &end, 10);
  if (end == value.c_str()) return FALSE;
  *lpdwValue = (DWORD)number;
  return TRUE;
}
BOOL GetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD *lpdwValue) {
  return IsUserKey(hKey) && GetRegistryDWord(szSubKey, szValueName, lpdwValue);
}
BOOL SetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD dwValue) {
  return IsUserKey(hKey) && SetRegistryDWord(szSubKey, szValueName, dwValue);
}

BOOL GetRegistryString(const char *szSubKey, const char *szValueName, char *szValue, DWORD dwValSize) {
  std::string value;
  if (!dwValSize || !Registry().Get(CStdIniRegistry::SectionOfSubKey(szSubKey), szValueName, value)) return FALSE;
  size_t len = std::min<size_t>(value.size(), dwValSize - 1);
  memcpy(szValue, value.c_str(), len);
  szValue[len] = '\0';
  return TRUE;
}
BOOL SetRegistryString(const char *szSubKey, const char *szValueName, const char *szValue) {
  Registry().Set(CStdIniRegistry::SectionOfSubKey(szSubKey), szValueName, szValue);
  SaveRegistry();
  return TRUE;
}
BOOL SetRegClassesRoot(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL SetRegClassesRootString(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL StoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey, BOOL fStoreSize) { return FALSE; }
BOOL RestoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey) { return FALSE; }
BOOL SetRegFileClass(const char *szClassRoot, const char *szExtension, const char *szClassName, const char *szIconPath, int iIconIndex, const char *szContentType) { return FALSE; }
