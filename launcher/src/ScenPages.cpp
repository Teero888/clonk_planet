#include "ScenPages.h"
#include "IDListCtrl.h"
#include "LauncherRes.h"
#include "ScenStyle.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QPainter>
#include <QSpinBox>

namespace {

// Disabled statics are drawn etched (white offset text below gray text) like DrawState(DSS_DISABLED)
class EtchedLabels : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override {
        if (e->type() != QEvent::Paint)
            return false;
        auto *l = qobject_cast<QLabel *>(o);
        if (!l || l->isEnabled() || l->text().isEmpty())
            return false;
        QPainter p(l);
        p.setFont(l->font());
        const QRect r = l->contentsRect();
        const int flags = int(l->alignment()) | (l->wordWrap() ? Qt::TextWordWrap : 0);
        p.setPen(Qt::white);
        p.drawText(r.translated(1, 1), flags, l->text());
        p.setPen(SysColor::btnShadow());
        p.drawText(r, flags, l->text());
        return true;
    }
};

// image list of a bitmap strip with the magenta mask (FUN_0042a5c0)
QPixmap maskedStrip(int bitmap_id) {
    static std::map<int, QPixmap> cache;
    auto it = cache.find(bitmap_id);
    if (it != cache.end())
        return it->second;
    QImage img = LauncherRes::bitmap(bitmap_id).toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            if ((img.pixel(x, y) & 0xffffff) == 0xff00ff)
                img.setPixel(x, y, qRgba(0, 0, 0, 0));
    return cache[bitmap_id] = QPixmap::fromImage(img);
}

} // namespace

// ======================================================================================= ScenPage

ScenPage::ScenPage(int idd, ScenContext *ctx, QWidget *parent) : QWidget(parent), ctx_(ctx) {
    resize(LauncherRes::dialogSize(idd));
    setFont(LauncherRes::feFont());
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, Qt::black);
    setPalette(pal);
    builder_ = std::make_unique<DialogBuilder>(idd, this, DialogBuilder::Skinned);
    applyClassicButtons(this);
    auto *etch = new EtchedLabels(this);
    for (const auto &w : builder_->widgets())
        if (qobject_cast<QLabel *>(w.second))
            w.second->installEventFilter(etch);
}

ScenPage::~ScenPage() = default;

void ScenPage::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawTiledPixmap(rect(), LauncherRes::bitmap(1019));
}

void ScenPage::setText(int id, int str_id) {
    // CtrlSetTextRes_*
    if (QWidget *w = control(id)) {
        if (auto *l = qobject_cast<QLabel *>(w))
            l->setText(LauncherRes::str(str_id));
        else if (auto *b = qobject_cast<QAbstractButton *>(w))
            b->setText(LauncherRes::str(str_id));
    }
}

QWidget *ScenPage::widget(int id) const {
    auto it = replaced_.find(id);
    return it != replaced_.end() ? it->second : control(id);
}

ScenSlider *ScenPage::slider(int id) {
    auto *s = new ScenSlider(control(id));
    replaced_[id] = s;
    return s;
}

IDListBox *ScenPage::list(int list_id, int buttons_id, int scroll_id, uint32_t category, int caption_id) {
    auto *lb = new IDListBox(control(list_id));
    replaced_[list_id] = lb;
    IDListButtons *buttons = nullptr;
    if (buttons_id) {
        buttons = new IDListButtons(control(buttons_id));
        replaced_[buttons_id] = buttons;
    }
    ScenSlider *scroll = nullptr;
    if (scroll_id)
        scroll = slider(scroll_id);
    lb->init(category, caption_id, &ctx_->defs, buttons, scroll);
    lists_[list_id] = lb;
    return lb;
}

void ScenPage::enable(int id, bool on) {
    // FUN_00455cbd = CWnd::EnableWindow
    if (QWidget *w = widget(id))
        w->setEnabled(on);
}

void ScenPage::show(int id, bool on) {
    // FUN_00455b47(WS_VISIBLE) = ModifyStyle
    if (QWidget *w = widget(id))
        w->setVisible(on);
}

void ScenPage::drawIcon(QPainter &p, const QPixmap &strip, int size, int index, int ctrl_id) {
    // FUN_0042b3c0: ImageList_Draw at the window rect of the control
    QWidget *w = control(ctrl_id);
    if (!w || !widget(ctrl_id)->isVisible())
        return;
    p.drawPixmap(w->geometry().topLeft(), strip, QRect(index * size, 0, size, size));
}

// ======================================================================================= Game

namespace {
enum GameIds {
    G_LOCK = 2109, G_ACCESS_LABEL = 2264, G_ACCESS = 2074, G_MAXPLAYER_LABEL = 2319, G_MAXPLAYER = 2146,
    G_ICON_LABEL = 2307, G_ICON = 2106, G_GOALS_LABEL = 2300, G_GOALS = 2196, G_GOALS_BUTTONS = 2065,
    G_GOALS_SCROLL = 2229, G_RULES_LABEL = 2341, G_RULES = 2201, G_RULES_BUTTONS = 2069, G_RULES_SCROLL = 2245,
};
}

ScenPageGame::ScenPageGame(ScenContext *ctx, QWidget *parent) : ScenPage(LauncherRes::IDD_SCEN_GAME, ctx, parent) {
    // ScenPageGame::OnInitDialog
    goals_ = list(G_GOALS, G_GOALS_BUTTONS, G_GOALS_SCROLL, C4D::Goal, 51704);
    rules_ = list(G_RULES, G_RULES_BUTTONS, G_RULES_SCROLL, C4D::Rule, 51712);
    rules_->setShowCounts(false);
    max_players_ = get<QSpinBox>(G_MAXPLAYER);
    max_players_->setRange(1, 12); // UDM_SETRANGE 0x1000c
    // UDS_ALIGNRIGHT: the up-down sits inside the edit
    max_players_->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_SCEN_GAME, G_MAXPLAYER));
    // FUN_004184f0
    setText(G_GOALS_LABEL, 50187);
    setText(G_RULES_LABEL, 50295);
    setText(G_MAXPLAYER_LABEL, 50228);
    setText(G_ICON_LABEL, 50203);
    setText(G_ACCESS_LABEL, 50100);
    setText(G_ACCESS, 50171);
    // FUN_00402360(1013, 16, 16, ...): owner drawn combo box with the scenario icons
    icon_ = get<QComboBox>(G_ICON);
    icon_->setIconSize(QSize(16, 16));
    // CB_SETITEMHEIGHT 16: the field is 16 + 6 pixels high
    icon_->setGeometry(icon_->x(), icon_->y(), icon_->width(), 22);
    const QPixmap icons = maskedStrip(1013);
    for (int i = 0; i < icons.width() / 16; ++i)
        icon_->addItem(QIcon(icons.copy(i * 16, 0, 16, 16)), QString());
    if (auto *v = qobject_cast<QListView *>(icon_->view()))
        v->setIconSize(QSize(16, 16));
}

void ScenPageGame::load(const Scen::Core &core) {
    // FUN_004186a0 + OnInitDialog
    goals_->setList(core.goals);
    rules_->setList(core.rules);
    get<QAbstractButton>(G_ACCESS)->setChecked(core.roundOptions != 0);
    icon_->setCurrentIndex(core.icon >= 0 && core.icon < icon_->count() ? core.icon : -1);
    max_players_->setValue(core.maxPlayer);
    if (core.noInitialize) {
        for (int id : {G_GOALS_LABEL, G_GOALS, G_GOALS_BUTTONS, G_GOALS_SCROLL, G_RULES_LABEL, G_RULES, G_RULES_BUTTONS,
                       G_RULES_SCROLL})
            enable(id, false);
    }
}

void ScenPageGame::store(Scen::Core &core) {
    // FUN_00418710
    core.goals = goals_->list();
    core.rules = rules_->list();
    core.roundOptions = get<QAbstractButton>(G_ACCESS)->isChecked() ? 1 : 0;
    core.icon = icon_->currentIndex();
    core.maxPlayer = max_players_->value();
}

void ScenPageGame::paintEvent(QPaintEvent *e) {
    // ScenPageGame::OnPaint: lock icon (image 5 of bitmap 1011)
    ScenPage::paintEvent(e);
    QPainter p(this);
    drawIcon(p, maskedStrip(1011), 16, 5, G_LOCK);
}

// ======================================================================================= Environment

namespace {
enum EnvIds {
    E_ANIMALS_LABEL = 2266, E_ANIMALS = 2192, E_ANIMALS_BUTTONS = 2062, E_ANIMALS_SCROLL = 2223,
    E_VEG_LABEL = 2367, E_VEG = 2204, E_VEG_BUTTONS = 2071, E_VEG_SCROLL = 2250,
    E_VEG_AMOUNT_LABEL = 2366, E_VEG_AMOUNT = 2251,
    E_NESTS_LABEL = 2326, E_NESTS = 2200, E_NESTS_BUTTONS = 2068, E_NESTS_SCROLL = 2240,
    E_INEARTH_LABEL = 2309, E_INEARTH = 2198, E_INEARTH_BUTTONS = 2066, E_INEARTH_SCROLL = 2234,
    E_INEARTH_AMOUNT_LABEL = 2308, E_INEARTH_AMOUNT = 2235,
    E_ENV_LABEL = 2295, E_ENV = 2195, E_ENV_BUTTONS = 2064, E_ENV_SCROLL = 2228,
};
}

ScenPageEnvironment::ScenPageEnvironment(ScenContext *ctx, QWidget *parent)
    : ScenPage(LauncherRes::IDD_SCEN_ENVIRONMENT, ctx, parent) {
    // ScenPageEnvironment::OnInitDialog
    setText(E_ANIMALS_LABEL, 50105);
    setText(E_NESTS_LABEL, 50243);
    setText(E_VEG_LABEL, 50356);
    setText(E_VEG_AMOUNT_LABEL, 50355);
    setText(E_INEARTH_LABEL, 50207);
    setText(E_INEARTH_AMOUNT_LABEL, 50206);
    setText(E_ENV_LABEL, 50174);
    animals_ = list(E_ANIMALS, E_ANIMALS_BUTTONS, E_ANIMALS_SCROLL, C4D::SelectAnimal, 51700);
    nests_ = list(E_NESTS, E_NESTS_BUTTONS, E_NESTS_SCROLL, C4D::SelectNest, 51710);
    vegetation_ = list(E_VEG, E_VEG_BUTTONS, E_VEG_SCROLL, C4D::SelectVegetation, 51714);
    in_earth_ = list(E_INEARTH, E_INEARTH_BUTTONS, E_INEARTH_SCROLL, C4D::SelectInEarth, 51706);
    environment_ = list(E_ENV, E_ENV_BUTTONS, E_ENV_SCROLL, C4D::Environment, 51703);
    veg_level_ = slider(E_VEG_AMOUNT);
    in_earth_level_ = slider(E_INEARTH_AMOUNT);
}

void ScenPageEnvironment::load(const Scen::Core &core) {
    // FUN_00420b40 + OnInitDialog
    animals_->setList(core.animals);
    nests_->setList(core.nests);
    vegetation_->setList(core.landscape.vegetation);
    in_earth_->setList(core.landscape.inEarth);
    veg_level_->setSVal(core.landscape.vegLevel);
    in_earth_level_->setSVal(core.landscape.inEarthLevel);
    environment_->setList(core.environment);
    veg_level_->applyColor(0);
    in_earth_level_->applyColor(0);
    if (core.noInitialize) {
        for (int id : {E_VEG_LABEL, E_VEG_AMOUNT_LABEL, E_NESTS_LABEL, E_INEARTH_LABEL, E_INEARTH_AMOUNT_LABEL,
                       E_ANIMALS_LABEL, E_ENV_LABEL, E_VEG_AMOUNT, E_INEARTH_AMOUNT, E_VEG, E_NESTS, E_INEARTH, E_ANIMALS,
                       E_ENV, E_VEG_SCROLL, E_NESTS_SCROLL, E_INEARTH_SCROLL, E_ANIMALS_SCROLL, E_ENV_SCROLL, E_VEG_BUTTONS,
                       E_NESTS_BUTTONS, E_INEARTH_BUTTONS, E_ANIMALS_BUTTONS, E_ENV_BUTTONS})
            enable(id, false);
    }
    // no environment definitions: the environment objects are hidden
    if (ctx_->defs.count(C4D::Environment) == 0) {
        show(E_ENV_LABEL, false);
        show(E_ENV_BUTTONS, false);
        show(E_ENV, false);
        show(E_ENV_SCROLL, false);
    }
}

void ScenPageEnvironment::store(Scen::Core &core) {
    // FUN_00420be0
    core.animals = animals_->list();
    core.nests = nests_->list();
    core.landscape.vegetation = vegetation_->list();
    core.landscape.inEarth = in_earth_->list();
    core.landscape.vegLevel = veg_level_->sval();
    core.landscape.inEarthLevel = in_earth_level_->sval();
    core.environment = environment_->list();
}

// ======================================================================================= Weather

namespace {
enum WeatherIds {
    W_CLIMATE = 2225, W_SEASON = 2246, W_TIME = 2249, W_RAIN = 2243, W_THUNDER = 2248, W_WIND = 2256,
    W_EARTHQUAKE = 2227, W_VOLCANO = 2253, W_METEORITE = 2239, W_GRAVITY = 2230,
};
}

ScenPageWeather::ScenPageWeather(ScenContext *ctx, QWidget *parent) : ScenPage(LauncherRes::IDD_SCEN_WEATHER, ctx, parent) {
    // FUN_00433bf0
    const std::pair<int, int> texts[] = {
        {2270, 50133}, {2271, 50134}, {2272, 50135}, {2344, 50298}, {2345, 50299}, {2346, 50300}, {2362, 50344},
        {2363, 50345}, {2364, 50346}, {2334, 50281}, {2359, 50341}, {2375, 50371}, {2335, 50282}, {2360, 50342},
        {2376, 50372}, {2336, 50283}, {2361, 50343}, {2377, 50373}, {2291, 50164}, {2292, 50165}, {2293, 50166},
        {2369, 50363}, {2370, 50364}, {2371, 50365}, {2321, 50234}, {2322, 50235}, {2323, 50236}, {2301, 50194},
        {2302, 50195}, {2303, 50196}};
    for (const auto &t : texts)
        setText(t.first, t.second);
    climate_ = slider(W_CLIMATE);
    season_ = slider(W_SEASON);
    time_ = slider(W_TIME);
    rain_ = slider(W_RAIN);
    thunder_ = slider(W_THUNDER);
    wind_ = slider(W_WIND);
    earthquake_ = slider(W_EARTHQUAKE);
    volcano_ = slider(W_VOLCANO);
    meteorite_ = slider(W_METEORITE);
    gravity_ = slider(W_GRAVITY);
}

void ScenPageWeather::load(const Scen::Core &core) {
    // FUN_00433eb0 + OnInitDialog
    climate_->setSVal(core.climate);
    season_->setSVal(core.startSeason);
    time_->setSVal(core.yearSpeed);
    rain_->setSVal(core.rain);
    thunder_->setSVal(core.lightning);
    wind_->setSVal(core.wind);
    earthquake_->setSVal(core.earthquake);
    volcano_->setSVal(core.volcano);
    meteorite_->setSVal(core.meteorite);
    gravity_->setSVal(core.landscape.gravity);
    climate_->applyColor(0x50a0c8);
    season_->applyColor(0x2878a0);
    time_->applyColor(0x5078);
    rain_->applyColor(0x822828);
    thunder_->applyColor(0xc85a5a);
    wind_->applyColor(0xfa8c8c);
    earthquake_->applyColor(0x3250ff);
    volcano_->applyColor(0x1e3cdc);
    meteorite_->applyColor(0x1ec8);
    gravity_->applyColor(0x505050);
}

void ScenPageWeather::store(Scen::Core &core) {
    // FUN_00433de0
    core.climate = climate_->sval();
    core.startSeason = season_->sval();
    core.yearSpeed = time_->sval();
    core.rain = rain_->sval();
    core.lightning = thunder_->sval();
    core.wind = wind_->sval();
    core.earthquake = earthquake_->sval();
    core.volcano = volcano_->sval();
    core.meteorite = meteorite_->sval();
    core.landscape.gravity = gravity_->sval();
}

void ScenPageWeather::paintEvent(QPaintEvent *e) {
    // ScenPageWeather::OnPaint: icons of bitmap 1012
    ScenPage::paintEvent(e);
    QPainter p(this);
    const QPixmap strip = maskedStrip(1012);
    const std::pair<int, int> icons[] = {{2001, 0}, {2020, 1}, {2022, 2}, {2019, 3}, {2021, 4},
                                         {2024, 5}, {2017, 6}, {2023, 7}, {2018, 8}};
    for (const auto &i : icons)
        drawIcon(p, strip, 32, i.second, i.first);
}
