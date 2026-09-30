#pragma once

// Application style: Fusion with Windows XP (Luna) scrollbars as the original looked on XP.
// The scrollbar parts are pixels taken from a screenshot of the original (data/res/xp/*.png).

#include <QPixmap>
#include <QProxyStyle>

class XPStyle : public QProxyStyle {
public:
    XPStyle();

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *widget = nullptr) const override;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget = nullptr) const override;
    // WS_EX_CLIENTEDGE of edits and list / tree views: classic 2 pixel sunken edge in the XP colors
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *widget = nullptr) const override;

private:
    struct Parts {
        QPixmap arrow_up, arrow_down, thumb_top, thumb_mid, thumb_bottom, grip, track;
    };
    Parts v_, h_;
    bool loaded_ = false;
};
