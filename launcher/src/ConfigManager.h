#pragma once

// clonk.ini, the registry emulation shared with the engine (standard/inc/StdIniRegistry.h).
// Keys are value paths below HKCU\Software\RedWolf Design\Clonk 4, e.g. "General\\Language"
// ([General] Language=... in the file).
//
// The engine writes the same file while a round runs: the file is read again when it changed on
// disk, and save() only writes the values set by the launcher into the current file.

#include <StdIniRegistry.h>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class ConfigManager {
public:
    bool load(const std::string &path);
    bool save();

    std::string getValue(const std::string &key, const std::string &defaultValue = "") const;
    void setValue(const std::string &key, const std::string &value);

private:
    static std::pair<std::string, std::string> split(const std::string &key);
    void refresh() const;
    std::filesystem::file_time_type fileTime() const;

    std::string path_;
    mutable CStdIniRegistry registry_;
    mutable std::filesystem::file_time_type loaded_time_{};
    std::vector<std::pair<std::string, std::string>> changes_; // key, value set since the last save
};
