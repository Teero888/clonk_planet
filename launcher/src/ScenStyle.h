#pragma once

// Classic (non themed) Windows controls as Planet.exe shows them on XP (the launcher has no visual
// styles manifest): 13x13 sunken check box with a black check mark, 12x12 sunken radio circle,
// disabled text etched, sunken combo boxes / up-down edits with raised arrow buttons.

#include <QProxyStyle>

class QWidget;

class ScenClassicStyle : public QProxyStyle {
public:
    ScenClassicStyle();
    static ScenClassicStyle *instance();

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr) const override;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr) const override;
    int pixelMetric(PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *w = nullptr) const override;
    QRect subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w = nullptr) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w = nullptr) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w = nullptr) const override;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pos, const QWidget *w = nullptr) const override;
};

// applies the classic style to all check boxes, radio buttons, combo boxes and spin boxes below root
void applyClassicButtons(QWidget *root);
