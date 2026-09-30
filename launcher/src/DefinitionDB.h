#pragma once

// Object definitions as the launcher's scenario dialogs know them (Planet.exe C4DefList at
// FUN_00406470 / FUN_0040f9f0): id, localised name, category, value, MaxUserSelect, description
// and the 32x32 picture (DefCore Picture rect of Graphics.bmp, palette index 0 transparent).
//
// Definitions come from the modules the scenario uses (its [Definitions] or the activated
// definition packs), from the .c4f folders around the scenario and from the scenario itself.
// The first definition of an id wins, like C4DefList::Add without overwrite.

#include "GroupEdit.h"

#include <QImage>
#include <QString>
#include <QStringList>
#include <cstdint>
#include <memory>
#include <vector>

namespace Scen {
struct Core;
}

// C4D_ categories (engine/inc/C4Def.h)
namespace C4D {
constexpr uint32_t Goal = 1 << 5, Environment = 1 << 6, SelectBuilding = 1 << 7, SelectVehicle = 1 << 8,
                   SelectMaterial = 1 << 9, SelectKnowledge = 1 << 10, SelectHomebase = 1 << 11,
                   SelectAnimal = 1 << 12, SelectNest = 1 << 13, SelectInEarth = 1 << 14,
                   SelectVegetation = 1 << 15, Magic = 1 << 17, CrewMember = 1 << 18, Rule = 1 << 19;
}

struct Def {
    QString id;
    QString name;
    uint32_t category = 0;
    int value = 0;
    int maxUserSelect = 0;
    QString desc;
    QImage picture; // 32x32 ARGB
};

class DefinitionDB {
public:
    // Loads the definitions available to a scenario (FUN_00406470 with flags 0x28 = picture +
    // description). Returns the number of loaded definitions.
    int loadForScenario(const ItemPath &scenario, const Scen::Core &core);

    void clear() { defs_.clear(); }

    const Def *byId(const QString &id) const;           // FUN_00406390
    int indexOf(const QString &id) const;               // FUN_004063c0 (image list index)
    const Def *get(int index, uint32_t category) const; // FUN_00406420
    int count(uint32_t category) const;                 // FUN_004063f0
    int size() const { return static_cast<int>(defs_.size()); }
    const Def &at(int i) const { return *defs_[i]; }

    // Module paths of the activated definitions (relative to the game directory where possible)
    static QStringList activatedModules();
    // Game directory (the original's working directory)
    static QString gameDir();
    // Module path of a scenario definition entry ("Objects.c4d") as ItemPath
    static ItemPath modulePath(const QString &module);

private:
    std::vector<std::shared_ptr<const Def>> defs_; // list order: last added first (linked list head)
    bool add(const std::shared_ptr<const Def> &def);
    int addAll(const std::vector<std::shared_ptr<const Def>> &defs);
};
