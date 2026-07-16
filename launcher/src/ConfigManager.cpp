#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

ConfigManager::ConfigManager() {}

bool ConfigManager::load(const std::string &path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    softwareSection.clear();
    otherSections.clear();

    std::string currentSection = "";
    std::string line;
    while (std::getline(f, line)) {
        // Strip carriage returns and leading/trailing whitespace
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || std::isspace(line.back()))) {
            line.pop_back();
        }
        size_t start = 0;
        while (start < line.size() && std::isspace(line[start])) {
            start++;
        }
        if (start >= line.size()) continue;
        line = line.substr(start);

        if (line.empty() || line[0] == ';') continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        if (currentSection.empty()) continue;

        std::string lowerSection = currentSection;
        std::transform(lowerSection.begin(), lowerSection.end(), lowerSection.begin(), ::tolower);

        if (lowerSection == "software") {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                
                // trim
                while (!key.empty() && std::isspace(key.back())) key.pop_back();
                size_t ks = 0;
                while (ks < key.size() && std::isspace(key[ks])) ks++;
                key = key.substr(ks);

                while (!val.empty() && std::isspace(val.back())) val.pop_back();
                size_t vs = 0;
                while (vs < val.size() && std::isspace(val[vs])) vs++;
                val = val.substr(vs);

                softwareSection[key] = val;
            }
        } else {
            otherSections[currentSection].push_back(line);
        }
    }
    f.close();
    return true;
}

bool ConfigManager::save(const std::string &path) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    f << "[Software]\r\n";
    for (const auto &pair : softwareSection) {
        f << pair.first << "=" << pair.second << "\r\n";
    }

    for (const auto &section : otherSections) {
        f << "[" << section.first << "]\r\n";
        for (const auto &line : section.second) {
            f << line << "\r\n";
        }
    }

    f.close();
    return true;
}

std::string ConfigManager::getValue(const std::string &key, const std::string &defaultValue) const {
    auto it = softwareSection.find(key);
    if (it != softwareSection.end()) {
        return it->second;
    }
    return defaultValue;
}

void ConfigManager::setValue(const std::string &key, const std::string &value) {
    softwareSection[key] = value;
}
