#include "Win3DWidgets.h"
#include "LauncherRes.h"
#include <QMouseEvent>
#include <QApplication>
#include <QFile>

// Static initialization
QSoundEffect *Win3DButton::click_sound = nullptr;
QString Win3DButton::font_family = "Comic Sans MS";
QSoundEffect *ClonkButton::click_sound = nullptr;
QString ClonkButton::font_family = "Comic Sans MS";

// Win3DFrame
Win3DFrame::Win3DFrame(QWidget *parent, const std::vector<std::string> &colors_in, const std::string &bg_color_in)
    : QFrame(parent), bg_color(bg_color_in) {
    if (!colors_in.empty()) {
        colors = colors_in;
    } else {
        colors = {"#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a"};
    }
}

void Win3DFrame::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int w = width(), h = height();

    if (!bg_color.empty()) {
        painter.fillRect(2, 2, w-4, h-4, QColor(bg_color.c_str()));
    }

    painter.setPen(QColor(colors[0].c_str()));
    painter.drawLine(0, 0, w-1, 0);
    painter.drawLine(0, 0, 0, h-1);

    painter.setPen(QColor(colors[1].c_str()));
    painter.drawLine(1, 1, w-2, 1);
    painter.drawLine(1, 1, 1, h-2);

    painter.setPen(QColor(colors[2].c_str()));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, 1, w-2, h-2);

    painter.setPen(QColor(colors[3].c_str()));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, 0, w-1, h-1);
}

// Win3DButton
Win3DButton::Win3DButton(QWidget *parent, const std::vector<std::string> &raised, const std::vector<std::string> &sunken, const std::string &bg, const std::string &arrow_in)
    : QPushButton(parent), bg_color(bg), arrow(arrow_in) {
    raised_colors = raised.empty() ? std::vector<std::string>{"#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a"} : raised;
    sunken_colors = sunken.empty() ? std::vector<std::string>{"#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff"} : sunken;
    setFocusPolicy(Qt::NoFocus);
    QFont btn_font = QApplication::font();
    btn_font.setStyleStrategy(QFont::NoAntialias);
    setFont(btn_font);
    connect(this, &QPushButton::clicked, this, &Win3DButton::playClick);
}

Win3DButton::Win3DButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent), bg_color("#ece9d8") {
    raised_colors = {"#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a"};
    sunken_colors = {"#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff"};
    setFocusPolicy(Qt::NoFocus);
    QFont btn_font = QApplication::font();
    btn_font.setStyleStrategy(QFont::NoAntialias);
    setFont(btn_font);
    // connect(this, &QPushButton::clicked, this, &Win3DButton::playClick); // these dont have sounds
}

void Win3DButton::playClick() {
    // standard Windows push buttons are silent in the original
}

void Win3DButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect rect = this->rect();
    int w = rect.width(), h = rect.height();

    bool is_pressed = isDown() || isChecked();
    const auto &colors = is_pressed ? sunken_colors : raised_colors;

    int offset = 0;
    if (isDefault()) {
        painter.setPen(QColor("#9e9e9e"));
        painter.drawRect(0, 0, w-1, h-1);
        offset = 1;
    }

    painter.fillRect(offset, offset, w - (offset*2), h - (offset*2), QColor(bg_color.c_str()));

    painter.setPen(QColor(colors[0].c_str()));
    painter.drawLine(offset, offset, w-1-offset, offset);
    painter.drawLine(offset, offset, offset, h-1-offset);

    painter.setPen(QColor(colors[1].c_str()));
    painter.drawLine(offset+1, offset+1, w-2-offset, offset+1);
    painter.drawLine(offset+1, offset+1, offset+1, h-2-offset);

    painter.setPen(QColor(colors[2].c_str()));
    painter.drawLine(offset+1, h-2-offset, w-1-offset, h-2-offset);
    painter.drawLine(w-2-offset, offset+1, w-2-offset, h-1-offset);

    painter.setPen(QColor(colors[3].c_str()));
    painter.drawLine(offset, h-1-offset, w-1-offset, h-1-offset);
    painter.drawLine(w-1-offset, offset, w-1-offset, h-1-offset);

    painter.setFont(this->font());
    painter.setPen(isEnabled() ? QColor("black") : QColor("#808080"));

    QRect text_rect = is_pressed ? rect.adjusted(1, 1, 0, 0) : rect;
    painter.drawText(text_rect, Qt::AlignCenter, text());

    if (!arrow.empty()) {
        painter.setPen(QColor("black"));
        int mid_x = w / 2;
        int mid_y = h / 2;
        if (is_pressed) {
            mid_x++;
            mid_y++;
        }
        if (arrow == "left") {
            painter.drawLine(mid_x + 1, mid_y - 2, mid_x + 1, mid_y + 2);
            painter.drawLine(mid_x, mid_y - 1, mid_x, mid_y + 1);
            painter.drawPoint(mid_x - 1, mid_y);
        } else if (arrow == "right") {
            painter.drawLine(mid_x - 1, mid_y - 2, mid_x - 1, mid_y + 2);
            painter.drawLine(mid_x, mid_y - 1, mid_x, mid_y + 1);
            painter.drawPoint(mid_x + 1, mid_y);
        }
    }
}

// Win3DGroupBox
Win3DGroupBox::Win3DGroupBox(const QString &title, QWidget *parent, const std::vector<std::string> &colors_in)
    : QWidget(parent), m_title(title) {
    colors = colors_in.empty() ? std::vector<std::string>{"#a6a6a6", "#ffffff", "#a6a6a6", "#ffffff"} : colors_in;
}

void Win3DGroupBox::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, false);
    QFont font = painter.font();
    if (font.bold() || font.weight() >= QFont::Bold) {
        font.setStyleStrategy(QFont::PreferAntialias);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
    } else {
        font.setStyleStrategy(QFont::NoAntialias);
        font.setHintingPreference(QFont::PreferFullHinting);
        painter.setRenderHint(QPainter::TextAntialiasing, false);
    }
    painter.setFont(font);
    QFontMetrics fm = painter.fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int text_w = fm.horizontalAdvance(m_title);
#else
    int text_w = fm.width(m_title);
#endif
    int text_h = fm.height();
    int text_x = 8;
    int line_y = (text_h / 2) - 1;

    int w = width(), h = height();
    int gap_start = text_x - 2;
    int gap_end = text_x + text_w + 4;
    if (m_title.isEmpty()) {
        gap_start = gap_end = 0;
    }

    // Outer Top/Left (dark)
    painter.setPen(QColor(colors[0].c_str()));
    painter.drawLine(0, line_y, gap_start, line_y);
    if (gap_end < w) painter.drawLine(gap_end, line_y, w-1, line_y);
    painter.drawLine(0, line_y, 0, h-1);

    // Inner Top/Left (light)
    painter.setPen(QColor(colors[1].c_str()));
    painter.drawLine(1, line_y+1, gap_start, line_y+1);
    if (gap_end < w) painter.drawLine(gap_end, line_y+1, w-2, line_y+1);
    painter.drawLine(1, line_y+1, 1, h-2);

    // Inner Bottom/Right (dark)
    painter.setPen(QColor(colors[2].c_str()));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, line_y+1, w-2, h-2);

    // Outer Bottom/Right (light)
    painter.setPen(QColor(colors[3].c_str()));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, line_y, w-1, h-1);

    if (!m_title.isEmpty()) {
        painter.setPen(Qt::black);
        painter.drawText(text_x, fm.ascent() + 1, m_title);
    }
}

// Win3DTabWidget
// Layout and look of the comctl32 tab control of Windows XP without visual styles (the original has
// no manifest): TAB_SetItemBounds (row balancing, justification), TAB_EnsureSelectionVisible (row
// rotation), TAB_DrawItem (edges with cut corners).
namespace {
constexpr int TAB_ICON_SIZE = 16;
constexpr int TAB_ICON_PADDING = 3;  // image to text
constexpr int TAB_H_PADDING = 6;     // uHItemPadding
const QColor TAB_FACE(0xec, 0xe9, 0xd8);
} // namespace

Win3DTabWidget::Win3DTabWidget(QWidget *parent, const std::vector<std::string> &colors_in)
    : QWidget(parent) {
    // highlight, light, shadow, dark shadow (XP color scheme)
    colors = colors_in.size() >= 4 ? colors_in : std::vector<std::string>{"#ffffff", "#f1efe2", "#aca899", "#716f64"};
}

void Win3DTabWidget::addTab(QWidget *widget, const QString &text, const QIcon &icon) {
    TabData t;
    t.text = text;
    t.icon = icon;
    t.widget = widget;
    tabs.push_back(t);
    if (widget) {
        widget->setParent(this);
        widget->hide();
    }
    layoutTabs();
    if (active_index < 0)
        setActiveIndex(0);
    update();
}

QWidget *Win3DTabWidget::page(int index) const {
    return index >= 0 && index < count() ? tabs[index].widget : nullptr;
}

void Win3DTabWidget::setActiveIndex(int index) {
    if (index < 0 || index >= count())
        return;
    const bool changed = index != active_index;
    active_index = index;
    // TAB_EnsureSelectionVisible: the row of the selected tab moves next to the frame, the rows
    // that were in front of it move one row away
    const int sel_row = tabs[index].row;
    if (sel_row < static_cast<int>(row_order.size())) {
        const int dist = row_order[sel_row];
        if (dist != 0)
            for (int &d : row_order)
                d = d == dist ? 0 : (d < dist ? d + 1 : d);
    }
    for (int i = 0; i < count(); ++i)
        if (tabs[i].widget)
            tabs[i].widget->setVisible(i == index);
    if (tabs[index].widget)
        tabs[index].widget->raise();
    update();
    if (changed)
        emit currentChanged(index);
}

int Win3DTabWidget::naturalWidth(const TabData &t) const {
    const int text_w = fontMetrics().horizontalAdvance(t.text);
    const int icon_w = t.icon.isNull() ? 0 : TAB_ICON_SIZE + TAB_ICON_PADDING;
    return text_w + icon_w + 2 * TAB_H_PADDING;
}

int Win3DTabWidget::rowCount(int w) const {
    const int avail = w - 2 * SELECTED_OFFSET;
    int rows = 1, pos = 0;
    for (const auto &t : tabs) {
        const int tw = naturalWidth(t);
        if (pos > 0 && pos + tw > avail) {
            ++rows;
            pos = 0;
        }
        pos += tw;
    }
    return rows;
}

void Win3DTabWidget::layoutTabs() {
    const int n = count();
    num_rows = n ? rowCount(width()) : 1;
    const bool reset_rows = static_cast<int>(row_order.size()) != num_rows;
    if (reset_rows) {
        row_order.resize(num_rows);
        for (int r = 0; r < num_rows; ++r)
            row_order[r] = r;
    }
    if (!n)
        return;
    // the same number of tabs on each row, the first rows take the remainder
    const int per_row = n / num_rows, rem = n % num_rows;
    for (int i = 0, row = 0, cnt = 0; i < n; ++i, ++cnt) {
        if (cnt >= (row < rem ? per_row + 1 : per_row)) {
            ++row;
            cnt = 0;
        }
        tabs[i].row = row;
    }
    // justify every row to the full width
    const int avail = width() - 2 * SELECTED_OFFSET;
    for (int row = 0, first = 0; first < n; ++row) {
        int last = first;
        int sum = 0;
        while (last < n && tabs[last].row == row)
            sum += naturalWidth(tabs[last++]);
        const int cnt = last - first;
        if (cnt == 1) {
            tabs[first].left = SELECTED_OFFSET;
            tabs[first].right = SELECTED_OFFSET + avail - 1;
        } else {
            const int extra = (avail - sum) / cnt, remainder = (avail - sum) % cnt;
            int x = SELECTED_OFFSET;
            for (int i = first; i < last; ++i) {
                int w = naturalWidth(tabs[i]) + extra;
                if (i == last - 1)
                    w += remainder;
                tabs[i].left = x;
                tabs[i].right = x + w - 1;
                x += w;
            }
        }
        first = last;
    }
    // new rows: the row of the selected tab goes next to the frame again
    if (reset_rows && active_index >= 0 && active_index < n) {
        const int dist = row_order[tabs[active_index].row];
        if (dist != 0)
            for (int &d : row_order)
                d = d == dist ? 0 : (d < dist ? d + 1 : d);
    }
}

int Win3DTabWidget::frameTop() const { return SELECTED_OFFSET + num_rows * ROW_HEIGHT; }

int Win3DTabWidget::visualRow(int logical_row) const {
    const int dist = logical_row < static_cast<int>(row_order.size()) ? row_order[logical_row] : 0;
    return num_rows - 1 - dist;
}

QRect Win3DTabWidget::tabRect(int index) const {
    const TabData &t = tabs[index];
    const int top = SELECTED_OFFSET + visualRow(t.row) * ROW_HEIGHT;
    return QRect(QPoint(t.left, top), QPoint(t.right, top + ROW_HEIGHT - 1));
}

QSize Win3DTabWidget::sizeForPageSize(const QSize &page) const {
    const int w = page.width() + 8;
    const int rows = count() ? rowCount(w) : 1;
    return QSize(w, SELECTED_OFFSET + rows * ROW_HEIGHT + 2 + page.height() + 2);
}

QRect Win3DTabWidget::pageRect() const {
    return QRect(4, frameTop() + 2, width() - 8, height() - frameTop() - 4);
}

void Win3DTabWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    layoutTabs();
    for (auto &t : tabs)
        if (t.widget)
            t.widget->setGeometry(pageRect());
}

void Win3DTabWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton)
        return;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint pos = event->position().toPoint();
#else
    const QPoint pos = event->pos();
#endif
    // the selected tab is enlarged and lies on top
    if (active_index >= 0 && tabRect(active_index).adjusted(-2, -2, 2, 0).contains(pos))
        return;
    for (int i = 0; i < count(); ++i)
        if (tabRect(i).contains(pos)) {
            setActiveIndex(i);
            return;
        }
}

void Win3DTabWidget::drawTab(QPainter &p, int index, bool selected) {
    const QColor hi(colors[0].c_str()), light(colors[1].c_str()), shadow(colors[2].c_str()),
        dark(colors[3].c_str());
    const QRect r = tabRect(index);
    int L = r.left(), R = r.right(), T = r.top(), B = r.bottom();
    if (selected) {
        L -= SELECTED_OFFSET;
        R += SELECTED_OFFSET;
        T -= SELECTED_OFFSET;
        B = frameTop();
        // the selected tab covers the frame lines below it (but not the frame's side edges)
        const int fl = qMax(L, 2), fr = qMin(R, width() - 3);
        p.fillRect(QRect(QPoint(fl, T), QPoint(fr, B + 1)), TAB_FACE);
        p.fillRect(QRect(QPoint(L, T), QPoint(R, B)), TAB_FACE);
    } else {
        p.fillRect(QRect(QPoint(L, T), QPoint(R, B)), TAB_FACE);
    }
    auto vline = [&](int x, int y0, int y1, const QColor &c) { p.fillRect(QRect(QPoint(x, y0), QPoint(x, y1)), c); };
    auto hline = [&](int x0, int x1, int y, const QColor &c) { p.fillRect(QRect(QPoint(x0, y), QPoint(x1, y)), c); };
    // left and top: highlight outside, light inside, cut corner
    vline(L, T + 2, B, hi);
    vline(L + 1, T + 2, B, light);
    p.fillRect(L + 1, T + 1, 1, 1, hi);
    hline(L + 2, R - 2, T, hi);
    hline(L + 2, R - 2, T + 1, light);
    // right: dark shadow outside, shadow inside
    p.fillRect(R - 1, T + 1, 1, 1, dark);
    vline(R - 1, T + 2, B, shadow);
    vline(R, T + 2, B, dark);

    // icon and text, centered (TAB_DrawItemInterior)
    const TabData &t = tabs[index];
    const QFontMetrics fm = p.fontMetrics();
    const int text_w = fm.horizontalAdvance(t.text);
    const bool has_icon = !t.icon.isNull();
    const int content_w = text_w + (has_icon ? TAB_ICON_SIZE + TAB_H_PADDING : 0);
    const int w = R - L + 1;
    int x = L + (w - content_w + 1) / 2;
    if (has_icon) {
        p.drawPixmap(x, T + 2, t.icon.pixmap(TAB_ICON_SIZE, TAB_ICON_SIZE));
        x += TAB_ICON_SIZE + TAB_H_PADDING;
    }
    p.setPen(Qt::black);
    p.drawText(QRect(x, T + 3, text_w + 2, TAB_ICON_SIZE), Qt::AlignLeft | Qt::AlignVCenter, t.text);
}

void Win3DTabWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setFont(font());
    p.fillRect(rect(), TAB_FACE);
    const QColor hi(colors[0].c_str()), light(colors[1].c_str()), shadow(colors[2].c_str()),
        dark(colors[3].c_str());

    // page frame (DrawEdge EDGE_RAISED)
    const int ft = frameTop(), w = width(), h = height();
    p.fillRect(0, ft, w - 1, 1, hi);
    p.fillRect(0, ft, 1, h - ft - 1, hi);
    p.fillRect(1, ft + 1, w - 3, 1, light);
    p.fillRect(1, ft + 1, 1, h - ft - 3, light);
    p.fillRect(w - 1, ft, 1, h - ft, dark);
    p.fillRect(0, h - 1, w, 1, dark);
    p.fillRect(w - 2, ft + 1, 1, h - ft - 2, shadow);
    p.fillRect(1, h - 2, w - 2, 1, shadow);

    if (tabs.empty())
        return;
    // rows from the farthest to the one next to the frame, the selected tab last
    for (int vis = 0; vis < num_rows; ++vis)
        for (int i = 0; i < count(); ++i)
            if (i != active_index && visualRow(tabs[i].row) == vis)
                drawTab(p, i, false);
    if (active_index >= 0)
        drawTab(p, active_index, true);
}

// ClonkArea
ClonkArea::ClonkArea(QWidget *parent, const std::string &bg, const std::vector<std::string> &borders)
    : QFrame(parent), bg_color(bg) {
    border_colors = borders.empty() ? std::vector<std::string>{"#aca899", "#716f64", "#f1efe2", "#ffffff"} : borders;
}

void ClonkArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int w = width(), h = height();

    if (!bg_color.empty()) {
        painter.fillRect(2, 2, w-4, h-4, QColor(bg_color.c_str()));
    }

    painter.setPen(QColor(border_colors[0].c_str()));
    painter.drawLine(0, 0, w-1, 0);
    painter.drawLine(0, 0, 0, h-1);

    painter.setPen(QColor(border_colors[1].c_str()));
    painter.drawLine(1, 1, w-2, 1);
    painter.drawLine(1, 1, 1, h-2);

    painter.setPen(QColor(border_colors[2].c_str()));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, 1, w-2, h-2);

    painter.setPen(QColor(border_colors[3].c_str()));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, 0, w-1, h-1);
}

// ClonkPreviewLabel
ClonkPreviewLabel::ClonkPreviewLabel(QWidget *parent)
    : QLabel(parent) {}

void ClonkPreviewLabel::paintEvent(QPaintEvent *event) {
    QLabel::paintEvent(event);

    QPainter painter(this);
    int w = width(), h = height();

    painter.setPen(QColor("#aca899"));
    painter.drawLine(0, 0, w-1, 0);
    painter.drawLine(0, 0, 0, h-1);

    painter.setPen(QColor("#716f64"));
    painter.drawLine(1, 1, w-2, 1);
    painter.drawLine(1, 1, 1, h-2);

    painter.setPen(QColor("#f1efe2"));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, 1, w-2, h-2);

    painter.setPen(QColor("#ffffff"));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, 0, w-1, h-1);
}

// ClonkTextArea
ClonkTextArea::ClonkTextArea(QWidget *parent, const std::string &bg)
    : QFrame(parent), bg_color(bg) {}

void ClonkTextArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int w = width(), h = height();

    if (!bg_color.empty()) {
        painter.fillRect(2, 2, w-4, h-4, QColor(bg_color.c_str()));
    }

    painter.setPen(QColor("#aca899"));
    painter.drawLine(0, 0, w-1, 0);
    painter.drawLine(0, 0, 0, h-1);
    painter.drawLine(1, 1, w-2, 1);
    painter.drawLine(1, 1, 1, h-2);
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, 1, w-2, h-2);

    painter.setPen(QColor("#ffffff"));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, 0, w-1, h-1);

/*     if (!text_content.isEmpty()) {
        painter.setPen(Qt::black);
        
        int offsetX = 3;
        int offsetY = 4;

        QRect textRect(offsetX, offsetY, w - offsetX - 2, h - offsetY - 2);
        
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, text_content);
    } */
}

// ClonkButton
ClonkButton::ClonkButton(const QString &text, QWidget *parent, const QString &bg_path, const QPoint &bg_offset_in, const QSize &size)
    : QPushButton(text, parent), bg_offset(bg_offset_in) {
    if (!bg_path.isEmpty() && QFile::exists(bg_path)) {
        bg_pix.load(bg_path);
    }
    setFixedSize(size);
    setFocusPolicy(Qt::NoFocus);
    QFont btn_font(font_family, 9);
    btn_font.setStyleStrategy(QFont::NoAntialias);
    setFont(btn_font);
    connect(this, &QPushButton::pressed, this, &ClonkButton::playClick);
}

void ClonkButton::playClick() {
    // skinned push button class (0x401060): wave 7002 if Sound\FESamples
    LauncherRes::playSound(7002);
}

void ClonkButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, false);
    QRect rect = this->rect();
    int w = rect.width(), h = rect.height();

    if (!bg_pix.isNull()) {
        painter.drawTiledPixmap(rect, bg_pix, bg_offset);
    } else {
        painter.fillRect(rect, QColor("#c0c0c0"));
    }

    QFont btn_font = this->font();
    if (btn_font.bold() || btn_font.weight() >= QFont::Bold) {
        btn_font.setStyleStrategy(QFont::PreferAntialias);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
    } else {
        btn_font.setStyleStrategy(QFont::NoAntialias);
        btn_font.setHintingPreference(QFont::PreferFullHinting);
        painter.setRenderHint(QPainter::TextAntialiasing, false);
    }
    painter.setFont(btn_font);
    QRect t_rect = isDown() ? rect.adjusted(1, 1, 1, 1) : rect;
    t_rect.translate(0, -1);

    if (isDown()) {
        // Sunken
        painter.setPen(QColor("#aca899"));
        painter.drawLine(0, 0, w-1, 0);
        painter.drawLine(0, 0, 0, h-1);

        painter.setPen(QColor("#716f64"));
        painter.drawLine(1, 1, w-2, 1);
        painter.drawLine(1, 1, 1, h-2);

        painter.setPen(QColor("#f1efe2"));
        painter.drawLine(1, h-2, w-2, h-2);
        painter.drawLine(w-2, 1, w-2, h-2);

        painter.setPen(QColor("#ffffff"));
        painter.drawLine(0, h-1, w-1, h-1);
        painter.drawLine(w-1, 0, w-1, h-1);
    } else {
        // Raised
        painter.setPen(QColor("#f1efe2"));
        painter.drawLine(0, 0, w-1, 0);
        painter.drawLine(0, 0, 0, h-1);

        painter.setPen(QColor("#ffffff"));
        painter.drawLine(1, 1, w-2, 1);
        painter.drawLine(1, 1, 1, h-2);

        painter.setPen(QColor("#aca899"));
        painter.drawLine(1, h-2, w-2, h-2);
        painter.drawLine(w-2, 1, w-2, h-2);

        painter.setPen(QColor("#716f64"));
        painter.drawLine(0, h-1, w-1, h-1);
        painter.drawLine(w-1, 0, w-1, h-1);
    }

    // Shadowed text
    if (!isEnabled()) {
        painter.setPen(Qt::white);
        painter.drawText(t_rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text());
        painter.setPen(QColor("#a6a6a6"));
        painter.drawText(t_rect, Qt::AlignCenter, text());
    } else {
        painter.setPen(Qt::white);
        painter.drawText(t_rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text());
        painter.setPen(Qt::black);
        painter.drawText(t_rect, Qt::AlignCenter, text());
    }
}

// ClonkAtlasWidget
ClonkAtlasWidget::ClonkAtlasWidget(QWidget *parent, const QString &bg_path, const QSize &sub_size_in, int index_in)
    : QWidget(parent), sub_size(sub_size_in), index(index_in) {
    if (!bg_path.isEmpty() && QFile::exists(bg_path)) {
        bg_pix.load(bg_path);
    }
}

void ClonkAtlasWidget::setIndex(int index_in) {
    index = index_in;
    update();
}

void ClonkAtlasWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void ClonkAtlasWidget::paintEvent(QPaintEvent *event) {
    if (bg_pix.isNull() || sub_size.width() <= 0 || sub_size.height() <= 0) return;
    QPainter painter(this);

    int src_x = index * sub_size.width();
    int src_y = 0;
    QRect src_rect(src_x, src_y, sub_size.width(), sub_size.height());
    QRect target_rect = rect();

    painter.drawPixmap(target_rect, bg_pix, src_rect);
}

// ClonkTexturedWidget
ClonkTexturedWidget::ClonkTexturedWidget(QWidget *parent, const QString &bg_path)
    : QWidget(parent) {
    if (!bg_path.isEmpty() && QFile::exists(bg_path)) {
        bg_pix.load(bg_path);
    }
}

void ClonkTexturedWidget::setTexture(const QString &bg_path) {
    if (!bg_path.isEmpty() && QFile::exists(bg_path)) {
        bg_pix.load(bg_path);
    } else {
        bg_pix = QPixmap();
    }
    update();
}

void ClonkTexturedWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (!bg_pix.isNull()) {
        painter.drawTiledPixmap(rect(), bg_pix);
    } else {
        painter.fillRect(rect(), QColor("#c0c0c0"));
    }
}
