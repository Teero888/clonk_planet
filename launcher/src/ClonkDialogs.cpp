#include "ClonkDialogs.h"
#include "ClonkLauncher.h"
#include <QDir>
#include <QFile>

QString CD::GetComicFontFamily(QWidget *parent) {
    QWidget *p = parent;
    while (p) {
        if (auto launcher = qobject_cast<ClonkLauncher*>(p)) {
            return launcher->getComicFontFamily();
        }
        p = p->parentWidget();
    }
    return "Comic Sans MS";
}

// ClonkPopupDialog
ClonkPopupDialog::ClonkPopupDialog(QWidget *parent, const QString &text, const QString &dump_path)
    : QDialog(parent) {
    setFixedSize(338, 156);
    setWindowTitle("Clonk Planet");

    QString bg_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1019_1031.bmp");
    QString btn_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1006_1031.bmp");

    ClonkTexturedWidget *bg = new ClonkTexturedWidget(this, bg_texture);
    bg->setGeometry(0, 0, 338, 156);

    ClonkButton *btn_ok = new ClonkButton("OK", bg, btn_texture, QPoint(124, 13), QSize(75, 23));
    btn_ok->move(5, 127);
    connect(btn_ok, &QPushButton::clicked, this, &QDialog::accept);

    ClonkTextArea *border = new ClonkTextArea(bg);
    border->setGeometry(5, 5, 329, 117);

    QLabel *label = new QLabel(text, border);
    label->setGeometry(8, 8, 329 - 16, 117 - 16);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    label->setStyleSheet("background: transparent; color: black;");

    QString comic_family = CD::GetComicFontFamily(parent);
    QFont label_font(comic_family, 12);
    label_font.setStyleStrategy(QFont::NoAntialias);
    label->setFont(label_font);
}

// ClonkPlayerPropertiesDialog
ClonkPlayerPropertiesDialog::ClonkPlayerPropertiesDialog(QWidget *parent, const QString &dump_path)
    : QDialog(parent) {
    setFixedSize(368, 180);
    setWindowTitle("Clonk Planet");

    QString bg_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1019_1031.bmp");
    QString btn_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1006_1031.bmp");
    QString flag_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1007_1031.bmp");
    QString keyboard_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1009_1031.bmp");
    QString mouse_texture = QDir(dump_path).filePath("Planet_fixed.bin_2_1017_1031.bmp");

    bg = new ClonkTexturedWidget(this, bg_texture);
    bg->setGeometry(0, 0, 368, 180);

    btn_ok = new ClonkButton("OK", bg, btn_texture, QPoint(121, 12), QSize(77, 23));
    btn_ok->move(101, 143);
    connect(btn_ok, &QPushButton::clicked, this, &QDialog::accept);

    btn_cancel = new ClonkButton("Cancel", bg, btn_texture, QPoint(121, 12), QSize(77, 23));
    btn_cancel->move(186, 143);
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

    std::vector<std::string> border_colors = {"#a6a6a6", "#6a6a6a", "#6a6a6a", "#ffffff"};
    color_border = new ClonkArea(bg, "", border_colors);
    color_border->setGeometry(27, 26, 99, 78);

    keyboard_border = new ClonkArea(bg, "", border_colors);
    keyboard_border->setGeometry(144, 26, 101, 78);

    mouse_border = new ClonkArea(bg, "", border_colors);
    mouse_border->setGeometry(249, 26, 83, 78);

    color_atlas = new ClonkAtlasWidget(color_border, flag_texture, QSize(100, 80));
    color_atlas->setGeometry(2, 2, 99 - 4, 78 - 4);

    keyboard_atlas = new ClonkAtlasWidget(keyboard_border, keyboard_texture, QSize(132, 90));
    keyboard_atlas->setGeometry(2, 2, 101 - 4, 78 - 4);

    mouse_atlas = new ClonkAtlasWidget(mouse_border, mouse_texture, QSize(90, 90));
    mouse_atlas->setGeometry(2, 2, 83 - 4, 78 - 4);

    std::vector<std::string> raised_colors = {"#e3e3e3", "#ffffff", "#a6a6a6", "#6a6a6a"};
    std::vector<std::string> sunken_colors = {"#6a6a6a", "#a6a6a6", "#e3e3e3", "#ffffff"};

    btn_color_left = new Win3DButton(bg, raised_colors, sunken_colors, "#ece9d8", "left");
    btn_color_left->setGeometry(27, 109, 49, 15);
    connect(btn_color_left, &QPushButton::clicked, this, &ClonkPlayerPropertiesDialog::onColorLeft);

    btn_color_right = new Win3DButton(bg, raised_colors, sunken_colors, "#ece9d8", "right");
    btn_color_right->setGeometry(77, 109, 49, 15);
    connect(btn_color_right, &QPushButton::clicked, this, &ClonkPlayerPropertiesDialog::onColorRight);

    btn_keyboard_left = new Win3DButton(bg, raised_colors, sunken_colors, "#ece9d8", "left");
    btn_keyboard_left->setGeometry(165, 109, 49, 15);
    connect(btn_keyboard_left, &QPushButton::clicked, this, &ClonkPlayerPropertiesDialog::onControlLeft);

    btn_keyboard_right = new Win3DButton(bg, raised_colors, sunken_colors, "#ece9d8", "right");
    btn_keyboard_right->setGeometry(215, 109, 49, 15);
    connect(btn_keyboard_right, &QPushButton::clicked, this, &ClonkPlayerPropertiesDialog::onControlRight);

    btn_mouse_toggle = new Win3DButton(bg, {}, {}, "#ece9d8", "");
    btn_mouse_toggle->setGeometry(266, 109, 45, 15);
    connect(btn_mouse_toggle, &QPushButton::clicked, this, &ClonkPlayerPropertiesDialog::onMouseToggle);
    connect(mouse_atlas, &ClonkAtlasWidget::clicked, this, &ClonkPlayerPropertiesDialog::onMouseToggle);

    QString comic_family = CD::GetComicFontFamily(parent);
    QFont label_font(comic_family, 12);
    label_font.setStyleStrategy(QFont::NoAntialias);

    lbl_color = new QLabel("Color", bg);
    lbl_color->setGeometry(27, 7, 99, 18);
    lbl_color->setAlignment(Qt::AlignCenter);
    lbl_color->setStyleSheet("background: transparent; color: black;");
    lbl_color->setFont(label_font);

    lbl_controls = new QLabel("Control", bg);
    lbl_controls->setGeometry(143, 7, 188, 18);
    lbl_controls->setAlignment(Qt::AlignCenter);
    lbl_controls->setStyleSheet("background: transparent; color: black;");
    lbl_controls->setFont(label_font);
}

void ClonkPlayerPropertiesDialog::loadSettings(int color, int control, int mouse) {
    color_atlas->setIndex(std::min(std::max(0, color), 5));
    keyboard_atlas->setIndex(std::min(std::max(0, control), 4));
    pref_mouse_index = (mouse != 0) ? 0 : 1; // 0: ON, 1: OFF

    if (keyboard_atlas->getIndex() == 4) {
        mouse_atlas->setIndex(1); // Forced OFF for gamepad
    } else {
        mouse_atlas->setIndex(pref_mouse_index);
    }
}

std::map<std::string, int> ClonkPlayerPropertiesDialog::getSettings() const {
    int mouse_val = (pref_mouse_index == 0) ? 1 : 0;
    return {
        {"Color", color_atlas->getIndex()},
        {"Control", keyboard_atlas->getIndex()},
        {"Mouse", mouse_val}
    };
}

void ClonkPlayerPropertiesDialog::onColorLeft() {
    cycleAtlas(color_atlas, 6, -1);
}

void ClonkPlayerPropertiesDialog::onColorRight() {
    cycleAtlas(color_atlas, 6, 1);
}

void ClonkPlayerPropertiesDialog::onControlLeft() {
    cycleAtlas(keyboard_atlas, 5, -1);
}

void ClonkPlayerPropertiesDialog::onControlRight() {
    cycleAtlas(keyboard_atlas, 5, 1);
}

void ClonkPlayerPropertiesDialog::onMouseToggle() {
    cycleAtlas(mouse_atlas, 2, 1);
}

void ClonkPlayerPropertiesDialog::cycleAtlas(ClonkAtlasWidget *widget, int count, int delta) {
    if (widget == mouse_atlas) {
        pref_mouse_index = (pref_mouse_index + delta + count) % count;
        if (keyboard_atlas->getIndex() != 4) {
            widget->setIndex(pref_mouse_index);
        }
    } else {
        int new_index = widget->getIndex() + delta;
        if (new_index >= 0 && new_index < count) {
            widget->setIndex(new_index);
            if (widget == keyboard_atlas) {
                if (new_index == 4) {
                    mouse_atlas->setIndex(1);
                } else {
                    mouse_atlas->setIndex(pref_mouse_index);
                }
            }
        }
    }
}
