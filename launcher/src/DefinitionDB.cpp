#include "DefinitionDB.h"
#include "C4TextDoc.h"
#include "ClonkLauncher.h"
#include "ScenData.h"

#include <QDir>
#include <QFileInfo>
#include <cstring>
#include <map>

namespace {

using DefPtr = std::shared_ptr<const Def>;

bool endsWithNoCase(const std::string &s, const char *ext) {
    const size_t n = std::strlen(ext);
    if (s.size() < n)
        return false;
    for (size_t i = 0; i < n; ++i)
        if (std::tolower(static_cast<unsigned char>(s[s.size() - n + i])) != std::tolower(static_cast<unsigned char>(ext[i])))
            return false;
    return true;
}

QString latin1(const std::vector<uint8_t> &d) {
    return QString::fromLatin1(reinterpret_cast<const char *>(d.data()), static_cast<int>(d.size()));
}

// FUN_0040a440: 8 bit Graphics.bmp, palette index 0 made transparent (magenta mask of the image
// list), picture rect stretched into 32x32 keeping the aspect ratio (FUN_00434dd0 / FUN_00434ca0)
bool loadPicture(const std::vector<uint8_t> &bmp, int px, int py, int pw, int ph, QImage &out) {
    auto u16 = [&](size_t o) { return o + 2 <= bmp.size() ? int(bmp[o] | bmp[o + 1] << 8) : 0; };
    auto u32 = [&](size_t o) {
        return o + 4 <= bmp.size() ? int32_t(uint32_t(bmp[o]) | uint32_t(bmp[o + 1]) << 8 | uint32_t(bmp[o + 2]) << 16 |
                                             uint32_t(bmp[o + 3]) << 24)
                                   : 0;
    };
    if (bmp.size() < 14 + 40 + 1024 || bmp[0] != 'B' || bmp[1] != 'M')
        return false;
    const int bits_offset = u32(10);
    const int width = u32(18);
    const int height = u32(22);
    if (u16(28) != 8 || u32(30) != 0 || u32(34) == 0 || width <= 0 || height == 0)
        return false;
    QRgb palette[256];
    for (int i = 0; i < 256; ++i) {
        const size_t o = 54 + i * 4;
        palette[i] = qRgba(bmp[o + 2], bmp[o + 1], bmp[o], 255);
    }
    palette[0] = qRgba(0, 0, 0, 0);
    const int stride = (width + 3) & ~3;
    const int abs_h = height < 0 ? -height : height;
    if (bits_offset + size_t(stride) * abs_h > bmp.size())
        return false;
    if (px < 0 || py < 0 || pw <= 0 || ph <= 0 || px + pw > width || py + ph > abs_h)
        return false;
    auto pixel = [&](int x, int y) -> uint8_t {
        const int row = height > 0 ? abs_h - 1 - y : y;
        return bmp[bits_offset + size_t(row) * stride + x];
    };
    // FUN_00434dd0: keep aspect ratio, center
    int dx = 0, dy = 0, dw = 32, dh = 32;
    const int fx = dw * 100 / pw, fy = dh * 100 / ph;
    if (fx < fy) {
        const int h = ph * dw / pw;
        dy += (dh - h) / 2;
        dh = h;
    } else if (fy < fx) {
        const int w = pw * dh / ph;
        dx += (dw - w) / 2;
        dw = w;
    }
    out = QImage(32, 32, QImage::Format_ARGB32);
    out.fill(Qt::transparent);
    if (dw <= 0 || dh <= 0)
        return true;
    for (int y = 0; y < dh; ++y) {
        const int sy = py + y * ph / dh;
        for (int x = 0; x < dw; ++x) {
            const int sx = px + x * pw / dw;
            out.setPixel(dx + x, dy + y, palette[pixel(sx, sy)]);
        }
    }
    return true;
}

// FUN_004056d0 with flags 0x28: DefCore, picture, Names.txt, Desc<lang>.txt
DefPtr loadDef(const C4Group &grp, const QString &lang) {
    if (!grp.hasEntry("DefCore.txt"))
        return nullptr;
    C4TextDoc core(grp.getFile("DefCore.txt"));
    auto d = std::make_shared<Def>();
    d->id = core.get("DefCore", "id").left(4);
    if (d->id.size() != 4)
        return nullptr;
    d->name = core.get("DefCore", "Name", "Undefined").left(30);
    d->category = static_cast<uint32_t>(core.getInt("DefCore", "Category", 0));
    d->maxUserSelect = core.getInt("DefCore", "MaxUserSelect", 0);
    d->value = core.getInt("DefCore", "Value", 0);
    if (core.getInt("DefCore", "CrewMember", 0))
        d->category |= C4D::CrewMember; // FUN_00405310
    std::vector<int> pic = core.getInts("DefCore", "Picture");
    pic.resize(4, 0);
    if (pic[2] == 0 || pic[3] == 0)
        pic = {0, 0, core.getInt("DefCore", "Width", 0), core.getInt("DefCore", "Height", 0)};
    if (!grp.hasEntry("Graphics.bmp"))
        return nullptr;
    if (!loadPicture(grp.getFile("Graphics.bmp"), pic[0], pic[1], pic[2], pic[3], d->picture))
        return nullptr;
    if (grp.hasEntry("Names.txt")) {
        const QString n = Scen::localizedText(grp.getFile("Names.txt"), lang);
        if (!n.isEmpty())
            d->name = n.left(30);
    }
    const std::string desc_name = "Desc" + lang.toStdString() + ".txt";
    if (grp.hasEntry(desc_name)) {
        QString desc = latin1(grp.getFile(desc_name));
        desc.replace('\t', ' ').replace('\r', ' ').replace('\n', ' ');
        d->desc = desc;
    }
    return d;
}

// FUN_00405d10: the group's own definition, then all *.c4d children
void loadGroupDefs(const C4Group &grp, const QString &lang, std::vector<DefPtr> &out) {
    if (DefPtr d = loadDef(grp, lang))
        out.push_back(d);
    for (const C4GroupEntry &e : grp.getEntries()) {
        if (!endsWithNoCase(e.name, ".c4d"))
            continue;
        C4Group child;
        if (child.loadFromMemory(grp.getFile(e.name)))
            loadGroupDefs(child, lang, out);
    }
}

// parsed definitions of a group file, cached by path and modification time
const std::vector<DefPtr> &groupDefs(const ItemPath &path_in, const QString &lang) {
    static std::map<QString, std::vector<DefPtr>> cache;
    static const std::vector<DefPtr> empty;
    const ItemPath path = path_in.normalized();
    const QFileInfo fi(path.disk);
    if (!fi.exists())
        return empty;
    const QString key = path.toString() + "|" + lang + "|" + QString::number(fi.lastModified().toMSecsSinceEpoch());
    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;
    std::vector<DefPtr> defs;
    C4Group grp;
    if (GroupEdit::open(path, grp))
        loadGroupDefs(grp, lang, defs);
    return cache[key] = std::move(defs);
}

} // namespace

bool DefinitionDB::add(const DefPtr &def) {
    // FUN_004062a0 without overwrite: an existing id is kept, new definitions go to the list head
    if (!def || byId(def->id))
        return false;
    defs_.insert(defs_.begin(), def);
    return true;
}

int DefinitionDB::addAll(const std::vector<DefPtr> &defs) {
    int n = 0;
    for (const DefPtr &d : defs)
        if (add(d))
            ++n;
    return n;
}

QString DefinitionDB::gameDir() {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l ? l->planetDataPath() : QDir::currentPath();
}

ItemPath DefinitionDB::modulePath(const QString &module) {
    QString m = module;
    m.replace('\\', '/');
    QStringList parts = m.split('/', Qt::SkipEmptyParts);
    if (QDir::isAbsolutePath(m)) {
        // absolute path: everything that exists on disk is disk path
        QString disk = m.startsWith('/') ? "/" : QString();
        while (!parts.isEmpty()) {
            const QString next = QDir(disk).filePath(parts.first());
            if (!QFileInfo::exists(next))
                break;
            disk = next;
            parts.removeFirst();
            if (!QFileInfo(disk).isDir())
                break;
        }
        return ItemPath(disk, parts);
    }
    return ItemPath(gameDir(), parts).normalized();
}

QStringList DefinitionDB::activatedModules() {
    QStringList out;
    ClonkLauncher *l = ClonkLauncher::instance();
    if (!l)
        return out;
    const QDir game(gameDir());
    for (const ItemPath &p : l->activeDefinitions()) {
        QString rel = game.relativeFilePath(p.disk);
        if (rel.startsWith(".."))
            rel = p.disk;
        QStringList parts = rel.split('/', Qt::SkipEmptyParts);
        parts += p.subs;
        out << parts.join('\\');
    }
    if (out.isEmpty()) {
        // tree not available: the configured definition list (General\Definitions)
        const QString cfg = QString::fromStdString(l->get_cfg("General\\Definitions", "Objects.c4d"));
        for (const QString &s : cfg.split(';', Qt::SkipEmptyParts))
            out << s.trimmed();
    }
    return out;
}

int DefinitionDB::loadForScenario(const ItemPath &scenario_in, const Scen::Core &core) {
    // FUN_00406470
    const QString lang = Scen::language();
    const ItemPath scenario = scenario_in.normalized();
    int n = 0;

    // modules: [Definitions] of the scenario (LocalOnly: none) or the activated definitions
    QStringList modules;
    if (!core.getModules(modules))
        modules = activatedModules();
    for (const QString &m : modules)
        n += addAll(groupDefs(modulePath(m), lang));

    // FUN_00405f40: definitions in the .c4f folders the scenario is in
    QList<ItemPath> folders;
    {
        QDir dir = QFileInfo(scenario.disk).dir();
        QList<ItemPath> disk_folders;
        while (!dir.isRoot()) {
            if (dir.dirName().endsWith(".c4f", Qt::CaseInsensitive))
                disk_folders.prepend(ItemPath(dir.absolutePath()));
            if (!dir.cdUp())
                break;
        }
        folders += disk_folders;
        if (!scenario.subs.isEmpty()) {
            if (scenario.disk.endsWith(".c4f", Qt::CaseInsensitive))
                folders << ItemPath(scenario.disk);
            for (int i = 0; i + 1 < scenario.subs.size(); ++i)
                if (scenario.subs[i].endsWith(".c4f", Qt::CaseInsensitive))
                    folders << ItemPath(scenario.disk, scenario.subs.mid(0, i + 1));
        }
    }
    for (const ItemPath &f : folders) {
        C4Group grp;
        if (!GroupEdit::open(f, grp))
            continue;
        // only the child definitions of the folder (the folder itself is no definition)
        for (const C4GroupEntry &e : grp.getEntries())
            if (endsWithNoCase(e.name, ".c4d"))
                n += addAll(groupDefs(f.child(QString::fromStdString(e.name)), lang));
    }

    // local definitions of the scenario
    {
        C4Group grp;
        if (GroupEdit::open(scenario, grp)) {
            std::vector<DefPtr> local;
            loadGroupDefs(grp, lang, local);
            n += addAll(local);
        }
    }
    return n;
}

const Def *DefinitionDB::byId(const QString &id) const {
    if (id.isEmpty())
        return nullptr;
    for (const auto &d : defs_)
        if (d->id == id)
            return d.get();
    return nullptr;
}

int DefinitionDB::indexOf(const QString &id) const {
    for (size_t i = 0; i < defs_.size(); ++i)
        if (defs_[i]->id == id)
            return static_cast<int>(i);
    return -1;
}

const Def *DefinitionDB::get(int index, uint32_t category) const {
    if (index < 0)
        return nullptr;
    int n = -1;
    for (const auto &d : defs_)
        if ((d->category & category) && ++n == index)
            return d.get();
    return nullptr;
}

int DefinitionDB::count(uint32_t category) const {
    int n = 0;
    for (const auto &d : defs_)
        if (d->category & category)
            ++n;
    return n;
}
