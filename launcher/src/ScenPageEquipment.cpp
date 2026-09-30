#include "ClonkDialog.h"
#include "IDListCtrl.h"
#include "LauncherRes.h"
#include "ScenPages.h"

#include <QAbstractButton>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>

namespace {
enum EquipIds {
    Q_VALUES_FOR = 2176, Q_PLAYER1 = 2171, Q_ALL = 2175, Q_WEALTH_LABEL = 2185, Q_WEALTH = 2183, Q_EXTEND = 2050,
    Q_CREW_LABEL = 2277, Q_CREW = 2193, Q_CREW_BUTTONS = 2063, Q_CREW_SCROLL = 2226,
    Q_BASE_LABEL = 2161, Q_BASE = 2160, Q_BASE_BUTTONS = 2159, Q_BASE_SCROLL = 2224,
    Q_MATERIAL_LABEL = 2170, Q_MATERIAL = 2169, Q_MATERIAL_BUTTONS = 2168, Q_MATERIAL_SCROLL = 2238,
    Q_VEHIC_LABEL = 2182, Q_VEHIC = 2181, Q_VEHIC_BUTTONS = 2180, Q_VEHIC_SCROLL = 2252,
    Q_KNOWLEDGE_LABEL = 2164, Q_KNOWLEDGE = 2163, Q_KNOWLEDGE_BUTTONS = 2162, Q_KNOWLEDGE_SCROLL = 2236,
    Q_MAGIC_LABEL = 2317, Q_MAGIC = 2199, Q_MAGIC_BUTTONS = 2067, Q_MAGIC_SCROLL = 2237,
    Q_HBMAT_LABEL = 2167, Q_HBMAT = 2166, Q_HBMAT_BUTTONS = 2165, Q_HBMAT_SCROLL = 2232,
    Q_HBPROD_LABEL = 2179, Q_HBPROD = 2178, Q_HBPROD_BUTTONS = 2177, Q_HBPROD_SCROLL = 2233,
};

const int basic_ids[] = {Q_CREW_LABEL, Q_CREW, Q_CREW_BUTTONS, Q_BASE_LABEL, Q_BASE, Q_BASE_BUTTONS,
                         Q_MATERIAL_LABEL, Q_MATERIAL, Q_MATERIAL_BUTTONS, Q_VEHIC_LABEL, Q_VEHIC, Q_VEHIC_BUTTONS};
const int extended_ids[] = {Q_KNOWLEDGE_LABEL, Q_KNOWLEDGE, Q_KNOWLEDGE_BUTTONS, Q_MAGIC_LABEL, Q_MAGIC, Q_MAGIC_BUTTONS,
                            Q_HBMAT_LABEL, Q_HBMAT, Q_HBMAT_BUTTONS, Q_HBPROD_LABEL, Q_HBPROD, Q_HBPROD_BUTTONS};
} // namespace

ScenPageEquipment::ScenPageEquipment(ScenContext *ctx, QWidget *parent)
    : ScenPage(LauncherRes::IDD_SCEN_EQUIPMENT, ctx, parent) {
    // ScenPageEquipment::OnInitDialog
    // FUN_00422fe0
    setText(Q_EXTEND, 50018);
    setText(Q_CREW_LABEL, 50152);
    setText(Q_MAGIC_LABEL, 50224);
    setText(Q_VEHIC_LABEL, 50358);
    setText(Q_BASE_LABEL, 50124);
    setText(Q_VALUES_FOR, 50354);
    setText(Q_WEALTH_LABEL, 50368);
    setText(Q_ALL, 50103);
    for (int p = 0; p < 4; ++p)
        setText(Q_PLAYER1 + p, 50264 + p);
    setText(Q_MATERIAL_LABEL, 50227);
    setText(Q_KNOWLEDGE_LABEL, 50212);
    setText(Q_HBPROD_LABEL, 50278);
    setText(Q_HBMAT_LABEL, 50200);

    wealth_ = get<QSpinBox>(Q_WEALTH);
    // UDS_ALIGNRIGHT: the up-down sits inside the edit
    wealth_->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_SCEN_EQUIPMENT, Q_WEALTH));
    // EM_SETMARGINS 3, 3
    if (QLineEdit *e = wealth_->findChild<QLineEdit *>())
        e->setTextMargins(3, 0, 3, 0);

    crew_ = list(Q_CREW, Q_CREW_BUTTONS, Q_CREW_SCROLL, C4D::CrewMember, 51702);
    base_ = list(Q_BASE, Q_BASE_BUTTONS, Q_BASE_SCROLL, C4D::SelectBuilding, 51713);
    vehic_ = list(Q_VEHIC, Q_VEHIC_BUTTONS, Q_VEHIC_SCROLL, C4D::SelectVehicle, 51715);
    material_ = list(Q_MATERIAL, Q_MATERIAL_BUTTONS, Q_MATERIAL_SCROLL, C4D::SelectMaterial, 51709);
    knowledge_ = list(Q_KNOWLEDGE, Q_KNOWLEDGE_BUTTONS, Q_KNOWLEDGE_SCROLL, C4D::SelectKnowledge, 51707);
    hb_material_ = list(Q_HBMAT, Q_HBMAT_BUTTONS, Q_HBMAT_SCROLL, C4D::SelectHomebase, 51705);
    hb_production_ = list(Q_HBPROD, Q_HBPROD_BUTTONS, Q_HBPROD_SCROLL, C4D::SelectHomebase, 51711);
    magic_ = list(Q_MAGIC, Q_MAGIC_BUTTONS, Q_MAGIC_SCROLL, C4D::Magic, 51708);
    magic_->setShowCounts(false);
    knowledge_->setShowCounts(false);

    // ScenPageEquipment::OnPlayer1Clicked (radio range 2171..2175)
    for (int p = 0; p <= 4; ++p) {
        auto *radio = get<QAbstractButton>(Q_PLAYER1 + p);
        radio->setAutoExclusive(false);
        connect(radio, &QAbstractButton::clicked, this, [this, p]() {
            if (get<QAbstractButton>(Q_PLAYER1 + p)->isChecked() || p == current_)
                selectPlayer(p);
            checkPlayerRadio(current_);
        });
    }
    // ScenPageEquipment::OnExtendClicked
    connect(get<QAbstractButton>(Q_EXTEND), &QAbstractButton::clicked, this, [this]() {
        setExtended(!extended_);
    });
    // ScenPageEquipment::On2183EN_KILLFOCUS: the value is bound to the wealth range (the spin box)
}

void ScenPageEquipment::load(const Scen::Core &core) {
    // FUN_00422e70
    players_ = core.plrStart;
    for (Scen::PlrStart &p : players_) {
        // obsolete Clonks / StandardCrew become the crew list
        if (p.readyCrew.isClear())
            p.readyCrew.setIDCount(p.nativeCrew.isEmpty() ? QString("CLNK") : p.nativeCrew, p.crew.evaluate(), true);
        // no magic list: all magic is available
        if (p.magic.isClear())
            p.magic.load(ctx_->defs, C4D::Magic);
    }
    current_ = 0;
    if (players_[0].equipmentEqual(players_[1]) && players_[1].equipmentEqual(players_[2]) &&
        players_[2].equipmentEqual(players_[3]))
        current_ = 4;
    no_init_ = core.noInitialize != 0;

    // OnInitDialog
    wealth_->setRange(players_[0].wealth.min, players_[0].wealth.max); // UDM_SETRANGE
    loadPlayer(current_);
    checkPlayerRadio(current_);
    setExtended(extended_);
    if (no_init_) {
        for (int id : {Q_BASE_BUTTONS, Q_BASE_LABEL, Q_BASE, Q_BASE_SCROLL, Q_MATERIAL_BUTTONS, Q_MATERIAL_LABEL,
                       Q_MATERIAL, Q_MATERIAL_SCROLL, Q_VEHIC_BUTTONS, Q_VEHIC_LABEL, Q_VEHIC, Q_VEHIC_SCROLL})
            enable(id, false);
    }
}

void ScenPageEquipment::store(Scen::Core &core) {
    // FUN_00422f90
    savePlayer(current_);
    core.plrStart = players_;
}

void ScenPageEquipment::checkPlayerRadio(int player) {
    // FUN_00422d00: CheckRadioButton(2171, 2175, 2171 + player)
    for (int p = 0; p <= 4; ++p)
        get<QAbstractButton>(Q_PLAYER1 + p)->setChecked(p == player);
}

void ScenPageEquipment::loadPlayer(int player) {
    // FUN_004230f0
    if (player == 4)
        player = 0;
    const Scen::PlrStart &p = players_[player];
    wealth_->setValue(p.wealth.std);
    crew_->setList(p.readyCrew);
    magic_->setList(p.magic);
    base_->setList(p.readyBase);
    vehic_->setList(p.readyVehic);
    material_->setList(p.readyMaterial);
    knowledge_->setList(p.buildKnowledge);
    hb_material_->setList(p.homeBaseMaterial);
    hb_production_->setList(p.homeBaseProduction);
}

void ScenPageEquipment::savePlayer(int player) {
    // FUN_004231c0: "All" writes the values to every player
    wealth_->interpretText();
    const int first = player == 4 ? 0 : player;
    const int last = player == 4 ? 3 : player;
    for (int i = first; i <= last; ++i) {
        Scen::PlrStart &p = players_[i];
        p.wealth.std = wealth_->value();
        p.readyCrew = crew_->list();
        p.magic = magic_->list();
        p.readyBase = base_->list();
        p.readyVehic = vehic_->list();
        p.readyMaterial = material_->list();
        p.buildKnowledge = knowledge_->list();
        p.homeBaseMaterial = hb_material_->list();
        p.homeBaseProduction = hb_production_->list();
    }
}

bool ScenPageEquipment::selectPlayer(int player) {
    // 0x422d20
    if (player == current_)
        return false;
    if (player == 4 && !clonkMessage(window(), LauncherRes::str(51100), MsgButtons::OKCancel)) {
        checkPlayerRadio(current_);
        return false;
    }
    savePlayer(current_);
    if (player == 4)
        for (int i = 0; i < 4; ++i)
            if (i != current_)
                players_[i] = players_[current_];
    current_ = player;
    loadPlayer(player);
    checkPlayerRadio(player);
    return true;
}

void ScenPageEquipment::setExtended(bool on) {
    // ScenPageEquipment::OnExtendClicked / FUN_00423440
    extended_ = on;
    setText(Q_EXTEND, on ? 50032 : 50018);
    for (int id : basic_ids)
        show(id, !on);
    for (int id : extended_ids)
        show(id, on);
    for (IDListBox *l : {crew_, base_, vehic_, material_, knowledge_, hb_material_, hb_production_, magic_})
        l->updateScroll();
    update();
}
