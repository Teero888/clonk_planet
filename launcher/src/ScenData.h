#pragma once

// Scenario core (Scenario.txt) as the scenario properties dialogs of Planet.exe use it.
//
// The original loads Scenario.txt into its own copy of C4Scenario (compiler table at 0x47b9f8,
// FUN_0040d0f0), converts old goals / realism settings to goal and rule objects
// (C4SGame::ConvertGoals, FUN_0040dbd0), lets the pages edit the structure and writes it back with
// C4Compiler::DecompileStructure (FUN_0040d160: only values that differ from the defaults).
// Here the edited values are written into the existing text with C4TextDoc, so comments and keys
// the dialogs don't know are kept.

#include "C4TextDoc.h"

#include <QString>
#include <QStringList>
#include <array>
#include <cstdint>

class DefinitionDB;

namespace Scen {

// rand() of the VC6 runtime (Planet.exe 0x440cd4). The launcher never seeds it, so it starts at 1.
int crtRand();
// C4 Random(n): 0 for n == 0, rand() % n otherwise (MSVC semantics for negative n)
int random(int n);

// C4SVal: standard value, random deviation and bounds
struct SVal {
    int std = 0, rnd = 0, min = 0, max = 100;
    SVal() = default;
    SVal(int s, int r = 0, int mn = 0, int mx = 100) : std(s), rnd(r), min(mn), max(mx) {}
    // C4SVal::Evaluate (FUN_0040cd30): BoundBy(Std + Random(2 * Rnd + 1) - Rnd, Min, Max)
    int evaluate() const;
    bool operator==(const SVal &o) const { return std == o.std && rnd == o.rnd && min == o.min && max == o.max; }
    bool operator!=(const SVal &o) const { return !(*this == o); }
};

// C4IDList of the launcher (FUN_0040ae50...): 150 slots of id / count, empty id = free slot.
class IDList {
public:
    static constexpr int Size = 150;

    void clear();                                   // FUN_0040ae70
    bool isClear() const;                           // FUN_0040aea0
    QString getID(int index, int *count = nullptr) const; // FUN_0040aed0
    int getCount(int index) const;                  // FUN_0040af00
    bool setCount(int index, int count);            // FUN_0040af20
    int getIDCount(const QString &id) const;
    bool setIDCount(const QString &id, int count, bool add); // FUN_0040af90
    bool increaseIDCount(const QString &id, bool add);       // FUN_0040aff0
    bool add(const IDList &other);                           // FUN_0040b070
    bool deleteItem(int index);                     // FUN_0040b4b0
    bool swapItems(int a, int b);                   // FUN_0040b440
    void consolidate();                             // FUN_0040b0e0: close gaps
    void consolidateValids(const DefinitionDB &defs); // FUN_0040b150
    void sortByCategory(const DefinitionDB &defs);  // FUN_0040b190
    void load(const DefinitionDB &defs, uint32_t category); // FUN_0040b240
    int numIDs() const; // number of leading used slots

    // C4IDList::Read (count 0 for entries without "=") / Write ("ID=count;")
    void read(const QString &text);
    QString write() const;

    bool operator==(const IDList &o) const { return ids_ == o.ids_ && counts_ == o.counts_; }
    bool operator!=(const IDList &o) const { return !(*this == o); }

private:
    std::array<QString, Size> ids_;
    std::array<int, Size> counts_{};
};

struct NameList {
    static constexpr int Size = 10;
    std::array<QString, Size> names;
    std::array<int, Size> counts{};
    void read(const QString &text);
    int num() const;
};

struct PlrStart {
    QString nativeCrew; // obsolete StandardCrew
    SVal crew{1, 0, 1, 10};
    SVal wealth{0, 0, 0, 250};
    int positionX = -1, positionY = -1, enforcePosition = 0;
    IDList readyCrew, readyBase, readyVehic, readyMaterial, buildKnowledge, homeBaseMaterial, homeBaseProduction, magic;

    // C4SPlrStart::EquipmentEqual (FUN_0040d330): everything but the position
    bool equipmentEqual(const PlrStart &o) const;
};

struct Landscape {
    int exactLandscape = 0;
    SVal vegLevel{50, 30, 0, 100};
    IDList vegetation;
    SVal inEarthLevel{50, 0, 0, 100};
    IDList inEarth;
    int bottomOpen = 0, topOpen = 1;
    std::array<int, 6> skyDefFade{};
    SVal gravity{100, 0, 10, 200};
    SVal mapWdt{100, 0, 64, 250}, mapHgt{50, 0, 40, 250}, mapZoom{10, 0, 5, 15};
    SVal amplitude{0, 0}, phase{50}, period{15}, random{0};
    SVal liquidLevel;
    int mapPlayerExtend = 0;
    NameList layers;
    QString material = "Earth", liquid = "Water";

    // C4SLandscape::GetMapSize (FUN_0040d7b0)
    void getMapSize(int &wdt, int &hgt, int players) const;
};

struct Core {
    // [Head]
    QString title = "Default Title";
    int icon = 18, roundOptions = 1, access = 0, noInitialize = 0, maxPlayer = 4, saveGame = 0;
    // [Definitions]
    int localOnly = 0;
    std::array<QString, 10> definitions;
    // [Game]
    int mode = 0, elimination = 1, enableRemoveFlag = 0, enableSurrender = 1, valueGain = 0, cooperativeGoal = 0;
    bool hasOldGoals = false; // CreateObjects / ClearObjects / ClearMaterials present
    IDList goals, rules;
    // Realism
    int structNeedMaterial = 0, structNeedEnergy = 1;
    std::array<PlrStart, 4> plrStart;
    Landscape landscape;
    IDList animals, nests;
    SVal climate{50, 10}, startSeason{50, 50}, yearSpeed{50}, rain, lightning, wind{0, 70, -100, 100};
    SVal volcano, earthquake, meteorite;
    IDList environment;

    // C4Scenario::Load: compile + Game.ConvertGoals
    void load(const C4TextDoc &doc);
    // Writes everything the dialogs edit (DecompileStructure: keys equal to the default are removed)
    void save(C4TextDoc &doc) const;

    // C4SDefinitions::GetModules (FUN_0040da80): false if the scenario doesn't restrict definitions
    bool getModules(QStringList &modules) const;

private:
    void convertGoals();
};

// Title of a scenario from Title.txt in the launcher language ("US:Gold Mine"), else the core title
QString localizedText(const std::vector<uint8_t> &data, const QString &lang);

// Language of the launcher ("US" / "DE")
QString language();

// Frontend sample playback (Planet.exe FUN_00429220; Sound\FESamples)
void playSound(int wave_id);

} // namespace Scen
