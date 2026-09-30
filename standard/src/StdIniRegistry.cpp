/* Registry emulation file (clonk.ini), see StdIniRegistry.h */

#include <StdIniRegistry.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace {

const char *const RootKey = "Software\\RedWolf Design\\Clonk 4\\";

// Sections in the order of C4Config (used when converting the old format, which was sorted by name)
const char *const SectionOrder[] = {"General", "Developer", "Graphics", "Sound", "Network",
                                    "Explorer", "Controls", "Gamepad"};

std::string Trim(const std::string &s) {
  size_t first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  size_t last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

bool EqualNoCase(const std::string &a, const std::string &b) {
  return a.size() == b.size() &&
         std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) { return std::tolower(x) == std::tolower(y); });
}

bool StartsWithNoCase(const std::string &s, const std::string &prefix) {
  return s.size() >= prefix.size() && EqualNoCase(s.substr(0, prefix.size()), prefix);
}

// "Kbd1Key2" < "Kbd1Key10": numbers compared by value
bool NaturalLess(const std::string &a, const std::string &b) {
  size_t i = 0, j = 0;
  while (i < a.size() && j < b.size()) {
    if (std::isdigit((unsigned char)a[i]) && std::isdigit((unsigned char)b[j])) {
      size_t ei = i, ej = j;
      while (ei < a.size() && std::isdigit((unsigned char)a[ei])) ei++;
      while (ej < b.size() && std::isdigit((unsigned char)b[ej])) ej++;
      std::string na = a.substr(i, ei - i), nb = b.substr(j, ej - j);
      na.erase(0, std::min(na.find_first_not_of('0'), na.size()));
      nb.erase(0, std::min(nb.find_first_not_of('0'), nb.size()));
      if (na.size() != nb.size()) return na.size() < nb.size();
      if (na != nb) return na < nb;
      i = ei;
      j = ej;
      continue;
    }
    int ca = std::tolower((unsigned char)a[i]), cb = std::tolower((unsigned char)b[j]);
    if (ca != cb) return ca < cb;
    i++;
    j++;
  }
  return a.size() - i < b.size() - j;
}

size_t OrderIndex(const std::string &section) {
  const size_t count = sizeof(SectionOrder) / sizeof(SectionOrder[0]);
  for (size_t i = 0; i < count; i++)
    if (EqualNoCase(section, SectionOrder[i])) return i;
  return count;
}

} // namespace

std::string CStdIniRegistry::SectionOfSubKey(const std::string &subKey) {
  if (StartsWithNoCase(subKey, RootKey)) return subKey.substr(strlen(RootKey));
  return subKey;
}

std::string CStdIniRegistry::PrepareUserConfig(const std::string &defaultsPath) {
  namespace fs = std::filesystem;
  auto env = [](const char *name) -> std::string {
    const char *v = std::getenv(name);
    return v ? v : "";
  };
  fs::path path;
  if (!env("CLONK_CONFIG").empty()) {
    path = env("CLONK_CONFIG");
  } else {
#if defined(_WIN32)
    if (env("APPDATA").empty()) return defaultsPath;
    path = fs::path(env("APPDATA")) / "Clonk Planet";
#elif defined(__APPLE__)
    if (env("HOME").empty()) return defaultsPath;
    path = fs::path(env("HOME")) / "Library" / "Application Support" / "Clonk Planet";
#else
    if (!env("XDG_CONFIG_HOME").empty())
      path = fs::path(env("XDG_CONFIG_HOME")) / "clonk-planet";
    else if (!env("HOME").empty())
      path = fs::path(env("HOME")) / ".config" / "clonk-planet";
    else
      return defaultsPath;
#endif
    path /= "clonk.ini";
  }

  std::error_code ec;
  if (!fs::exists(path, ec)) {
    fs::create_directories(path.parent_path(), ec);
    CStdIniRegistry defaults;
    defaults.Load(defaultsPath);
    defaults.Header.clear(); // the explanation at the top of the defaults file
    if (!defaults.Save(path.string())) return defaultsPath;
  }
  return path.string();
}

void CStdIniRegistry::Clear() {
  Header.clear();
  Sections.clear();
  fLegacy = false;
}

bool CStdIniRegistry::Load(const std::string &path) {
  Clear();
  std::ifstream f(path, std::ios::binary);
  if (!f.is_open()) return false;

  std::vector<Line> legacy; // entries of an old [Software] section: full value path -> value
  size_t current = std::string::npos; // index of the section being read (npos: before the first one)
  bool inLegacy = false;
  std::string raw;
  while (std::getline(f, raw)) {
    std::string line = Trim(raw);
    if (!line.empty() && line.front() == '[' && line.back() == ']') {
      std::string name = Trim(line.substr(1, line.size() - 2));
      inLegacy = EqualNoCase(name, "Software");
      current = std::string::npos;
      if (!inLegacy) {
        AddSection(name);
        for (size_t i = 0; i < Sections.size(); i++)
          if (EqualNoCase(Sections[i].Name, name)) current = i;
      }
      continue;
    }
    if (line.empty() || line[0] == ';' || line[0] == '#') {
      if (inLegacy) continue;
      (current != std::string::npos ? Sections[current].Lines : Header).push_back({"", line});
      continue;
    }
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    Line entry{Trim(line.substr(0, eq)), Trim(line.substr(eq + 1))};
    if (inLegacy)
      legacy.push_back(entry);
    else if (current != std::string::npos)
      SetLine(Sections[current], entry.Name, entry.Value);
  }

  // blank lines at the end of a section are written anew between sections
  for (Section &s : Sections)
    while (!s.Lines.empty() && s.Lines.back().Name.empty() && s.Lines.back().Value.empty()) s.Lines.pop_back();
  while (!Header.empty() && Header.back().Value.empty()) Header.pop_back();

  if (!legacy.empty()) {
    // "RedWolf Design\Clonk 4\General\Language" was the value Language of Software\RedWolf Design\Clonk 4\General
    fLegacy = true;
    // old files were sorted by name: sections in C4Config order (others in the order of the
    // file), numbered values by number
    struct Entry {
      size_t Order, FirstPos;
      std::string Section, Name, Value;
    };
    std::vector<Entry> entries;
    for (const Line &l : legacy) {
      size_t slash = l.Name.rfind('\\');
      if (slash == std::string::npos) continue;
      Entry e{0, entries.size(), SectionOfSubKey("Software\\" + l.Name.substr(0, slash)), l.Name.substr(slash + 1), l.Value};
      e.Order = OrderIndex(e.Section);
      for (const Entry &prev : entries)
        if (EqualNoCase(prev.Section, e.Section)) {
          e.FirstPos = prev.FirstPos;
          break;
        }
      entries.push_back(e);
    }
    std::stable_sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
      if (a.Order != b.Order) return a.Order < b.Order;
      if (a.FirstPos != b.FirstPos) return a.FirstPos < b.FirstPos;
      return NaturalLess(a.Name, b.Name);
    });
    for (const Entry &e : entries) {
      std::string existing;
      if (!Get(e.Section, e.Name, existing)) Set(e.Section, e.Name, e.Value);
    }
  }
  return true;
}

bool CStdIniRegistry::Save(const std::string &path) const {
  std::ofstream f(path, std::ios::binary | std::ios::trunc);
  if (!f.is_open()) return false;
  for (const Line &l : Header) f << l.Value << "\n";
  if (!Header.empty()) f << "\n";
  bool first = true;
  for (const Section &s : Sections) {
    if (!first) f << "\n";
    first = false;
    f << "[" << s.Name << "]\n";
    for (const Line &l : s.Lines) {
      if (l.Name.empty())
        f << l.Value << "\n";
      else
        f << l.Name << "=" << l.Value << "\n";
    }
  }
  return f.good();
}

bool CStdIniRegistry::Get(const std::string &section, const std::string &name, std::string &value) const {
  const Section *s = FindSection(section);
  if (!s) return false;
  for (const Line &l : s->Lines)
    if (!l.Name.empty() && EqualNoCase(l.Name, name)) {
      value = l.Value;
      return true;
    }
  return false;
}

void CStdIniRegistry::Set(const std::string &section, const std::string &name, const std::string &value) {
  Section *s = FindSection(section);
  SetLine(s ? *s : AddSection(section), name, value);
}

bool CStdIniRegistry::Delete(const std::string &section, const std::string &name) {
  Section *s = FindSection(section);
  if (!s) return false;
  for (auto it = s->Lines.begin(); it != s->Lines.end(); ++it)
    if (!it->Name.empty() && EqualNoCase(it->Name, name)) {
      s->Lines.erase(it);
      return true;
    }
  return false;
}

CStdIniRegistry::Section *CStdIniRegistry::FindSection(const std::string &name) {
  for (Section &s : Sections)
    if (EqualNoCase(s.Name, name)) return &s;
  return nullptr;
}

const CStdIniRegistry::Section *CStdIniRegistry::FindSection(const std::string &name) const {
  for (const Section &s : Sections)
    if (EqualNoCase(s.Name, name)) return &s;
  return nullptr;
}

CStdIniRegistry::Section &CStdIniRegistry::AddSection(const std::string &name) {
  if (Section *s = FindSection(name)) return *s;
  Sections.push_back({name, {}});
  return Sections.back();
}

void CStdIniRegistry::SetLine(Section &section, const std::string &name, const std::string &value) {
  for (Line &l : section.Lines)
    if (!l.Name.empty() && EqualNoCase(l.Name, name)) {
      l.Value = value;
      return;
    }
  section.Lines.push_back({name, value});
}
