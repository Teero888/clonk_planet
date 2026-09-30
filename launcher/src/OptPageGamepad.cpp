// Options page Game Pad (PageGamepad, IDD 3014)

#include "LauncherRes.h"
#include "OptJoystick.h"
#include "OptPages.h"

#include <QBitmap>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTimer>

namespace {
// per control index (Config.Gamepad.Button[i]): icon static, label (id, text), assign button
const int PAD_ICON[12] = {2002, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2003, 2006, 2007};
const int PAD_LABEL[12][2] = {{2278, 50307}, {2282, 50311}, {2283, 50310}, {2284, 50340}, {2285, 50350},
                              {2286, 50161}, {2287, 50214}, {2288, 50162}, {2289, 50293}, {2279, 50269},
                              {2280, 50330}, {2281, 50331}};
// PageGamepad::OnAssignClicked..OnAssignClicked_8: button -> control index
const int PAD_ASSIGN[][2] = {{2026, 0}, {2030, 1}, {2031, 2}, {2032, 3}, {2033, 5}, {2027, 9}, {2028, 10}, {2029, 11}};
// the directions are always on the pad (up, left, down, right)
const int PAD_FIXED[] = {2147, 2148, 2149, 2150};
constexpr int IDC_PROMPT = 2333;
constexpr int IDC_RESET_CALIBRATION = 2059;
// icon index of the directions (OnUserMsg3)
constexpr int ICON_UP = 4, ICON_LEFT = 6, ICON_DOWN = 7, ICON_RIGHT = 8;

QPixmap controlIcons() {
    static QPixmap pix;
    if (pix.isNull()) {
        pix = LauncherRes::bitmap(1008);
        if (!pix.isNull())
            pix.setMask(pix.createMaskFromColor(QColor(255, 0, 255)));
    }
    return pix;
}

std::string buttonKey(int i) { return "Gamepad\\Button" + std::to_string(i + 1); }
} // namespace

OptGamepadPage::OptGamepadPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_GAMEPAD, parent) {
    joystick_ = new OptJoystick();
    // FUN_00417320 (ctor): calibration and button assignment from the config
    OptJoystick::setCalibration(cfgInt("Gamepad\\MinX", 0), cfgInt("Gamepad\\MaxX", 0), cfgInt("Gamepad\\MinY", 0),
                                cfgInt("Gamepad\\MaxY", 0));
    for (int i = 0; i < 12; ++i)
        buttons_[i] = cfgInt(buttonKey(i), -1);

    // FUN_004173e0: texts
    get<QLabel>(IDC_PROMPT)->setText(LauncherRes::str(50127));
    get<QPushButton>(IDC_RESET_CALIBRATION)->setText(LauncherRes::str(50005));
    for (const auto &a : PAD_ASSIGN)
        get<QPushButton>(a[0])->setText(LauncherRes::str(50003));
    for (const auto &l : PAD_LABEL)
        get<QLabel>(l[0])->setText(LauncherRes::str(l[1]));
    for (int id : PAD_FIXED) {
        auto *e = get<QLineEdit>(id);
        e->setText(LauncherRes::str(50260)); // "Joy pad"
        e->setAlignment(Qt::AlignHCenter);    // ES_CENTER | ES_READONLY
        setReadOnlyEdit(e);
    }
    for (int id : PAD_ICON)
        control(id)->setAttribute(Qt::WA_TransparentForMouseEvents);

    for (const auto &a : PAD_ASSIGN) {
        const int button_id = a[0], ctrl = a[1];
        connect(get<QPushButton>(button_id), &QPushButton::clicked, this, [this, ctrl, button_id]() { assign(ctrl, button_id); });
    }
    // PageGamepad::OnResetcalibrationClicked
    connect(get<QPushButton>(IDC_RESET_CALIBRATION), &QPushButton::clicked, this,
            []() { OptJoystick::setCalibration(0, 0, 0, 0); });

    // FUN_00417980: a thread polls the game pad every 300 ms and posts changes (WM_USER + 3)
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, [this]() { poll(); });
    timer_->start(300);
}

OptGamepadPage::~OptGamepadPage() { delete joystick_; }

void OptGamepadPage::poll() {
    unsigned pos = 0, buttons = 0;
    if (!joystick_->getGamepad(pos, buttons))
        return;
    if (pos != last_pos_ || buttons != last_buttons_) {
        last_pos_ = pos;
        last_buttons_ = buttons;
        onJoystick(pos, buttons);
    }
}

// PageGamepad::OnUserMsg3
void OptGamepadPage::onJoystick(unsigned pos, unsigned buttons) {
    for (int b = 0; b < 32; ++b)
        for (int i = 0; i < 12; ++i) // func 0x417bb0
            if (buttons_[i] - 1 == b)
                setState(i, (buttons >> b) & 1);
    setState(ICON_UP, pos & OptJoystick::PAD_Up);
    setState(ICON_LEFT, pos & OptJoystick::PAD_Left);
    setState(ICON_DOWN, pos & OptJoystick::PAD_Down);
    setState(ICON_RIGHT, pos & OptJoystick::PAD_Right);
}

// func 0x417a90
void OptGamepadPage::setState(int ctrl, bool on) {
    if (state_[ctrl] == static_cast<int>(on))
        return;
    state_[ctrl] = on;
    if (QWidget *w = control(PAD_ICON[ctrl]))
        update(QRect(w->pos(), QSize(32, 32)));
}

// PageGamepad::OnAssignClicked (FUN_00417bf0): wait for a game pad button
void OptGamepadPage::assign(int ctrl, int button_id) {
    if (assigning_ >= 0)
        return;
    unsigned pos = 0, buttons = 0;
    if (!joystick_->getGamepad(pos, buttons))
        return; // no game pad: nothing happens
    assigning_ = ctrl;
    assign_button_ = button_id;
    get<QLabel>(IDC_PROMPT)->setText(LauncherRes::str(50279)); // "Please press game pad button."
    get<QPushButton>(button_id)->setDown(true);                 // BM_SETSTATE
    auto *wait = new QTimer(this);
    connect(wait, &QTimer::timeout, this, [this, wait]() {
        unsigned p = 0, b = 0;
        const bool ok = joystick_->getGamepad(p, b);
        if (ok && b == 0)
            return;
        if (ok)
            buttons_[assigning_] = OptJoystick::firstSetBit(b) + 1;
        get<QPushButton>(assign_button_)->setDown(false);
        get<QLabel>(IDC_PROMPT)->setText(LauncherRes::str(50127));
        assigning_ = -1;
        wait->deleteLater();
    });
    wait->start(20);
}

// FUN_00417380
void OptGamepadPage::apply() {
    int min_x, max_x, min_y, max_y;
    OptJoystick::getCalibration(min_x, max_x, min_y, max_y);
    setCfgInt("Gamepad\\MinX", min_x);
    setCfgInt("Gamepad\\MaxX", max_x);
    setCfgInt("Gamepad\\MinY", min_y);
    setCfgInt("Gamepad\\MaxY", max_y);
    for (int i = 0; i < 12; ++i)
        setCfgInt(buttonKey(i), buttons_[i]);
}

// PageGamepad::OnPaint: icon i (+12 when active) at the top left of the icon statics
void OptGamepadPage::paintEvent(QPaintEvent *event) {
    OptPage::paintEvent(event);
    const QPixmap icons = controlIcons();
    if (icons.isNull())
        return;
    QPainter p(this);
    for (int i = 0; i < 12; ++i)
        if (QWidget *w = control(PAD_ICON[i]))
            p.drawPixmap(w->pos(), icons, QRect((state_[i] * 12 + i) * 32, 0, 32, 32));
}
