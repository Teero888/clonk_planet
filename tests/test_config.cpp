// clonk.ini: conversion of the old flat [Software] format, round trips keeping comments and order,
// case insensitive names and the launcher merging its changes with values the engine wrote.
#include "../launcher/src/ConfigManager.h"

#include <StdIniRegistry.h>

#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                            \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static void writeText(const std::string &path, const std::string &text) {
    std::ofstream(path, std::ios::binary) << text;
}

static std::string readText(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}

static void setEnv(const char *name, const std::string &value) {
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    if (value.empty())
        unsetenv(name);
    else
        setenv(name, value.c_str(), 1);
#endif
}

static std::string get(const CStdIniRegistry &r, const std::string &section, const std::string &name) {
    std::string v;
    return r.Get(section, name, v) ? v : "<missing>";
}

int main() {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "clonk_test_config";
    fs::remove_all(dir);
    fs::create_directories(dir);
    const std::string ini = (dir / "clonk.ini").string();

    CHECK(CStdIniRegistry::SectionOfSubKey("Software\\RedWolf Design\\Clonk 4\\General") == "General");
    CHECK(CStdIniRegistry::SectionOfSubKey("software\\redwolf design\\clonk 4\\ClonkRanks") == "ClonkRanks");
    CHECK(CStdIniRegistry::SectionOfSubKey("Software\\Other") == "Software\\Other");

    // old format (CRLF, sorted by name) is converted, sections in C4Config order
    writeText(ini, "[Software]\r\n"
                   "RedWolf Design\\Clonk 4\\Controls\\Kbd1Key1=81\r\n"
                   "RedWolf Design\\Clonk 4\\General\\Language=US\r\n"
                   "RedWolf Design\\Clonk 4\\General\\Name=\r\n"
                   "RedWolf Design\\Clonk 4\\PlayerRanks\\Rank001=Clonk\r\n");
    {
        CStdIniRegistry r;
        CHECK(r.Load(ini));
        CHECK(r.WasLegacy());
        CHECK(get(r, "General", "Language") == "US");
        CHECK(get(r, "general", "language") == "US");
        CHECK(get(r, "General", "Name") == "");
        CHECK(get(r, "Controls", "Kbd1Key1") == "81");
        CHECK(get(r, "PlayerRanks", "Rank001") == "Clonk");
        CHECK(r.Save(ini));
    }
    const std::string converted = readText(ini);
    CHECK(converted == "[General]\nLanguage=US\nName=\n\n[Controls]\nKbd1Key1=81\n\n[PlayerRanks]\nRank001=Clonk\n");

    // comments, order and unknown sections survive a rewrite; set / delete by any case
    writeText(ini, "; my settings\n\n[General]\n; the language\nLanguage=DE\n\n[Custom\\Key]\nA = 1\n");
    {
        CStdIniRegistry r;
        CHECK(r.Load(ini));
        CHECK(!r.WasLegacy());
        CHECK(get(r, "Custom\\Key", "A") == "1");
        r.Set("GENERAL", "language", "US");
        r.Set("General", "Name", "Joe");
        r.Set("Sound", "RXSound", "1");
        CHECK(r.Delete("custom\\key", "a"));
        CHECK(!r.Delete("custom\\key", "a"));
        CHECK(r.Save(ini));
    }
    CHECK(readText(ini) == "; my settings\n\n[General]\n; the language\nLanguage=US\nName=Joe\n\n[Custom\\Key]\n\n[Sound]\nRXSound=1\n");

    // launcher: keys below the Clonk 4 root; values the engine writes meanwhile are kept on save
    writeText(ini, "[General]\nLanguage=US\n\n[Graphics]\nResolution=0\n");
    {
        ConfigManager cfg;
        CHECK(cfg.load(ini));
        CHECK(cfg.getValue("General\\Language") == "US");
        CHECK(cfg.getValue("General\\Missing", "def") == "def");
        cfg.setValue("Graphics\\Resolution", "2");

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        CStdIniRegistry engine;
        CHECK(engine.Load(ini));
        engine.Set("ClonkRanks", "Rank001", "Clonk");
        engine.Set("General", "Language", "DE");
        CHECK(engine.Save(ini));

        CHECK(cfg.getValue("General\\Language") == "DE");     // reloaded
        CHECK(cfg.getValue("Graphics\\Resolution") == "2");   // own unsaved change still in effect
        CHECK(cfg.save());
    }
    {
        CStdIniRegistry r;
        CHECK(r.Load(ini));
        CHECK(get(r, "Graphics", "Resolution") == "2");
        CHECK(get(r, "ClonkRanks", "Rank001") == "Clonk");
        CHECK(get(r, "General", "Language") == "DE");
    }

    // user config: created from the defaults on the first start, then left alone
    {
        const std::string defaults = (dir / "defaults.ini").string();
        const std::string user = (dir / "user" / "clonk.ini").string();
        writeText(defaults, "[General]\nLanguage=US\n");
        setEnv("CLONK_CONFIG", user);
        CHECK(CStdIniRegistry::PrepareUserConfig(defaults) == user);
        CHECK(readText(user) == "[General]\nLanguage=US\n");
        writeText(user, "[General]\nLanguage=DE\n");
        CHECK(CStdIniRegistry::PrepareUserConfig(defaults) == user);
        CHECK(readText(user) == "[General]\nLanguage=DE\n");
        setEnv("CLONK_CONFIG", "");
    }

    fs::remove_all(dir);
    if (failures)
        std::printf("%d failures\n", failures);
    else
        std::printf("all config tests passed\n");
    return failures ? 1 : 0;
}
