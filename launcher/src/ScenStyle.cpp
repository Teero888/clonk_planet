#include "ScenStyle.h"
#include "IDListCtrl.h"

#include <QApplication>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPainter>
#include <QRadioButton>
#include <QStyleFactory>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>

namespace {

// glyph size and text offset of BS_CHECKBOX / BS_RADIOBUTTON (text starts 17 pixels right of the
// control, measured on the screenshots)
constexpr int Glyph = 13, TextOffset = 17;

void drawCheckGlyph(QPainter *p, const QRect &r, bool on, bool enabled) {
    const int x = r.left(), y = r.top();
    p->save();
    p->fillRect(x + 2, y + 2, 9, 9, enabled ? QColor(Qt::white) : SysColor::btnFace());
    p->setPen(SysColor::btnShadow());
    p->drawLine(x, y, x + 11, y);
    p->drawLine(x, y, x, y + 11);
    p->setPen(SysColor::dkShadow());
    p->drawLine(x + 1, y + 1, x + 10, y + 1);
    p->drawLine(x + 1, y + 1, x + 1, y + 10);
    p->setPen(SysColor::light());
    p->drawLine(x + 1, y + 11, x + 11, y + 11);
    p->drawLine(x + 11, y + 1, x + 11, y + 11);
    p->setPen(Qt::white);
    p->drawLine(x, y + 12, x + 12, y + 12);
    p->drawLine(x + 12, y, x + 12, y + 12);
    if (on) {
        static const int check[][2] = {{7, 1}, {6, 2}, {7, 2}, {1, 3}, {5, 3}, {6, 3}, {7, 3}, {1, 4}, {2, 4}, {4, 4},
                                       {5, 4}, {6, 4}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}, {2, 6}, {3, 6}, {4, 6}, {3, 7}};
        p->setPen(enabled ? QColor(Qt::black) : SysColor::btnShadow());
        for (const auto &c : check)
            p->drawPoint(x + 2 + c[0], y + 2 + c[1]);
    }
    p->restore();
}

void drawRadioGlyph(QPainter *p, const QRect &r, bool on, bool enabled) {
    // 12x12 classic radio circle
    static const char *rows[12] = {
        "....ssss....", "..ssddddss..", ".sddwwwwddh.", ".sdwwwwwwlh.", "sdwwwwwwwwlh", "sdwwwwwwwwlh",
        "sdwwwwwwwwlh", "sdwwwwwwwwlh", ".sdwwwwwwlh.", ".sllwwwwllh.", "..hhllllhh..", "....hhhh....",
    };
    static const char *dot[4] = {".XX.", "XXXX", "XXXX", ".XX."};
    const QColor inner = enabled ? QColor(Qt::white) : SysColor::btnFace();
    const int x = r.left(), y = r.top();
    p->save();
    for (int j = 0; j < 12; ++j)
        for (int i = 0; i < 12; ++i) {
            QColor c;
            switch (rows[j][i]) {
            case 's': c = SysColor::btnShadow(); break;
            case 'd': c = SysColor::dkShadow(); break;
            case 'l': c = SysColor::light(); break;
            case 'h': c = Qt::white; break;
            case 'w': c = inner; break;
            default: continue;
            }
            p->setPen(c);
            p->drawPoint(x + i, y + j);
        }
    if (on) {
        p->setPen(enabled ? QColor(Qt::black) : SysColor::btnShadow());
        for (int j = 0; j < 4; ++j)
            for (int i = 0; i < 4; ++i)
                if (dot[j][i] == 'X')
                    p->drawPoint(x + 4 + i, y + 4 + j);
    }
    p->restore();
}

} // namespace

ScenClassicStyle::ScenClassicStyle() : QProxyStyle(QStyleFactory::create("Fusion")) {}

ScenClassicStyle *ScenClassicStyle::instance() {
    static ScenClassicStyle *s = new ScenClassicStyle;
    return s;
}

void ScenClassicStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const {
    const bool enabled = opt->state & State_Enabled;
    if (pe == PE_IndicatorCheckBox) {
        drawCheckGlyph(p, opt->rect, opt->state & State_On, enabled);
        return;
    }
    if (pe == PE_IndicatorRadioButton) {
        drawRadioGlyph(p, opt->rect, opt->state & State_On, enabled);
        return;
    }
    if (pe == PE_FrameFocusRect) {
        // DrawFocusRect around the text; XP hides the focus cues until the keyboard is used
        if (!(opt->state & State_KeyboardFocusChange))
            return;
        drawFocusRect(*p, opt->rect);
        return;
    }
    QProxyStyle::drawPrimitive(pe, opt, p, w);
}

void ScenClassicStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const {
    // the whole button is laid out here: a style sheet style in front of this style computes the
    // sub element rectangles itself
    const auto *b = qstyleoption_cast<const QStyleOptionButton *>(opt);
    if (ce == CE_ComboBoxLabel) {
        // owner drawn combo box: the item's icon at the top left of the field, text after it
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            const QRect f = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, w);
            p->save();
            if (!cb->currentIcon.isNull())
                p->drawPixmap(f.topLeft(), cb->currentIcon.pixmap(cb->iconSize));
            if (!cb->currentText.isEmpty()) {
                p->setPen(Qt::black);
                p->drawText(f.adjusted(cb->currentIcon.isNull() ? 2 : cb->iconSize.width() + 4, 0, 0, 0),
                            Qt::AlignLeft | Qt::AlignVCenter, cb->currentText);
            }
            p->restore();
            return;
        }
    }
    if (b && (ce == CE_CheckBox || ce == CE_RadioButton)) {
        const bool check = ce == CE_CheckBox;
        const QRect r = b->rect;
        const bool enabled = b->state & State_Enabled;
        const int gy = r.top() + (r.height() - Glyph) / 2;
        if (check)
            drawCheckGlyph(p, QRect(r.left(), gy, Glyph, Glyph), b->state & State_On, enabled);
        else
            drawRadioGlyph(p, QRect(r.left() + 1, gy, 12, 12), b->state & State_On, enabled);
        const QRect text(r.left() + TextOffset + 1, r.top(), r.width() - TextOffset - 1, r.height());
        const int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic;
        p->save();
        p->setFont(w ? w->font() : p->font());
        if (!enabled) {
            // disabled text etched
            p->setPen(Qt::white);
            p->drawText(text.translated(1, 1), flags, b->text);
            p->setPen(SysColor::btnShadow());
        } else {
            p->setPen(Qt::black);
        }
        p->drawText(text, flags, b->text);
        p->restore();
        if ((b->state & State_HasFocus) && (b->state & State_KeyboardFocusChange)) {
            const QFontMetrics fm(w ? w->font() : p->font());
            drawFocusRect(*p, QRect(text.left() - 1, text.top() + (text.height() - fm.height()) / 2,
                                    fm.horizontalAdvance(b->text) + 3, fm.height()));
        }
        return;
    }
    QProxyStyle::drawControl(ce, opt, p, w);
}

int ScenClassicStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *w) const {
    switch (m) {
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return Glyph;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
        return TextOffset - Glyph;
    default:
        return QProxyStyle::pixelMetric(m, opt, w);
    }
}

QRect ScenClassicStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const {
    const QRect r = opt->rect;
    switch (se) {
    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator: {
        // glyph at the left, vertically centered (rounded up like the screenshots)
        // (the radio circle is 12x12 in the right part of the 13x13 cell)
        if (se == SE_CheckBoxIndicator)
            return QRect(r.left(), r.top() + (r.height() - Glyph) / 2, Glyph, Glyph);
        return QRect(r.left() + 1, r.top() + (r.height() - Glyph) / 2, 12, 12);
    }
    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
        return QRect(r.left() + TextOffset + 1, r.top(), r.width() - TextOffset - 1, r.height());
    case SE_CheckBoxFocusRect:
    case SE_RadioButtonFocusRect: {
        const auto *b = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (b && w) {
            const QFontMetrics fm(w->font());
            const QRect text = fm.boundingRect(b->text);
            const QRect c = subElementRect(se == SE_CheckBoxFocusRect ? SE_CheckBoxContents : SE_RadioButtonContents, opt, w);
            return QRect(c.left() - 1, c.top() + (c.height() - fm.height()) / 2, text.width() + 3, fm.height());
        }
        return QProxyStyle::subElementRect(se, opt, w);
    }
    default:
        return QProxyStyle::subElementRect(se, opt, w);
    }
}

void applyClassicButtons(QWidget *root) {
    for (QCheckBox *c : root->findChildren<QCheckBox *>())
        c->setStyle(ScenClassicStyle::instance());
    for (QRadioButton *r : root->findChildren<QRadioButton *>())
        r->setStyle(ScenClassicStyle::instance());
    for (QComboBox *c : root->findChildren<QComboBox *>())
        c->setStyle(ScenClassicStyle::instance());
    for (QAbstractSpinBox *s : root->findChildren<QAbstractSpinBox *>())
        s->setStyle(ScenClassicStyle::instance());
}

namespace {

// raised classic push button face (DrawFrameControl DFC_SCROLL / DFC_BUTTON)
void drawRaisedButton(QPainter *p, const QRect &r, bool sunken) {
    p->fillRect(r, SysColor::btnFace());
    const int l = r.left(), t = r.top(), rr = r.right(), b = r.bottom();
    if (sunken) {
        p->setPen(SysColor::btnShadow());
        p->drawRect(r.adjusted(0, 0, -1, -1));
        return;
    }
    p->setPen(SysColor::light());
    p->drawLine(l, t, rr - 1, t);
    p->drawLine(l, t, l, b - 1);
    p->setPen(Qt::white);
    p->drawLine(l + 1, t + 1, rr - 2, t + 1);
    p->drawLine(l + 1, t + 1, l + 1, b - 2);
    p->setPen(SysColor::btnShadow());
    p->drawLine(l + 1, b - 1, rr - 1, b - 1);
    p->drawLine(rr - 1, t + 1, rr - 1, b - 1);
    p->setPen(SysColor::dkShadow());
    p->drawLine(l, b, rr, b);
    p->drawLine(rr, t, rr, b);
}

// triangle arrow: rows of 2*n-1 .. 1 pixels (down) or 1 .. 2*n-1 (up)
void drawArrow(QPainter *p, int cx, int top, int rows, bool down, bool enabled) {
    auto draw = [&](const QColor &c, int dx, int dy) {
        p->setPen(c);
        for (int i = 0; i < rows; ++i) {
            const int half = down ? rows - 1 - i : i;
            p->drawLine(cx - half + dx, top + i + dy, cx + half + dx, top + i + dy);
        }
    };
    if (!enabled) {
        draw(Qt::white, 1, 1);
        draw(SysColor::btnShadow(), 0, 0);
    } else {
        draw(Qt::black, 0, 0);
    }
}

} // namespace

QRect ScenClassicStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const {
    const QRect r = opt->rect;
    if (cc == CC_ComboBox) {
        // drop button: 17 wide inside the 2 pixel client edge
        const QRect button(r.right() - 2 - 16, r.top() + 2, 17, r.height() - 4);
        switch (sc) {
        case SC_ComboBoxArrow: return button;
        case SC_ComboBoxEditField: return QRect(r.left() + 2, r.top() + 2, button.left() - r.left() - 2, r.height() - 4);
        case SC_ComboBoxFrame:
        case SC_ComboBoxListBoxPopup: return r;
        default: break;
        }
    }
    if (cc == CC_SpinBox) {
        // UDS_ALIGNRIGHT up-down: 16 wide inside the edit's client edge, two halves
        const int bh = (r.height() - 4) / 2;
        const QRect up(r.right() - 2 - 15, r.top() + 2, 16, bh);
        const QRect down(up.left(), up.bottom() + 1, 16, r.height() - 4 - bh);
        switch (sc) {
        case SC_SpinBoxUp: return up;
        case SC_SpinBoxDown: return down;
        case SC_SpinBoxEditField: return QRect(r.left() + 2, r.top() + 2, up.left() - r.left() - 2, r.height() - 4);
        case SC_SpinBoxFrame: return r;
        default: break;
        }
    }
    return QProxyStyle::subControlRect(cc, opt, sc, w);
}

QStyle::SubControl ScenClassicStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos,
                                                           const QWidget *w) const {
    if (cc == CC_SpinBox) {
        if (subControlRect(cc, opt, SC_SpinBoxUp, w).contains(pos))
            return SC_SpinBoxUp;
        if (subControlRect(cc, opt, SC_SpinBoxDown, w).contains(pos))
            return SC_SpinBoxDown;
        return SC_SpinBoxEditField;
    }
    if (cc == CC_ComboBox)
        return subControlRect(cc, opt, SC_ComboBoxArrow, w).contains(pos) ? SC_ComboBoxArrow : SC_ComboBoxEditField;
    return QProxyStyle::hitTestComplexControl(cc, opt, pos, w);
}

void ScenClassicStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w) const {
    const bool enabled = opt->state & State_Enabled;
    if (cc == CC_ComboBox || cc == CC_SpinBox) {
        p->save();
        const QRect r = opt->rect;
        p->fillRect(r.adjusted(2, 2, -2, -2), enabled ? QColor(Qt::white) : SysColor::btnFace());
        drawSunkenEdge(*p, r);
        if (cc == CC_ComboBox) {
            const QRect b = subControlRect(cc, opt, SC_ComboBoxArrow, w);
            drawRaisedButton(p, b, (opt->activeSubControls & SC_ComboBoxArrow) && (opt->state & State_Sunken));
            drawArrow(p, b.left() + b.width() / 2, b.top() + (b.height() - 4) / 2, 4, true, enabled);
        } else {
            for (SubControl sc : {SC_SpinBoxUp, SC_SpinBoxDown}) {
                const QRect b = subControlRect(cc, opt, sc, w);
                drawRaisedButton(p, b, (opt->activeSubControls & sc) && (opt->state & State_Sunken));
                drawArrow(p, b.left() + b.width() / 2, b.top() + (b.height() - 2) / 2, 2, sc == SC_SpinBoxDown, enabled);
            }
        }
        p->restore();
        return;
    }
    QProxyStyle::drawComplexControl(cc, opt, p, w);
}
