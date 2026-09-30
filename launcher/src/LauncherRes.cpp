#include "LauncherRes.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSize>
#include <iostream>
#include <map>
#include <QSoundEffect>
#include <QUrl>

namespace LauncherRes {

namespace {

constexpr int LANG_OFFSET = 5000; // the original adds 5000 to the id for the English table

// Dialog base units of MS Sans Serif 8 at 96 dpi (all original dialogs use this font)
constexpr int BASE_UNIT_X = 6;
constexpr int BASE_UNIT_Y = 13;

QString data_path;
QString fe_family = "Comic Sans MS";
int fe_point_size = 9;
QString sys_family = "MS Sans Serif";
QJsonObject strings_de;
QJsonObject strings_us;
QJsonObject dialogs;
bool english = true;

QJsonObject readJson(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        std::cerr << "LauncherRes: cannot open " << path.toStdString() << std::endl;
        return {};
    }
    return QJsonDocument::fromJson(f.readAll()).object();
}

// Windows MulDiv: rounds half away from zero
int mulDiv(int a, int b, int c) {
    long long p = static_cast<long long>(a) * b;
    return static_cast<int>((p >= 0 ? p + c / 2 : p - c / 2) / c);
}

} // namespace

bool load(const QString &data_dir) {
    data_path = data_dir;
    QJsonObject s = readJson(QDir(data_dir).filePath("strings.json"));
    strings_de = s.value("DE").toObject();
    strings_us = s.value("US").toObject();
    dialogs = readJson(QDir(data_dir).filePath("dialogs.json"));
    return !strings_de.isEmpty() && !dialogs.isEmpty();
}

void setLanguage(const std::string &lang) {
    english = lang != "DE";
}

QString str(int id) {
    if (id >= 50000 + LANG_OFFSET) // accept English ids too
        id -= LANG_OFFSET;
    const QString key = QString::number(id);
    const QJsonObject &primary = english ? strings_us : strings_de;
    const QJsonObject &fallback = english ? strings_de : strings_us;
    if (primary.contains(key))
        return primary.value(key).toString();
    return fallback.value(key).toString();
}

QString resPath(const QString &type, int id) {
    QDir dir(QDir(data_path).filePath("res/" + type));
    const QStringList matches = dir.entryList({QString::number(id) + ".*"}, QDir::Files);
    if (matches.isEmpty())
        return {};
    return dir.filePath(matches.first());
}

QPixmap bitmap(int id) {
    static std::map<int, QPixmap> cache;
    auto it = cache.find(id);
    if (it != cache.end())
        return it->second;
    QPixmap pix(resPath("bitmap", id));
    if (pix.isNull())
        std::cerr << "LauncherRes: missing bitmap " << id << std::endl;
    cache[id] = pix;
    return pix;
}

std::vector<uint8_t> binary(int id) {
    QFile f(resPath("binary", id));
    if (!f.open(QIODevice::ReadOnly))
        return {};
    QByteArray b = f.readAll();
    return std::vector<uint8_t>(b.begin(), b.end());
}

QString text(int id) {
    QFile f(resPath("text", id));
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QString::fromLatin1(f.readAll());
}

static bool sounds_enabled = true;

void setSoundsEnabled(bool enabled) {
    sounds_enabled = enabled;
}

void playSound(int wave_id) {
    if (!sounds_enabled)
        return;
    static std::map<int, QSoundEffect *> effects;
    QSoundEffect *&fx = effects[wave_id];
    if (!fx) {
        fx = new QSoundEffect();
        fx->setSource(QUrl::fromLocalFile(resPath("wave", wave_id)));
    }
    fx->stop();
    fx->play();
}

void setFonts(const QString &fe, int fe_pt, const QString &sys) {
    fe_family = fe;
    fe_point_size = fe_pt;
    sys_family = sys;
}

QFont feFont() {
    QFont f(fe_family);
    // the original runs at 96 dpi: point size * 96 / 72 pixels
    f.setPixelSize((fe_point_size * 96 + 36) / 72);
    f.setStyleStrategy(QFont::NoAntialias);
    f.setHintingPreference(QFont::PreferFullHinting);
    return f;
}

QFont sysFont() {
    QFont f(sys_family);
    f.setPixelSize(11);
    f.setStyleStrategy(QFont::NoAntialias);
    f.setHintingPreference(QFont::PreferFullHinting);
    return f;
}

int dluToPxX(int x) {
    return mulDiv(x, BASE_UNIT_X, 4);
}

int dluToPxY(int y) {
    return mulDiv(y, BASE_UNIT_Y, 8);
}

QSize dialogSize(int idd) {
    QJsonObject d = dialogs.value(QString::number(idd)).toObject();
    return {dluToPxX(d.value("w").toInt()), dluToPxY(d.value("h").toInt())};
}

QJsonObject dialogTemplate(int idd) {
    return dialogs.value(QString::number(idd)).toObject();
}

QJsonArray dialogControls(int idd) {
    return dialogs.value(QString::number(idd)).toObject().value("controls").toArray();
}

QJsonObject control(int idd, int ctrl_id) {
    const QJsonArray controls = dialogControls(idd);
    for (const QJsonValue &c : controls) {
        if (c.toObject().value("id").toInt() == ctrl_id)
            return c.toObject();
    }
    return {};
}

QRect controlRect(int idd, int ctrl_id) {
    QJsonObject c = control(idd, ctrl_id);
    if (c.isEmpty()) {
        std::cerr << "LauncherRes: dialog " << idd << " has no control " << ctrl_id << std::endl;
        return {};
    }
    return {dluToPxX(c.value("x").toInt()), dluToPxY(c.value("y").toInt()),
            dluToPxX(c.value("w").toInt()), dluToPxY(c.value("h").toInt())};
}

QString controlText(int idd, int ctrl_id) {
    QJsonObject c = control(idd, ctrl_id);
    if (c.contains("text_id"))
        return str(c.value("text_id").toInt());
    return c.value("key").toString();
}

} // namespace LauncherRes
