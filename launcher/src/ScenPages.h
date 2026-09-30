#pragma once

// Pages of the scenario properties (Planet.exe ScenPageGame 3013, ScenPageEquipment 3019,
// ScenPageLandscape 3017, ScenPageEnvironment 3012, ScenPageWeather 3022).
//
// Every page copies its part of the scenario core in (the "data in" functions called by
// ScenarioPropertiesDlg FUN_00428bd0, applied in OnInitDialog) and back out on OK (FUN_00428c40).

#include "DefinitionDB.h"
#include "DialogBuilder.h"
#include "GroupEdit.h"
#include "ScenData.h"

#include <QPixmap>
#include <QWidget>
#include <array>
#include <memory>

class IDListBox;
class IDListButtons;
class ScenSlider;
class MapPreview;
class QComboBox;
class QSpinBox;
class QAbstractButton;

// what the pages share: the scenario and its definitions
struct ScenContext {
    ItemPath scenario;
    DefinitionDB defs;
};

class ScenPage : public QWidget {
    Q_OBJECT
public:
    ScenPage(int idd, ScenContext *ctx, QWidget *parent);
    ~ScenPage() override;

    virtual void load(const Scen::Core &core) = 0; // data in + OnInitDialog
    virtual void store(Scen::Core &core) = 0;      // data out

    QWidget *control(int id) const { return builder_->widget(id); }
    template <class T> T *get(int id) const { return builder_->get<T>(id); }

protected:
    void paintEvent(QPaintEvent *) override; // marble background (ExplorerDlg::OnErasebkgnd 0x420f90)

    void setText(int id, int str_id);
    ScenSlider *slider(int id);
    // list box with its buttons and scroll bar (ListInitWithCaption_4199d0)
    IDListBox *list(int list_id, int buttons_id, int scroll_id, uint32_t category, int caption_id);
    // enable a control and the replacement widget of it
    void enable(int id, bool on);
    void show(int id, bool on);
    QWidget *widget(int id) const; // replacement if there is one, else the template control
    // icon of an image list drawn at the top left of a control (FUN_0042b3c0)
    void drawIcon(QPainter &p, const QPixmap &strip, int size, int index, int ctrl_id);

    ScenContext *ctx_;
    std::unique_ptr<DialogBuilder> builder_;
    std::map<int, QWidget *> replaced_;
    std::map<int, IDListBox *> lists_;
};

class ScenPageGame : public ScenPage {
    Q_OBJECT
public:
    ScenPageGame(ScenContext *ctx, QWidget *parent);
    void load(const Scen::Core &core) override;
    void store(Scen::Core &core) override;

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    IDListBox *goals_ = nullptr, *rules_ = nullptr;
    QComboBox *icon_ = nullptr;
    QSpinBox *max_players_ = nullptr;
};

class ScenPageEquipment : public ScenPage {
    Q_OBJECT
public:
    ScenPageEquipment(ScenContext *ctx, QWidget *parent);
    void load(const Scen::Core &core) override;
    void store(Scen::Core &core) override;

    // switch the player radio like a click (with the "all players" question)
    bool selectPlayer(int player); // 0x422d20
    void setExtended(bool on);     // FUN_00423440

private:
    void loadPlayer(int player); // FUN_004230f0
    void savePlayer(int player); // FUN_004231c0
    void checkPlayerRadio(int player); // FUN_00422d00

    std::array<Scen::PlrStart, 4> players_;
    int current_ = 0;
    bool extended_ = false;
    bool no_init_ = false;
    QSpinBox *wealth_ = nullptr;
    IDListBox *crew_, *base_, *vehic_, *material_, *knowledge_, *hb_material_, *hb_production_, *magic_;
};

class ScenPageLandscape : public ScenPage {
    Q_OBJECT
public:
    ScenPageLandscape(ScenContext *ctx, QWidget *parent);
    void load(const Scen::Core &core) override;
    void store(Scen::Core &core) override;

private:
    void storeControls(); // FUN_00421cc0
    void redraw();        // FUN_00421dc0
    void updateEnable();  // FUN_00421e00
    void updateZoomText(); // FUN_00421f80

    Scen::Landscape l_;
    MapPreview *preview_ = nullptr;
    ScenSlider *width_, *height_, *zoom_, *amplitude_, *phase_, *period_, *random_, *water_;
    int map_type_ = 0; // 0 dynamic, 1 static, 2 exact
    int players_ = 0;  // radio index of "Preview for"
    bool loaded_ = false;
};

class ScenPageEnvironment : public ScenPage {
    Q_OBJECT
public:
    ScenPageEnvironment(ScenContext *ctx, QWidget *parent);
    void load(const Scen::Core &core) override;
    void store(Scen::Core &core) override;

private:
    IDListBox *animals_, *nests_, *vegetation_, *in_earth_, *environment_;
    ScenSlider *veg_level_, *in_earth_level_;
};

class ScenPageWeather : public ScenPage {
    Q_OBJECT
public:
    ScenPageWeather(ScenContext *ctx, QWidget *parent);
    void load(const Scen::Core &core) override;
    void store(Scen::Core &core) override;

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    ScenSlider *climate_, *season_, *time_, *rain_, *thunder_, *wind_, *earthquake_, *volcano_, *meteorite_, *gravity_;
};
