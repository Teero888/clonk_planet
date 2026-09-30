#include "IDListCtrl.h"
#include "LauncherRes.h"
#include "MapPreview.h"
#include "ScenPages.h"

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>

namespace {
enum LandscapeIds {
    L_WIDTH_LABEL = 2374, L_WIDTH = 2255, L_HEIGHT_LABEL = 2304, L_HEIGHT = 2231, L_ZOOM_LABEL = 2379,
    L_ZOOM = 2257, L_ZOOM_EDIT = 2158, L_AMPLITUDE_LABEL = 2265, L_AMPLITUDE = 2222, L_PHASE_LABEL = 2328,
    L_PHASE = 2242, L_PERIOD_LABEL = 2327, L_PERIOD = 2241, L_RANDOM_LABEL = 2337, L_RANDOM = 2244,
    L_WATER_LABEL = 2373, L_WATER = 2254, L_OPEN_TOP = 2093, L_OPEN_BOTTOM = 2092, L_PLAYER_EXTEND = 2088,
    L_MAP_TYPE = 2318, L_PREVIEW = 2107, L_SHOW = 2349, L_SHOW_SCREEN = 2099, L_SHOW_LAYERS = 2097,
    L_PREVIEW_FOR = 2330, L_PLAYERS1 = 2210,
};
}

ScenPageLandscape::ScenPageLandscape(ScenContext *ctx, QWidget *parent)
    : ScenPage(LauncherRes::IDD_SCEN_LANDSCAPE, ctx, parent) {
    width_ = slider(L_WIDTH);
    height_ = slider(L_HEIGHT);
    zoom_ = slider(L_ZOOM);
    amplitude_ = slider(L_AMPLITUDE);
    phase_ = slider(L_PHASE);
    period_ = slider(L_PERIOD);
    random_ = slider(L_RANDOM);
    water_ = slider(L_WATER);
    // read only edit with ES_CENTER
    if (auto *e = get<QLineEdit>(L_ZOOM_EDIT)) {
        e->setAlignment(Qt::AlignCenter);
        e->setReadOnly(true);
        QPalette pal = e->palette();
        pal.setColor(QPalette::Base, SysColor::btnFace());
        e->setPalette(pal);
    }
    preview_ = new MapPreview(control(L_PREVIEW));
    replaced_[L_PREVIEW] = preview_;
    // FUN_00421590 ctor: screen size and layers shown, preview for 1 player
    get<QAbstractButton>(L_SHOW_SCREEN)->setChecked(true);
    get<QAbstractButton>(L_SHOW_LAYERS)->setChecked(true);
    get<QAbstractButton>(L_PLAYERS1)->setChecked(true);

    // message map: every slider (code 7) redraws the map and shows the zoom factor
    for (ScenSlider *s : {width_, height_, zoom_, amplitude_, phase_, period_, random_, water_})
        connect(s, &ScenSlider::changed, this, [this]() {
            // ScenPageLandscape::On22550x7
            redraw();
            updateZoomText();
        });
    // ScenPageLandscape::OnMapPlayerExtendClicked
    connect(get<QAbstractButton>(L_PLAYER_EXTEND), &QAbstractButton::clicked, this, [this]() {
        redraw();
        updateEnable();
    });
    // ScenPageLandscape::OnOpenBottomClicked (the other check boxes and radios)
    for (int id : std::initializer_list<int>{L_OPEN_BOTTOM, L_OPEN_TOP, L_SHOW_LAYERS, L_SHOW_SCREEN, L_PLAYERS1, L_PLAYERS1 + 1, L_PLAYERS1 + 2,
                   L_PLAYERS1 + 3})
        connect(get<QAbstractButton>(id), &QAbstractButton::clicked, this, [this]() { redraw(); });
}

void ScenPageLandscape::load(const Scen::Core &core) {
    // FUN_00421960 / FUN_00421bc0
    l_ = core.landscape;
    width_->setSVal(l_.mapWdt);
    height_->setSVal(l_.mapHgt);
    zoom_->setSVal(l_.mapZoom);
    amplitude_->setSVal(l_.amplitude);
    phase_->setSVal(l_.phase);
    period_->setSVal(l_.period);
    random_->setSVal(l_.random);
    water_->setSVal(l_.liquidLevel);
    get<QLineEdit>(L_ZOOM_EDIT)->setText(QString::number(l_.mapZoom.evaluate()));
    get<QAbstractButton>(L_PLAYER_EXTEND)->setChecked(l_.mapPlayerExtend && !preview_->hasStaticMap());
    get<QAbstractButton>(L_OPEN_BOTTOM)->setChecked(l_.bottomOpen);
    get<QAbstractButton>(L_OPEN_TOP)->setChecked(l_.topOpen);

    // OnInitDialog
    width_->applyColor(0xff);
    height_->applyColor(0xd7);
    zoom_->applyColor(0xc3);
    amplitude_->applyColor(0xd700);
    period_->applyColor(0xaf00);
    phase_->applyColor(0xc300);
    random_->applyColor(0x9b00);
    water_->applyColor(0xff0000);
    height_->setRndDrag(false);
    zoom_->setRndDrag(false);
    width_->setRndDrag(false);
    preview_->load(ctx_->scenario);
    preview_->setOptions(get<QAbstractButton>(L_SHOW_SCREEN)->isChecked(), get<QAbstractButton>(L_SHOW_LAYERS)->isChecked(),
                         players_ + 1);
    preview_->render(l_);
    map_type_ = 0;
    if (preview_->hasStaticMap())
        map_type_ = 1;
    if (l_.exactLandscape)
        map_type_ = 2;
    loaded_ = true;
    updateEnable();
    // FUN_004219c0
    setText(L_WIDTH_LABEL, 50370);
    setText(L_WATER_LABEL, 50367);
    setText(L_SHOW, 50317);
    setText(L_RANDOM_LABEL, 50284);
    setText(L_PREVIEW_FOR, 50276);
    setText(L_PHASE_LABEL, 50262);
    setText(L_PERIOD_LABEL, 50261);
    setText(L_ZOOM_LABEL, 50375);
    setText(L_HEIGHT_LABEL, 50198);
    setText(L_AMPLITUDE_LABEL, 50104);
    for (int p = 0; p < 4; ++p)
        setText(L_PLAYERS1 + p, 50271 + p);
    setText(L_SHOW_SCREEN, 50320);
    setText(L_SHOW_LAYERS, 50318);
    setText(L_OPEN_TOP, 50258);
    setText(L_OPEN_BOTTOM, 50256);
    setText(L_PLAYER_EXTEND, 50225);
    QString type = LauncherRes::str(map_type_ == 0 ? 50163 : map_type_ == 1 ? 50335 : 50176);
    if (preview_->localMaterials())
        type += LauncherRes::str(50218);
    type += ":";
    get<QLabel>(L_MAP_TYPE)->setText(type);
}

void ScenPageLandscape::storeControls() {
    // FUN_00421cc0 (UpdateData(TRUE) and the sliders)
    l_.mapWdt = width_->sval();
    l_.mapHgt = height_->sval();
    l_.mapZoom = zoom_->sval();
    l_.amplitude = amplitude_->sval();
    l_.phase = phase_->sval();
    l_.period = period_->sval();
    l_.random = random_->sval();
    l_.liquidLevel = water_->sval();
    l_.mapPlayerExtend = get<QAbstractButton>(L_PLAYER_EXTEND)->isChecked() ? 1 : 0;
    l_.topOpen = get<QAbstractButton>(L_OPEN_TOP)->isChecked() ? 1 : 0;
    l_.bottomOpen = get<QAbstractButton>(L_OPEN_BOTTOM)->isChecked() ? 1 : 0;
    players_ = 0;
    for (int p = 0; p < 4; ++p)
        if (get<QAbstractButton>(L_PLAYERS1 + p)->isChecked())
            players_ = p;
}

void ScenPageLandscape::redraw() {
    // FUN_00421dc0
    storeControls();
    preview_->setOptions(get<QAbstractButton>(L_SHOW_SCREEN)->isChecked(), get<QAbstractButton>(L_SHOW_LAYERS)->isChecked(),
                         players_ + 1);
    preview_->render(l_);
}

void ScenPageLandscape::updateEnable() {
    // FUN_00421e00
    const bool extend = get<QAbstractButton>(L_PLAYER_EXTEND)->isChecked();
    enable(L_PREVIEW_FOR, extend);
    for (int p = 0; p < 4; ++p)
        enable(L_PLAYERS1 + p, extend);
    const bool dynamic = map_type_ == 0;
    enable(L_WIDTH, dynamic);
    enable(L_HEIGHT, dynamic);
    enable(L_ZOOM, map_type_ != 2);
    enable(L_AMPLITUDE, dynamic);
    enable(L_PHASE, dynamic);
    enable(L_PERIOD, dynamic);
    enable(L_RANDOM, dynamic);
    enable(L_WATER, dynamic);
    enable(L_SHOW_LAYERS, dynamic);
    enable(L_PLAYER_EXTEND, dynamic);
}

void ScenPageLandscape::updateZoomText() {
    // FUN_00421f80
    get<QLineEdit>(L_ZOOM_EDIT)->setText(QString::number(zoom_->value()));
}

void ScenPageLandscape::store(Scen::Core &core) {
    // FUN_00421990
    if (loaded_)
        storeControls();
    core.landscape.mapWdt = l_.mapWdt;
    core.landscape.mapHgt = l_.mapHgt;
    core.landscape.mapZoom = l_.mapZoom;
    core.landscape.amplitude = l_.amplitude;
    core.landscape.phase = l_.phase;
    core.landscape.period = l_.period;
    core.landscape.random = l_.random;
    core.landscape.liquidLevel = l_.liquidLevel;
    core.landscape.mapPlayerExtend = l_.mapPlayerExtend;
    core.landscape.topOpen = l_.topOpen;
    core.landscape.bottomOpen = l_.bottomOpen;
}
