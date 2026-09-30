#include "ExplorerModel.h"
#include "C4TextDoc.h"
#include "LauncherRes.h"
#include "Utils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <cstring>

namespace {

// Planet.exe 0x47cbb0: name, extension, icon, open icon, group, shown in player view, editor,
// newable, template
const ExplorerType TYPES[T_Count] = {
    {51905, "", 2, 21, true, true, 0, 2, 0},            // Directory
    {51917, "c4p", 6, 6, true, true, 0, 2, 5005},       // Player
    {51908, "c4f", 4, 22, true, true, 0, 2, 5003},      // Scenario folder
    {51920, "c4s", 14, 14, true, true, 0, 2, 5007},     // Scenario
    {51909, "c4g", 5, 5, true, false, 0, 0, 0},         // Group file
    {51913, "c4m", 20, 20, false, false, 1, 0, 0},      // Material definition
    {51910, "hlp", 7, 7, false, false, 0, 0, 0},        // Help file
    {51912, "log", 8, 8, false, false, 1, 0, 0},        // Log file
    {51923, "txt", 9, 9, false, false, 1, 1, 5010},     // Text
    {51919, "rtf", 11, 11, false, false, 4, 1, 5006},   // Description
    {51902, "bmp", 15, 15, false, false, 2, 1, 5000},   // Bitmap
    {51921, "c", 10, 10, false, false, 6, 1, 5008},     // Script
    {51904, "c4d", 12, 12, true, true, 0, 1, 5002},     // Object definition
    {51903, "c4d", 12, 12, false, false, 0, 2, 5001},   // Object folder
    {51916, "c4i", 13, 13, true, true, 0, 0, 0},        // Crew member
    {51922, "wav", 17, 17, false, false, 3, 1, 5009},   // Sound
    {51914, "mid", 16, 16, false, false, 5, 0, 0},      // Music file
    {51924, "avi", 18, 18, false, false, 0, 0, 0},      // Video
    {51901, "c4b", 19, 19, false, false, 0, 0, 0},      // Binary component
    {51907, "exe", 1, 1, false, false, 0, 0, 0},        // Executable
    {51906, "c4x", 3, 3, false, false, 0, 0, 0},        // Engine
    {51925, "zip", 68, 68, false, true, 7, 0, 0},       // Zip archive
    {51900, "c4v", 18, 18, false, false, 0, 0, 0},      // Animation
    {51911, "html", 9, 9, false, false, 10, 0, 0},      // Hypertext
    {51915, "", 0, 0, false, false, 0, 0, 0},           // Unknown
};

// C4ComponentHost::GetLanguageString: "US:" anywhere in the text, up to the line end
QString languageString(const std::vector<uint8_t> &data, const QString &language) {
    if (data.empty())
        return {};
    const QString text = QString::fromLatin1(reinterpret_cast<const char *>(data.data()), static_cast<int>(data.size()));
    const int i = text.indexOf(language.left(2) + ":");
    if (i < 0)
        return {};
    QString s = text.mid(i + 3);
    const int cr = s.indexOf('\r');
    const int lf = s.indexOf('\n');
    int end = cr < 0 ? lf : (lf < 0 ? cr : qMin(cr, lf));
    if (end >= 0)
        s = s.left(end);
    return s;
}

QString withoutExtension(const QString &filename) {
    const int dot = filename.lastIndexOf('.');
    return dot < 0 ? filename : filename.left(dot);
}

QString formatTime(int seconds) {
    return QString::asprintf("%02d:%02d:%02d", seconds / 3600, (seconds % 3600) / 60, seconds % 60);
}

QString formatDate(int time) {
    const QDateTime dt = QDateTime::fromSecsSinceEpoch(time);
    return QString::asprintf("%02d.%02d.%d %02d:%02d", dt.date().day(), dt.date().month(), dt.date().year(),
                             dt.time().hour(), dt.time().minute());
}

QString cformat(QString fmt, const QStringList &args) {
    // printf style %s / %i with QString arguments (string table formats)
    QString out;
    int a = 0;
    for (int i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '%' && i + 1 < fmt.size() && (fmt[i + 1] == 's' || fmt[i + 1] == 'i' || fmt[i + 1] == 'd')) {
            out += a < args.size() ? args[a++] : QString();
            ++i;
        } else {
            out += fmt[i];
        }
    }
    return out;
}

bool sameDir(const QString &a, const QString &b) {
    return QDir::cleanPath(QFileInfo(a).absoluteFilePath()).compare(QDir::cleanPath(QFileInfo(b).absoluteFilePath()),
                                                                     Qt::CaseInsensitive) == 0;
}

} // namespace

const ExplorerType &explorerType(int type) {
    if (type < 0 || type >= T_Count)
        return TYPES[T_Unknown];
    return TYPES[type];
}

int explorerTypeFor(const QString &filename, bool is_directory) {
    const int dot = filename.lastIndexOf('.');
    if (dot >= 0) {
        const QString ext = filename.mid(dot + 1);
        for (int t = 0; t < T_Count; ++t)
            if (*TYPES[t].ext && ext.compare(TYPES[t].ext, Qt::CaseInsensitive) == 0)
                return t;
    }
    return is_directory ? T_Directory : T_Unknown;
}

bool isModule(const QStringList &list, const QString &name) {
    for (const QString &m : list)
        if (m.compare(name, Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

QString groupTitle(const C4Group &grp, const QString &filename, const QString &language) {
    const int type = explorerTypeFor(filename, false);
    QString title;
    if (type == T_Player) {
        title = C4TextDoc(grp.getFile("Player.txt")).get("Player", "Name");
    } else if (type == T_Definition) {
        // C4Def: Names.txt in the current language, else DefCore name
        title = languageString(grp.getFile("Names.txt"), language);
        if (title.isEmpty())
            title = C4TextDoc(grp.getFile("DefCore.txt")).get("DefCore", "Name");
    } else {
        title = languageString(grp.getFile("Title.txt"), language);
        if (title.isEmpty() && type == T_Scenario)
            title = C4TextDoc(grp.getFile("Scenario.txt")).get("Head", "Title");
    }
    if (title.isEmpty())
        title = withoutExtension(filename);
    return title;
}

// Item::Init (0x4319a0) + item evaluation (0x431ea0)
bool initExplorerItem(ExplorerItem &item, const ItemPath &path_in, const ExplorerContext &ctx) {
    item = ExplorerItem();
    item.path = path_in;
    item.filename = path_in.name();
    const bool is_dir = GroupEdit::isDirectory(path_in);
    item.type = explorerTypeFor(item.filename, is_dir);

    // unknown entries that are groups are shown as directories
    C4Group grp;
    bool is_group = false;
    if (explorerType(item.type).group || item.type == T_Unknown || item.type == T_Directory) {
        is_group = GroupEdit::open(path_in, grp);
        if (item.type == T_Unknown && is_group)
            item.type = T_Directory;
    }
    const ExplorerType &type = explorerType(item.type);

    if (!ctx.developer_view && !type.player_view)
        return false;

    if (is_group) {
        item.maker = QString::fromStdString(grp.getMaker());
        item.original = grp.isOriginal();
        item.creation = grp.getCreation();
        item.group_packed = grp.isPacked() || !path_in.subs.isEmpty();
    }

    item.icon = type.icon;
    item.icon_open = type.icon_open;
    item.title = is_group ? groupTitle(grp, item.filename, ctx.language) : withoutExtension(item.filename);
    item.expandable = type.group && (ctx.developer_view || item.type != T_Player);
    item.bold = !ctx.developer_view && item.type == T_ScenarioFolder;
    const bool in_root = path_in.subs.isEmpty() && sameDir(QFileInfo(path_in.disk).absolutePath(), ctx.data_dir);

    switch (item.type) {
    case T_Player: {
        if (!is_group)
            break;
        C4TextDoc p(grp.getFile("Player.txt"));
        const QString name = p.get("Player", "Name");
        if (!name.isEmpty())
            item.title = name;
        // statistics shown instead of a description
        QString info = cformat(LauncherRes::str(50410),
                               {QString::number(p.getInt("Player", "Score")), QString::number(p.getInt("Player", "Rounds")),
                                QString::number(p.getInt("Player", "RoundsWon")), QString::number(p.getInt("Player", "RoundsLost")),
                                formatTime(p.getInt("Player", "TotalPlayingTime")), p.get("Player", "Comment")});
        if (p.has("LastRound", "Title") && !p.get("LastRound", "Title").isEmpty()) {
            info += cformat(LauncherRes::str(50406),
                            {p.get("LastRound", "Title"), formatDate(p.getInt("LastRound", "Date")),
                             formatTime(p.getInt("LastRound", "Duration")), QString::number(p.getInt("LastRound", "Score"))});
        }
        item.info_title = QString("%1 %2").arg(p.get("Player", "RankName"), item.title);
        item.info_text = info;
        const int color = qBound(0, p.getInt("Preferences", "Color"), 8);
        item.icon = item.icon_open = color + 99;
        // players can only be activated in the data directory
        if (in_root) {
            item.activatable = true;
            item.activated = isModule(ctx.participants, item.filename);
        }
        item.has_properties = true;
        break;
    }
    case T_Scenario: {
        if (!is_group)
            break;
        C4TextDoc s(grp.getFile("Scenario.txt"));
        item.scenario_icon = s.getInt("Head", "Icon");
        item.icon = item.icon_open = item.scenario_icon + 24;
        item.sort_icon = item.icon;
        item.scenario_access = s.getInt("Head", "Access") != 0;
        // scenario properties: always for registered users, else only with [Head] Access=1
        item.has_properties = item.scenario_access || ctx.registered;
        break;
    }
    case T_Definition: {
        // only definition packs in the data directory can be activated
        if (in_root) {
            item.activatable = true;
            item.activated = isModule(ctx.definitions, item.filename);
        }
        break;
    }
    default:
        break;
    }

    // custom tree icon (0x432441: Icon.bmp added to the image list masked with magenta)
    if (is_group && !ctx.developer_view) {
        const auto icon = grp.getFile("Icon.bmp");
        if (!icon.empty()) {
            QImage img;
            if (img.loadFromData(icon.data(), static_cast<uint>(icon.size()))) {
                img = img.convertToFormat(QImage::Format_ARGB32);
                for (int y = 0; y < img.height(); ++y)
                    for (int x = 0; x < img.width(); ++x)
                        if ((img.pixel(x, y) & 0xFFFFFF) == 0xFF00FF)
                            img.setPixel(x, y, 0);
                item.custom_icon = QPixmap::fromImage(img);
            }
        }
    }

    // this port runs from the build directory, which is also the data directory: hide the build
    // system's files and plain directories without game content in the root (CMakeFiles, res, ...)
    if (in_root) {
        if (item.type == T_Unknown)
            return false;
        static const QStringList port_files = {"CMakeCache.txt", "clonk.ini", "install_manifest.txt"};
        if (isModule(port_files, item.filename))
            return false;
        if (item.type == T_Directory && is_dir) {
            const QStringList content = QDir(path_in.disk).entryList({"*.c4?"}, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
            if (content.isEmpty())
                return false;
        }
    }

    if (ctx.developer_view)
        item.title = item.filename;
    return true;
}

std::vector<ExplorerItem> listExplorerItems(const ItemPath &group_in, const ExplorerContext &ctx) {
    std::vector<ExplorerItem> out;
    const ItemPath group = group_in.normalized();
    QStringList names;
    if (GroupEdit::isDirectory(group)) {
        names = QDir(group.disk).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
    } else {
        C4Group grp;
        if (!GroupEdit::open(group, grp))
            return out;
        for (const auto &e : grp.getEntries())
            names << QString::fromStdString(e.name);
    }
    for (const QString &n : names) {
        // skip the engine's temporary files and the midi soundfont of this port
        if (n.endsWith(".tmp", Qt::CaseInsensitive))
            continue;
        ExplorerItem item;
        if (initExplorerItem(item, group.child(n), ctx))
            out.push_back(item);
    }
    return out;
}

size_t explorerInsertPos(const std::vector<ExplorerItem> &siblings, const ExplorerItem &item) {
    // position = after the last sibling that satisfies the condition, first if none does
    size_t pos = 0;
    const bool numbered = item.icon >= 27 && item.icon <= 35;
    const QByteArray title = item.title.toLatin1();
    for (size_t i = 0; i < siblings.size(); ++i) {
        const ExplorerItem &s = siblings[i];
        bool after;
        if (numbered)
            after = s.icon < item.icon;
        else
            after = s.type < item.type ||
                    (s.type == item.type && std::strcmp(s.title.toLatin1().constData(), title.constData()) < 0);
        if (after)
            pos = i + 1;
    }
    return pos;
}

void sortExplorerItems(std::vector<ExplorerItem> &items) {
    std::vector<ExplorerItem> sorted;
    sorted.reserve(items.size());
    for (const ExplorerItem &item : items)
        sorted.insert(sorted.begin() + explorerInsertPos(sorted, item), item);
    items.swap(sorted);
}

namespace {

QString htmlEscape(const QString &s) {
    return s.toHtmlEscaped().replace("\r\n", "<br>").replace("\n", "<br>").replace("\r", "<br>");
}

// FUN_0042a850: RTF with a bold 10pt heading and 8pt text
QString headedText(const QString &title, const QString &text) {
    QString html;
    if (!title.isEmpty())
        html += "<p style=\"margin:0; font-size:10pt; font-weight:bold\">" + htmlEscape(title) + "</p><p style=\"margin:0; font-size:8pt\">&nbsp;</p>";
    html += "<p style=\"margin:0; font-size:8pt\">" + htmlEscape(text) + "</p>";
    return html;
}

QString bytesToString(const std::vector<uint8_t> &d) {
    QString s = QString::fromLatin1(reinterpret_cast<const char *>(d.data()), static_cast<int>(d.size()));
    while (s.endsWith(QChar(0)))
        s.chop(1);
    return s;
}

// file of a group: <prefix><language><suffix>, else any language (Desc*.rtf)
std::vector<uint8_t> languageFile(const C4Group &grp, const QString &prefix, const QString &suffix, const QString &lang) {
    auto d = grp.getFile((prefix + lang + suffix).toStdString());
    if (!d.empty())
        return d;
    for (const auto &e : grp.getEntries()) {
        const QString n = QString::fromStdString(e.name);
        if (n.startsWith(prefix, Qt::CaseInsensitive) && n.endsWith(suffix, Qt::CaseInsensitive))
            return grp.getFile(e.name);
    }
    return {};
}

QPixmap loadBitmap(const std::vector<uint8_t> &d) {
    QPixmap pix;
    if (!d.empty())
        pix.loadFromData(d.data(), static_cast<uint>(d.size()));
    return pix;
}

} // namespace

ExplorerInfo loadExplorerInfo(const ExplorerItem &item, const ExplorerContext &ctx) {
    ExplorerInfo info;
    C4Group grp;
    const bool is_group = explorerType(item.type).group || item.type == T_Directory;
    if (is_group)
        GroupEdit::open(item.path, grp);

    // description (0x431c50)
    switch (item.type) {
    case T_Directory:
    case T_ScenarioFolder:
    case T_Scenario:
    case T_Group: {
        const auto d = languageFile(grp, "Desc", ".rtf", ctx.language);
        if (!d.empty())
            info.html = QString::fromStdString(rtfToHtml(std::string(d.begin(), d.end())));
        break;
    }
    case T_Material:
    case T_Log:
    case T_Text:
    case T_Script: {
        std::vector<uint8_t> d;
        GroupEdit::readItem(item.path, d);
        if (d.size() <= 10000)
            info.html = headedText({}, bytesToString(d));
        info.frontend_font = false;
        break;
    }
    case T_RichText: {
        std::vector<uint8_t> d;
        GroupEdit::readItem(item.path, d);
        info.html = QString::fromStdString(rtfToHtml(std::string(d.begin(), d.end())));
        break;
    }
    case T_Definition: {
        const auto d = languageFile(grp, "Desc", ".txt", ctx.language);
        if (!d.empty())
            info.html = headedText(ctx.developer_view ? QString() : item.title, bytesToString(d));
        break;
    }
    case T_Player:
    case T_CrewMember:
        info.html = headedText(item.info_title, item.info_text);
        break;
    default:
        break;
    }

    // picture (0x432fa0; definition graphics in 0x431ea0)
    switch (item.type) {
    case T_Directory:
    case T_ScenarioFolder:
    case T_Scenario:
    case T_Group:
        info.picture = loadBitmap(grp.getFile("Title.bmp"));
        break;
    case T_Player:
    case T_CrewMember:
        info.picture = loadBitmap(grp.getFile("Portrait.bmp"));
        break;
    case T_Bitmap: {
        std::vector<uint8_t> d;
        GroupEdit::readItem(item.path, d);
        info.picture = loadBitmap(d);
        break;
    }
    case T_Definition: {
        info.picture = loadBitmap(grp.getFile("Title.bmp"));
        if (info.picture.isNull()) {
            // DefCore Picture=x,y,w,h of Graphics.bmp, drawn transparent
            QPixmap g = loadBitmap(grp.getFile("Graphics.bmp"));
            const std::vector<int> r = C4TextDoc(grp.getFile("DefCore.txt")).getInts("DefCore", "Picture");
            if (!g.isNull() && r.size() >= 4 && r[2] > 0 && r[3] > 0)
                g = g.copy(r[0], r[1], r[2], r[3]);
            if (!g.isNull()) {
                info.picture = g;
                info.transparent = true;
            }
        }
        break;
    }
    default:
        break;
    }
    return info;
}
