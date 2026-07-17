#include <Standard.h>
#include <StdRegistry.h>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

static std::map<std::string, std::string> *g_registry = nullptr;
static bool g_registryLoaded = false;

static std::map<std::string, std::string> &GetRegistryMap() {
  if (!g_registry) {
    g_registry = new std::map<std::string, std::string>();
  }
  return *g_registry;
}

static std::string GetConfigPath() {
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

static std::string Trim(const std::string &s) {
  size_t first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  size_t last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, (last - first + 1));
}

static void LoadRegistry() {
  if (g_registryLoaded)
    return;
  auto &reg = GetRegistryMap();
  std::string path = "clonk.ini";
  std::ifstream f(path);
  if (!f.is_open()) {
    path = GetConfigPath();
    f.open(path);
  }
  if (!f.is_open()) {
    g_registryLoaded = true;
    return;
  }
  std::string line, section;
  while (std::getline(f, line)) {
    line = Trim(line);
    if (line.empty() || line[0] == ';')
      continue;
    if (line[0] == '[' && line.back() == ']') {
      section = Trim(line.substr(1, line.size() - 2));
      continue;
    }
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string keyName = Trim(line.substr(0, pos));
      std::string val = Trim(line.substr(pos + 1));
      std::string key = section + "\\" + keyName;
      reg[key] = val;
    }
  }
  g_registryLoaded = true;
}

static void SaveRegistry() {
  if (!g_registry)
    return;
  auto &reg = *g_registry;
  std::string path = "clonk.ini";
  std::ifstream check(path);
  if (!check.is_open()) {
    path = GetConfigPath();
  }
  std::ofstream f(path);
  std::string currentSection;
  for (auto const &[key, val] : reg) {
    size_t pos = key.find('\\');
    if (pos != std::string::npos) {
      std::string section = key.substr(0, pos);
      if (section != currentSection) {
        f << "[" << section << "]\n";
        currentSection = section;
      }
      f << key.substr(pos + 1) << "=" << val << "\n";
    } else {
      f << key << "=" << val << "\n";
    }
  }
}

BOOL DeleteRegistryValue(const char *szSubKey, const char *szValueName) {
  LoadRegistry();
  GetRegistryMap().erase(std::string(szSubKey) + "\\" + szValueName);
  SaveRegistry();
  return TRUE;
}
BOOL DeleteRegistryValue(HKEY hKey, const char *szSubKey, const char *szValueName) { return DeleteRegistryValue(szSubKey, szValueName); }

BOOL SetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD dwValue) {
  LoadRegistry();
  GetRegistryMap()[std::string(szSubKey) + "\\" + szValueName] = std::to_string(dwValue);
  SaveRegistry();
  return TRUE;
}
BOOL GetRegistryDWord(const char *szSubKey, const char *szValueName, DWORD *lpdwValue) {
  LoadRegistry();
  std::string key = std::string(szSubKey) + "\\" + szValueName;
  auto &reg = GetRegistryMap();
  if (reg.count(key)) {
    *lpdwValue = std::stoul(reg[key]);
    return TRUE;
  }
  return FALSE;
}
BOOL GetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD *lpdwValue) { return GetRegistryDWord(szSubKey, szValueName, lpdwValue); }
BOOL SetRegistryDWord(HKEY hKey, const char *szSubKey, const char *szValueName, DWORD dwValue) { return SetRegistryDWord(szSubKey, szValueName, dwValue); }

BOOL GetRegistryString(const char *szSubKey, const char *szValueName, char *szValue, DWORD dwValSize) {
  LoadRegistry();
  std::string key = std::string(szSubKey) + "\\" + szValueName;
  auto &reg = GetRegistryMap();
  if (reg.count(key)) {
    strncpy(szValue, reg[key].c_str(), dwValSize);
    return TRUE;
  }
  return FALSE;
}
BOOL SetRegistryString(const char *szSubKey, const char *szValueName, const char *szValue) {
  LoadRegistry();
  GetRegistryMap()[std::string(szSubKey) + "\\" + szValueName] = szValue;
  SaveRegistry();
  return TRUE;
}
BOOL SetRegClassesRoot(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL SetRegClassesRootString(const char *szSubKey, const char *szValueName, const char *szStringValue) { return FALSE; }
BOOL StoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey, BOOL fStoreSize) { return FALSE; }
BOOL RestoreWindowPosition(HWND hwnd, const char *szWindowName, const char *szSubKey) { return FALSE; }
BOOL SetRegFileClass(const char *szClassRoot, const char *szExtension, const char *szClassName, const char *szIconPath, int iIconIndex, const char *szContentType) { return FALSE; }
