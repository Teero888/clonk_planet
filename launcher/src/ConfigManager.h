#pragma once

#include <string>
#include <map>
#include <vector>

class ConfigManager {
public:
    ConfigManager();
    bool load(const std::string &path);
    bool save(const std::string &path);

    std::string getValue(const std::string &key, const std::string &defaultValue = "") const;
    void setValue(const std::string &key, const std::string &value);

private:
    struct CaseInsensitiveLess {
        bool operator()(const std::string &a, const std::string &b) const {
            std::string al = a;
            std::string bl = b;
            for (auto &c : al) c = std::tolower(c);
            for (auto &c : bl) c = std::tolower(c);
            return al < bl;
        }
    };

    std::map<std::string, std::string, CaseInsensitiveLess> softwareSection;
    std::map<std::string, std::vector<std::string>> otherSections;
};
