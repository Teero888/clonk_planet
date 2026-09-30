#include "MapPreview.h"
#include "C4TextDoc.h"
#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "DefinitionDB.h"
#include "IDListCtrl.h"
#include "LauncherRes.h"

#include <QPainter>
#include <cmath>
#include <cstring>

namespace {

QSize screenResolution() {
    // C4ConfigGraphics::DetermineResolution (Graphics\Resolution)
    int res = 1;
    if (ClonkLauncher *l = ClonkLauncher::instance())
        res = QString::fromStdString(l->get_cfg("Graphics\\Resolution", "1")).toInt();
    switch (res) {
    case 0: return {320, 240};
    case 2: return {800, 600};
    case 3: return {1024, 768};
    case 4: return {1280, 1024};
    default: return {640, 480};
    }
}

bool wildcardMatch(const QString &a, const QString &b) {
    return a.compare(b, Qt::CaseInsensitive) == 0;
}

} // namespace

// ======================================================================================= texture map

bool ScenTextureMap::load(const ItemPath &scenario) {
    // FUN_0041d270
    entries_.fill(QString());
    palette.fill(qRgb(0, 0, 0));
    local_ = false;
    C4Group grp;
    const ItemPath local = scenario.child("Material.c4g");
    C4Group scen;
    if (GroupEdit::open(scenario, scen) && scen.hasEntry("Material.c4g") && GroupEdit::open(local, grp)) {
        local_ = true;
    } else {
        grp = C4Group();
        GroupEdit::open(ItemPath(DefinitionDB::gameDir() + "/Material.c4g"), grp);
    }
    // TexMap.txt: "index=Material-Texture"
    const std::string texmap = grp.getFileAsString("TexMap.txt");
    for (const QString &line : QString::fromLatin1(texmap.data(), static_cast<int>(texmap.size())).split('\n')) {
        const QString t = line.trimmed();
        if (t.isEmpty() || t.startsWith('#'))
            continue;
        const int eq = t.indexOf('=');
        bool ok = false;
        const int idx = t.left(eq).trimmed().toInt(&ok);
        if (eq < 0 || !ok || idx < 0 || idx >= 128)
            continue;
        entries_[idx] = t.mid(eq + 1).trimmed();
    }
    // materials: name and first color
    struct Mat {
        QString name;
        QRgb color;
    };
    std::vector<Mat> mats;
    for (const C4GroupEntry &e : grp.getEntries()) {
        const QString name = QString::fromStdString(e.name);
        if (!name.endsWith(".c4m", Qt::CaseInsensitive))
            continue;
        C4TextDoc m(grp.getFile(e.name));
        std::vector<int> c = m.getInts("Material", "Color");
        c.resize(3, 0);
        mats.push_back({m.get("Material", "Name"), qRgb(c[0], c[1], c[2])});
    }
    for (int i = 0; i < 128; ++i) {
        const QString mat = entries_[i].section('-', 0, 0); // SpanExcluding("-")
        int found = -1;
        for (size_t m = 0; m < mats.size(); ++m)
            if (wildcardMatch(mat, mats[m].name)) {
                found = static_cast<int>(m);
                break;
            }
        if (found < 0) {
            palette[i] = qRgb(i, i, i);
        } else {
            palette[i] = mats[found].color;
            palette[i + 128] = palette[i];
        }
    }
    palette[0] = qRgb(100, 100, 255);
    return !mats.empty();
}

int ScenTextureMap::getIndex(const QString &name, bool add) {
    for (int i = 0; i < 128; ++i)
        if (!entries_[i].isEmpty() && wildcardMatch(entries_[i], name))
            return i;
    if (!add)
        return 0;
    for (int i = 1; i < 128; ++i)
        if (entries_[i].isEmpty()) {
            entries_[i] = name;
            return i;
        }
    return 0;
}

// ======================================================================================= map creator

void ScenMapCreator::setPix(int x, int y, uint8_t col) {
    if (x < 0 || x >= wdt_ || y < 0 || y >= hgt_)
        return;
    if (exclusive_ > -1 && getPix(x, y) != exclusive_)
        return;
    buf_[pitch_ * y + x] = col;
}

uint8_t ScenMapCreator::getPix(int x, int y) const {
    if (x < 0 || x >= wdt_ || y < 0 || y >= hgt_)
        return 0;
    return buf_[pitch_ * y + x];
}

void ScenMapCreator::drawLayer(int x, int y, int size, uint8_t col) {
    for (int cnt = 0; cnt < size; ++cnt) {
        x += Scen::random(9) - 4;
        y += Scen::random(3) - 1;
        for (int cnt2 = Scen::random(3); cnt2 < 5; ++cnt2) {
            setPix(x + cnt2, y, col);
            setPix(x + cnt2 + 1, y + 1, col);
        }
    }
}

void ScenMapCreator::create(uint8_t *buf, int pitch, const Scen::Landscape &l, ScenTextureMap &tex, bool layers, int players) {
    // C4MapCreator::Create
    const double pi = 3.1415926535;
    const double fullperiod = 20.0 * pi;
    const int map_ift = 128;
    if (!buf)
        return;
    players = std::clamp(players, 1, 4);
    buf_ = buf;
    pitch_ = pitch;
    l.getMapSize(wdt_, hgt_, players);
    std::memset(buf_, 0, size_t(pitch_) * hgt_);

    // surface
    QString earth = l.material.left(31);
    if (!earth.contains('-'))
        earth += "-Smooth";
    uint8_t ccol = uint8_t(tex.getIndex(earth, true) + map_ift);
    const float amplitude = float(l.amplitude.evaluate());
    const float phase = float(l.phase.evaluate());
    float period = float(l.period.evaluate());
    if (l.mapPlayerExtend)
        period *= players;
    const float natural = float(l.random.evaluate());
    const int level0 = std::min(wdt_, hgt_) / 2;
    const int maxrange = level0 * 3 / 4;
    double rnd_cy = double(Scen::random(2000 + 1) - 1000) / 1000.0;
    double rnd_tend = double(Scen::random(200 + 1) - 100) / 20000.0;
    for (int cx = 0; cx < wdt_; ++cx) {
        rnd_cy += rnd_tend;
        rnd_tend += double(Scen::random(100 + 1) - 50) / 10000;
        if (rnd_tend > +0.05)
            rnd_tend = +0.05;
        if (rnd_tend < -0.05)
            rnd_tend = -0.05;
        if (rnd_cy < -0.5)
            rnd_tend += 0.01;
        if (rnd_cy > +0.5)
            rnd_tend -= 0.01;
        const double cy_natural = rnd_cy * natural / 100.0;
        const double cy_curve = std::sin(fullperiod * period / 100.0 * float(cx) / float(wdt_) + 2.0 * pi * phase / 100.0) *
                                amplitude / 100.0;
        const int cy = level0 + std::clamp(int(float(maxrange) * (cy_curve + cy_natural)), -maxrange, +maxrange);
        setPix(cx, cy, ccol);
    }

    // raise bottom to surface
    ccol = uint8_t(tex.getIndex(earth, true) + map_ift);
    for (int cx = 0; cx < wdt_; ++cx)
        for (int cy = hgt_ - 1; cy >= 0 && !getPix(cx, cy); --cy)
            setPix(cx, cy, ccol);

    // raise liquid level
    exclusive_ = 0;
    QString liquid = l.liquid.left(31);
    if (!liquid.contains('-'))
        liquid += "-Smooth";
    ccol = uint8_t(tex.getIndex(liquid, true));
    const int wtr_level = l.liquidLevel.evaluate();
    for (int cx = 0; cx < wdt_; ++cx)
        for (int cy = hgt_ * (100 - wtr_level) / 100; cy < hgt_; ++cy)
            setPix(cx, cy, ccol);
    exclusive_ = -1;

    // layers
    if (layers) {
        exclusive_ = uint8_t(tex.getIndex(earth, true) + map_ift);
        for (int cl = 0; cl < Scen::NameList::Size; ++cl) {
            if (l.layers.names[cl].isEmpty())
                continue;
            QString layer = l.layers.names[cl].left(31);
            if (!layer.contains('-'))
                layer += "-Rough";
            ccol = uint8_t(tex.getIndex(layer, true) + map_ift);
            const int layer_num = l.layers.counts[cl] * wdt_ * hgt_ / 15000;
            for (int cnt = 0; cnt < layer_num; ++cnt) {
                const int sptx = Scen::random(wdt_);
                int spty = 0;
                while (spty < hgt_ && getPix(sptx, spty) != exclusive_)
                    ++spty;
                spty += 5 + Scen::random((hgt_ - spty) - 10);
                drawLayer(sptx, spty, Scen::random(15), ccol);
            }
        }
        exclusive_ = -1;
    }
}

// ======================================================================================= widget

MapPreview::MapPreview(QWidget *placeholder) : QWidget(placeholder->parentWidget()) {
    setGeometry(placeholder->geometry());
    setObjectName(placeholder->objectName());
    setVisible(placeholder->isVisibleTo(placeholder->parentWidget()));
    placeholder->hide();
}

void MapPreview::load(const ItemPath &scenario) {
    // FUN_0041ce60 = FUN_0041d270 (texture map) + FUN_0041d190 (Landscape.bmp)
    if (!tex_.load(scenario))
        clonkMessage(window(), LauncherRes::str(51116));
    static_map_ = false;
    const std::vector<uint8_t> bmp = GroupEdit::readFile(scenario, "Landscape.bmp");
    if (!bmp.empty()) {
        QImage img;
        if (img.loadFromData(bmp.data(), static_cast<int>(bmp.size()), "BMP")) {
            static_map_ = true;
            map_ = img;
            map_wdt_ = img.width();
            map_hgt_ = img.height();
        }
    }
}

void MapPreview::render(const Scen::Landscape &l) {
    // FUN_0041ce80
    // FUN_0041d1e0: sky color from SkyFade (the last three values)
    const auto &f = l.skyDefFade;
    if (f[3] + f[4] + f[5] == 0)
        tex_.palette[0] = qRgb(100, 100, 255);
    else
        tex_.palette[0] = qRgb(f[3] & 0xff, f[4] & 0xff, f[5] & 0xff);
    if (static_map_) {
        zoom_ = l.mapZoom.evaluate();
        update();
        return;
    }
    const int players = l.mapPlayerExtend ? players_ : 1;
    l.getMapSize(map_wdt_, map_hgt_, players);
    zoom_ = l.mapZoom.evaluate();
    const int pitch = (l.mapWdt.max + 3) & ~3;
    const int buf_hgt = l.mapHgt.max;
    std::vector<uint8_t> buf(size_t(std::max(pitch, (map_wdt_ + 3) & ~3)) * std::max(buf_hgt, map_hgt_), 0);
    const int bpitch = (map_wdt_ + 3) & ~3; // DWordAlign(map width)
    creator_.create(buf.data(), bpitch, l, tex_, layers_, players);
    // FUN_00401610: 8 bit DIB with the material palette
    map_ = QImage(map_wdt_, map_hgt_, QImage::Format_Indexed8);
    QVector<QRgb> table(tex_.palette.begin(), tex_.palette.end());
    map_.setColorTable(table);
    for (int y = 0; y < map_hgt_; ++y)
        std::memcpy(map_.scanLine(y), buf.data() + size_t(bpitch) * y, map_wdt_);
    update();
}

void MapPreview::paintEvent(QPaintEvent *) {
    QPainter p(this);
    // client edge of the static (WS_BORDER)
    drawSunkenEdge(p, rect());
    const QRect c = rect().adjusted(2, 2, -2, -2);
    p.fillRect(c, Qt::white);
    if (map_.isNull())
        return;
    // FUN_00401be0: fit into the client rect
    const int bw = map_.width(), bh = map_.height();
    const int W = c.width(), H = c.height();
    int dw = W, dh = H;
    if (bh < bw) {
        dh = W * bh / bw;
        if (dh >= H)
            dh = H;
    } else {
        dw = std::min(H * bw / bh, W);
        dh = H;
    }
    p.drawImage(QRect(c.left(), c.top(), dw, dh), map_);
    // FUN_0041d010: part of the map on the screen, framed with a hatched brush
    if (screen_ && map_wdt_ && map_hgt_ && zoom_) {
        const QSize res = screenResolution();
        const int fw = std::min(res.width() * dw / (zoom_ * map_wdt_), dw);
        const int fh = std::min(dh * res.height() / (zoom_ * map_hgt_), dh);
        auto px = [&](int x, int y) {
            // HS_BDIAGONAL, black on the white background color
            p.setPen((x + y) % 8 == 7 ? Qt::black : Qt::white);
            p.drawPoint(c.left() + x, c.top() + y);
        };
        if (fw > 0 && fh > 0) {
            for (int x = 0; x < fw; ++x) {
                px(x, 0);
                px(x, fh - 1);
            }
            for (int y = 0; y < fh; ++y) {
                px(0, y);
                px(fw - 1, y);
            }
        }
    }
}
