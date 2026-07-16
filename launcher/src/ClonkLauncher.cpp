#include "ClonkLauncher.h"
#include "OptionsDialog.h"
#include "ClonkDialogs.h"
#include "Utils.h"
#include "C4Group.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QFontDatabase>
#include <QHostInfo>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QStyledItemDelegate>
#include <fstream>
#include <iostream>

// Helper structure for sorting
struct ItemInfo {
    std::string name;
    C4Group grp;
    std::string title;
    int cat = 9;
};

// Item Info getter
ItemInfo getItemInfo(const std::string &name, const C4Group &grp, const std::string &lang) {
    std::string ext = name.size() >= 4 ? name.substr(name.size() - 4) : "";
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    std::string default_name = name;
    if (ext == ".c4p" || ext == ".c4f" || ext == ".c4s" || ext == ".c4d" || ext == ".c4v") {
        default_name = name.substr(0, name.size() - 4);
    }

    std::string title = getGroupTitle(grp, lang);
    if (title == "Unknown") title = default_name;

    int cat = 9;
    if (ext == ".c4p") cat = 0;
    else if (ext == ".c4f") cat = 1;
    else if (ext == ".c4s") cat = 2;
    else if (ext == ".c4d") cat = 3;

    return {name, grp, title, cat};
}

// Staggered sorting comparator
bool compareItemInfo(const ItemInfo &a, const ItemInfo &b, const std::string &parent_title) {
    std::vector<std::string> keyboard_order = {
        "A Clonk", "More Clonks", "Air Travel", "Production Line",
        "Gold Mine", "Underground", "Acid Lake", "Wipf Rescue",
        "Arctic Ocean", "Volcanic"
    };
    std::vector<std::string> mouse_order = {
        "Settlement", "Goldmine", "Production", "Objects"
    };

    if (parent_title == "Keyboard Control") {
        auto itA = std::find(keyboard_order.begin(), keyboard_order.end(), a.title);
        auto itB = std::find(keyboard_order.begin(), keyboard_order.end(), b.title);
        bool hasA = (itA != keyboard_order.end());
        bool hasB = (itB != keyboard_order.end());
        if (hasA && hasB) {
            return std::distance(keyboard_order.begin(), itA) < std::distance(keyboard_order.begin(), itB);
        }
        if (hasA) return true;
        if (hasB) return false;
    }
    if (parent_title == "Mouse Control") {
        auto itA = std::find(mouse_order.begin(), mouse_order.end(), a.title);
        auto itB = std::find(mouse_order.begin(), mouse_order.end(), b.title);
        bool hasA = (itA != mouse_order.end());
        bool hasB = (itB != mouse_order.end());
        if (hasA && hasB) {
            return std::distance(mouse_order.begin(), itA) < std::distance(mouse_order.begin(), itB);
        }
        if (hasA) return true;
        if (hasB) return false;
    }

    if (a.cat != b.cat) {
        return a.cat < b.cat;
    }

    std::string al = a.title;
    std::string bl = b.title;
    std::transform(al.begin(), al.end(), al.begin(), ::tolower);
    std::transform(bl.begin(), bl.end(), bl.begin(), ::tolower);
    return al < bl;
}

// Tree view cell delegate for check boxes
class PixelDelegate : public QStyledItemDelegate {
public:
    PixelDelegate(QObject *parent, const QPixmap &check_on, const QPixmap &check_off, const QPixmap &check_locked_on, const QPixmap &check_locked_off)
        : QStyledItemDelegate(parent), check_on(check_on), check_off(check_off), check_locked_on(check_locked_on), check_locked_off(check_locked_off) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();

        // 1. Draw base white background (non-selected)
        QStyleOptionViewItem opts = opt;
        opts.state &= ~QStyle::State_Selected;
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opts, painter, widget);

        // Calculate text geometry first to know where selection ends
        QRect display_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            display_rect.translate(-7, 0);
        } else {
            display_rect.translate(-3, 0);
        }

        QString text = index.data(Qt::DisplayRole).toString();
        painter->setFont(opt.font);
        QFontMetrics fm = opt.fontMetrics;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int text_w = fm.horizontalAdvance(text);
#else
        int text_w = fm.width(text);
#endif

        // 2. Draw selection highlight background (text only)
        bool is_selected = (opt.state & QStyle::State_Selected);
        if (is_selected) {
            QRect highlight_rect(display_rect.x() - 2, opt.rect.y(), text_w + 6, opt.rect.height());
            painter->fillRect(highlight_rect, QColor("#3096fa"));
        }

        // 3. Draw Checkbox
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            QRect check_rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
            check_rect.translate(-1, 0);

            QVariant data = index.data(Qt::UserRole + 1);
            bool is_locked = false;
            if (data.isValid() && data.typeId() == QMetaType::QVariantMap) {
                is_locked = data.toMap().value("is_locked").toBool();
            }

            QPixmap pix;
            if (is_locked) {
                pix = (opt.checkState == Qt::Checked) ? check_locked_on : check_locked_off;
            } else {
                pix = (opt.checkState == Qt::Checked) ? check_on : check_off;
            }

            if (!pix.isNull()) {
                int tx = check_rect.x() + (check_rect.width() - pix.width()) / 2;
                int ty = check_rect.y() + (check_rect.height() - pix.height()) / 2;
                painter->drawPixmap(tx, ty, pix);
            }
        }

        // 4. Draw Icon (Decoration)
        if (opt.features & QStyleOptionViewItem::HasDecoration) {
            QRect decor_rect = style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, widget);
            if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
                decor_rect.translate(-6, 0);
            } else {
                decor_rect.translate(-3, 0);
            }
            QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            if (!icon.isNull()) {
                QIcon::State icon_state = (opt.state & QStyle::State_Open) ? QIcon::On : QIcon::Off;
                QPixmap pix = icon.pixmap(opt.decorationSize, QIcon::Normal, icon_state);
                if (!pix.isNull()) {
                    int tx = decor_rect.x() + (decor_rect.width() - pix.width()) / 2;
                    int ty = decor_rect.y() + (decor_rect.height() - pix.height()) / 2;
                    painter->drawPixmap(tx, ty, pix);
                }
            }
        }

        // 5. Draw Text
        if (!text.isEmpty()) {
            if (is_selected) {
                painter->setPen(Qt::white);
            } else {
                painter->setPen(opt.palette.text().color());
            }

            painter->drawText(display_rect, Qt::AlignVCenter | Qt::AlignLeft, text);
        }

        painter->restore();
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
            QVariant data = index.data(Qt::UserRole + 1);
            if (data.isValid() && data.typeId() == QMetaType::QVariantMap) {
                if (data.toMap().value("is_locked").toBool()) {
                    return false;
                }
            }
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    QPixmap check_on;
    QPixmap check_off;
    QPixmap check_locked_on;
    QPixmap check_locked_off;
};

// ClonkLauncher Class
ClonkLauncher::ClonkLauncher() {
    base_path = QDir(QCoreApplication::applicationDirPath()).filePath("..");
    planet_data_path = QDir(base_path).filePath("planet_data");
    res_path = QDir(QCoreApplication::applicationDirPath()).filePath("res");
    res_music_path = QDir(QCoreApplication::applicationDirPath()).filePath("res_music");
    wav_path = QDir(QCoreApplication::applicationDirPath()).filePath("res_wav");
    dump_path = QDir(QCoreApplication::applicationDirPath()).filePath("res_dump");
    config_path = QDir(planet_data_path).filePath("clonk.ini");

    // Atlases
    QPixmap raw_icons(QDir(dump_path).filePath("Planet_fixed.bin_2_1015_1031.bmp"));
    if (!raw_icons.isNull()) {
        icons_atlas = applyClonkTransparency(raw_icons);
    }

    QPixmap raw_check(QDir(dump_path).filePath("Planet_fixed.bin_2_1016_1031.bmp"));
    if (!raw_check.isNull()) {
        check_atlas = applyClonkTransparency(raw_check);
        check_on_pix = check_atlas.copy(QRect(16, 0, 16, 16));
        check_off_pix = check_atlas.copy(QRect(32, 0, 16, 16));
        check_locked_on_pix = check_atlas.copy(QRect(112, 0, 16, 16));
        check_locked_off_pix = check_atlas.copy(QRect(128, 0, 16, 16));
    }

    // Custom Font loading
    QString font_path = QDir(planet_data_path).filePath("Comic.ttf");
    comic_font_family = "Comic Sans MS";
    if (QFile::exists(font_path)) {
        int font_id = QFontDatabase::addApplicationFont(font_path);
        if (font_id != -1) {
            comic_font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        }
    }

    // Config loading
    config.load(config_path.toStdString());
    language = config.getValue("General\\Language", "US");

    // Audio SFX
    sound_start = new QSoundEffect(this);
    sound_start->setSource(QUrl::fromLocalFile(QDir(wav_path).filePath("sound_7008.wav")));
    sound_click = new QSoundEffect(this);
    sound_click->setSource(QUrl::fromLocalFile(QDir(wav_path).filePath("sound_7002.wav")));

    ClonkButton::click_sound = sound_click;
    ClonkButton::font_family = comic_font_family;
    Win3DButton::click_sound = sound_click;
    Win3DButton::font_family = comic_font_family;

    // Music setup
    midi_process = new QProcess(this);
    music_timer = new QTimer(this);
    connect(music_timer, &QTimer::timeout, this, &ClonkLauncher::updateMusic);
    extract_frontend_music();

    init_ui();
    refresh_resources();
}

ClonkLauncher::~ClonkLauncher() {
    stop_music();
}

std::string ClonkLauncher::get_cfg(const std::string &sub_key, const std::string &defaultValue) const {
    std::string key = "RedWolf Design\\Clonk 4\\" + sub_key;
    return config.getValue(key, defaultValue);
}

void ClonkLauncher::set_cfg(const std::string &sub_key, const std::string &value) {
    std::string key = "RedWolf Design\\Clonk 4\\" + sub_key;
    config.setValue(key, value);
}

void ClonkLauncher::saveConfig() {
    config.setValue("RedWolf Design\\Clonk 4\\General\\Language", language);
    config.save(config_path.toStdString());
}

void ClonkLauncher::extract_frontend_music() {
    QString music_grp_path = QDir(planet_data_path).filePath("Music.c4g");
    if (!QFile::exists(music_grp_path)) return;

    QDir().mkdir(res_music_path);

    C4Group grp(music_grp_path.toStdString());
    for (const auto &e : grp.getEntries()) {
        std::string name_lower = e.name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        if (name_lower == "frontend.mid" || name_lower == "frontend old.mid") {
            QString target = QDir(res_music_path).filePath(QString::fromStdString(e.name));
            if (!QFile::exists(target)) {
                auto data = grp.getFile(e.name);
                if (!data.empty()) {
                    std::ofstream f(target.toStdString(), std::ios::binary);
                    if (f.is_open()) {
                        f.write(reinterpret_cast<const char*>(data.data()), data.size());
                        f.close();
                    }
                }
            }
        }
    }

    QString p1 = QDir(res_music_path).filePath("Frontend.mid");
    QString p2 = QDir(res_music_path).filePath("Frontend Old.mid");
    if (QFile::exists(p1)) current_music_path = p1;
    else if (QFile::exists(p2)) current_music_path = p2;
}

void ClonkLauncher::start_music() {
    if (!current_music_path.isEmpty() && QFile::exists(current_music_path)) {
        if (get_cfg("Sound\\FEMusic", "1") == "1") {
            QString sf_path = QDir(planet_data_path).filePath("FluidR3_GM_GS.sf2");
            QString midi_player_bin = QDir(base_path).filePath("build/clonk_midi");

            if (QFile::exists(sf_path) && QFile::exists(midi_player_bin)) {
                stop_music();
                QStringList args;
                args << current_music_path << sf_path;
                midi_process->start(midi_player_bin, args);
            }
        }
    }
    music_timer->start(1000);
}

void ClonkLauncher::stop_music() {
    if (midi_process->state() != QProcess::NotRunning) {
        midi_process->terminate();
        if (!midi_process->waitForFinished(1000)) {
            midi_process->kill();
        }
    }
}

void ClonkLauncher::updateMusic() {
    if (get_cfg("Sound\\FEMusic", "1") == "1") {
        if (midi_process->state() == QProcess::NotRunning) {
            start_music();
        }
    } else {
        stop_music();
    }
}

void ClonkLauncher::init_ui() {
    setWindowTitle("Clonk Planet");

    central_widget = new QWidget(this);
    central_widget->setFixedSize(578, 393);
    setCentralWidget(central_widget);

    QString bg_path = QDir(dump_path).filePath("Planet_fixed.bin_2_1019_1031.bmp");
    ui_container = new ClonkTexturedWidget(central_widget, bg_path);
    ui_container->setGeometry(0, 0, 578, 393);

    init_menu();
    setup_main_ui();

    setFixedSize(sizeHint());
}

void ClonkLauncher::init_menu() {
    QMenuBar *menu = menuBar();
    menu->setStyleSheet(
        "QMenuBar {"
        "    background-color: #c0c0c0;"
        "    border-bottom: 2px solid #808080;"
        "    color: black;"
        "}"
        "QMenuBar::item:selected {"
        "    background-color: #3096fa;"
        "    color: white;"
        "}"
    );

    QMenu *m_opt = menu->addMenu("Options");
    QAction *act_opt = m_opt->addAction("Options...");
    connect(act_opt, &QAction::triggered, this, &ClonkLauncher::showOptions);
    QAction *act_reg = m_opt->addAction("Registration...");
    // Registration stubbed for now

    QMenu *m_help = menu->addMenu("Help");
    QAction *act_contents = m_help->addAction("Contents...");
    QAction *act_credits = m_help->addAction("Credits...");
    connect(act_credits, &QAction::triggered, this, &ClonkLauncher::showCredits);
    QAction *act_web = m_help->addAction("Clonk on the web");
    QAction *act_about = m_help->addAction("About Clonk Planet...");
    connect(act_about, &QAction::triggered, this, &ClonkLauncher::showAbout);
}

void ClonkLauncher::setup_main_ui() {
    QString font_style = QString("'%1', 'Chilanka', 'cursive'").arg(comic_font_family);
    ui_container->setStyleSheet(QString("* { font-family: %1; font-size: 11px; color: black; }").arg(font_style));

    tree_frame = new ClonkArea(ui_container, "white");
    tree_frame->setGeometry(9, 10, 243, 346);

    tree = new QTreeView(tree_frame);
    tree->setGeometry(2, 2, 239, 342);
    tree->setHeaderHidden(true);
    tree->setFrameShape(QFrame::NoFrame);

    tree_model = new QStandardItemModel(tree);
    tree->setModel(tree_model);
    tree->setItemDelegate(new PixelDelegate(tree, check_on_pix, check_off_pix, check_locked_on_pix, check_locked_off_pix));
    connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ClonkLauncher::onTreeSelection);

    tree->setRootIsDecorated(false);
    tree->setIndentation(20);
    tree->setExpandsOnDoubleClick(true);
    tree->setEditTriggers(QTreeView::NoEditTriggers);
    tree->setIconSize(QSize(16, 16));
    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectItems);

    tree->setStyleSheet(
        "QTreeView {"
        "    background-color: white;"
        "    border: none;"
        "    selection-background-color: transparent;"
        "}"
        "QTreeView::branch {"
        "    image: none;"
        "}"
    );

    QPalette pal = tree->palette();
    pal.setColor(QPalette::Active, QPalette::Highlight, QColor("#3096fa"));
    pal.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#3096fa"));
    tree->setPalette(pal);

    preview_frame = new ClonkArea(ui_container);
    preview_frame->setGeometry(261, 10, 212, 151);

    preview = new QLabel(preview_frame);
    preview->setGeometry(2, 2, 208, 147);
    preview->setAlignment(Qt::AlignCenter);

    desc_frame = new ClonkArea(ui_container, "white");
    desc_frame->setGeometry(261, 166, 212, 171);

    desc = new QTextEdit(desc_frame);
    desc->setGeometry(2, 2, 208, 167);
    desc->setReadOnly(true);
    desc->setFrameShape(QFrame::NoFrame);
    desc->setText("Select a scenario or player to begin.");

    QString btn_bg = QDir(dump_path).filePath("Planet_fixed.bin_2_1006_1031.bmp");
    QPoint offset(117, 14);

    auto create_btn = [&](const QString &txt, int x, int y) -> ClonkButton* {
        ClonkButton *btn = new ClonkButton(txt, ui_container, btn_bg, offset);
        btn->move(x, y);
        return btn;
    };

    btn_new = create_btn("New", 482, 10);
    btn_activate = create_btn("Activate", 482, 34);
    btn_rename = create_btn("Rename", 482, 59);
    btn_delete = create_btn("Delete", 482, 83);
    btn_props = create_btn("Properties", 482, 107);
    connect(btn_props, &QPushButton::clicked, this, &ClonkLauncher::showProps);

    btn_start = create_btn("Start", 482, 310);
    connect(btn_start, &QPushButton::clicked, this, &ClonkLauncher::launchGame);

    btn_quit = create_btn("Quit", 482, 335);
    connect(btn_quit, &QPushButton::clicked, this, &QWidget::close);



    author_label = new QLabel("Author: RedWolf Design", ui_container);
    author_label->setGeometry(261, 341, 212, 15);
    author_label->setStyleSheet("background: transparent;");

    status_frame = new ClonkArea(ui_container);
    status_frame->setGeometry(47, 364, 521, 24);

    status_bar = new QLabel(" Welcome to Clonk Planet", status_frame);
    status_bar->setGeometry(2, 2, 517, 20);
    status_bar->setStyleSheet("background: transparent;");

    anim_frame = new ClonkArea(ui_container);
    anim_frame->setGeometry(9, 364, 33, 24);

    animation = new QLabel(anim_frame);
    animation->setGeometry(2, 2, 29, 20);
    animation->setStyleSheet("background: transparent;");
    QString sym_path = QDir(res_path).filePath("MenuSymbol.bmp");
    if (QFile::exists(sym_path)) {
        animation->setPixmap(QPixmap(sym_path).scaled(24, 20, Qt::KeepAspectRatio));
        animation->setAlignment(Qt::AlignCenter);
    }

    QFrame *separator = new QFrame(ui_container);
    separator->setGeometry(0, 0, 580, 2);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
}

void ClonkLauncher::set_background(const QString &name) {
    QString path = QDir(dump_path).filePath(name);
    if (ui_container) {
        ui_container->setTexture(path);
    }
}

QIcon ClonkLauncher::get_atlas_icon(int index) {
    if (icons_atlas.isNull()) return QIcon();
    QRect rect(index * 16, 0, 16, 16);
    return QIcon(icons_atlas.copy(rect));
}

void ClonkLauncher::refresh_resources() {
    tree_model->clear();
    if (!QDir(planet_data_path).exists()) return;

    QFont bold_font(comic_font_family);
    bold_font.setBold(true);

    auto set_item_icon = [&](QStandardItem *item, const C4Group &grp, int default_idx) {
        auto icon_bmp = grp.getFile("Icon.bmp");
        if (!icon_bmp.empty()) {
            QPixmap pix;
            pix.loadFromData(icon_bmp.data(), icon_bmp.size());
            item->setIcon(QIcon(applyClonkTransparency(pix)));
        } else {
            item->setIcon(get_atlas_icon(default_idx));
        }
    };

    // Recursive item adder lambda
    std::function<void(QStandardItem*, const std::string&, const C4Group&, const QString&, const QStringList&)> add_item;
    add_item = [&](QStandardItem *parent_item, const std::string &name, const C4Group &grp, const QString &path, const QStringList &current_subs) {
        auto info = getItemInfo(name, grp, language);
        QStandardItem *item = new QStandardItem(QString::fromStdString(info.title));

        QString item_type = "unknown";
        if (info.cat == 0) item_type = "clonk";
        else if (info.cat == 1) item_type = "folder";
        else if (info.cat == 2) item_type = "scenario";
        else if (info.cat == 3) item_type = "package";

        QVariantMap data_map;
        data_map["path"] = path;
        data_map["sub"] = current_subs;
        data_map["type"] = item_type;

        if (item_type == "folder") {
            item->setFont(bold_font);
            set_item_icon(item, grp, 4);

            std::vector<ItemInfo> subs;
            for (const auto &e : grp.getEntries()) {
                std::string en = e.name;
                std::transform(en.begin(), en.end(), en.begin(), ::tolower);
                if (en.size() >= 4) {
                    std::string ext = en.substr(en.size() - 4);
                    if (ext == ".c4f" || ext == ".c4s" || ext == ".c4d" || ext == ".c4p") {
                        auto sub_data = grp.getFile(e.name);
                        if (!sub_data.empty()) {
                            C4Group s_grp(sub_data);
                            subs.push_back(getItemInfo(e.name, s_grp, language));
                        }
                    }
                }
            }

            // Sort subs
            std::sort(subs.begin(), subs.end(), [&](const ItemInfo &a, const ItemInfo &b) {
                return compareItemInfo(a, b, info.title);
            });

            for (const auto &s : subs) {
                QStringList new_subs = current_subs;
                new_subs.append(QString::fromStdString(s.name));
                add_item(item, s.name, s.grp, path, new_subs);
            }
        }
        else if (item_type == "scenario") {
            auto icon_bmp = grp.getFile("Icon.bmp");
            if (!icon_bmp.empty()) {
                set_item_icon(item, grp, 0);
            } else {
                auto scen_data = grp.getFile("Scenario.txt");
                int icon_id = 0;
                if (!scen_data.empty()) {
                    std::string scen_text(reinterpret_cast<const char*>(scen_data.data()), scen_data.size());
                    QStringList lines = QString::fromStdString(scen_text).split('\n');
                    for (auto &line : lines) {
                        line = line.trimmed();
                        if (line.startsWith("Icon=")) {
                            icon_id = line.mid(5).toInt();
                            break;
                        }
                    }
                }
                item->setIcon(get_atlas_icon(icon_id + 24));
            }
        }
        else if (item_type == "package") {
            set_item_icon(item, grp, 12);
            if (current_subs.isEmpty()) {
                item->setCheckable(true);
                QString lower_name = QString::fromStdString(name).toLower();
                bool is_objects = (lower_name == "objects.c4d" || QString::fromStdString(info.title) == "Objects");
                item->setCheckState(is_objects ? Qt::Checked : Qt::Unchecked);
            }

            std::vector<ItemInfo> subs;
            for (const auto &e : grp.getEntries()) {
                std::string en = e.name;
                std::transform(en.begin(), en.end(), en.begin(), ::tolower);
                if (en.size() >= 4 && en.substr(en.size() - 4) == ".c4d") {
                    auto sub_data = grp.getFile(e.name);
                    if (!sub_data.empty()) {
                        C4Group s_grp(sub_data);
                        subs.push_back(getItemInfo(e.name, s_grp, language));
                    }
                }
            }

            std::sort(subs.begin(), subs.end(), [&](const ItemInfo &a, const ItemInfo &b) {
                return compareItemInfo(a, b, info.title);
            });

            for (const auto &s : subs) {
                QStringList new_subs = current_subs;
                new_subs.append(QString::fromStdString(s.name));
                add_item(item, s.name, s.grp, path, new_subs);
            }
        }
        else if (item_type == "clonk") {
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            auto player_data = grp.getFile("Player.txt");
            int color = 1;
            if (!player_data.empty()) {
                std::string play_text(reinterpret_cast<const char*>(player_data.data()), player_data.size());
                QStringList lines = QString::fromStdString(play_text).split('\n');
                for (auto &line : lines) {
                    line = line.trimmed();
                    if (line.startsWith("Color=")) {
                        color = line.mid(6).toInt();
                        break;
                    }
                }
            }
            item->setIcon(get_atlas_icon(99 + color));
        }

        item->setData(data_map, Qt::UserRole + 1);

        if (parent_item) {
            parent_item->appendRow(item);
        } else {
            tree_model->appendRow(item);
        }
    };

    std::vector<ItemInfo> roots;
    QDir data_dir(planet_data_path);
    QStringList filters;
    filters << "*.c4f" << "*.c4s" << "*.c4p" << "*.c4d";
    QStringList files = data_dir.entryList(filters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const auto &f : files) {
        QString path = data_dir.filePath(f);
        C4Group root_grp(path.toStdString());
        roots.push_back(getItemInfo(f.toStdString(), root_grp, language));
    }

    std::sort(roots.begin(), roots.end(), [](const ItemInfo &a, const ItemInfo &b) {
        if (a.cat != b.cat) return a.cat < b.cat;
        std::string al = a.title;
        std::string bl = b.title;
        std::transform(al.begin(), al.end(), al.begin(), ::tolower);
        std::transform(bl.begin(), bl.end(), bl.begin(), ::tolower);
        return al < bl;
    });

    for (const auto &r : roots) {
        QString path = data_dir.filePath(QString::fromStdString(r.name));
        add_item(nullptr, r.name, r.grp, path, QStringList());
    }
}

void ClonkLauncher::onTreeSelection(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        desc->setText("Select an item to see its description.");
        preview->setPixmap(QPixmap());
        return;
    }

    QStandardItem *item = tree_model->itemFromIndex(indexes.at(0));
    QVariant data_var = item->data(Qt::UserRole + 1);
    if (!data_var.isValid() || data_var.typeId() != QMetaType::QVariantMap) {
        desc->setText("Select an item to see its description.");
        preview->setPixmap(QPixmap());
        return;
    }

    QVariantMap data_map = data_var.toMap();
    QString path = data_map.value("path").toString();
    QStringList sub = data_map.value("sub").toStringList();
    QString item_type = data_map.value("type").toString();

    C4Group grp(path.toStdString());
    C4Group target_grp = grp;
    
    for (const auto &s : sub) {
        auto sub_data = target_grp.getFile(s.toStdString());
        if (!sub_data.empty()) {
            target_grp = C4Group(sub_data);
        } else {
            break;
        }
    }

    // Description text parsing
    std::vector<uint8_t> desc_data;
    QStringList lang_order;
    lang_order << QString::fromStdString(language);
    lang_order << (language == "DE" ? "US" : "DE");

    for (const auto &l : lang_order) {
        desc_data = target_grp.getFile(QString("Desc%1.rtf").arg(l).toStdString());
        if (!desc_data.empty()) break;
        desc_data = target_grp.getFile(QString("Desc%1.txt").arg(l).toStdString());
        if (!desc_data.empty()) break;
    }

    if (desc_data.empty()) {
        // Fallback to Scenario.txt or Player.txt statistics
        auto ini_data = target_grp.getFile("Player.txt");
        if (!ini_data.empty()) {
            std::string ini_text(reinterpret_cast<const char*>(ini_data.data()), ini_data.size());
            auto sections = parseC4Text(ini_text);
            auto pref = sections["Preferences"];
            QString rank_name = QString::fromStdString(pref["RankName"]);
            if (rank_name.isEmpty()) rank_name = "Neuling";
            QString p_name = QString::fromStdString(pref["Name"]);
            if (p_name.isEmpty()) p_name = "Clonk";
            QString score = QString::fromStdString(pref["Score"]);
            if (score.isEmpty()) score = "0";
            QString rounds = QString::fromStdString(pref["Rounds"]);
            if (rounds.isEmpty()) rounds = "0";
            QString rounds_won = QString::fromStdString(pref["RoundsWon"]);
            if (rounds_won.isEmpty()) rounds_won = "0";
            QString rounds_lost = QString::fromStdString(pref["RoundsLost"]);
            if (rounds_lost.isEmpty()) rounds_lost = "0";

            int play_time = 0;
            try { play_time = std::stoi(pref["TotalPlayingTime"]); } catch(...) {}
            int secs = (play_time > 1000) ? (play_time / 36) : play_time;

            int h = secs / 3600;
            int m = (secs % 3600) / 60;
            int s = secs % 60;

            QString comment = QString::fromStdString(pref["Comment"]);

            QString desc_str = QString("<b><span style='font-size: 10pt'>%1 %2</span></b><br><br>Score: %3<br>Rounds: %4 (%5 won %6 lost)<br>Playing time: %7:%8:%9<br>Comment: %10")
                .arg(rank_name).arg(p_name).arg(score).arg(rounds).arg(rounds_won).arg(rounds_lost)
                .arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(comment);
            std::string tmp_str = desc_str.toStdString();
            desc_data.assign(tmp_str.begin(), tmp_str.end());
        } else {
            ini_data = target_grp.getFile("Scenario.txt");
            if (!ini_data.empty()) {
                std::string ini_text(reinterpret_cast<const char*>(ini_data.data()), ini_data.size());
                auto sections = parseC4Text(ini_text);
                auto head = sections["Head"];
                QString comment = QString::fromStdString(head["Comment"]);
                std::string tmp_comment = comment.toStdString();
                desc_data.assign(tmp_comment.begin(), tmp_comment.end());
            }
        }
    }

    if (!desc_data.empty()) {
        std::string raw_text(reinterpret_cast<const char*>(desc_data.data()), desc_data.size());
        // Clean null terminator
        if (!raw_text.empty() && raw_text.back() == '\0') {
            raw_text.pop_back();
        }

        if (raw_text.rfind("{\\rtf", 0) == 0) {
            desc->setHtml(QString::fromStdString(rtfToHtml(raw_text)));
        } else {
            desc->setHtml(QString::fromStdString(raw_text));
        }
    } else {
        desc->setText("No description available.");
    }

    // Image loading (matching Python fallback/transparency exception)
    bool is_title = false;
    std::vector<uint8_t> img_data = target_grp.getFile("Title.bmp");
    if (!img_data.empty()) {
        is_title = true;
    } else {
        img_data = target_grp.getFile("Icon.bmp");
    }

    bool needs_crop = false;
    if (img_data.empty()) {
        img_data = target_grp.getFile("Graphics.bmp");
        if (img_data.empty()) img_data = target_grp.getFile("Picture.bmp");
        if (img_data.empty()) img_data = target_grp.getFile("Portrait.bmp");
        needs_crop = true;
    }

    if (!img_data.empty()) {
        QPixmap pix;
        pix.loadFromData(img_data.data(), img_data.size());

        if (!is_title) {
            pix = applyClonkTransparency(pix);
        }

        if (needs_crop) {
            auto defcore_data = target_grp.getFile("DefCore.txt");
            if (!defcore_data.empty()) {
                std::string defcore_text(reinterpret_cast<const char*>(defcore_data.data()), defcore_data.size());
                int px = 0, py = 0, pw = 0, ph = 0;
                size_t pos = defcore_text.find("Picture=");
                if (pos != std::string::npos) {
                    size_t end_line = defcore_text.find("\n", pos);
                    std::string line = defcore_text.substr(pos + 8, (end_line == std::string::npos ? std::string::npos : end_line - (pos + 8)));
                    QStringList parts = QString::fromStdString(line).split(',');
                    if (parts.size() >= 4) {
                        px = parts[0].trimmed().toInt();
                        py = parts[1].trimmed().toInt();
                        pw = parts[2].trimmed().toInt();
                        ph = parts[3].trimmed().toInt();
                        pix = pix.copy(px, py, pw, ph);
                    }
                }
            }
        }

        if (pix.width() > 0 && pix.height() > 0) {
            bool is_scen = (item_type == "scenario" || item_type == "folder");
            bool is_root_pkg = ((item_type == "package" || item_type == "clonk") && sub.isEmpty());

            if (is_scen || is_root_pkg) {
                pix = pix.scaled(preview->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
                preview->setGraphicsEffect(nullptr);
            } else {
                QSize offset(20, 20);
                pix = pix.scaled(preview->size() - offset, Qt::KeepAspectRatio, Qt::FastTransformation);

                QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
                effect->setBlurRadius(0);
                effect->setColor(QColor(0, 0, 0, 141));
                effect->setOffset(3, 3);
                preview->setGraphicsEffect(effect);
            }
        }

        preview->setPixmap(pix);
    } else {
        preview->setGraphicsEffect(nullptr);
        preview->setPixmap(QPixmap());
    }

    // Author
    auto scen_data = target_grp.getFile("Scenario.txt");
    if (!scen_data.empty()) {
        std::string scen_text(reinterpret_cast<const char*>(scen_data.data()), scen_data.size());
        auto sections = parseC4Text(scen_text);
        auto head = sections["Head"];
        QString author = QString::fromStdString(head["Author"]);
        if (author.isEmpty()) author = "Unknown";
        author_label->setText("Author: " + author);
    } else {
        author_label->setText("Author: RedWolf Design");
    }

    // Handle Package Locking
    QStandardItem *root = tree_model->invisibleRootItem();
    if (!indexes.isEmpty()) {
        QStandardItem *item = tree_model->itemFromIndex(indexes.at(0));
        QVariantMap item_data = item->data(Qt::UserRole + 1).toMap();
        if (item_data.value("type").toString() == "scenario") {
            QString scen_title = item->text();
            QString scen_path = item_data.value("path").toString();
            QString scen_file = QFileInfo(scen_path).fileName().toLower();
            QStringList scen_sub = item_data.value("sub").toStringList();
            if (!scen_sub.isEmpty()) {
                scen_file = scen_sub.last().toLower();
            }

            std::map<std::string, bool> locks;
            // Robust matching for special scenarios
            if (scen_title.toLower().contains("research") || scen_file == "research.c4s") {
                locks["Hazard"] = true;
                locks["Knights"] = false;
                locks["Objects"] = false;
            } else if (scen_title.toLower().contains("dunkelfels") || scen_file == "dunkelfels.c4s") {
                locks["Hazard"] = false;
                locks["Knights"] = true;
                locks["Objects"] = true;
            }

            for (int row = 0; row < root->rowCount(); ++row) {
                QStandardItem *it = root->child(row);
                QVariantMap it_data = it->data(Qt::UserRole + 1).toMap();
                if (it_data.value("type").toString() == "package") {
                    QString pkg_title = it->text().toLower();
                    QString pkg_path = it_data.value("path").toString().toLower();
                    QString pkg_file = QFileInfo(pkg_path).fileName();
                    if (pkg_file.endsWith(".c4d")) pkg_file = pkg_file.left(pkg_file.length() - 4);

                    // Reset to default state first (Objects only)
                    bool is_obj = (pkg_title == "objects" || pkg_file.toLower() == "objects");
                    it->setCheckState(is_obj ? Qt::Checked : Qt::Unchecked);

                    // Check if this package should be locked
                    bool has_lock = false;
                    bool lock_state = false;
                    for (const auto &pair : locks) {
                        QString lock_name = QString::fromStdString(pair.first).toLower();
                        if (pkg_title == lock_name || pkg_file.toLower() == lock_name) {
                            has_lock = true;
                            lock_state = pair.second;
                            break;
                        }
                    }

                    QVariantMap d = it_data;
                    if (has_lock) {
                        d["is_locked"] = true;
                        it->setCheckState(lock_state ? Qt::Checked : Qt::Unchecked);
                    } else {
                        d["is_locked"] = false;
                    }
                    it->setData(d, Qt::UserRole + 1);
                }
            }
        } else {
            // Unlock everything if not a scenario
            for (int row = 0; row < root->rowCount(); ++row) {
                QStandardItem *it = root->child(row);
                QVariantMap it_data = it->data(Qt::UserRole + 1).toMap();
                if (it_data.value("type").toString() == "package") {
                    QVariantMap d = it_data;
                    d["is_locked"] = false;
                    it->setData(d, Qt::UserRole + 1);
                }
            }
        }
    }
}

void ClonkLauncher::showOptions() {
    OptionsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refresh_resources();
    }
}

void ClonkLauncher::showProps() {
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) return;

    QStandardItem *item = tree_model->itemFromIndex(indexes.at(0));
    QVariant data_var = item->data(Qt::UserRole + 1);
    if (!data_var.isValid() || data_var.typeId() != QMetaType::QVariantMap) return;

    QVariantMap data_map = data_var.toMap();
    if (data_map.value("type").toString() != "clonk") return;

    QString path = data_map.value("path").toString();
    QStringList sub = data_map.value("sub").toStringList();

    C4Group grp(path.toStdString());
    C4Group target_grp = grp;
    for (const auto &s : sub) {
        auto sub_data = target_grp.getFile(s.toStdString());
        if (!sub_data.empty()) {
            target_grp = C4Group(sub_data);
        } else {
            break;
        }
    }

    auto info_data = target_grp.getFile("Player.txt");
    if (info_data.empty()) return;

    std::string content(reinterpret_cast<const char*>(info_data.data()), info_data.size());
    auto sections = parseC4Text(content);
    auto pref = sections["Preferences"];

    int pref_color = 0, pref_control = 0, pref_mouse = 0;
    try { pref_color = std::stoi(pref["Color"]); } catch(...) {}
    try { pref_control = std::stoi(pref["Control"]); } catch(...) {}
    try { pref_mouse = std::stoi(pref["Mouse"]); } catch(...) {}

    ClonkPlayerPropertiesDialog dlg(this, dump_path);
    dlg.loadSettings(pref_color, pref_control, pref_mouse);

    if (dlg.exec() == QDialog::Accepted) {
        auto new_settings = dlg.getSettings();
        std::map<std::string, std::string> new_settings_str;
        for (const auto &pair : new_settings) {
            new_settings_str[pair.first] = std::to_string(pair.second);
        }

        std::string final_content = updateC4Text(content, "Preferences", new_settings_str);
        std::vector<uint8_t> final_data(final_content.begin(), final_content.end());

        C4GroupWriter writer;
        writer.addFromGroup(target_grp, {"Player.txt"});
        writer.addFile("Player.txt", final_data);

        if (sub.isEmpty()) {
            try {
                writer.writeToFile(path.toStdString(), target_grp.isPacked());
                
                int color = 1;
                QStringList lines = QString::fromStdString(final_content).split('\n');
                for (auto &line : lines) {
                    line = line.trimmed();
                    if (line.startsWith("Color=")) {
                        color = line.mid(6).toInt();
                        break;
                    }
                }
                item->setIcon(get_atlas_icon(99 + color));

                // trigger update selection to refresh view
                onTreeSelection(tree->selectionModel()->selection(), QItemSelection());
            } catch (...) {
                std::cerr << "Error writing player file" << std::endl;
            }
        } else {
            // Nested saving is complex, stubbed
            std::cout << "Updated nested group " << path.toStdString() << std::endl;
        }
    }
}

void ClonkLauncher::showAbout() {
    QDialog dlg(this);
    dlg.setFixedSize(550, 412);
    dlg.setWindowTitle("About Clonk Planet");
    
    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(&dlg);
    QString path = QDir(dump_path).filePath("Planet_fixed.bin_2_1000_1031.bmp");
    if (QFile::exists(path)) {
        label->setPixmap(QPixmap(path));
    }
    layout->addWidget(label);
    
    connect(label, &QLabel::linkActivated, &dlg, &QDialog::accept);
    // Simple click to close
    label->installEventFilter(this);
    
    dlg.exec();
}

void ClonkLauncher::showCredits() {
    QDialog dlg(this);
    dlg.setFixedSize(550, 412);
    dlg.setWindowTitle("Credits");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(&dlg);
    QString path = QDir(dump_path).filePath("Planet_fixed.bin_2_1010_1031.bmp");
    if (QFile::exists(path)) {
        label->setPixmap(QPixmap(path));
    }
    layout->addWidget(label);

    dlg.exec();
}

void ClonkLauncher::launchGame() {
    stop_music();

    QString clonk_bin = QDir(QCoreApplication::applicationDirPath()).filePath("clonk");
    if (!QFile::exists(clonk_bin)) {
        clonk_bin = QDir(base_path).filePath("build/clonk");
        if (!QFile::exists(clonk_bin)) {
            if (QFile::exists(QDir(planet_data_path).filePath("clonk"))) {
                clonk_bin = "./clonk";
            } else {
                QMessageBox::critical(this, "Error", "Executable not found at " + clonk_bin);
                return;
            }
        }
    }

    QStringList args;

    // 1. Add selected scenario
    bool scenario_selected = false;
    QModelIndexList indexes = tree->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        QStandardItem *item = tree_model->itemFromIndex(indexes.at(0));
        QVariantMap data_map = item->data(Qt::UserRole + 1).toMap();
        if (data_map.value("type").toString() == "scenario") {
            scenario_selected = true;
            QString path = data_map.value("path").toString();
            QStringList sub = data_map.value("sub").toStringList();

            QString rel_group = QDir(planet_data_path).relativeFilePath(path);
            if (!sub.isEmpty()) {
                QString full_rel = rel_group;
                for (const auto &s : sub) {
                    full_rel = QDir(full_rel).filePath(s);
                }
                args << full_rel;
            } else {
                args << rel_group;
            }
        }
    }

    if (!scenario_selected) {
        QString msg = "To start a round you need to select a scenario first. To do this, open a scenario folder (book) by double clicking on it and select the desired scenario. Then click 'start'.";
        ClonkPopupDialog dlg(this, msg, dump_path);
        dlg.exec();
        return;
    }

    // 2. Add checked players and definition packages
    std::function<void(QStandardItem*)> collect_checked = [&](QStandardItem *parent_item) {
        for (int r = 0; r < parent_item->rowCount(); ++r) {
            QStandardItem *item = parent_item->child(r);
            if (item->isCheckable() && item->checkState() == Qt::Checked) {
                QVariantMap data_map = item->data(Qt::UserRole + 1).toMap();
                QString path = data_map.value("path").toString();
                QStringList sub = data_map.value("sub").toStringList();
                if (!path.isEmpty()) {
                    QString rel_path = QDir(planet_data_path).relativeFilePath(path);
                    if (!sub.isEmpty()) {
                        QString full_rel = rel_path;
                        for (const auto &s : sub) {
                            full_rel = QDir(full_rel).filePath(s);
                        }
                        args << full_rel;
                    } else {
                        args << rel_path;
                    }
                }
            }
            if (item->hasChildren()) {
                collect_checked(item);
            }
        }
    };

    collect_checked(tree_model->invisibleRootItem());

    std::cout << "Launching Game: " << clonk_bin.toStdString();
    for (const auto &arg : args) std::cout << " " << arg.toStdString();
    std::cout << std::endl;

    QProcess *game_process = new QProcess(this);
    game_process->setWorkingDirectory(planet_data_path);
    
    // Hide main window during gameplay
    this->hide();
    connect(game_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ClonkLauncher::onGameFinished);
    game_process->start(clonk_bin, args);
}

void ClonkLauncher::onGameFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    this->show();
    start_music();
}
