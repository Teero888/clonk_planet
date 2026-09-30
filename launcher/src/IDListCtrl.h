#pragma once

// Custom controls of the scenario property pages (Planet.exe 0x402d30.. and 0x419040..0x41a660):
//
//  ScenSlider    - value slider with a random range bar (C4SVal: thumb = Std, bar = Rnd, right mouse
//                  button drags the range), colored per page; also used as the thin scroll bar
//                  below the object lists (class at 0x402d30, vtable 0x466cb8)
//  IDListBox     - owner drawn multi column list box showing a C4IDList: definition picture with
//                  "1x" count below (class at 0x419450, vtable 0x468010)
//  IDListButtons - the "+", "-" and object selection buttons next to a list (bitmap 1014,
//                  class at 0x4190b0, vtable 0x467eb8)

#include "ScenData.h"

#include <QColor>
#include <QWidget>
#include <vector>

class DefinitionDB;
class IDListBox;

// Windows XP (Luna) system colors of the screenshots
namespace SysColor {
inline QColor btnFace() { return QColor(236, 233, 216); }
inline QColor btnShadow() { return QColor(172, 168, 153); }
inline QColor dkShadow() { return QColor(113, 111, 100); }
inline QColor light() { return QColor(241, 239, 226); }
inline QColor highlight() { return QColor(49, 106, 197); }
// COLORREF 0x00bbggrr
inline QColor fromRef(uint32_t c) { return QColor(c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff); }
} // namespace SysColor

class QPainter;
// DrawFocusRect: every other pixel of the border inverted
void drawFocusRect(QPainter &p, const QRect &r);
// DrawEdge(EDGE_SUNKEN / EDGE_RAISED, BF_RECT)
void drawSunkenEdge(QPainter &p, const QRect &r);
void drawRaisedEdge(QPainter &p, const QRect &r);
// FUN_0042b450: linear mapping with rounding (FUN_0042b4a0)
int mapRange(int v, int a0, int a1, int b0, int b1);

class ScenSlider : public QWidget {
    Q_OBJECT
public:
    // Replaces the placeholder of a template control (keeps its position, height 2 * thumb half)
    explicit ScenSlider(QWidget *placeholder);

    // FUN_00403550 / FUN_00403570: C4SVal in / out (Min/Max are the value range)
    void setSVal(const Scen::SVal &v);
    Scen::SVal sval() const;

    void setValue(int v); // FUN_00402eb0
    int value() const;    // FUN_00402e30
    void setRnd(int r);   // FUN_00402f50
    int rnd() const;      // FUN_00402e70
    void setRange(int mn, int mx) { min_ = mn; max_ = mx; }

    // FUN_004035b0: apply value/rnd, color != 0 sets both thumb colors and a white background
    void applyColor(uint32_t color);
    void setRndDrag(bool on) { rnd_drag_ = on; } // FUN_00403600
    // FUN_00403690: scroll bar of an object list
    void attachList(IDListBox *list);

signals:
    void changed(); // WM_COMMAND code 7

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *) override { update(); }
    void focusOutEvent(QFocusEvent *) override { update(); }
    void resizeEvent(QResizeEvent *) override;

private:
    void track(Qt::MouseButtons buttons, int x); // FUN_00403170
    void notify();

    int value_ = 0, rnd_value_ = 0;
    int min_ = 0, max_ = 100, rnd_min_ = 0, rnd_max_ = 100;
    int half_ = 5;  // thumb half width
    int thumb_ = 5; // thumb center (pixels)
    int span_ = 0;  // random range (pixels on each side)
    bool sound_ = true, rnd_drag_ = true;
    uint32_t color_, color_focus_, bg_ = 0xffffff;
    IDListBox *list_ = nullptr;
};

class IDListButtons;

class IDListBox : public QWidget {
    Q_OBJECT
public:
    explicit IDListBox(QWidget *placeholder);

    // ListInitWithCaption_4199d0
    void init(uint32_t category, int caption_id, const DefinitionDB *defs, IDListButtons *buttons, ScenSlider *scroll);
    void setShowCounts(bool on) { show_counts_ = on; update(); } // FUN_00419a70
    void setScroll(ScenSlider *scroll);                          // FUN_0041a530

    void setList(const Scen::IDList &list); // FUN_00419970
    const Scen::IDList &list() const { return list_; }

    // key actions of FUN_004198a0 (the buttons and the context menu send them as keys)
    enum Action { Plus, Minus, Insert, Delete, MoveLeft, MoveRight };
    void action(Action a);
    void showContextMenu(const QPoint &global); // FUN_00419ff0

    void updateScroll(); // FUN_0041a5f0
    void scrolled();     // slider notification (0x41a4a0)

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *) override { update(); }  // FUN_00419e90
    void focusOutEvent(QFocusEvent *) override { update(); } // FUN_00419ec0
    void contextMenuEvent(QContextMenuEvent *e) override;
    void showEvent(QShowEvent *) override { updateScroll(); }
    void hideEvent(QHideEvent *) override { updateScroll(); }

private:
    void refill();                 // FUN_00419820
    int visibleCount() const;      // FUN_0041a5b0
    int itemCount() const { return static_cast<int>(sel_.size()); }
    QRect clientRect() const { return rect().adjusted(2, 2, -2, -2); }
    int itemAt(const QPoint &p) const;
    void setCaret(int index, bool ensure_visible = true);
    void selChanged();             // LBN_SELCHANGE (0x41a550)
    void increase();               // FUN_00419b20
    void decrease();               // FUN_00419c20
    void insert();                 // FUN_00419d40
    void removeSelected();         // FUN_00419ef0
    void moveSelected(int dir);    // FUN_0041a260 / FUN_0041a3c0
    std::vector<int> selection() const;

    Scen::IDList list_;
    const DefinitionDB *defs_ = nullptr;
    uint32_t category_ = 0;
    QString caption_;
    bool show_counts_ = true;
    ScenSlider *scroll_ = nullptr;
    int top_ = 0;
    int caret_ = 0;
    int anchor_ = 0;
    std::vector<bool> sel_;
    static constexpr int ColumnWidth = 32, ItemHeight = 48;
};

class IDListButtons : public QWidget {
    Q_OBJECT
public:
    explicit IDListButtons(QWidget *placeholder);
    void setList(IDListBox *list) { list_ = list; } // FUN_004192b0

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override; // 0x4191d0

private:
    IDListBox *list_ = nullptr;
};
