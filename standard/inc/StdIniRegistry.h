/* Registry emulation file (clonk.ini), shared by the engine (StdRegistry.cpp) and the launcher.

   The original stored its settings in HKCU\Software\RedWolf Design\Clonk 4\<Section>\<Value>.
   The file has one INI section per registry key below that root:

     [General]
     Language=US

     [ClonkRanks]
     Rank001=Clonk

   Keys outside the root keep their full path as the section name. Names are case insensitive
   like registry names. Order, comments and blank lines of the file are kept when it is rewritten.

   Files of the old format (one [Software] section with "RedWolf Design\Clonk 4\General\Language"
   style keys) are converted when loaded.

   Like HKCU, the file is per user (PrepareUserConfig). On the first start it is created from the
   defaults shipped next to the executable. */

#pragma once

#include <string>
#include <vector>

class CStdIniRegistry {
public:
  // "Software\RedWolf Design\Clonk 4\General" -> "General"
  static std::string SectionOfSubKey(const std::string &subKey);

  // The user's clonk.ini: $CLONK_CONFIG if set, else
  //   Windows  %APPDATA%\Clonk Planet\clonk.ini
  //   macOS    ~/Library/Application Support/Clonk Planet/clonk.ini
  //   others   $XDG_CONFIG_HOME/clonk-planet/clonk.ini (default ~/.config)
  // If it does not exist yet, it is created from defaultsPath (the clonk.ini next to the
  // executable). Returns defaultsPath if there is no user directory.
  static std::string PrepareUserConfig(const std::string &defaultsPath);

  // Returns false if the file could not be read (the registry is then empty).
  bool Load(const std::string &path);
  bool Save(const std::string &path) const;
  void Clear();

  // True if Load converted a file of the old [Software] format (it should be saved again).
  bool WasLegacy() const { return fLegacy; }

  bool Get(const std::string &section, const std::string &name, std::string &value) const;
  void Set(const std::string &section, const std::string &name, const std::string &value);
  bool Delete(const std::string &section, const std::string &name);

private:
  struct Line {
    std::string Name, Value; // empty name: Value is a comment or blank line kept verbatim
  };
  struct Section {
    std::string Name;
    std::vector<Line> Lines;
  };
  std::vector<Line> Header; // comments before the first section
  std::vector<Section> Sections;
  bool fLegacy = false;

  Section *FindSection(const std::string &name);
  const Section *FindSection(const std::string &name) const;
  Section &AddSection(const std::string &name);
  void SetLine(Section &section, const std::string &name, const std::string &value);
};
