#include "ConfigManager.h"

#include <cctype>

std::pair<std::string, std::string> ConfigManager::split(const std::string &key) {
    const size_t slash = key.rfind('\\');
    if (slash == std::string::npos)
        return {"", key};
    return {key.substr(0, slash), key.substr(slash + 1)};
}

std::filesystem::file_time_type ConfigManager::fileTime() const {
    std::error_code ec;
    auto t = std::filesystem::last_write_time(path_, ec);
    return ec ? std::filesystem::file_time_type{} : t;
}

bool ConfigManager::load(const std::string &path) {
    path_ = path;
    changes_.clear();
    const bool ok = registry_.Load(path_);
    // a file of the old flat [Software] format is converted right away (like the engine does)
    if (ok && registry_.WasLegacy())
        registry_.Save(path_);
    loaded_time_ = fileTime();
    return ok;
}

void ConfigManager::refresh() const {
    if (path_.empty() || fileTime() == loaded_time_)
        return;
    // changed by the engine: values set by the launcher and not saved yet stay in effect
    registry_.Load(path_);
    loaded_time_ = fileTime();
    for (const auto &[key, value] : changes_) {
        const auto [section, name] = split(key);
        registry_.Set(section, name, value);
    }
}

bool ConfigManager::save() {
    if (path_.empty())
        return false;
    refresh();
    const bool ok = registry_.Save(path_);
    if (ok)
        changes_.clear();
    loaded_time_ = fileTime();
    return ok;
}

std::string ConfigManager::getValue(const std::string &key, const std::string &defaultValue) const {
    refresh();
    const auto [section, name] = split(key);
    std::string value;
    return registry_.Get(section, name, value) ? value : defaultValue;
}

void ConfigManager::setValue(const std::string &key, const std::string &value) {
    refresh();
    const auto [section, name] = split(key);
    std::string current;
    if (registry_.Get(section, name, current) && current == value)
        return;
    registry_.Set(section, name, value);
    changes_.emplace_back(key, value);
}
