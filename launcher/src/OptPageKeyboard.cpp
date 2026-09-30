// Options page Keyboard (PageKeyboard, IDD 3016) and the hotkey control replacement

#include "LauncherRes.h"
#include "OptPages.h"

#include <QBitmap>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QPainter>
#include <QRadioButton>

// ============================================================================ OptHotKeyEdit

namespace {
enum : int {
    VK_BACK = 0x08, VK_TAB = 0x09, VK_CLEAR = 0x0C, VK_RETURN = 0x0D, VK_SHIFT = 0x10, VK_CONTROL = 0x11,
    VK_MENU = 0x12, VK_PAUSE = 0x13, VK_CAPITAL = 0x14, VK_ESCAPE = 0x1B, VK_SPACE = 0x20, VK_PRIOR = 0x21,
    VK_NEXT = 0x22, VK_END = 0x23, VK_HOME = 0x24, VK_LEFT = 0x25, VK_UP = 0x26, VK_RIGHT = 0x27,
    VK_DOWN = 0x28, VK_SNAPSHOT = 0x2C, VK_INSERT = 0x2D, VK_DELETE = 0x2E, VK_NUMPAD0 = 0x60,
    VK_MULTIPLY = 0x6A, VK_ADD = 0x6B, VK_SUBTRACT = 0x6D, VK_DECIMAL = 0x6E, VK_DIVIDE = 0x6F,
    VK_F1 = 0x70, VK_NUMLOCK = 0x90, VK_SCROLL = 0x91, VK_OEM_1 = 0xBA, VK_OEM_PLUS = 0xBB,
    VK_OEM_COMMA = 0xBC, VK_OEM_MINUS = 0xBD, VK_OEM_PERIOD = 0xBE, VK_OEM_2 = 0xBF, VK_OEM_3 = 0xC0,
    VK_OEM_4 = 0xDB, VK_OEM_5 = 0xDC, VK_OEM_6 = 0xDD, VK_OEM_7 = 0xDE, VK_OEM_102 = 0xE2,
};
} // namespace

OptHotKeyEdit::OptHotKeyEdit(QWidget *parent) : QLineEdit(parent) {
    setContextMenuPolicy(Qt::NoContextMenu);
    setAttribute(Qt::WA_InputMethodEnabled, false);
}

void OptHotKeyEdit::setHotKey(int vk) {
    vk_ = vk;
    setText(keyName(vk));
}

QString OptHotKeyEdit::keyName(int vk, bool ext) {
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z'))
        return QString(QChar(vk));
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD0 + 9)
        return QString("Num %1").arg(vk - VK_NUMPAD0);
    if (vk >= VK_F1 && vk < VK_F1 + 24)
        return QString("F%1").arg(vk - VK_F1 + 1);
    switch (vk) {
    case VK_BACK: return "Backspace";
    case VK_TAB: return "Tab";
    case VK_CLEAR: return "Num 5";
    case VK_RETURN: return ext ? "Num Enter" : "Enter";
    case VK_SHIFT: return "Shift";
    case VK_CONTROL: return ext ? "Right Ctrl" : "Ctrl";
    case VK_MENU: return ext ? "Right Alt" : "Alt";
    case VK_PAUSE: return "Pause";
    case VK_CAPITAL: return "Caps Lock";
    case VK_ESCAPE: return "Esc";
    case VK_SPACE: return "Space";
    // navigation keys without the extended flag are the keys of the numeric keypad
    case VK_PRIOR: return ext ? "Page Up" : "Num 9";
    case VK_NEXT: return ext ? "Page Down" : "Num 3";
    case VK_END: return ext ? "End" : "Num 1";
    case VK_HOME: return ext ? "Home" : "Num 7";
    case VK_LEFT: return ext ? "Left" : "Num 4";
    case VK_UP: return ext ? "Up" : "Num 8";
    case VK_RIGHT: return ext ? "Right" : "Num 6";
    case VK_DOWN: return ext ? "Down" : "Num 2";
    case VK_SNAPSHOT: return "Prnt Scrn";
    case VK_INSERT: return ext ? "Insert" : "Num 0";
    case VK_DELETE: return ext ? "Delete" : "Num Del";
    case VK_MULTIPLY: return "Num *";
    case VK_ADD: return "Num +";
    case VK_SUBTRACT: return "Num -";
    case VK_DECIMAL: return "Num Del";
    case VK_DIVIDE: return ext ? "Num /" : "/";
    case VK_NUMLOCK: return ext ? "Num Lock" : "Pause";
    case VK_SCROLL: return "Scroll Lock";
    case VK_OEM_1: return ";";
    case VK_OEM_PLUS: return "=";
    case VK_OEM_COMMA: return ",";
    case VK_OEM_MINUS: return "-";
    case VK_OEM_PERIOD: return ".";
    case VK_OEM_2: return "/";
    case VK_OEM_3: return "`";
    case VK_OEM_4: return "[";
    case VK_OEM_5: return "\\";
    case VK_OEM_6: return "]";
    case VK_OEM_7: return "'";
    case VK_OEM_102: return "\\";
    default: return {};
    }
}

int OptHotKeyEdit::virtualKey(const QKeyEvent *e, bool *extended) {
    const int k = e->key();
    const bool keypad = e->modifiers() & Qt::KeypadModifier;
    const bool shift = e->modifiers() & Qt::ShiftModifier;
    const bool key102 = e->nativeScanCode() == 94 || (e->nativeScanCode() == 0 && !shift);
    bool ext = false;
    int vk = 0;
    if (k >= Qt::Key_A && k <= Qt::Key_Z)
        vk = 'A' + (k - Qt::Key_A);
    else if (k >= Qt::Key_0 && k <= Qt::Key_9)
        vk = keypad ? VK_NUMPAD0 + (k - Qt::Key_0) : '0' + (k - Qt::Key_0);
    else if (k >= Qt::Key_F1 && k <= Qt::Key_F24)
        vk = VK_F1 + (k - Qt::Key_F1);
    else {
        switch (k) {
        case Qt::Key_Backspace: vk = VK_BACK; break;
        case Qt::Key_Tab: case Qt::Key_Backtab: vk = VK_TAB; break;
        case Qt::Key_Clear: vk = VK_CLEAR; break;
        case Qt::Key_Return: vk = VK_RETURN; break;
        case Qt::Key_Enter: vk = VK_RETURN; ext = true; break;
        case Qt::Key_Shift: vk = VK_SHIFT; break;
        case Qt::Key_Control: vk = VK_CONTROL; break;
        case Qt::Key_Alt: vk = VK_MENU; break;
        case Qt::Key_AltGr: vk = VK_MENU; ext = true; break;
        case Qt::Key_Pause: vk = VK_PAUSE; break;
        case Qt::Key_CapsLock: vk = VK_CAPITAL; break;
        case Qt::Key_Escape: vk = VK_ESCAPE; break;
        case Qt::Key_Space: vk = VK_SPACE; break;
        case Qt::Key_PageUp: vk = VK_PRIOR; ext = !keypad; break;
        case Qt::Key_PageDown: vk = VK_NEXT; ext = !keypad; break;
        case Qt::Key_End: vk = VK_END; ext = !keypad; break;
        case Qt::Key_Home: vk = VK_HOME; ext = !keypad; break;
        case Qt::Key_Left: vk = VK_LEFT; ext = !keypad; break;
        case Qt::Key_Up: vk = VK_UP; ext = !keypad; break;
        case Qt::Key_Right: vk = VK_RIGHT; ext = !keypad; break;
        case Qt::Key_Down: vk = VK_DOWN; ext = !keypad; break;
        case Qt::Key_Print: vk = VK_SNAPSHOT; ext = true; break;
        case Qt::Key_Insert: vk = VK_INSERT; ext = !keypad; break;
        case Qt::Key_Delete: vk = VK_DELETE; ext = !keypad; break;
        case Qt::Key_NumLock: vk = VK_NUMLOCK; ext = true; break;
        case Qt::Key_ScrollLock: vk = VK_SCROLL; break;
        case Qt::Key_Asterisk: vk = keypad ? int(VK_MULTIPLY) : int('8'); break;
        case Qt::Key_Plus: vk = keypad ? VK_ADD : VK_OEM_PLUS; break;
        case Qt::Key_Minus: vk = keypad ? VK_SUBTRACT : VK_OEM_MINUS; break;
        case Qt::Key_Slash: vk = keypad ? VK_DIVIDE : VK_OEM_2; ext = keypad; break;
        case Qt::Key_Period: vk = keypad ? VK_DECIMAL : VK_OEM_PERIOD; break;
        case Qt::Key_Comma: vk = keypad ? VK_DECIMAL : VK_OEM_COMMA; break;
        case Qt::Key_Semicolon: case Qt::Key_Colon: vk = VK_OEM_1; break;
        case Qt::Key_Equal: vk = VK_OEM_PLUS; break;
        case Qt::Key_Underscore: vk = VK_OEM_MINUS; break;
        case Qt::Key_Question: vk = VK_OEM_2; break;
        case Qt::Key_QuoteLeft: case Qt::Key_AsciiTilde: vk = VK_OEM_3; break;
        case Qt::Key_BracketLeft: case Qt::Key_BraceLeft: vk = VK_OEM_4; break;
        case Qt::Key_Backslash: vk = VK_OEM_5; break;
        case Qt::Key_BracketRight: case Qt::Key_BraceRight: vk = VK_OEM_6; break;
        case Qt::Key_Apostrophe: case Qt::Key_QuoteDbl: vk = VK_OEM_7; break;
        // '<' '>' '|': the 102nd key (X11 key code 94) or shifted keys of the US layout
        case Qt::Key_Less: vk = key102 ? VK_OEM_102 : VK_OEM_COMMA; break;
        case Qt::Key_Greater: vk = key102 ? VK_OEM_102 : VK_OEM_PERIOD; break;
        case Qt::Key_Bar: vk = key102 ? VK_OEM_102 : VK_OEM_5; break;
        case Qt::Key_Exclam: vk = '1'; break;
        case Qt::Key_At: vk = '2'; break;
        case Qt::Key_NumberSign: vk = shift ? int('3') : int(VK_OEM_2); break;
        case Qt::Key_Dollar: vk = '4'; break;
        case Qt::Key_Percent: vk = '5'; break;
        case Qt::Key_AsciiCircum: case Qt::Key_Dead_Circumflex: vk = shift ? int('6') : int(VK_OEM_5); break;
        case Qt::Key_Ampersand: vk = '7'; break;
        case Qt::Key_ParenLeft: vk = '9'; break;
        case Qt::Key_ParenRight: vk = '0'; break;
        case Qt::Key_Dead_Acute: case Qt::Key_Dead_Grave: vk = VK_OEM_6; break;
        case Qt::Key_ssharp: vk = VK_OEM_4; break;
        case Qt::Key_Udiaeresis: vk = VK_OEM_1; break;
        case Qt::Key_Odiaeresis: vk = VK_OEM_3; break;
        case Qt::Key_Adiaeresis: vk = VK_OEM_7; break;
        default: break;
        }
    }
    if (extended)
        *extended = ext;
    return vk;
}

void OptHotKeyEdit::keyPressEvent(QKeyEvent *event) {
    bool ext = false;
    const int vk = virtualKey(event, &ext);
    switch (vk) {
    case VK_TAB: // dialog navigation
        QLineEdit::keyPressEvent(event);
        return;
    case VK_SHIFT: case VK_CONTROL: case VK_MENU: // modifiers are not allowed (HKM_SETRULES 0xfe)
    case VK_RETURN: case VK_SPACE: case VK_ESCAPE:
        event->ignore(); // not taken by the hotkey control (default button, cancel, ...)
        return;
    case VK_BACK: case VK_DELETE:
        if (!ext || vk == VK_BACK) { // the hotkey control clears on backspace
            vk_ = 0;
            setText(QString());
            return;
        }
        break;
    case 0:
        return;
    default:
        break;
    }
    vk_ = vk;
    setText(keyName(vk, ext));
}

void OptHotKeyEdit::keyReleaseEvent(QKeyEvent *) {}

void OptHotKeyEdit::mousePressEvent(QMouseEvent *) {
    setFocus(Qt::MouseFocusReason);
    setCursorPosition(text().size());
}

void OptHotKeyEdit::contextMenuEvent(QContextMenuEvent *) {}

// ============================================================================ PageKeyboard

namespace {
// per key index (Config.Keyboard[block][i]): hotkey, block 4 edit, icon static, label (id, text)
const int KBD_HOTKEY[12] = {2112, 2116, 2117, 2118, 2119, 2120, 2121, 2122, 2123, 2113, 2114, 2115};
const int KBD_FIXED[12] = {2131, 2135, 2136, 2137, 2138, 2139, 2140, 2141, 2142, 2132, 2133, 2134};
const int KBD_ICON[12] = {2002, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2003, 2004, 2005};
const int KBD_LABEL[12][2] = {{2278, 50307}, {2282, 50311}, {2283, 50310}, {2284, 50340}, {2285, 50350},
                              {2286, 50161}, {2287, 50214}, {2288, 50162}, {2289, 50293}, {2279, 50269},
                              {2280, 50330}, {2281, 50331}};
const int KBD_BLOCK_RADIO[4] = {2035, 2036, 2037, 2038};

// C4Config defaults of the "Controls" section (engine/src/C4Config.cpp); block 4 is fixed
const int KBD_DEFAULT[4][12] = {
    {'Q', 'W', 'E', 'A', 'S', 'D', 'Y', 'X', 'C', 226, 'V', 'F'},
    {103, 104, 105, 100, 101, 102, 97, 98, 99, 96, 110, 107},
    {'I', 'O', 'P', 'K', 'L', 192, 188, 190, 189, 'M', 222, 186},
    {VK_INSERT, VK_HOME, VK_PRIOR, VK_DELETE, VK_UP, VK_NEXT, VK_LEFT, VK_DOWN, VK_RIGHT, VK_END, VK_RETURN, VK_BACK},
};

std::string kbdKey(int block, int i) { return "Controls\\Kbd" + std::to_string(block) + "Key" + std::to_string(i + 1); }

// FUN_0041beb0: names of the fixed block 4 keys
QString fixedKeyName(int vk) {
    switch (vk) {
    case VK_BACK: return LauncherRes::str(50900);
    case VK_RETURN: return LauncherRes::str(50909);
    case VK_PRIOR: return LauncherRes::str(50908);
    case VK_NEXT: return LauncherRes::str(50907);
    case VK_END: return LauncherRes::str(50903);
    case VK_HOME: return LauncherRes::str(50904);
    case VK_LEFT: return LauncherRes::str(50906);
    case VK_UP: return LauncherRes::str(50911);
    case VK_RIGHT: return LauncherRes::str(50910);
    case VK_DOWN: return LauncherRes::str(50902);
    case VK_INSERT: return LauncherRes::str(50905);
    case VK_DELETE: return LauncherRes::str(50901);
    default: return {};
    }
}

// the image list of the page (bitmap 1008, 32 px, mask color 0xff00ff)
QPixmap controlIcons() {
    static QPixmap pix;
    if (pix.isNull()) {
        pix = LauncherRes::bitmap(1008);
        if (!pix.isNull())
            pix.setMask(pix.createMaskFromColor(QColor(255, 0, 255)));
    }
    return pix;
}
} // namespace

OptKeyboardPage::OptKeyboardPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_KEYBOARD, parent) {
    // PageKeyboard::ctor: the page works on the config values
    for (int b = 0; b < 4; ++b)
        for (int i = 0; i < 12; ++i)
            keys_[b][i] = b < 3 ? cfgInt(kbdKey(b + 1, i), KBD_DEFAULT[b][i]) : KBD_DEFAULT[b][i];

    // FUN_0041bfe0: texts
    for (const auto &l : KBD_LABEL)
        if (auto *w = get<QLabel>(l[0]))
            w->setText(LauncherRes::str(l[1]));
    for (int i = 0; i < 12; ++i)
        if (auto *e = get<QLineEdit>(KBD_FIXED[i])) {
            e->setText(fixedKeyName(keys_[3][i]));
            setReadOnlyEdit(e);
        }

    // the hotkey controls
    for (int i = 0; i < 12; ++i) {
        auto *hk = new OptHotKeyEdit(this);
        replaceControl(KBD_HOTKEY[i], hk);
        hotkeys_[i] = hk;
    }
    // the icons are painted by the page (PageKeyboard::OnPaint)
    for (int id : KBD_ICON)
        if (QWidget *w = control(id))
            w->setAttribute(Qt::WA_TransparentForMouseEvents);

    for (int b = 0; b < 4; ++b) {
        auto *radio = get<QRadioButton>(KBD_BLOCK_RADIO[b]);
        connect(radio, &QRadioButton::clicked, this, [this, b]() { onBlockClicked(b + 1); });
    }
    connect(get<QPushButton>(2060), &QPushButton::clicked, this, [this]() { onResetClicked(); });

    // PageKeyboard::OnInitDialog: check BLOCK1, OnBlock1Clicked (without storing the controls)
    loading_ = true;
    get<QRadioButton>(KBD_BLOCK_RADIO[0])->setChecked(true);
    onBlockClicked(1);
}

void OptKeyboardPage::selectBlock(int block) {
    if (block < 1 || block > 4)
        return;
    get<QRadioButton>(KBD_BLOCK_RADIO[block - 1])->setChecked(true);
    onBlockClicked(block);
}

void OptKeyboardPage::storeBlock() {
    for (int i = 0; i < 12; ++i)
        keys_[block_ - 1][i] = hotkeys_[i]->hotKey();
}

void OptKeyboardPage::showBlock() {
    // FUN_0041cb40: block 4 shows the fixed keys instead of the hotkey controls
    const bool fixed = block_ == 4;
    for (int i = 0; i < 12; ++i) {
        control(KBD_FIXED[i])->setVisible(fixed);
        hotkeys_[i]->setVisible(!fixed);
        // HKM_SETHOTKEY with the low byte only
        hotkeys_[i]->setHotKey(keys_[block_ - 1][i] & 0xff);
    }
    update();
}

// PageKeyboard::OnBlock1Clicked (BLOCK1..BLOCK4)
void OptKeyboardPage::onBlockClicked(int block) {
    if (!loading_)
        storeBlock();
    loading_ = false;
    block_ = block;
    showBlock();
}

// PageKeyboard::OnResetClicked
void OptKeyboardPage::onResetClicked() {
    if (QMessageBox::warning(this, LauncherRes::str(50507), LauncherRes::str(51131), // "Reset all controls?"
                             QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    // C4Config::ResetControls
    for (int b = 0; b < 4; ++b)
        for (int i = 0; i < 12; ++i)
            keys_[b][i] = KBD_DEFAULT[b][i];
    loading_ = true;
    selectBlock(1);
}

// PageKeyboard::OnOK
void OptKeyboardPage::onOK() { onBlockClicked(1); }

void OptKeyboardPage::apply() {
    // only the three configurable blocks are stored in the registry
    for (int b = 0; b < 3; ++b)
        for (int i = 0; i < 12; ++i)
            setCfgInt(kbdKey(b + 1, i), keys_[b][i]);
}

// PageKeyboard::OnPaint: ImageList_Draw(icon i) at the top left of the icon statics
void OptKeyboardPage::paintEvent(QPaintEvent *event) {
    OptPage::paintEvent(event);
    const QPixmap icons = controlIcons();
    if (icons.isNull())
        return;
    QPainter p(this);
    for (int i = 0; i < 12; ++i)
        if (QWidget *w = control(KBD_ICON[i]))
            p.drawPixmap(w->pos(), icons, QRect(i * 32, 0, 32, 32));
}
