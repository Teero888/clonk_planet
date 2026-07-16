#include "OptionsDialog.h"
#include "ClonkLauncher.h"
#include "ClonkDialogs.h"
#include "Utils.h"
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QDir>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHostInfo>
#include <algorithm>
#include <iostream>
#include <sstream>

// KeyBindInput
KeyBindInput::KeyBindInput(const QString &action_name_in, int key_code_in, QWidget *parent)
    : QLineEdit(parent), action_name(action_name_in), key_code(key_code_in) {
    setReadOnly(true);
    setFixedSize(65, 20);
    setCursor(Qt::ArrowCursor);
}

void KeyBindInput::setKeyCode(int code) {
    key_code = code;
}

void KeyBindInput::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Control || 
        event->key() == Qt::Key_Alt || event->key() == Qt::Key_Meta) {
        return;
    }

    std::map<int, int> qt_to_vk = {
        {Qt::Key_Left, 37}, {Qt::Key_Up, 38}, {Qt::Key_Right, 39}, {Qt::Key_Down, 40},
        {Qt::Key_Insert, 45}, {Qt::Key_Delete, 46}, {Qt::Key_Home, 36}, {Qt::Key_End, 35},
        {Qt::Key_PageUp, 33}, {Qt::Key_PageDown, 34}, {Qt::Key_Return, 13}, {Qt::Key_Enter, 13},
        {Qt::Key_Backspace, 8}, {Qt::Key_Space, 32}, {Qt::Key_Tab, 9}, {Qt::Key_Escape, 27}
    };

    int raw_key = event->key();
    auto it = qt_to_vk.find(raw_key);
    key_code = (it != qt_to_vk.end()) ? it->second : raw_key;

    if (key_code >= 0x61 && key_code <= 0x7A) { // a-z -> A-Z
        key_code -= 0x20;
    }

    QString key_str = QKeySequence(raw_key).toString();
    setText(key_str);
    emit keyChanged(action_name, key_code);
}

// HostDialog
HostDialog::HostDialog(QWidget *parent, const QString &title, const QString &initial_text)
    : QDialog(parent) {
    setWindowTitle(title);
    setFixedSize(280, 110);

    if (parent) {
        setStyleSheet(parent->styleSheet());
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    QLabel *label = new QLabel(title + ":");
    layout->addWidget(label);

    input = new QLineEdit(initial_text);
    layout->addWidget(input);

    layout->addSpacing(10);

    QHBoxLayout *btn_layout = new QHBoxLayout();
    btn_layout->setContentsMargins(0, 0, 0, 0);
    btn_layout->addStretch();

    Win3DButton *btn_ok = new Win3DButton("OK");
    btn_ok->setFixedSize(75, 23);
    btn_ok->setDefault(true);
    connect(btn_ok, &QPushButton::clicked, this, &QDialog::accept);

    Win3DButton *btn_cancel = new Win3DButton("Cancel");
    btn_cancel->setFixedSize(75, 23);
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

    btn_layout->addWidget(btn_ok);
    btn_layout->addWidget(btn_cancel);

    layout->addLayout(btn_layout);
}

QString HostDialog::getAddress() const {
    return input->text();
}

// OptionsDialog
OptionsDialog::OptionsDialog(ClonkLauncher *parent)
    : QDialog(parent), launcher(parent) {
    setWindowTitle("Properties");
    setFixedSize(380, 411);

    setStyleSheet(
        "QDialog, .QWidget {"
        "    background-color: #f5f5f5;"
        "    font-size: 11px;"
        "    color: black;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 7, 6, 6);
    layout->setSpacing(0);

    // Initialize key_codes cache
    for (int b = 1; b <= 4; ++b) {
        for (int i = 1; i <= 12; ++i) {
            std::string key_name = "Kbd" + std::to_string(b) + "Key" + std::to_string(i);
            std::string cfg_val = launcher->get_cfg("Controls\\" + key_name, "0");
            try {
                key_codes[key_name] = std::stoi(cfg_val);
            } catch (...) {
                key_codes[key_name] = 0;
            }
        }
    }

    tab_widget = new Win3DTabWidget(this);

    // Load texture atlas
    QString atlas_path = QDir(launcher->getDumpPath()).filePath("Planet_fixed.bin_2_1037_1031.bmp");
    QPixmap atlas_pixmap = applyClonkTransparency(QPixmap(atlas_path));

    auto get_icon = [&](int index) -> QIcon {
        if (atlas_pixmap.isNull()) return QIcon();
        QRect rect(index * 16, 0, 16, 16);
        return QIcon(atlas_pixmap.copy(rect));
    };

    tab_widget->addTab(createKeyboardPage(), "Keyboard", get_icon(10));
    
    // Stub game pad page
    QWidget *gamepad_page = new QWidget();
    QVBoxLayout *gp_layout = new QVBoxLayout(gamepad_page);
    gp_layout->addWidget(new QLabel("Game Pad Settings", gamepad_page), 0, Qt::AlignCenter);
    tab_widget->addTab(gamepad_page, "Game Pad", get_icon(12));

    tab_widget->addTab(createNetworkPage(), "Network", get_icon(13));
    tab_widget->addTab(createProgramPage(), "Program", get_icon(6));
    tab_widget->addTab(createGraphicsPage(), "Graphics", get_icon(2));
    tab_widget->addTab(createSoundPage(), "Sound", get_icon(5));

    layout->addWidget(tab_widget, 1);

    // Set "Program" (index 3) as the default active tab
    tab_widget->setActiveIndex(3);

    // Bottom buttons
    QHBoxLayout *btn_layout = new QHBoxLayout();
    btn_layout->setContentsMargins(0, 8, 0, 0);
    btn_layout->setSpacing(6);
    btn_layout->addStretch();

    Win3DButton *btn_ok = new Win3DButton("OK");
    btn_ok->setFixedSize(75, 23);
    btn_ok->setDefault(true);
    connect(btn_ok, &QPushButton::clicked, this, &OptionsDialog::onOK);

    Win3DButton *btn_cancel = new Win3DButton("Cancel");
    btn_cancel->setFixedSize(75, 23);
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

    btn_layout->addWidget(btn_ok);
    btn_layout->addWidget(btn_cancel);

    layout->addLayout(btn_layout);
}

void OptionsDialog::onKeyInputChanged(const QString &action_name, int code) {
    std::string key_id = "Kbd" + std::to_string(current_block) + action_name.toStdString();
    key_codes[key_id] = code;
}

void OptionsDialog::loadKeyProfile(int block_num) {
    current_block = block_num;
    bool is_hardcoded = (block_num == 4);

    std::map<std::string, int> hardcoded_map = {
        {"Key1", Qt::Key_Insert}, {"Key2", Qt::Key_Home},   {"Key3", Qt::Key_PageUp},
        {"Key4", Qt::Key_Delete}, {"Key5", Qt::Key_Up},     {"Key6", Qt::Key_PageDown},
        {"Key7", Qt::Key_Left},   {"Key8", Qt::Key_Down},   {"Key9", Qt::Key_Right},
        {"Key10", Qt::Key_End},   {"Key11", Qt::Key_Return}, {"Key12", Qt::Key_Backspace}
    };

    std::map<int, int> vk_to_qt = {
        {37, Qt::Key_Left}, {38, Qt::Key_Up}, {39, Qt::Key_Right}, {40, Qt::Key_Down},
        {45, Qt::Key_Insert}, {46, Qt::Key_Delete}, {36, Qt::Key_Home}, {35, Qt::Key_End},
        {33, Qt::Key_PageUp}, {34, Qt::Key_PageDown}, {13, Qt::Key_Return},
        {8, Qt::Key_Backspace}, {32, Qt::Key_Space}, {9, Qt::Key_Tab}, {27, Qt::Key_Escape}
    };

    for (auto &pair : key_inputs) {
        std::string key_id = pair.first;
        int code = 0;
        if (is_hardcoded) {
            code = hardcoded_map[key_id];
        } else {
            std::string full_key = "Kbd" + std::to_string(block_num) + key_id;
            auto it = key_codes.find(full_key);
            code = (it != key_codes.end()) ? it->second : 0;
        }

        pair.second->setEnabled(!is_hardcoded);
        pair.second->setKeyCode(code);
        
        if (code != 0) {
            auto vit = vk_to_qt.find(code);
            int qt_key = (vit != vk_to_qt.end()) ? vit->second : code;
            pair.second->setText(QKeySequence(qt_key).toString());
        } else {
            pair.second->setText("");
        }
    }

    if (btn_reset) {
        btn_reset->setEnabled(!is_hardcoded);
    }
}

void OptionsDialog::promptResetControls() {
    auto reply = QMessageBox::warning(
        this, 
        "Clonk Planet", 
        "Reset all controls?", 
        QMessageBox::Ok | QMessageBox::Cancel
    );

    if (reply == QMessageBox::Ok) {
        // Reset the current block keys
        for (int i = 1; i <= 12; ++i) {
            std::string key_id = "Kbd" + std::to_string(current_block) + "Key" + std::to_string(i);
            key_codes[key_id] = 0;
        }
        loadKeyProfile(current_block);
    }
}

void OptionsDialog::onOK() {
    auto set_cfg = [&](const std::string &sub_key, const std::string &val) {
        launcher->set_cfg(sub_key, val);
    };

    // Program Page
    std::string lang = radio_german->isChecked() ? "DE" : "US";
    launcher->setLanguage(lang);
    set_cfg("General\\Language", lang);
    set_cfg("General\\FEFontName", combo_font->currentText().toStdString());
    set_cfg("General\\FEFontSize", std::to_string(spin_menus->value()));
    set_cfg("General\\RXFontName", combo_font->currentText().toStdString());
    set_cfg("General\\RXFontSize", std::to_string(spin_game->value()));
    set_cfg("Explorer\\ShowQuickStart", check_quick->isChecked() ? "1" : "0");
    set_cfg("Developer\\Active", check_dev->isChecked() ? "1" : "0");

    // Graphics Page
    int res = 0;
    if (radio_800->isChecked()) res = 1;
    else if (radio_1024->isChecked()) res = 2;
    set_cfg("Graphics\\Resolution", std::to_string(res));
    set_cfg("Graphics\\SmokeLevel", std::to_string(slider_smoke->value() * 2));
    set_cfg("Graphics\\SplitscreenDividers", chk_viewport->isChecked() ? "1" : "0");
    set_cfg("Graphics\\ShowPlayerInfoAlways", chk_ext_info->isChecked() ? "1" : "0");
    set_cfg("Graphics\\ColorAnimation", chk_color_anim->isChecked() ? "1" : "0");
    set_cfg("Graphics\\ShowStartupMessages", chk_startup->isChecked() ? "1" : "0");
    set_cfg("Graphics\\ShowCommands", chk_obj_cmds->isChecked() ? "1" : "0");
    set_cfg("Graphics\\ShowPortraits", chk_portraits->isChecked() ? "1" : "0");
    set_cfg("Graphics\\DDrawAccel", chk_ddraw->isChecked() ? "1" : "0");

    // Sound Page
    set_cfg("Sound\\RXSound", chk_game_sfx->isChecked() ? "1" : "0");
    set_cfg("Sound\\RXSoundLoops", chk_game_loops->isChecked() ? "1" : "0");
    set_cfg("Sound\\RXMusic", chk_game_music->isChecked() ? "1" : "0");
    set_cfg("Sound\\FESamples", chk_front_sfx->isChecked() ? "1" : "0");
    set_cfg("Sound\\FEMusic", chk_front_music->isChecked() ? "1" : "0");

    // Keyboard Page
    for (int b = 1; b <= 3; ++b) {
        for (int i = 1; i <= 12; ++i) {
            std::string key = "Kbd" + std::to_string(b) + "Key" + std::to_string(i);
            auto it = key_codes.find(key);
            if (it != key_codes.end()) {
                set_cfg("Controls\\" + key, std::to_string(it->second));
            }
        }
    }

    // Network Page
    set_cfg("Network\\Active", chk_net_active->isChecked() ? "1" : "0");
    set_cfg("Network\\LocalName", inp_hostname->text().toStdString());
    set_cfg("Network\\MasterServerAddress", inp_master->text().toStdString());

    std::vector<std::string> hosts;
    for (int i = 0; i < list_hosts->count(); ++i) {
        hosts.push_back(list_hosts->item(i)->text().toStdString());
    }
    std::string hosts_str = "";
    for (size_t i = 0; i < hosts.size(); ++i) {
        hosts_str += hosts[i] + (i + 1 < hosts.size() ? ";" : "");
    }
    set_cfg("Network\\Hosts", hosts_str);

    launcher->saveConfig();
    accept();
}

QWidget* OptionsDialog::createProgramPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);

    // Language Group
    Win3DGroupBox *lang_group = new Win3DGroupBox("Language", page);
    QVBoxLayout *lang_layout = new QVBoxLayout(lang_group);
    lang_layout->setContentsMargins(8, 23, 8, 8);
    lang_layout->setSpacing(4);
    radio_german = new QRadioButton("German", lang_group);
    radio_english = new QRadioButton("English", lang_group);
    
    if (launcher->getLanguage() == "DE") {
        radio_german->setChecked(true);
    } else {
        radio_english->setChecked(true);
    }
    lang_layout->addWidget(radio_german);
    lang_layout->addWidget(radio_english);
    layout->addWidget(lang_group);

    // Font Group
    Win3DGroupBox *font_group = new Win3DGroupBox("Font", page);
    QVBoxLayout *font_layout = new QVBoxLayout(font_group);
    font_layout->setContentsMargins(8, 23, 8, 8);
    font_layout->setSpacing(8);

    combo_font = new QComboBox(font_group);
    QString font_name = QString::fromStdString(launcher->get_cfg("General\\FEFontName", launcher->getComicFontFamily().toStdString()));
    combo_font->addItem(font_name);
    combo_font->setFixedWidth(220);
    font_layout->addWidget(combo_font);

    QHBoxLayout *spin_container = new QHBoxLayout();
    spin_container->setSpacing(15);

    QVBoxLayout *menus_layout = new QVBoxLayout();
    menus_layout->setSpacing(2);
    menus_layout->addWidget(new QLabel("Menus", font_group));
    spin_menus = new QSpinBox(font_group);
    spin_menus->setValue(std::stoi(launcher->get_cfg("General\\FEFontSize", "9")));
    spin_menus->setFixedWidth(45);
    menus_layout->addWidget(spin_menus);

    QVBoxLayout *game_layout = new QVBoxLayout();
    game_layout->setSpacing(2);
    game_layout->addWidget(new QLabel("Game", font_group));
    spin_game = new QSpinBox(font_group);
    spin_game->setValue(std::stoi(launcher->get_cfg("General\\RXFontSize", "10")));
    spin_game->setFixedWidth(45);
    game_layout->addWidget(spin_game);

    spin_container->addLayout(menus_layout);
    spin_container->addLayout(game_layout);
    spin_container->addStretch();
    font_layout->addLayout(spin_container);
    layout->addWidget(font_group);

    // Frontend Group
    Win3DGroupBox *front_group = new Win3DGroupBox("Frontend", page);
    QVBoxLayout *front_layout = new QVBoxLayout(front_group);
    front_layout->setContentsMargins(8, 16, 8, 8);
    check_quick = new QCheckBox("Display quick start screen", front_group);
    check_quick->setChecked(launcher->get_cfg("Explorer\\ShowQuickStart", "1") == "1");
    front_layout->addWidget(check_quick);
    layout->addWidget(front_group);

    // Mode Group
    Win3DGroupBox *mode_group = new Win3DGroupBox("Mode", page);
    QVBoxLayout *mode_layout = new QVBoxLayout(mode_group);
    mode_layout->setContentsMargins(8, 16, 8, 8);
    check_dev = new QCheckBox("Enable developer mode", mode_group);
    check_dev->setChecked(launcher->get_cfg("Developer\\Active", "0") == "1");
    mode_layout->addWidget(check_dev);
    layout->addWidget(mode_group);

    layout->addStretch();
    return page;
}

QWidget* OptionsDialog::createGraphicsPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);

    QHBoxLayout *top_row = new QHBoxLayout();
    top_row->setSpacing(8);

    // Resolution
    Win3DGroupBox *res_group = new Win3DGroupBox("Resolution", page);
    QVBoxLayout *res_layout = new QVBoxLayout(res_group);
    res_layout->setContentsMargins(8, 16, 8, 8);
    res_layout->setSpacing(2);

    radio_640 = new QRadioButton("640x480", res_group);
    radio_800 = new QRadioButton("800x600", res_group);
    radio_1024 = new QRadioButton("1024x768", res_group);

    int res_val = std::stoi(launcher->get_cfg("Graphics\\Resolution", "1"));
    if (res_val == 0) radio_640->setChecked(true);
    else if (res_val == 1) radio_800->setChecked(true);
    else radio_1024->setChecked(true);

    res_layout->addWidget(radio_640);
    res_layout->addWidget(radio_800);
    res_layout->addWidget(radio_1024);
    top_row->addWidget(res_group);

    // Smoke level
    Win3DGroupBox *smoke_group = new Win3DGroupBox("Smoke level", page);
    QVBoxLayout *smoke_layout = new QVBoxLayout(smoke_group);
    smoke_layout->setContentsMargins(8, 16, 8, 8);
    smoke_layout->setSpacing(2);

    slider_smoke = new QSlider(Qt::Horizontal, smoke_group);
    slider_smoke->setRange(0, 100);
    slider_smoke->setValue(std::stoi(launcher->get_cfg("Graphics\\SmokeLevel", "200")) / 2);
    smoke_layout->addWidget(slider_smoke);

    QHBoxLayout *labels_layout = new QHBoxLayout();
    labels_layout->addWidget(new QLabel("low", smoke_group));
    labels_layout->addStretch();
    labels_layout->addWidget(new QLabel("high", smoke_group));
    smoke_layout->addLayout(labels_layout);
    smoke_layout->addStretch();
    top_row->addWidget(smoke_group, 1);

    layout->addLayout(top_row);

    // Display
    Win3DGroupBox *disp_group = new Win3DGroupBox("Display", page);
    QVBoxLayout *disp_layout = new QVBoxLayout(disp_group);
    disp_layout->setContentsMargins(8, 16, 8, 8);
    disp_layout->setSpacing(2);

    chk_viewport = new QCheckBox("Viewport delimiters", disp_group);
    chk_viewport->setChecked(launcher->get_cfg("Graphics\\SplitscreenDividers", "1") == "1");

    chk_ext_info = new QCheckBox("Extended player info", disp_group);
    chk_ext_info->setChecked(launcher->get_cfg("Graphics\\ShowPlayerInfoAlways", "1") == "1");

    chk_color_anim = new QCheckBox("Color animation", disp_group);
    chk_color_anim->setChecked(launcher->get_cfg("Graphics\\ColorAnimation", "1") == "1");

    chk_startup = new QCheckBox("Startup messages", disp_group);
    chk_startup->setChecked(launcher->get_cfg("Graphics\\ShowStartupMessages", "1") == "1");

    chk_obj_cmds = new QCheckBox("Object commands", disp_group);
    chk_obj_cmds->setChecked(launcher->get_cfg("Graphics\\ShowCommands", "1") == "1");

    chk_portraits = new QCheckBox("Portraits", disp_group);
    chk_portraits->setChecked(launcher->get_cfg("Graphics\\ShowPortraits", "0") == "1");

    disp_layout->addWidget(chk_viewport);
    disp_layout->addWidget(chk_ext_info);
    disp_layout->addWidget(chk_color_anim);
    disp_layout->addWidget(chk_startup);
    disp_layout->addWidget(chk_obj_cmds);
    disp_layout->addWidget(chk_portraits);
    layout->addWidget(disp_group);

    // Troubleshooting
    Win3DGroupBox *trouble_group = new Win3DGroupBox("Troubleshooting", page);
    QVBoxLayout *trouble_layout = new QVBoxLayout(trouble_group);
    trouble_layout->setContentsMargins(8, 16, 8, 8);

    chk_ddraw = new QCheckBox("DirectDraw software emulation", trouble_group);
    chk_ddraw->setChecked(launcher->get_cfg("Graphics\\DDrawAccel", "0") == "1");
    trouble_layout->addWidget(chk_ddraw);
    layout->addWidget(trouble_group);

    layout->addStretch();
    return page;
}

QWidget* OptionsDialog::createSoundPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);

    // Game
    Win3DGroupBox *game_group = new Win3DGroupBox("Game", page);
    QVBoxLayout *game_layout = new QVBoxLayout(game_group);
    game_layout->setContentsMargins(8, 16, 8, 8);
    game_layout->setSpacing(4);

    chk_game_sfx = new QCheckBox("Sound effects", game_group);
    chk_game_sfx->setChecked(launcher->get_cfg("Sound\\RXSound", "1") == "1");

    chk_game_loops = new QCheckBox("Sound loops", game_group);
    chk_game_loops->setChecked(launcher->get_cfg("Sound\\RXSoundLoops", "1") == "1");

    chk_game_music = new QCheckBox("Music", game_group);
    chk_game_music->setChecked(launcher->get_cfg("Sound\\RXMusic", "1") == "1");

    game_layout->addWidget(chk_game_sfx);
    game_layout->addWidget(chk_game_loops);
    game_layout->addWidget(chk_game_music);
    layout->addWidget(game_group);

    // Frontend
    Win3DGroupBox *front_group = new Win3DGroupBox("Frontend", page);
    QVBoxLayout *front_layout = new QVBoxLayout(front_group);
    front_layout->setContentsMargins(8, 16, 8, 8);
    front_layout->setSpacing(4);

    chk_front_sfx = new QCheckBox("Sound effects", front_group);
    chk_front_sfx->setChecked(launcher->get_cfg("Sound\\FESamples", "1") == "1");

    chk_front_music = new QCheckBox("Music", front_group);
    chk_front_music->setChecked(launcher->get_cfg("Sound\\FEMusic", "1") == "1");

    front_layout->addWidget(chk_front_sfx);
    front_layout->addWidget(chk_front_music);
    layout->addWidget(front_group);

    // Volume
    Win3DGroupBox *vol_group = new Win3DGroupBox("Volume", page);
    QHBoxLayout *vol_layout = new QHBoxLayout(vol_group);
    vol_layout->setContentsMargins(8, 16, 8, 12);

    btn_volume = new Win3DButton("Volume control", vol_group);
    btn_volume->setFixedSize(160, 23);
    vol_layout->addStretch();
    vol_layout->addWidget(btn_volume);
    vol_layout->addStretch();
    layout->addWidget(vol_group);

    layout->addStretch();
    return page;
}

QWidget* OptionsDialog::createKeyboardPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(8, 14, 8, 8);
    layout->setSpacing(12);

    current_block = 1;

    QHBoxLayout *top_row = new QHBoxLayout();
    top_row->addWidget(new QLabel("Keyboard", page));
    top_row->addSpacing(15);

    radio_block1 = new QRadioButton("Block 1", page);
    radio_block2 = new QRadioButton("Block 2", page);
    radio_block3 = new QRadioButton("Block 3", page);
    radio_block4 = new QRadioButton("Block 4", page);

    radio_block1->setChecked(true);

    connect(radio_block1, &QRadioButton::toggled, this, [this](bool checked) { if (checked) loadKeyProfile(1); });
    connect(radio_block2, &QRadioButton::toggled, this, [this](bool checked) { if (checked) loadKeyProfile(2); });
    connect(radio_block3, &QRadioButton::toggled, this, [this](bool checked) { if (checked) loadKeyProfile(3); });
    connect(radio_block4, &QRadioButton::toggled, this, [this](bool checked) { if (checked) loadKeyProfile(4); });

    top_row->addWidget(radio_block1);
    top_row->addWidget(radio_block2);
    top_row->addWidget(radio_block3);
    top_row->addWidget(radio_block4);
    top_row->addStretch();
    layout->addLayout(top_row);

    QFrame *separator = new QFrame(page);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separator);

    // Keyboard Atlas
    QString atlas_path = QDir(launcher->getDumpPath()).filePath("Planet_fixed.bin_2_1008_1031.bmp");
    QPixmap kb_atlas = applyClonkTransparency(QPixmap(atlas_path));

    auto get_kb_icon = [&](int index) -> QPixmap {
        if (kb_atlas.isNull()) return QPixmap(32, 32);
        return kb_atlas.copy(QRect(index * 32, 0, 32, 32));
    };

    QGridLayout *grid = new QGridLayout();
    grid->setHorizontalSpacing(15);
    grid->setVerticalSpacing(6);

    std::vector<std::tuple<QString, std::string, int, int>> keybinds_info = {
        {"Select left",   "Key1",  0, 0}, {"Select toggle", "Key2",  0, 1}, {"Select right", "Key3",  0, 2},
        {"Throw",         "Key4",  1, 0}, {"Up / jump",     "Key5",  1, 1}, {"Dig",           "Key6",  1, 2},
        {"Left",          "Key7",  2, 0}, {"Down / stop",   "Key8",  2, 1}, {"Right",         "Key9",  2, 2},
        {"Player menu",   "Key10", 3, 0}, {"Special 1",    "Key11", 3, 1}, {"Special 2",    "Key12", 3, 2},
    };

    for (const auto &tpl : keybinds_info) {
        QString label_text = std::get<0>(tpl);
        std::string key_id = std::get<1>(tpl);
        int row = std::get<2>(tpl);
        int col = std::get<3>(tpl);

        QHBoxLayout *item_layout = new QHBoxLayout();
        item_layout->setSpacing(6);

        QLabel *icon_label = new QLabel(page);
        icon_label->setPixmap(get_kb_icon(std::stoi(key_id.substr(3)) - 1));
        item_layout->addWidget(icon_label);

        QVBoxLayout *text_layout = new QVBoxLayout();
        text_layout->setSpacing(1);

        QLabel *lbl = new QLabel(label_text, page);
        
        int code = key_codes["Kbd1" + key_id];
        KeyBindInput *inp = new KeyBindInput(QString::fromStdString(key_id), code, page);
        key_inputs[key_id] = inp;
        connect(inp, &KeyBindInput::keyChanged, this, &OptionsDialog::onKeyInputChanged);

        text_layout->addWidget(lbl);
        text_layout->addWidget(inp);
        text_layout->addStretch();

        item_layout->addLayout(text_layout);
        grid->addLayout(item_layout, row, col);
    }

    layout->addLayout(grid);
    layout->addStretch();

    QHBoxLayout *reset_layout = new QHBoxLayout();
    btn_reset = new Win3DButton("Reset all", page);
    btn_reset->setFixedSize(100, 23);
    connect(btn_reset, &QPushButton::clicked, this, &OptionsDialog::promptResetControls);

    reset_layout->addStretch();
    reset_layout->addWidget(btn_reset);
    reset_layout->addStretch();
    layout->addLayout(reset_layout);

    loadKeyProfile(1);
    return page;
}

QWidget* OptionsDialog::createNetworkPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(8, 14, 8, 8);
    layout->setSpacing(7);

    QHBoxLayout *top_row = new QHBoxLayout();
    chk_net_active = new QCheckBox("Active", page);
    chk_net_active->setChecked(launcher->get_cfg("Network\\Active", "0") == "1");
    top_row->addWidget(chk_net_active);
    top_row->addStretch();

    btn_ip_config = new Win3DButton("IP configuration", page);
    btn_ip_config->setFixedSize(110, 23);
    top_row->addWidget(btn_ip_config);
    layout->addLayout(top_row);

    // Local computer
    local_group = new Win3DGroupBox("Local computer", page);
    QVBoxLayout *local_layout = new QVBoxLayout(local_group);
    local_layout->setContentsMargins(8, 16, 8, 8);
    local_layout->setSpacing(4);

    local_layout->addWidget(new QLabel("Name", local_group));
    
    // Qt host name
    QString sys_hostname = QHostInfo::localHostName().toUpper();
    if (sys_hostname.isEmpty()) sys_hostname = "CLONK-PC";

    inp_hostname = new QLineEdit(QString::fromStdString(launcher->get_cfg("Network\\LocalName", sys_hostname.toStdString())), local_group);
    local_layout->addWidget(inp_hostname);
    local_layout->addWidget(new QLabel("Address: " + sys_hostname.toLower(), local_group));
    layout->addWidget(local_group);

    // Network hosts
    hosts_group = new Win3DGroupBox("Network hosts", page);
    QHBoxLayout *hosts_layout = new QHBoxLayout(hosts_group);
    hosts_layout->setContentsMargins(8, 16, 8, 8);

    list_hosts = new QListWidget(hosts_group);
    std::string hosts_str = launcher->get_cfg("Network\\Hosts", "");
    if (!hosts_str.empty()) {
        std::stringstream ss(hosts_str);
        std::string host;
        while (std::getline(ss, host, ';')) {
            if (!host.empty()) {
                list_hosts->addItem(QString::fromStdString(host));
            }
        }
    }
    hosts_layout->addWidget(list_hosts);

    QVBoxLayout *hosts_btn_layout = new QVBoxLayout();
    hosts_btn_layout->setSpacing(4);

    btn_host_new = new Win3DButton("New", hosts_group);
    btn_host_del = new Win3DButton("Delete", hosts_group);
    btn_host_edit = new Win3DButton("Edit", hosts_group);

    btn_host_new->setFixedSize(65, 23);
    btn_host_del->setFixedSize(65, 23);
    btn_host_edit->setFixedSize(65, 23);

    connect(btn_host_new, &QPushButton::clicked, this, &OptionsDialog::addHost);
    connect(btn_host_del, &QPushButton::clicked, this, &OptionsDialog::deleteHost);
    connect(btn_host_edit, &QPushButton::clicked, this, &OptionsDialog::editHost);

    hosts_btn_layout->addWidget(btn_host_new);
    hosts_btn_layout->addWidget(btn_host_del);
    hosts_btn_layout->addWidget(btn_host_edit);
    hosts_btn_layout->addStretch();

    hosts_layout->addLayout(hosts_btn_layout);
    layout->addWidget(hosts_group);

    // Internet Master
    master_group = new Win3DGroupBox("Internet master server", page);
    QVBoxLayout *master_layout = new QVBoxLayout(master_group);
    master_layout->setContentsMargins(8, 16, 8, 8);

    inp_master = new QLineEdit(QString::fromStdString(launcher->get_cfg("Network\\MasterServerAddress", "www.clonk.de")), master_group);
    master_layout->addWidget(inp_master);
    layout->addWidget(master_group);

    layout->addStretch();

    connect(chk_net_active, &QCheckBox::toggled, this, &OptionsDialog::updateNetworkState);
    updateNetworkState(chk_net_active->isChecked());

    return page;
}

void OptionsDialog::updateNetworkState(bool is_active) {
    btn_ip_config->setEnabled(is_active);
    local_group->setEnabled(is_active);
    hosts_group->setEnabled(is_active);
    master_group->setEnabled(is_active);
}

void OptionsDialog::addHost() {
    HostDialog dlg(this, "New host address");
    if (dlg.exec() == QDialog::Accepted) {
        QString addr = dlg.getAddress().trimmed();
        if (!addr.isEmpty()) {
            list_hosts->addItem(addr);
        }
    }
}

void OptionsDialog::editHost() {
    QListWidgetItem *curr = list_hosts->currentItem();
    if (curr) {
        HostDialog dlg(this, "Edit host address", curr->text());
        if (dlg.exec() == QDialog::Accepted) {
            QString addr = dlg.getAddress().trimmed();
            if (!addr.isEmpty()) {
                curr->setText(addr);
            }
        }
    }
}

void OptionsDialog::deleteHost() {
    int curr_row = list_hosts->currentRow();
    if (curr_row >= 0) {
        delete list_hosts->takeItem(curr_row);
    }
}
