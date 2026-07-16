#include "Win3DWidgets.h"
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
    setFont(QFont(font_family, 9));
    connect(this, &QPushButton::clicked, this, &Win3DButton::playClick);
}

Win3DButton::Win3DButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent), bg_color("#f5f5f5") {
    raised_colors = {"#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a"};
    sunken_colors = {"#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff"};
    setFocusPolicy(Qt::NoFocus);
    setFont(QFont(font_family, 9));
    connect(this, &QPushButton::clicked, this, &Win3DButton::playClick);
}

void Win3DButton::playClick() {
    if (click_sound) click_sound->play();
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
Win3DTabWidget::Win3DTabWidget(QWidget *parent, const std::vector<std::string> &colors_in)
    : QWidget(parent) {
    colors = colors_in.empty() ? std::vector<std::string>{"#ffffff", "#e3e3e3", "#a6a6a6", "#6a6a6a"} : colors_in;
    stacked = new QStackedWidget(this);
    main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(12, 50, 14, 14);
    main_layout->addWidget(stacked);
    setMouseTracking(true);
}

void Win3DTabWidget::addTab(QWidget *widget, const QString &text, const QIcon &icon) {
    tabs.push_back({text, icon, widget});
    stacked->addWidget(widget);
    update();
}

void Win3DTabWidget::setActiveIndex(int index) {
    if (index >= 0 && index < (int)tabs.size()) {
        active_index = index;
        stacked->setCurrentIndex(index);
        update();
    }
}

void Win3DTabWidget::mousePressEvent(QMouseEvent *event) {
    int w = width();
    int cols = 3;
    int spacing = 0;
    int start_x = 2;
    int base_tab_w = (w - 4) / cols;
    int remainder = (w - 4) % cols;
    int tab_h = 22;
    int num_tabs = tabs.size();
    int num_rows = (num_tabs + cols - 1) / cols;
    int active_row = active_index / cols;

    int visual_row_counter = 0;
    for (int r = 0; r < num_rows; ++r) {
        int v_row = (r == active_row) ? (num_rows - 1) : visual_row_counter;
        if (r != active_row) visual_row_counter++;

        int start_idx = r * cols;
        int end_idx = std::min(start_idx + cols, num_tabs);
        for (int i = start_idx; i < end_idx; ++i) {
            int col = i % cols;
            int extra_x = 0;
            int current_w = base_tab_w;

            if (v_row == num_rows - 1) {
                if (col == 0) { extra_x = 0; current_w = base_tab_w + 2; }
                else if (col == 1) { extra_x = 2; current_w = base_tab_w + 2; }
                else if (col == 2) { extra_x = 4; current_w = base_tab_w - 4 + remainder; }
            } else if (col == 2) {
                current_w = base_tab_w + remainder;
            }

            int x = start_x + col * (base_tab_w + spacing) + extra_x;
            int y = 2 + v_row * tab_h;

            bool is_active = (i == active_index);
            int draw_x = is_active ? (x - 2) : x;
            int draw_y = is_active ? (y - 2) : y;
            int draw_w = is_active ? (current_w + 4) : current_w;
            int draw_h = is_active ? (tab_h + 3) : tab_h;

            QRect r(draw_x, draw_y, draw_w, draw_h);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (r.contains(event->position().toPoint())) {
#else
            if (r.contains(event->pos())) {
#endif
                setActiveIndex(i);
                return;
            }
        }
    }
}

void Win3DTabWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#f5f5f5"));

    if (tabs.empty()) return;

    int w = width();
    int h = height();
    int cols = 3;
    int spacing = 0;
    int start_x = 2;
    int base_tab_w = (w - 4) / cols;
    int remainder = (w - 4) % cols;
    int tab_h = 20;
    int num_tabs = tabs.size();
    int num_rows = (num_tabs + cols - 1) / cols;
    int active_row = active_index / cols;

    int frame_y = num_rows * tab_h + 2;

    auto draw_tab = [&](int i, int visual_row) {
        const auto &tab_data = tabs[i];
        int col = i % cols;
        int extra_x = 0;
        int current_w = base_tab_w;

        if (visual_row == num_rows - 1) {
            if (col == 0) { extra_x = 0; current_w = base_tab_w + 2; }
            else if (col == 1) { extra_x = 2; current_w = base_tab_w + 2; }
            else if (col == 2) { extra_x = 4; current_w = base_tab_w - 4 + remainder; }
        } else if (col == 2) {
            current_w = base_tab_w + remainder;
        }

        int x = start_x + col * (base_tab_w + spacing) + extra_x;
        int y = 2 + visual_row * tab_h;

        bool is_active = (i == active_index);
        int draw_x = is_active ? (x - 2) : x;
        int draw_y = is_active ? (y - 2) : y;
        int draw_w = is_active ? (current_w + 4) : current_w;
        int draw_h = is_active ? ((frame_y + 2) - draw_y) : tab_h;

        painter.fillRect(draw_x, draw_y, draw_w, draw_h, QColor("#f5f5f5"));

        // White Top/Left
        painter.setPen(QColor(colors[0].c_str()));
        painter.drawLine(draw_x + 2, draw_y, draw_x + draw_w - 3, draw_y);
        painter.drawLine(draw_x, draw_y + 2, draw_x, draw_y + draw_h - 1);
        painter.drawPoint(draw_x + 1, draw_y + 1);

        // Light Gray Top/Left Inner
        painter.setPen(QColor(colors[1].c_str()));
        painter.drawLine(draw_x + 2, draw_y + 1, draw_x + draw_w - 3, draw_y + 1);
        painter.drawLine(draw_x + 1, draw_y + 2, draw_x + 1, draw_y + draw_h - 1);

        // Dark Gray Right Inner
        painter.setPen(QColor(colors[2].c_str()));
        painter.drawLine(draw_x + draw_w - 2, draw_y + 2, draw_x + draw_w - 2, draw_y + draw_h - 1);

        // Black Right Outer
        painter.setPen(QColor(colors[3].c_str()));
        painter.drawLine(draw_x + draw_w - 1, draw_y + 2, draw_x + draw_w - 1, draw_y + draw_h - 1);
        painter.drawPoint(draw_x + draw_w - 2, draw_y + 1);

        if (is_active) {
            painter.setPen(QColor("#f5f5f5"));
            if (col == 0) {
                painter.drawLine(draw_x + draw_w - 1, draw_y + draw_h - 1, draw_x + draw_w - 2, draw_y + draw_h - 1);
            } else if (col == 1) {
                painter.drawLine(draw_x, draw_y + draw_h - 1, draw_x + draw_w - 1, draw_y + draw_h - 1);
            } else if (col == 2) {
                painter.drawLine(draw_x, draw_y + draw_h - 1, draw_x + 1, draw_y + draw_h - 1);
            }
        }

        // Label / Icon
        painter.setPen(Qt::black);
        QFontMetrics fm = painter.fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int text_w = fm.horizontalAdvance(tab_data.text);
#else
        int text_w = fm.width(tab_data.text);
#endif
        int icon_size = 16;
        int content_spacing = 4;
        int total_w = !tab_data.icon.isNull() ? (icon_size + content_spacing + text_w) : text_w;

        int content_x = draw_x + (draw_w - total_w) / 2;

        if (!tab_data.icon.isNull()) {
            int icon_y = draw_y + (draw_h - icon_size) / 2;
            painter.drawPixmap(content_x, icon_y, tab_data.icon.pixmap(icon_size, icon_size));
            content_x += icon_size + content_spacing;
        }

        QRect text_rect(content_x, draw_y, text_w, draw_h);
        painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, tab_data.text);
    };

    int visual_row_counter = 0;
    for (int r = 0; r < num_rows; ++r) {
        int v_row = (r == active_row) ? (num_rows - 1) : visual_row_counter;
        if (r != active_row) visual_row_counter++;

        int start_idx = r * cols;
        int end_idx = std::min(start_idx + cols, num_tabs);

        for (int i = end_idx - 1; i >= start_idx; --i) {
            if (i != active_index) {
                draw_tab(i, v_row);
            }
        }
    }

    // Main frame border
    painter.setPen(QColor(colors[0].c_str()));
    painter.drawLine(0, frame_y, w-1, frame_y);
    painter.drawLine(0, frame_y, 0, h-1);

    painter.setPen(QColor(colors[1].c_str()));
    painter.drawLine(1, frame_y+1, w-2, frame_y+1);
    painter.drawLine(1, frame_y+1, 1, h-2);

    painter.setPen(QColor(colors[2].c_str()));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, frame_y+1, w-2, h-2);

    // Black Right Outer
    painter.setPen(QColor(colors[3].c_str()));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, frame_y, w-1, h-1);

    if (active_index >= 0 && active_index < num_tabs) {
        draw_tab(active_index, num_rows - 1);
    }
}

// ClonkArea
ClonkArea::ClonkArea(QWidget *parent, const std::string &bg, const std::vector<std::string> &borders)
    : QFrame(parent), bg_color(bg) {
    border_colors = borders.empty() ? std::vector<std::string>{"#a6a6a6", "#6a6a6a", "#e3e3e3", "#ffffff"} : borders;
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

// ClonkTextArea
ClonkTextArea::ClonkTextArea(QWidget *parent, const std::string &bg)
    : QFrame(parent), bg_color(bg) {}

void ClonkTextArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int w = width(), h = height();

    if (!bg_color.empty()) {
        painter.fillRect(2, 2, w-4, h-4, QColor(bg_color.c_str()));
    }

    // Sunken frame: dark Top/Left, white Bottom/Right
    painter.setPen(QColor("#a6a6a6"));
    painter.drawLine(0, 0, w-1, 0);
    painter.drawLine(0, 0, 0, h-1);

    painter.setPen(QColor("#ffffff"));
    painter.drawLine(1, 1, w-2, 1);
    painter.drawLine(1, 1, 1, h-2);

    painter.setPen(QColor("#a6a6a6"));
    painter.drawLine(1, h-2, w-2, h-2);
    painter.drawLine(w-2, 1, w-2, h-2);

    painter.setPen(QColor("#ffffff"));
    painter.drawLine(0, h-1, w-1, h-1);
    painter.drawLine(w-1, 0, w-1, h-1);
}

// ClonkButton
ClonkButton::ClonkButton(const QString &text, QWidget *parent, const QString &bg_path, const QPoint &bg_offset_in, const QSize &size)
    : QPushButton(text, parent), bg_offset(bg_offset_in) {
    if (!bg_path.isEmpty() && QFile::exists(bg_path)) {
        bg_pix.load(bg_path);
    }
    setFixedSize(size);
    setFocusPolicy(Qt::NoFocus);
    setFont(QFont(font_family, 9));
    connect(this, &QPushButton::clicked, this, &ClonkButton::playClick);
}

void ClonkButton::playClick() {
    if (click_sound) click_sound->play();
}

void ClonkButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QRect rect = this->rect();
    int w = rect.width(), h = rect.height();

    if (!bg_pix.isNull()) {
        painter.drawTiledPixmap(rect, bg_pix, bg_offset);
    } else {
        painter.fillRect(rect, QColor("#c0c0c0"));
    }

    painter.setFont(this->font());
    QRect t_rect = isDown() ? rect.adjusted(1, 1, 1, 1) : rect;

    if (isDown()) {
        // Sunken
        painter.setPen(QColor("#6a6a6a"));
        painter.drawLine(0, 0, w-1, 0);
        painter.drawLine(0, 0, 0, h-1);
        painter.setPen(QColor("#a6a6a6"));
        painter.drawLine(1, 1, w-2, 1);
        painter.drawLine(1, 1, 1, h-2);
        painter.setPen(QColor("#ffffff"));
        painter.drawLine(0, h-1, w-1, h-1);
        painter.drawLine(w-1, 0, w-1, h-1);
    } else {
        // Raised
        painter.setPen(QColor("#e3e3e3"));
        painter.drawLine(0, 0, w-1, 0);
        painter.drawLine(0, 0, 0, h-1);

        painter.setPen(QColor("#ffffff"));
        painter.drawLine(1, 1, w-2, 1);
        painter.drawLine(1, 1, 1, h-2);

        painter.setPen(QColor("#a6a6a6"));
        painter.drawLine(1, h-2, w-2, h-2);
        painter.drawLine(w-2, 1, w-2, h-2);

        painter.setPen(QColor("#6a6a6a"));
        painter.drawLine(0, h-1, w-1, h-1);
        painter.drawLine(w-1, 0, w-1, h-1);
    }

    // Shadowed text
    painter.setPen(Qt::white);
    painter.drawText(t_rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text());
    painter.setPen(Qt::black);
    painter.drawText(t_rect, Qt::AlignCenter, text());
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
