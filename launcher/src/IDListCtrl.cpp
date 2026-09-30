#include "IDListCtrl.h"
#include "DefinitionDB.h"
#include "IDSelectDlg.h"
#include "LauncherRes.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace {

// sounds of the list (0x48fc70: 7002 insert, 0x48fc78: 7003 +/-) and the slider (7006 / 7007)
constexpr int SoundInsert = 7002, SoundCount = 7003, SoundSlide = 7006, SoundRange = 7007;

void replacePlaceholder(QWidget *self, QWidget *placeholder) {
    self->setParent(placeholder->parentWidget());
    self->setGeometry(placeholder->geometry());
    self->setFont(placeholder->font());
    self->setObjectName(placeholder->objectName());
    self->setVisible(placeholder->isVisibleTo(placeholder->parentWidget()));
    self->setEnabled(placeholder->isEnabled());
    placeholder->hide();
}

} // namespace

void drawFocusRect(QPainter &p, const QRect &r) {
    // 50% pattern brush: every other pixel of the border (checkerboard) inverted
    p.save();
    p.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    p.setPen(Qt::white);
    const int l = r.left(), t = r.top(), rr = r.right(), b = r.bottom();
    auto px = [&](int x, int y) {
        if ((x + y) & 1)
            p.drawPoint(x, y);
    };
    for (int x = l; x <= rr; ++x) {
        px(x, t);
        if (b != t)
            px(x, b);
    }
    for (int y = t + 1; y < b; ++y) {
        px(l, y);
        if (rr != l)
            px(rr, y);
    }
    p.restore();
}

void drawSunkenEdge(QPainter &p, const QRect &r) {
    // BDR_SUNKENOUTER (shadow / highlight) + BDR_SUNKENINNER (dark shadow / light)
    const int l = r.left(), t = r.top(), rr = r.right(), b = r.bottom();
    p.setPen(SysColor::btnShadow());
    p.drawLine(l, t, rr - 1, t);
    p.drawLine(l, t, l, b - 1);
    p.setPen(Qt::white);
    p.drawLine(l, b, rr, b);
    p.drawLine(rr, t, rr, b);
    p.setPen(SysColor::dkShadow());
    p.drawLine(l + 1, t + 1, rr - 2, t + 1);
    p.drawLine(l + 1, t + 1, l + 1, b - 2);
    p.setPen(SysColor::light());
    p.drawLine(l + 1, b - 1, rr - 1, b - 1);
    p.drawLine(rr - 1, t + 1, rr - 1, b - 1);
}

void drawRaisedEdge(QPainter &p, const QRect &r) {
    // BDR_RAISEDOUTER (light / dark shadow) + BDR_RAISEDINNER (highlight / shadow)
    const int l = r.left(), t = r.top(), rr = r.right(), b = r.bottom();
    p.setPen(SysColor::light());
    p.drawLine(l, t, rr - 1, t);
    p.drawLine(l, t, l, b - 1);
    p.setPen(SysColor::dkShadow());
    p.drawLine(l, b, rr, b);
    p.drawLine(rr, t, rr, b);
    p.setPen(Qt::white);
    p.drawLine(l + 1, t + 1, rr - 2, t + 1);
    p.drawLine(l + 1, t + 1, l + 1, b - 2);
    p.setPen(SysColor::btnShadow());
    p.drawLine(l + 1, b - 1, rr - 1, b - 1);
    p.drawLine(rr - 1, t + 1, rr - 1, b - 1);
}

int mapRange(int v, int a0, int a1, int b0, int b1) {
    if (a1 == a0)
        return b0;
    const double d = double(v - a0) * double(b1 - b0) / double(a1 - a0) + double(b0);
    const int t = static_cast<int>(d);
    return (d - t >= 0.5) ? t + 1 : t;
}

static int boundBy(int v, int lo, int hi) {
    // FUN_0042b4d0
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

// ======================================================================================= ScenSlider

ScenSlider::ScenSlider(QWidget *placeholder) : QWidget(nullptr) {
    // FUN_00402d30
    color_ = 0xd8e9ec; // GetSysColor(COLOR_BTNFACE)
    color_focus_ = 0xc56a31; // GetSysColor(COLOR_HIGHLIGHT)
    replacePlaceholder(this, placeholder);
    // 0x403370: the control is made 2 * thumb half width high
    setGeometry(x(), y(), width(), 2 * half_);
    setFocusPolicy(Qt::StrongFocus);
}

void ScenSlider::resizeEvent(QResizeEvent *) {
    setValue(value_);
    setRnd(rnd_value_);
}

void ScenSlider::setSVal(const Scen::SVal &v) {
    min_ = v.min;
    max_ = v.max;
    value_ = v.std;
    rnd_value_ = v.rnd;
}

Scen::SVal ScenSlider::sval() const {
    return Scen::SVal(value(), rnd(), min_, max_);
}

void ScenSlider::setValue(int v) {
    value_ = v;
    const int b = boundBy(v, min_, max_);
    thumb_ = std::max(mapRange(b, min_, max_, half_, width() - half_), half_);
    update();
}

int ScenSlider::value() const {
    return mapRange(thumb_, half_, width() - half_, min_, max_);
}

void ScenSlider::setRnd(int r) {
    rnd_value_ = r;
    const int b = boundBy(r, rnd_min_, rnd_max_);
    span_ = mapRange(b, rnd_min_, rnd_max_, 0, width());
    update();
}

int ScenSlider::rnd() const {
    return mapRange(span_, 0, width(), rnd_min_, rnd_max_);
}

void ScenSlider::applyColor(uint32_t color) {
    setValue(value_);
    setRnd(rnd_value_);
    if (color) {
        color_ = color_focus_ = color;
        bg_ = 0xffffff;
    }
    update();
}

void ScenSlider::attachList(IDListBox *list) {
    // FUN_00403690
    value_ = 0;
    rnd_value_ = 0;
    applyColor(0);
    sound_ = false;
    rnd_drag_ = false;
    color_ = color_focus_ = 0xdddddd;
    bg_ = 0xdddddd;
    list_ = list;
}

void ScenSlider::paintEvent(QPaintEvent *) {
    // FUN_00402fe0
    QPainter p(this);
    const QRect client = rect();
    const QRect thumb(thumb_ - half_, 0, 2 * half_, client.height());
    const int bar_right = std::min(thumb.left() + thumb.width() + span_, client.width() - 3);
    int bar_left = thumb.left() - span_;
    if (bar_left < 4)
        bar_left = 3;
    const QRect bar(bar_left, 3, bar_right - bar_left, client.height() - 6);
    const QColor bg = isEnabled() ? SysColor::fromRef(bg_) : SysColor::btnFace();
    const QColor fg = SysColor::fromRef(hasFocus() ? color_focus_ : color_);
    p.fillRect(client, bg);
    drawSunkenEdge(p, client);
    if (isEnabled()) {
        if (bar.width() > 0 && bar.height() > 0)
            p.fillRect(bar, fg);
        p.fillRect(thumb, fg);
        drawRaisedEdge(p, thumb);
        if (hasFocus())
            drawFocusRect(p, thumb.adjusted(1, 1, -1, -1));
    }
}

void ScenSlider::mousePressEvent(QMouseEvent *e) {
    // FUN_00403330: capture + track (Qt grabs the mouse while a button is down)
    track(e->buttons(), e->pos().x());
}

void ScenSlider::mouseMoveEvent(QMouseEvent *e) {
    track(e->buttons(), e->pos().x());
}

void ScenSlider::track(Qt::MouseButtons buttons, int x) {
    // FUN_00403170
    if (buttons & Qt::LeftButton) {
        setFocus(Qt::MouseFocusReason);
        const int old = thumb_;
        thumb_ = boundBy(x, half_, width() - half_);
        span_ = 0;
        value_ = value();
        rnd_value_ = rnd();
        update();
        notify();
        if (thumb_ / 10 != old / 10 && sound_)
            Scen::playSound(SoundSlide);
    } else if ((buttons & Qt::RightButton) && rnd_drag_) {
        setFocus(Qt::MouseFocusReason);
        if (x > thumb_ + half_)
            span_ = x - half_ - thumb_;
        else if (x < thumb_ - half_)
            span_ = thumb_ - half_ - x;
        else
            span_ = 0;
        span_ = boundBy(span_, 0, width());
        rnd_value_ = rnd();
        update();
        notify();
        if (sound_)
            Scen::playSound(SoundRange);
    }
}

void ScenSlider::keyPressEvent(QKeyEvent *e) {
    // 0x403430: left / right move the thumb by 2 pixels, up / down change the random range
    switch (e->key()) {
    case Qt::Key_Left:
        span_ = 0;
        thumb_ -= 2;
        break;
    case Qt::Key_Right:
        span_ = 0;
        thumb_ += 2;
        break;
    case Qt::Key_Up:
        if (!rnd_drag_)
            return;
        span_ += 2;
        break;
    case Qt::Key_Down:
        if (!rnd_drag_)
            return;
        span_ -= 2;
        break;
    default:
        QWidget::keyPressEvent(e);
        return;
    }
    thumb_ = boundBy(thumb_, half_, width() - half_);
    span_ = boundBy(span_, 0, width());
    value_ = value();
    rnd_value_ = rnd();
    update();
    notify();
}

void ScenSlider::notify() {
    // FUN_00403620: WM_COMMAND code 7 to the list box or the parent
    if (list_)
        list_->scrolled();
    emit changed();
}

// ======================================================================================= IDListBox

IDListBox::IDListBox(QWidget *placeholder) : QWidget(nullptr) {
    replacePlaceholder(this, placeholder);
    // 0x4197b0: height = item height + 4 (border)
    setGeometry(x(), y(), width(), ItemHeight + 4);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void IDListBox::init(uint32_t category, int caption_id, const DefinitionDB *defs, IDListButtons *buttons,
                     ScenSlider *scroll) {
    // ListInitWithCaption_4199d0
    defs_ = defs;
    category_ = category;
    caption_ = LauncherRes::str(caption_id);
    if (defs_)
        list_.consolidateValids(*defs_);
    refill();
    if (buttons)
        buttons->setList(this);
    if (scroll)
        setScroll(scroll);
}

void IDListBox::setScroll(ScenSlider *scroll) {
    scroll_ = scroll;
    scroll_->attachList(this);
    updateScroll();
}

void IDListBox::setList(const Scen::IDList &list) {
    list_ = list;
    if (defs_)
        list_.consolidateValids(*defs_);
    refill();
}

void IDListBox::refill() {
    // FUN_00419820: one item per used slot, item data = slot index
    sel_.assign(list_.numIDs(), false);
    top_ = 0;
    caret_ = 0;
    anchor_ = 0;
    update();
    updateScroll();
}

int IDListBox::visibleCount() const {
    // FUN_0041a5b0 (sic: 48 pixel columns)
    return clientRect().width() / 0x30 + 2;
}

void IDListBox::updateScroll() {
    // FUN_0041a5f0: the scroll bar is only shown if the items don't fit
    if (!scroll_)
        return;
    const bool show = isVisibleTo(parentWidget()) && itemCount() > visibleCount();
    scroll_->setVisible(show);
}

void IDListBox::scrolled() {
    // 0x41a4a0: LB_SETTOPINDEX(pos * (count - visible) / 100)
    const int range = itemCount() - visibleCount();
    if (!scroll_)
        return;
    top_ = std::clamp(scroll_->value() * range / 100, 0, std::max(0, itemCount() - 1));
    update();
}

void IDListBox::selChanged() {
    // 0x41a550: scroll bar follows the top index
    const int range = itemCount() - visibleCount();
    if (scroll_ && range != 0) {
        scroll_->setValue(top_ * 100 / range);
    }
}

int IDListBox::itemAt(const QPoint &p) const {
    const QRect c = clientRect();
    if (!c.contains(p))
        return -1;
    const int i = top_ + (p.x() - c.left()) / ColumnWidth;
    return i < itemCount() ? i : -1;
}

std::vector<int> IDListBox::selection() const {
    std::vector<int> out;
    for (int i = 0; i < itemCount(); ++i)
        if (sel_[i])
            out.push_back(i);
    return out;
}

void IDListBox::setCaret(int index, bool ensure_visible) {
    caret_ = std::clamp(index, 0, std::max(0, itemCount() - 1));
    if (ensure_visible && itemCount() > 0) {
        const int cols = std::max(1, clientRect().width() / ColumnWidth);
        if (caret_ < top_)
            top_ = caret_;
        else if (caret_ >= top_ + cols)
            top_ = caret_ - cols + 1;
    }
}

void IDListBox::paintEvent(QPaintEvent *) {
    QPainter p(this);
    const QRect c = clientRect();
    // reflected WM_CTLCOLOR (0x419790): white, light gray when disabled
    p.fillRect(rect(), isEnabled() ? QColor(Qt::white) : QColor(0xc0, 0xc0, 0xc0));
    drawSunkenEdge(p, rect());
    p.setClipRect(c);
    p.setFont(font());
    const QFontMetrics fm(font());
    for (int i = top_; i < itemCount(); ++i) {
        const QRect r(c.left() + (i - top_) * ColumnWidth, c.top(), ColumnWidth, ItemHeight);
        if (r.left() > c.right())
            break;
        // DrawItem (0x419510)
        int count = 0;
        const QString id = list_.getID(i, &count);
        const QString text = QString("%1x").arg(count);
        QColor text_color = Qt::black;
        if (sel_[i]) {
            p.fillRect(r, hasFocus() ? SysColor::highlight() : SysColor::btnFace());
            if (hasFocus())
                text_color = Qt::white;
        } else {
            p.fillRect(r, Qt::white);
        }
        if (defs_) {
            const int idx = defs_->indexOf(id);
            if (idx >= 0)
                p.drawImage(r.topLeft(), defs_->at(idx).picture);
        }
        if (show_counts_) {
            const int tw = fm.horizontalAdvance(text);
            const int x = r.left() + r.width() / 2 - tw / 2;
            const int y = r.bottom() + 1 - fm.height();
            p.setPen(text_color);
            p.drawText(QRect(x, y, tw + 4, fm.height()), Qt::AlignLeft | Qt::AlignTop, text);
        }
        if (hasFocus() && i == caret_)
            drawFocusRect(p, r);
    }
    if (hasFocus() && itemCount() == 0)
        drawFocusRect(p, QRect(c.left(), c.top(), ColumnWidth, ItemHeight));
}

void IDListBox::mousePressEvent(QMouseEvent *e) {
    setFocus(Qt::MouseFocusReason);
    if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
        return;
    const int i = itemAt(e->pos());
    if (e->button() == Qt::RightButton)
        return; // the context menu selects
    if (i < 0)
        return;
    // LBS_EXTENDEDSEL
    if (e->modifiers() & Qt::ShiftModifier) {
        const bool keep = e->modifiers() & Qt::ControlModifier;
        for (int k = 0; k < itemCount(); ++k)
            if (!keep)
                sel_[k] = false;
        for (int k = std::min(anchor_, i); k <= std::max(anchor_, i); ++k)
            sel_[k] = true;
    } else if (e->modifiers() & Qt::ControlModifier) {
        sel_[i] = !sel_[i];
        anchor_ = i;
    } else {
        std::fill(sel_.begin(), sel_.end(), false);
        sel_[i] = true;
        anchor_ = i;
    }
    setCaret(i, false);
    update();
    selChanged();
}

void IDListBox::keyPressEvent(QKeyEvent *e) {
    // FUN_004198a0
    switch (e->key()) {
    case Qt::Key_PageUp:
        moveSelected(-1);
        return;
    case Qt::Key_PageDown:
        moveSelected(1);
        return;
    case Qt::Key_Insert:
        insert();
        return;
    case Qt::Key_Delete:
        removeSelected();
        return;
    case Qt::Key_Plus:
        increase();
        return;
    case Qt::Key_Minus:
        decrease();
        return;
    default:
        break;
    }
    // default list box navigation
    int target = -1;
    switch (e->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
        target = caret_ - 1;
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
        target = caret_ + 1;
        break;
    case Qt::Key_Home:
        target = 0;
        break;
    case Qt::Key_End:
        target = itemCount() - 1;
        break;
    case Qt::Key_Space:
        if (itemCount() > 0) {
            sel_[caret_] = !sel_[caret_];
            update();
            selChanged();
        }
        return;
    default:
        QWidget::keyPressEvent(e);
        return;
    }
    if (itemCount() == 0)
        return;
    target = std::clamp(target, 0, itemCount() - 1);
    if (!(e->modifiers() & Qt::ControlModifier)) {
        std::fill(sel_.begin(), sel_.end(), false);
        if (e->modifiers() & Qt::ShiftModifier) {
            for (int k = std::min(anchor_, target); k <= std::max(anchor_, target); ++k)
                sel_[k] = true;
        } else {
            sel_[target] = true;
            anchor_ = target;
        }
    }
    setCaret(target);
    update();
    selChanged();
}

void IDListBox::contextMenuEvent(QContextMenuEvent *e) {
    // the pages forward WM_CONTEXTMENU of their lists to FUN_00419ff0
    showContextMenu(e->globalPos());
}

void IDListBox::showContextMenu(const QPoint &global) {
    // FUN_00419ff0
    const int i = itemAt(mapFromGlobal(global));
    if (i >= 0 && !sel_[i]) {
        // FUN_00419ac0: select only the item under the cursor
        std::fill(sel_.begin(), sel_.end(), false);
        sel_[i] = true;
        anchor_ = i;
        setCaret(i, false);
        update();
    }
    // FUN_00419a80: definition of the caret item
    const Def *def = nullptr;
    if (defs_ && itemCount() > 0)
        def = defs_->byId(list_.getID(caret_));
    const int nsel = static_cast<int>(selection().size());
    const QString what = nsel < 2 ? (def ? def->name : QString()) : LauncherRes::str(51006);
    QMenu menu(this);
    QAction *plus = menu.addAction(QString("+ %1").arg(what));
    QAction *minus = menu.addAction(QString("-  %1").arg(what));
    menu.addSeparator();
    QAction *add = menu.addAction(LauncherRes::str(51005));
    QAction *del = menu.addAction(LauncherRes::str(51003));
    plus->setEnabled(def);
    minus->setEnabled(def);
    del->setEnabled(def);
    QAction *chosen = menu.exec(global);
    if (chosen == plus)
        increase();
    else if (chosen == minus)
        decrease();
    else if (chosen == add)
        insert();
    else if (chosen == del)
        removeSelected();
}

void IDListBox::action(Action a) {
    switch (a) {
    case Plus: increase(); break;
    case Minus: decrease(); break;
    case Insert: insert(); break;
    case Delete: removeSelected(); break;
    case MoveLeft: moveSelected(-1); break;
    case MoveRight: moveSelected(1); break;
    }
}

void IDListBox::increase() {
    // FUN_00419b20
    const std::vector<int> s = selection();
    if (s.empty())
        return;
    for (int i : s) {
        int max = 10;
        if (defs_)
            if (const Def *d = defs_->byId(list_.getID(i)))
                max = d->maxUserSelect;
        int count = list_.getCount(i);
        if (count + 1 <= max)
            ++count;
        if (!show_counts_)
            count = 0;
        list_.setCount(i, count);
    }
    update();
    Scen::playSound(SoundCount);
}

void IDListBox::decrease() {
    // FUN_00419c20: counts down, items reaching 0 are removed
    const std::vector<int> s = selection();
    if (s.empty())
        return;
    for (int i : s) {
        int count = list_.getCount(i);
        if (count > 0)
            --count;
        if (!show_counts_)
            count = 0;
        list_.setCount(i, count);
    }
    for (auto it = s.rbegin(); it != s.rend(); ++it)
        if (list_.getCount(*it) == 0) {
            list_.deleteItem(*it);
            sel_.erase(sel_.begin() + *it);
        }
    top_ = std::clamp(top_, 0, std::max(0, itemCount() - 1));
    caret_ = std::clamp(caret_, 0, std::max(0, itemCount() - 1));
    update();
    updateScroll();
    Scen::playSound(SoundCount);
}

void IDListBox::insert() {
    // FUN_00419d40: object selection, selected objects are added and clamped to MaxUserSelect
    Scen::playSound(SoundInsert);
    if (!defs_)
        return;
    IDSelectDlg dlg(window());
    dlg.setup(category_, caption_, defs_);
    if (dlg.exec() != QDialog::Accepted)
        return;
    list_.add(dlg.selected());
    for (int i = 0; i < Scen::IDList::Size; ++i) {
        int count = 0;
        const QString id = list_.getID(i, &count);
        if (id.isEmpty())
            continue;
        if (const Def *d = defs_->byId(id))
            list_.setIDCount(id, std::min(count, d->maxUserSelect), false);
    }
    list_.sortByCategory(*defs_);
    refill();
}

void IDListBox::removeSelected() {
    // FUN_00419ef0
    const std::vector<int> s = selection();
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        list_.deleteItem(*it);
        sel_.erase(sel_.begin() + *it);
    }
    top_ = std::clamp(top_, 0, std::max(0, itemCount() - 1));
    caret_ = std::clamp(caret_, 0, std::max(0, itemCount() - 1));
    update();
    updateScroll();
}

void IDListBox::moveSelected(int dir) {
    // FUN_0041a260: swap the selected items with their neighbours, FUN_0041a3c0: move the selection
    const std::vector<int> s = selection();
    const int count = itemCount();
    if (s.empty())
        return;
    if (dir < 0) {
        for (int i : s) {
            if (i + dir < 0)
                break;
            list_.swapItems(i, i + dir);
        }
        if (!sel_[0]) {
            for (int k = 0; k < count; ++k)
                sel_[k] = (k - dir < count) ? sel_[k - dir] : false;
        }
    } else {
        for (auto it = s.rbegin(); it != s.rend(); ++it) {
            if (*it + dir >= count)
                break;
            list_.swapItems(*it, *it + dir);
        }
        if (!sel_[count - 1]) {
            for (int k = count - 1; k >= 0; --k)
                sel_[k] = (k - dir >= 0) ? sel_[k - dir] : false;
        }
    }
    update();
}

// ======================================================================================= IDListButtons

IDListButtons::IDListButtons(QWidget *placeholder) : QWidget(nullptr) {
    replacePlaceholder(this, placeholder);
    // 0x419230: 23 x 52
    setGeometry(x(), y(), 0x17, 0x34);
}

void IDListButtons::paintEvent(QPaintEvent *) {
    // FUN_00419160: bitmap 1014 at 0,0
    QPainter p(this);
    p.drawPixmap(0, 0, LauncherRes::bitmap(1014));
}

void IDListButtons::mousePressEvent(QMouseEvent *e) {
    // 0x4191d0: upper third "+", middle "-", lower the object selection
    if (!list_ || e->button() != Qt::LeftButton || !isEnabled())
        return;
    const double y = e->pos().y();
    const double h = height();
    if (y < h * (1.0f / 3.0f))
        list_->action(IDListBox::Plus);
    else if (y < h * (2.0f / 3.0f))
        list_->action(IDListBox::Minus);
    else
        list_->action(IDListBox::Insert);
}
