#include "XPStyle.h"
#include "LauncherRes.h"

#include <QDir>
#include <QPainter>
#include <QStyleFactory>
#include <QStyleOptionSlider>
#include <QStyleOptionFrame>

namespace {
constexpr int EXTENT = 17; // SM_CXVSCROLL
}

XPStyle::XPStyle() : QProxyStyle(QStyleFactory::create("Fusion")) {}

int XPStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const {
    switch (metric) {
    case PM_ScrollBarExtent:
        return EXTENT;
    case PM_ScrollBarSliderMin:
        return 8;
    case PM_DefaultFrameWidth:
        return 2;
    case PM_MenuBarHMargin:
        return 6;
    case PM_MenuBarPanelWidth:
    case PM_MenuBarItemSpacing:
        return 0;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QRect XPStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget) const {
    if (cc != CC_ScrollBar)
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt);
    if (!sb)
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    const bool horz = sb->orientation == Qt::Horizontal;
    const QRect r = sb->rect;
    const int len = horz ? r.width() : r.height();
    // buttons are square, shrinking when there is no room
    const int btn = qMin(EXTENT, len / 2);
    const int groove_start = btn;
    const int groove_len = qMax(0, len - 2 * btn);
    int slider_len = groove_len;
    int slider_pos = groove_start;
    const int range = sb->maximum - sb->minimum;
    if (range > 0) {
        slider_len = qMax(8, static_cast<int>(static_cast<qint64>(groove_len) * sb->pageStep / (range + sb->pageStep)));
        slider_len = qMin(slider_len, groove_len);
        slider_pos = groove_start + sliderPositionFromValue(sb->minimum, sb->maximum, sb->sliderPosition,
                                                            groove_len - slider_len, sb->upsideDown);
    }
    auto make = [&](int pos, int size) {
        return horz ? QRect(r.x() + pos, r.y(), size, r.height()) : QRect(r.x(), r.y() + pos, r.width(), size);
    };
    switch (sc) {
    case SC_ScrollBarSubLine:
        return make(0, btn);
    case SC_ScrollBarAddLine:
        return make(len - btn, btn);
    case SC_ScrollBarGroove:
        return make(groove_start, groove_len);
    case SC_ScrollBarSlider:
        return range > 0 ? make(slider_pos, slider_len) : QRect();
    case SC_ScrollBarSubPage:
        return make(groove_start, slider_pos - groove_start);
    case SC_ScrollBarAddPage:
        return make(slider_pos + slider_len, groove_start + groove_len - slider_pos - slider_len);
    default:
        return QProxyStyle::subControlRect(cc, opt, sc, widget);
    }
}

QStyle::SubControl XPStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos, const QWidget *widget) const {
    if (cc != CC_ScrollBar)
        return QProxyStyle::hitTestComplexControl(cc, opt, pos, widget);
    for (SubControl sc : {SC_ScrollBarSlider, SC_ScrollBarSubLine, SC_ScrollBarAddLine, SC_ScrollBarSubPage, SC_ScrollBarAddPage})
        if (subControlRect(cc, opt, sc, widget).contains(pos))
            return sc;
    return SC_None;
}

void XPStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget) const {
    if (cc != CC_ScrollBar) {
        QProxyStyle::drawComplexControl(cc, opt, p, widget);
        return;
    }
    if (!loaded_) {
        auto *self = const_cast<XPStyle *>(this);
        const QDir dir(QFileInfo(LauncherRes::resPath("bitmap", 1006)).absolutePath() + "/../xp");
        for (auto [prefix, parts] : {std::pair<const char *, Parts *>{"v_", &self->v_}, {"h_", &self->h_}}) {
            parts->arrow_up = QPixmap(dir.filePath(QString(prefix) + "arrow_up.png"));
            parts->arrow_down = QPixmap(dir.filePath(QString(prefix) + "arrow_down.png"));
            parts->thumb_top = QPixmap(dir.filePath(QString(prefix) + "thumb_top.png"));
            parts->thumb_mid = QPixmap(dir.filePath(QString(prefix) + "thumb_mid.png"));
            parts->thumb_bottom = QPixmap(dir.filePath(QString(prefix) + "thumb_bottom.png"));
            parts->grip = QPixmap(dir.filePath(QString(prefix) + "grip.png"));
            parts->track = QPixmap(dir.filePath(QString(prefix) + "track.png"));
        }
        self->loaded_ = true;
    }
    const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt);
    const Parts &P = (sb && sb->orientation == Qt::Horizontal) ? h_ : v_;
    if (!sb || P.track.isNull()) {
        QProxyStyle::drawComplexControl(cc, opt, p, widget);
        return;
    }
    const bool horz = sb->orientation == Qt::Horizontal;
    const QRect groove = subControlRect(cc, opt, SC_ScrollBarGroove, widget);
    const QRect sub = subControlRect(cc, opt, SC_ScrollBarSubLine, widget);
    const QRect add = subControlRect(cc, opt, SC_ScrollBarAddLine, widget);
    const QRect slider = subControlRect(cc, opt, SC_ScrollBarSlider, widget);

    // track: one row / column stretched
    p->drawPixmap(groove, P.track);
    // buttons (pressed: darker)
    auto button = [&](const QRect &r, const QPixmap &pix, SubControl sc) {
        p->drawPixmap(r, pix);
        if ((sb->activeSubControls & sc) && (sb->state & State_Sunken))
            p->fillRect(r.adjusted(1, 1, -1, -1), QColor(0, 0, 60, 40));
    };
    button(sub, P.arrow_up, SC_ScrollBarSubLine);
    button(add, P.arrow_down, SC_ScrollBarAddLine);
    // thumb: caps + stretched body + grip in the middle
    if (slider.isValid() && sb->maximum > sb->minimum) {
        const int cap_a = horz ? P.thumb_top.width() : P.thumb_top.height();
        const int cap_b = horz ? P.thumb_bottom.width() : P.thumb_bottom.height();
        const int len = horz ? slider.width() : slider.height();
        if (horz) {
            p->drawPixmap(QRect(slider.x(), slider.y(), cap_a, slider.height()), P.thumb_top);
            p->drawPixmap(QRect(slider.x() + cap_a, slider.y(), qMax(0, len - cap_a - cap_b), slider.height()), P.thumb_mid);
            p->drawPixmap(QRect(slider.right() - cap_b + 1, slider.y(), cap_b, slider.height()), P.thumb_bottom);
        } else {
            p->drawPixmap(QRect(slider.x(), slider.y(), slider.width(), cap_a), P.thumb_top);
            p->drawPixmap(QRect(slider.x(), slider.y() + cap_a, slider.width(), qMax(0, len - cap_a - cap_b)), P.thumb_mid);
            p->drawPixmap(QRect(slider.x(), slider.bottom() - cap_b + 1, slider.width(), cap_b), P.thumb_bottom);
        }
        const int grip_len = horz ? P.grip.width() : P.grip.height();
        if (len >= grip_len + cap_a + cap_b + 4) {
            const QPoint c = slider.center();
            if (horz)
                p->drawPixmap(c.x() - grip_len / 2, slider.y(), P.grip);
            else
                p->drawPixmap(slider.x(), c.y() - grip_len / 2, P.grip);
        }
    }
}

void XPStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *widget) const {
    auto edge = [&](const QRect &r) {
        // outer: #aca899 top/left, white bottom/right; inner: #716f64 top/left, #f1efe2 bottom/right
        const QRect o = r.adjusted(0, 0, -1, -1);
        p->setPen(QColor(0xac, 0xa8, 0x99));
        p->drawLine(o.topLeft(), o.topRight());
        p->drawLine(o.topLeft(), o.bottomLeft());
        p->setPen(Qt::white);
        p->drawLine(o.bottomLeft(), o.bottomRight());
        p->drawLine(o.topRight(), o.bottomRight());
        const QRect i = o.adjusted(1, 1, -1, -1);
        p->setPen(QColor(0x71, 0x6f, 0x64));
        p->drawLine(i.topLeft(), i.topRight());
        p->drawLine(i.topLeft(), i.bottomLeft());
        p->setPen(QColor(0xf1, 0xef, 0xe2));
        p->drawLine(i.bottomLeft(), i.bottomRight());
        p->drawLine(i.topRight(), i.bottomRight());
    };
    switch (pe) {
    case PE_PanelLineEdit:
        if (const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            p->fillRect(opt->rect.adjusted(f->lineWidth, f->lineWidth, -f->lineWidth, -f->lineWidth),
                        opt->palette.brush(QPalette::Base));
            if (f->lineWidth > 0)
                edge(opt->rect);
            return;
        }
        break;
    case PE_FrameLineEdit:
        edge(opt->rect);
        return;
    case PE_Frame:
        if (const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (f->state & State_Sunken) {
                edge(opt->rect);
                return;
            }
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawPrimitive(pe, opt, p, widget);
}
