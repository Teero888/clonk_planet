#pragma once

// Landscape preview of the scenario properties (Planet.exe class at 0x41cd10, vtable 0x468408):
// the dynamic map of C4MapCreator (engine/src/C4Map.cpp, launcher copy at 0x40bdf0) drawn with the
// material colors of the texture map, or the scenario's Landscape.bmp. Optionally the part that
// fits on the screen (Graphics\Resolution) is framed.

#include "GroupEdit.h"
#include "ScenData.h"

#include <QImage>
#include <QString>
#include <QWidget>
#include <array>
#include <vector>

// Texture map (TexMap.txt) and material colors of Material.c4g
class ScenTextureMap {
public:
    // FUN_0041d270: the scenario's Material.c4g, else the global one. Returns false without materials.
    bool load(const ItemPath &scenario);
    bool localMaterials() const { return local_; }
    // C4TextureMap::GetIndex (FUN_0040f3e0); adds unknown textures to the first free index
    int getIndex(const QString &name, bool add);
    // palette of the map (index 0 sky, +128 underground)
    std::array<QRgb, 256> palette{};

private:
    std::array<QString, 128> entries_;
    bool local_ = false;
};

// C4MapCreator (FUN_0040bdf0)
class ScenMapCreator {
public:
    void create(uint8_t *buf, int pitch, const Scen::Landscape &l, ScenTextureMap &tex, bool layers, int players);

private:
    void setPix(int x, int y, uint8_t col);
    uint8_t getPix(int x, int y) const;
    void drawLayer(int x, int y, int size, uint8_t col);
    uint8_t *buf_ = nullptr;
    int pitch_ = 0, wdt_ = 0, hgt_ = 0;
    int exclusive_ = -1;
};

class MapPreview : public QWidget {
    Q_OBJECT
public:
    explicit MapPreview(QWidget *placeholder);

    // FUN_0041ce60: texture map and a static map (Landscape.bmp) of the scenario
    void load(const ItemPath &scenario);
    bool hasStaticMap() const { return static_map_; } // FUN_0041d260
    bool localMaterials() const { return tex_.localMaterials(); } // FUN_0041d450

    // FUN_0041d240
    void setOptions(bool screen, bool layers, int players) {
        screen_ = screen;
        layers_ = layers;
        players_ = players;
    }
    // FUN_0041ce80: create the map for the landscape settings
    void render(const Scen::Landscape &l);

protected:
    void paintEvent(QPaintEvent *) override; // FUN_0041d010

private:
    ScenTextureMap tex_;
    ScenMapCreator creator_;
    QImage map_;
    bool static_map_ = false;
    bool screen_ = true, layers_ = true;
    int players_ = 1;
    int map_wdt_ = 0, map_hgt_ = 0, zoom_ = 0;
};
