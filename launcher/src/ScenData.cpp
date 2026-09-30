#include "ScenData.h"
#include "ClonkLauncher.h"
#include "DefinitionDB.h"
#include "LauncherRes.h"

#include <algorithm>
#include <map>

namespace Scen {

// ------------------------------------------------------------------------------------ random

static uint32_t crt_seed = 1;

int crtRand() {
    crt_seed = crt_seed * 0x343fd + 0x269ec3;
    return (crt_seed >> 16) & 0x7fff;
}

int random(int n) {
    if (n == 0)
        return 0;
    return crtRand() % n;
}

int SVal::evaluate() const {
    // FUN_0040cd30
    const int range = rnd * 2 + 1;
    const int r = range == 0 ? 0 : crtRand() % range;
    const int v = std - rnd + r;
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

// ------------------------------------------------------------------------------------ IDList

void IDList::clear() {
    for (int i = 0; i < Size; ++i) {
        ids_[i].clear();
        counts_[i] = 0;
    }
}

bool IDList::isClear() const {
    for (const QString &id : ids_)
        if (!id.isEmpty())
            return false;
    return true;
}

QString IDList::getID(int index, int *count) const {
    if (index < 0 || index >= Size)
        return {};
    if (count)
        *count = counts_[index];
    return ids_[index];
}

int IDList::getCount(int index) const {
    return (index < 0 || index >= Size) ? 0 : counts_[index];
}

bool IDList::setCount(int index, int count) {
    if (index < 0 || index >= Size)
        return false;
    counts_[index] = count;
    return true;
}

int IDList::getIDCount(const QString &id) const {
    for (int i = 0; i < Size; ++i)
        if (ids_[i] == id)
            return counts_[i];
    return 0;
}

bool IDList::setIDCount(const QString &id, int count, bool add) {
    for (int i = 0; i < Size; ++i)
        if (ids_[i] == id) {
            counts_[i] = count;
            return true;
        }
    if (!add)
        return false;
    for (int i = 0; i < Size; ++i)
        if (ids_[i].isEmpty()) {
            ids_[i] = id;
            counts_[i] = count;
            return true;
        }
    return false;
}

bool IDList::increaseIDCount(const QString &id, bool add) {
    for (int i = 0; i < Size; ++i)
        if (ids_[i] == id) {
            ++counts_[i];
            return true;
        }
    if (!add)
        return true;
    for (int i = 0; i < Size; ++i)
        if (ids_[i].isEmpty()) {
            ids_[i] = id;
            counts_[i] = 1;
            return true;
        }
    return false;
}

bool IDList::add(const IDList &other) {
    for (int i = 0; i < Size; ++i) {
        if (other.ids_[i].isEmpty())
            continue;
        for (int c = 0; c < other.counts_[i]; ++c)
            if (!increaseIDCount(other.ids_[i], true))
                return false;
    }
    return true;
}

bool IDList::deleteItem(int index) {
    if (index < 0 || index >= Size)
        return false;
    for (int i = index; i < Size - 1; ++i) {
        ids_[i] = ids_[i + 1];
        counts_[i] = counts_[i + 1];
    }
    ids_[Size - 1].clear();
    counts_[Size - 1] = 0;
    return true;
}

bool IDList::swapItems(int a, int b) {
    if (a < 0 || a >= Size || b < 0 || b >= Size)
        return false;
    std::swap(ids_[a], ids_[b]);
    std::swap(counts_[a], counts_[b]);
    return true;
}

void IDList::consolidate() {
    // FUN_0040b0e0: move every entry into the first free slot before it
    for (int i = 0; i < Size - 1; ++i) {
        if (!ids_[i].isEmpty())
            continue;
        for (int j = i + 1; j < Size; ++j)
            if (!ids_[j].isEmpty()) {
                ids_[i] = ids_[j];
                counts_[i] = counts_[j];
                ids_[j].clear();
                counts_[j] = 0;
                break;
            }
    }
}

void IDList::consolidateValids(const DefinitionDB &defs) {
    for (int i = 0; i < Size; ++i)
        if (!ids_[i].isEmpty() && !defs.byId(ids_[i]))
            ids_[i].clear();
    consolidate();
}

void IDList::sortByCategory(const DefinitionDB &defs) {
    // FUN_0040b190: bubble sort, higher (category & C4D_SortLimit) first
    bool swapped;
    do {
        swapped = false;
        for (int i = 1; i < Size; ++i) {
            if (ids_[i - 1].isEmpty() || ids_[i].isEmpty())
                continue;
            const Def *a = defs.byId(ids_[i - 1]);
            const Def *b = defs.byId(ids_[i]);
            if (a && b && (a->category & 0x1f) < (b->category & 0x1f)) {
                std::swap(ids_[i - 1], ids_[i]);
                std::swap(counts_[i - 1], counts_[i]);
                swapped = true;
            }
        }
    } while (swapped);
}

void IDList::load(const DefinitionDB &defs, uint32_t category) {
    clear();
    for (int i = 0; i < Size; ++i) {
        const Def *d = defs.get(i, category);
        if (!d)
            return;
        ids_[i] = d->id;
    }
}

int IDList::numIDs() const {
    int n = 0;
    while (n < Size && !ids_[n].isEmpty())
        ++n;
    return n;
}

void IDList::read(const QString &text) {
    // C4IDList::Read(szSource, iDefValue = 0)
    clear();
    for (const QString &seg : text.split(';')) {
        QString buf = seg.trimmed();
        int value = 0;
        const int eq = buf.indexOf('=');
        if (eq >= 0) {
            value = buf.mid(eq + 1).trimmed().toInt();
            buf = buf.left(eq).trimmed();
        }
        if (buf.size() == 4)
            if (!setIDCount(buf, value, true))
                return;
    }
}

QString IDList::write() const {
    QString out;
    for (int i = 0; i < Size; ++i)
        if (!ids_[i].isEmpty())
            out += QString("%1=%2;").arg(ids_[i]).arg(counts_[i]);
    return out;
}

void NameList::read(const QString &text) {
    for (int i = 0; i < Size; ++i) {
        names[i].clear();
        counts[i] = 0;
    }
    int n = 0;
    for (const QString &seg : text.split(';')) {
        QString buf = seg.trimmed();
        if (buf.isEmpty() || n >= Size)
            continue;
        int value = 0;
        const int eq = buf.indexOf('=');
        if (eq >= 0) {
            value = buf.mid(eq + 1).trimmed().toInt();
            buf = buf.left(eq).trimmed();
        }
        names[n] = buf;
        counts[n] = value;
        ++n;
    }
}

int NameList::num() const {
    int n = 0;
    for (const QString &s : names)
        if (!s.isEmpty())
            ++n;
    return n;
}

bool PlrStart::equipmentEqual(const PlrStart &o) const {
    return nativeCrew == o.nativeCrew && crew == o.crew && wealth == o.wealth &&
           enforcePosition == o.enforcePosition && readyCrew == o.readyCrew && readyBase == o.readyBase &&
           readyVehic == o.readyVehic && readyMaterial == o.readyMaterial && buildKnowledge == o.buildKnowledge &&
           homeBaseMaterial == o.homeBaseMaterial && homeBaseProduction == o.homeBaseProduction && magic == o.magic;
}

void Landscape::getMapSize(int &wdt, int &hgt, int players) const {
    wdt = mapWdt.evaluate();
    hgt = mapHgt.evaluate();
    players = std::max(players, 1);
    if (mapPlayerExtend)
        wdt = std::min(wdt * players, mapWdt.max);
}

// ------------------------------------------------------------------------------------ compile

namespace {

// C4Compiler integer segments: sscanf("%i") of each comma separated value
std::vector<int> parseInts(const QString &value, int count) {
    std::vector<int> out;
    const QStringList segs = value.split(',');
    for (int i = 0; i < count && i < segs.size(); ++i) {
        const QByteArray b = segs[i].trimmed().toLatin1();
        char *end = nullptr;
        const long v = std::strtol(b.constData(), &end, 0);
        if (end == b.constData()) {
            out.push_back(INT32_MIN); // not parsed: keep the old value
            continue;
        }
        out.push_back(static_cast<int>(v));
    }
    return out;
}

struct Doc {
    const C4TextDoc &doc;
    void readInt(const char *sec, const char *key, int &v) const {
        if (!doc.has(sec, key))
            return;
        auto vals = parseInts(doc.get(sec, key), 1);
        if (!vals.empty() && vals[0] != INT32_MIN)
            v = vals[0];
    }
    void readVal(const char *sec, const char *key, SVal &v, int count) const {
        if (!doc.has(sec, key))
            return;
        auto vals = parseInts(doc.get(sec, key), count);
        int *fields[4] = {&v.std, &v.rnd, &v.min, &v.max};
        for (size_t i = 0; i < vals.size() && i < 4; ++i)
            if (vals[i] != INT32_MIN)
                *fields[i] = vals[i];
    }
    void readStr(const char *sec, const char *key, QString &v, int max_len) const {
        if (doc.has(sec, key))
            v = doc.get(sec, key).left(max_len);
    }
    void readList(const char *sec, const char *key, IDList &v) const {
        if (doc.has(sec, key))
            v.read(doc.get(sec, key));
    }
};

// Decompile: write the key if it differs from the default, else drop it
struct Out {
    C4TextDoc &doc;
    void put(const char *sec, const char *key, bool is_default, const QString &text) {
        if (is_default)
            doc.remove(sec, key);
        else
            doc.set(sec, key, text);
    }
    void putInt(const char *sec, const char *key, int v, int def) { put(sec, key, v == def, QString::number(v)); }
    void putVal(const char *sec, const char *key, const SVal &v, const SVal &def, int count) {
        const int a[4] = {v.std, v.rnd, v.min, v.max};
        const int d[4] = {def.std, def.rnd, def.min, def.max};
        bool same = true;
        QStringList parts;
        for (int i = 0; i < count; ++i) {
            same = same && a[i] == d[i];
            parts << QString::number(a[i]);
        }
        put(sec, key, same, parts.join(','));
    }
    void putList(const char *sec, const char *key, const IDList &v) { put(sec, key, v.isClear(), v.write()); }
    void putStr(const char *sec, const char *key, const QString &v) { put(sec, key, v.isEmpty(), v); }
};

const char *playerSection(int i) {
    static const char *names[4] = {"Player1", "Player2", "Player3", "Player4"};
    return names[i];
}

} // namespace

void Core::load(const C4TextDoc &text) {
    *this = Core();
    Doc d{text};
    // compiler table of Planet.exe (0x47b9f8), value counts as there
    d.readInt("Head", "Icon", icon);
    d.readStr("Head", "Title", title, 512);
    d.readInt("Head", "RoundOptions", roundOptions);
    d.readInt("Head", "Access", access);
    d.readInt("Head", "MaxPlayer", maxPlayer);
    d.readInt("Head", "SaveGame", saveGame);
    d.readInt("Head", "NoInitialize", noInitialize);

    d.readInt("Definitions", "LocalOnly", localOnly);
    for (int i = 0; i < 10; ++i)
        d.readStr("Definitions", QString("Definition%1").arg(i + 1).toLatin1().constData(), definitions[i], 260);

    d.readInt("Game", "Mode", mode);
    d.readInt("Game", "Elimination", elimination);
    d.readInt("Game", "CooperativeGoal", cooperativeGoal);
    hasOldGoals = text.has("Game", "CreateObjects") || text.has("Game", "ClearObjects") ||
                  text.has("Game", "ClearMaterials") || text.has("Game", "ValueGain");
    d.readInt("Game", "ValueGain", valueGain);
    d.readInt("Game", "EnableRemoveFlag", enableRemoveFlag);
    d.readInt("Game", "EnableSurrender", enableSurrender);
    d.readInt("Game", "StructNeedMaterial", structNeedMaterial);
    d.readInt("Game", "StructNeedEnergy", structNeedEnergy);
    d.readList("Game", "Goals", goals);
    d.readList("Game", "Rules", rules);

    for (int p = 0; p < 4; ++p) {
        const char *s = playerSection(p);
        PlrStart &ps = plrStart[p];
        d.readStr(s, "StandardCrew", ps.nativeCrew, 4);
        d.readVal(s, "Clonks", ps.crew, 2);
        d.readVal(s, "Wealth", ps.wealth, 2);
        if (text.has(s, "Position")) {
            auto v = parseInts(text.get(s, "Position"), 2);
            if (v.size() > 0 && v[0] != INT32_MIN)
                ps.positionX = v[0];
            if (v.size() > 1 && v[1] != INT32_MIN)
                ps.positionY = v[1];
        }
        d.readInt(s, "EnforcePosition", ps.enforcePosition);
        d.readList(s, "Crew", ps.readyCrew);
        d.readList(s, "Buildings", ps.readyBase);
        d.readList(s, "Vehicles", ps.readyVehic);
        d.readList(s, "Material", ps.readyMaterial);
        d.readList(s, "Knowledge", ps.buildKnowledge);
        d.readList(s, "HomeBaseMaterial", ps.homeBaseMaterial);
        d.readList(s, "HomeBaseProduction", ps.homeBaseProduction);
        d.readList(s, "Magic", ps.magic);
    }

    Landscape &l = landscape;
    d.readInt("Landscape", "ExactLandscape", l.exactLandscape);
    d.readList("Landscape", "Vegetation", l.vegetation);
    d.readVal("Landscape", "VegetationLevel", l.vegLevel, 2);
    d.readList("Landscape", "InEarth", l.inEarth);
    d.readVal("Landscape", "InEarthLevel", l.inEarthLevel, 2);
    if (text.has("Landscape", "SkyFade")) {
        auto v = parseInts(text.get("Landscape", "SkyFade"), 6);
        for (size_t i = 0; i < v.size(); ++i)
            if (v[i] != INT32_MIN)
                l.skyDefFade[i] = v[i];
    }
    d.readInt("Landscape", "BottomOpen", l.bottomOpen);
    d.readInt("Landscape", "TopOpen", l.topOpen);
    d.readVal("Landscape", "MapWidth", l.mapWdt, 1);
    d.readVal("Landscape", "MapHeight", l.mapHgt, 1);
    d.readVal("Landscape", "MapZoom", l.mapZoom, 4);
    d.readVal("Landscape", "Amplitude", l.amplitude, 2);
    d.readVal("Landscape", "Phase", l.phase, 2);
    d.readVal("Landscape", "Period", l.period, 2);
    d.readVal("Landscape", "Random", l.random, 2);
    d.readStr("Landscape", "Material", l.material, 31);
    d.readStr("Landscape", "Liquid", l.liquid, 31);
    d.readVal("Landscape", "LiquidLevel", l.liquidLevel, 2);
    d.readInt("Landscape", "MapPlayerExtend", l.mapPlayerExtend);
    if (text.has("Landscape", "Layers"))
        l.layers.read(text.get("Landscape", "Layers"));
    d.readVal("Landscape", "Gravity", l.gravity, 2);

    d.readList("Animals", "Animal", animals);
    d.readList("Animals", "Nest", nests);

    d.readVal("Weather", "Climate", climate, 2);
    d.readVal("Weather", "StartSeason", startSeason, 2);
    d.readVal("Weather", "YearSpeed", yearSpeed, 2);
    d.readVal("Weather", "Rain", rain, 2);
    d.readVal("Weather", "Wind", wind, 2);
    d.readVal("Weather", "Lightning", lightning, 2);

    d.readVal("Disasters", "Meteorite", meteorite, 2);
    d.readVal("Disasters", "Volcano", volcano, 2);
    d.readVal("Disasters", "Earthquake", earthquake, 2);

    d.readList("Environment", "Objects", environment);

    convertGoals();
}

void Core::convertGoals() {
    // C4SGame::ConvertGoals (Planet.exe FUN_0040dbd0)
    bool clear_old = false;
    if (mode == 1) {
        goals.setIDCount("MELE", 1, true);
        clear_old = true;
    } else if (mode == 2) {
        goals.setIDCount("MEL2", 1, true);
        clear_old = true;
    }
    mode = 0;
    if (cooperativeGoal == 1) {
        goals.setIDCount("GLDM", 1, true);
        clear_old = true;
    } else if (cooperativeGoal == 2) {
        goals.setIDCount("MNTK", 1, true);
        clear_old = true;
    } else if (cooperativeGoal == 3) {
        goals.setIDCount("VALG", std::max(valueGain / 100, 1), true);
        clear_old = true;
    }
    cooperativeGoal = 0;
    if (clear_old) {
        hasOldGoals = false; // ClearOldGoals
        valueGain = 0;
    }
    if (structNeedMaterial)
        rules.setIDCount("CNMT", 1, true);
    structNeedMaterial = 0;
    if (structNeedEnergy)
        rules.setIDCount("ENRG", 1, true);
    structNeedEnergy = 0;
    if (enableSurrender)
        rules.setIDCount("SURR", 1, true);
    enableSurrender = 0;
    if (enableRemoveFlag)
        rules.setIDCount("FGRV", 1, true);
    enableRemoveFlag = 0;
    if (elimination == 0)
        rules.setIDCount("KILC", 1, true);
    else if (elimination == 2)
        rules.setIDCount("CTFL", 1, true);
    elimination = 1;
    if (rules.getIDCount("CTFL"))
        rules.setIDCount("FGRV", 1, true);
}

void Core::save(C4TextDoc &text) const {
    Out o{text};
    const Core def;
    o.putInt("Head", "Icon", icon, def.icon);
    o.putInt("Head", "RoundOptions", roundOptions, def.roundOptions);
    o.putInt("Head", "MaxPlayer", maxPlayer, def.maxPlayer);

    o.putInt("Definitions", "LocalOnly", localOnly, 0);
    for (int i = 0; i < 10; ++i)
        o.putStr("Definitions", QString("Definition%1").arg(i + 1).toLatin1().constData(), definitions[i]);

    // converted goals and realism (the original writes the converted structure)
    o.putInt("Game", "Mode", mode, def.mode);
    o.putInt("Game", "Elimination", elimination, def.elimination);
    o.putInt("Game", "CooperativeGoal", cooperativeGoal, def.cooperativeGoal);
    if (!hasOldGoals) {
        text.remove("Game", "CreateObjects");
        text.remove("Game", "ClearObjects");
        text.remove("Game", "ClearMaterials");
        o.putInt("Game", "ValueGain", valueGain, 0);
    }
    o.putInt("Game", "EnableRemoveFlag", enableRemoveFlag, def.enableRemoveFlag);
    o.putInt("Game", "EnableSurrender", enableSurrender, def.enableSurrender);
    o.putInt("Game", "StructNeedMaterial", structNeedMaterial, def.structNeedMaterial);
    o.putInt("Game", "StructNeedEnergy", structNeedEnergy, def.structNeedEnergy);
    o.putList("Game", "Goals", goals);
    o.putList("Game", "Rules", rules);

    const PlrStart dp;
    for (int p = 0; p < 4; ++p) {
        const char *s = playerSection(p);
        const PlrStart &ps = plrStart[p];
        o.putStr(s, "StandardCrew", ps.nativeCrew);
        o.putVal(s, "Clonks", ps.crew, dp.crew, 2);
        o.putVal(s, "Wealth", ps.wealth, dp.wealth, 2);
        o.put(s, "Position", ps.positionX == dp.positionX && ps.positionY == dp.positionY,
              QString("%1,%2").arg(ps.positionX).arg(ps.positionY));
        o.putInt(s, "EnforcePosition", ps.enforcePosition, 0);
        o.putList(s, "Crew", ps.readyCrew);
        o.putList(s, "Buildings", ps.readyBase);
        o.putList(s, "Vehicles", ps.readyVehic);
        o.putList(s, "Material", ps.readyMaterial);
        o.putList(s, "Knowledge", ps.buildKnowledge);
        o.putList(s, "HomeBaseMaterial", ps.homeBaseMaterial);
        o.putList(s, "HomeBaseProduction", ps.homeBaseProduction);
        o.putList(s, "Magic", ps.magic);
    }

    const Landscape &l = landscape;
    const Landscape dl;
    o.putList("Landscape", "Vegetation", l.vegetation);
    o.putVal("Landscape", "VegetationLevel", l.vegLevel, dl.vegLevel, 2);
    o.putList("Landscape", "InEarth", l.inEarth);
    o.putVal("Landscape", "InEarthLevel", l.inEarthLevel, dl.inEarthLevel, 2);
    o.putInt("Landscape", "BottomOpen", l.bottomOpen, dl.bottomOpen);
    o.putInt("Landscape", "TopOpen", l.topOpen, dl.topOpen);
    o.putVal("Landscape", "MapWidth", l.mapWdt, dl.mapWdt, 1);
    o.putVal("Landscape", "MapHeight", l.mapHgt, dl.mapHgt, 1);
    o.putVal("Landscape", "MapZoom", l.mapZoom, dl.mapZoom, 4);
    o.putVal("Landscape", "Amplitude", l.amplitude, dl.amplitude, 2);
    o.putVal("Landscape", "Phase", l.phase, dl.phase, 2);
    o.putVal("Landscape", "Period", l.period, dl.period, 2);
    o.putVal("Landscape", "Random", l.random, dl.random, 2);
    o.putVal("Landscape", "LiquidLevel", l.liquidLevel, dl.liquidLevel, 2);
    o.putInt("Landscape", "MapPlayerExtend", l.mapPlayerExtend, dl.mapPlayerExtend);
    o.putVal("Landscape", "Gravity", l.gravity, dl.gravity, 2);

    o.putList("Animals", "Animal", animals);
    o.putList("Animals", "Nest", nests);

    o.putVal("Weather", "Climate", climate, def.climate, 2);
    o.putVal("Weather", "StartSeason", startSeason, def.startSeason, 2);
    o.putVal("Weather", "YearSpeed", yearSpeed, def.yearSpeed, 2);
    o.putVal("Weather", "Rain", rain, def.rain, 2);
    o.putVal("Weather", "Wind", wind, def.wind, 2);
    o.putVal("Weather", "Lightning", lightning, def.lightning, 2);

    o.putVal("Disasters", "Meteorite", meteorite, def.meteorite, 2);
    o.putVal("Disasters", "Volcano", volcano, def.volcano, 2);
    o.putVal("Disasters", "Earthquake", earthquake, def.earthquake, 2);

    o.putList("Environment", "Objects", environment);
}

bool Core::getModules(QStringList &modules) const {
    // FUN_0040da80
    modules.clear();
    if (localOnly)
        return true;
    bool any = false;
    for (const QString &d : definitions)
        if (!d.isEmpty())
            any = true;
    if (!any)
        return false;
    for (const QString &d : definitions)
        if (!d.isEmpty())
            modules << d;
    return true;
}

// ------------------------------------------------------------------------------------ misc

QString localizedText(const std::vector<uint8_t> &data, const QString &lang) {
    // FUN_004048f0: "<lang>:" up to the line end
    const QString text = QString::fromLatin1(reinterpret_cast<const char *>(data.data()), static_cast<int>(data.size()));
    const int pos = text.indexOf(lang + ":");
    if (pos < 0)
        return {};
    const int start = pos + lang.size() + 1;
    int end = text.indexOf('\r', start);
    const int lf = text.indexOf('\n', start);
    if (end < 0 || (lf >= 0 && lf < end))
        end = lf;
    return (end < 0 ? text.mid(start) : text.mid(start, end - start)).trimmed();
}

QString language() {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l ? QString::fromStdString(l->getLanguage()) : QString("US");
}

void playSound(int wave_id) {
    LauncherRes::playSound(wave_id);
}

} // namespace Scen
