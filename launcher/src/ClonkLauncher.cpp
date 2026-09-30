#include "ClonkLauncher.h"
#include "OptionsDialog.h"
#include "ClonkDialogs.h"
#include "Utils.h"
#include "C4Group.h"
#include "LauncherRes.h"
#include "GroupEdit.h"
#include "C4TextDoc.h"
#include "ClonkDialog.h"
#include "NetworkBrowser.h"
#include <QApplication>
#include <QLineEdit>
#include <QDesktopServices>
#include <QUrl>
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
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>
#include <QTextCursor>
#include <QTextCharFormat>
#include <fstream>
#include <iostream>

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
        painter->setRenderHint(QPainter::TextAntialiasing, false);

        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();

        // 1. Draw base white background (non-selected)
        QStyleOptionViewItem opts = opt;
        opts.state &= ~QStyle::State_Selected;
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opts, painter, widget);

        // Calculate text geometry first to know where selection ends
        QRect display_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            display_rect.translate(-5, -1);
        } else {
            display_rect.translate(-1, -1);
        }

        QString text = index.data(Qt::DisplayRole).toString();
        QFont font = opt.font;
        if (font.bold() || font.weight() >= QFont::Bold) {
            font.setStyleStrategy(QFont::PreferAntialias);
            painter->setRenderHint(QPainter::TextAntialiasing, true);
        } else {
            font.setStyleStrategy(QFont::NoAntialias);
            font.setHintingPreference(QFont::PreferFullHinting);
            painter->setRenderHint(QPainter::TextAntialiasing, false);
        }
        painter->setFont(font);
        QFontMetrics fm(font);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        int text_w = fm.horizontalAdvance(text);
#else
        int text_w = fm.width(text);
#endif

        // 2. Draw selection highlight background (text only; not under the label editor)
        const auto *view = qobject_cast<const ExplorerTreeView *>(opt.widget);
        const bool editing = view && view->isEditing(index);
        bool is_selected = (opt.state & QStyle::State_Selected) && !editing;
        if (is_selected) {
            QRect highlight_rect(display_rect.x() - 2, opt.rect.y(), text_w + 6, opt.rect.height());
            painter->fillRect(highlight_rect, QColor("#3096fa"));
        }

        // 3. Draw Checkbox
        if (opt.features & QStyleOptionViewItem::HasCheckIndicator) {
            QRect check_rect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
            check_rect.translate(-2, 2);

            const bool is_locked = index.data(Qt::UserRole + 3).toBool();

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
                decor_rect.translate(-7, 0);
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

    // TVM_EDITLABEL: white edit box with a thin black frame over the label (PlayerRename.png)
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QWidget *w = QStyledItemDelegate::createEditor(parent, option, index);
        if (auto *edit = qobject_cast<QLineEdit *>(w)) {
            edit->setFrame(false);
            edit->setStyleSheet("QLineEdit { background: white; color: black; border: 1px solid black; padding: 0px 1px;"
                                " selection-background-color: #316ac5; selection-color: white; }");
            edit->setFont(option.font);
        }
        return w;
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        const QWidget *widget = opt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        QRect r = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
        r.translate((opt.features & QStyleOptionViewItem::HasCheckIndicator) ? -5 : -1, -1);
        const int w = QFontMetrics(opt.font).horizontalAdvance(index.data().toString()) + 12;
        editor->setGeometry(r.x() - 4, opt.rect.y(), qMax(w, 40), opt.rect.height());
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
            // checkboxes locked by the selected scenario can't be changed
            if (index.data(Qt::UserRole + 3).toBool())
                return false;
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    QPixmap check_on;
    QPixmap check_off;
    QPixmap check_locked_on;
    QPixmap check_locked_off;
};

static ClonkLauncher *launcher_instance = nullptr;

ClonkLauncher *ClonkLauncher::instance() {
    return launcher_instance;
}

// ExplorerDlg::SetStatus (0x414a50); cleared by the 3 second timer 0x8707 (ExplorerDlg::OnTimer)
// when no new message came since the last tick
void ClonkLauncher::logStatus(const QString &text) {
    if (status_bar)
        status_bar->setText(" " + text);
    status_fresh = true;
    if (!status_timer) {
        status_timer = new QTimer(this);
        connect(status_timer, &QTimer::timeout, this, [this]() {
            if (!status_fresh && status_bar)
                status_bar->setText(QString());
            status_fresh = false;
        });
        status_timer->start(3000);
    }
}

// ClonkLauncher Class
ClonkLauncher::ClonkLauncher() {
    launcher_instance = this;
    base_path = QDir(QCoreApplication::applicationDirPath()).filePath(".");
    planet_data_path = base_path; // QDir(base_path).filePath("planet_data");
    res_music_path = QDir(QCoreApplication::applicationDirPath()).filePath("res_music");
    // the user's clonk.ini, created from the defaults next to the executable (shared with the engine)
    config_path = QString::fromStdString(
        CStdIniRegistry::PrepareUserConfig(QDir(planet_data_path).filePath("clonk.ini").toStdString()));

    // String table, dialog templates and resources of the original Planet.exe (data/)
    if (!LauncherRes::load(QDir(QCoreApplication::applicationDirPath()).filePath("data"))) {
        std::cerr << "Failed to load launcher data (strings.json / dialogs.json)" << std::endl;
    }

    // Atlases
    QPixmap raw_icons(LauncherRes::resPath("bitmap", 1015));
    if (!raw_icons.isNull()) {
        icons_atlas = applyClonkTransparency(raw_icons);
    }

    QPixmap raw_check(LauncherRes::resPath("bitmap", 1016));
    if (!raw_check.isNull()) {
        check_atlas = applyClonkTransparency(raw_check);
        check_on_pix = check_atlas.copy(QRect(16, 0, 16, 16));
        check_off_pix = check_atlas.copy(QRect(32, 0, 16, 16));
        check_locked_on_pix = check_atlas.copy(QRect(112, 0, 16, 16));
        check_locked_off_pix = check_atlas.copy(QRect(128, 0, 16, 16));
    }

    // Custom Font loading
    QString font_path = QDir(planet_data_path).filePath("Comic.ttf");
    QString bold_font_path = QDir(planet_data_path).filePath("Comicbd.ttf");
    comic_font_family = "Comic Sans MS";
    if (QFile::exists(font_path)) {
        int font_id = QFontDatabase::addApplicationFont(font_path);
        if (font_id != -1) {
            comic_font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        }
    }
    if (QFile::exists(bold_font_path)) {
        QFontDatabase::addApplicationFont(bold_font_path);
    }

    QString serif_path = QDir(planet_data_path).filePath("micross.ttf");
    if (QFile::exists(serif_path)) {
        int font_id = QFontDatabase::addApplicationFont(serif_path);
        if (font_id != -1) {
            serif_font_family = QFontDatabase::applicationFontFamilies(font_id).at(0);
        }
    }

    // Config loading
    config.load(config_path.toStdString());
    language = get_cfg("General\\Language", "US");

    LauncherRes::setLanguage(language);
    // frontend font of the skinned dialogs (General\FEFontName / FEFontSize), MS Sans Serif for the rest
    {
        int fe_size = 9;
        try { fe_size = std::stoi(get_cfg("General\\FEFontSize", "9")); } catch (...) {}
        LauncherRes::setFonts(comic_font_family, fe_size, serif_font_family);
    }
    LauncherRes::setSoundsEnabled(get_cfg("Sound\\FESamples", "1") == "1");

    // Audio SFX
    sound_start = new QSoundEffect(this);
    sound_start->setSource(QUrl::fromLocalFile(LauncherRes::resPath("wave", 7008)));
    sound_click = new QSoundEffect(this);
    sound_click->setSource(QUrl::fromLocalFile(LauncherRes::resPath("wave", 7002)));

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
    initNetwork();
}

ClonkLauncher::~ClonkLauncher() {
    // ExplorerDlg::OnClose: the Network folder is removed
    if (net)
        net->shutdown();
    stop_music();
}

QString ClonkLauncher::executablePath(const QString &name) {
#ifdef Q_OS_WIN
    return QDir(QCoreApplication::applicationDirPath()).filePath(name + ".exe");
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath(name);
#endif
}

std::string ClonkLauncher::get_cfg(const std::string &sub_key, const std::string &defaultValue) const {
    return config.getValue(sub_key, defaultValue);
}

void ClonkLauncher::set_cfg(const std::string &sub_key, const std::string &value) {
    config.setValue(sub_key, value);
}

void ClonkLauncher::saveConfig() {
    config.setValue("General\\Language", language);
    config.save();
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
            QString sf_path = QDir(planet_data_path).filePath("FluidR3Mono_GM.sf3");
            QString midi_player_bin = executablePath("clonk_midi");

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
    applyRegistration();

    central_widget = new QWidget(this);
    central_widget->setFixedSize(LauncherRes::dialogSize(LauncherRes::IDD_EXPLORER));
    setCentralWidget(central_widget);

    QString bg_path = LauncherRes::resPath("bitmap", 1019);
    ui_container = new ClonkTexturedWidget(central_widget, bg_path);
    ui_container->setGeometry(QRect(QPoint(0, 0), LauncherRes::dialogSize(LauncherRes::IDD_EXPLORER)));

    init_menu();
    setup_main_ui();

    applyViewMode();
}

void ClonkLauncher::init_menu() {
    QMenuBar *menu = menuBar();
    QFont menu_font(getSerifFontFamily());
    menu_font.setStyleStrategy(QFont::NoAntialias);
    menu_font.setHintingPreference(QFont::PreferFullHinting);
    menu_font.setPixelSize(11);
    menu->setFont(menu_font);


    // menu texts as set by the original (0x413e10)
    QMenu *m_opt = menu->addMenu(LauncherRes::str(51007));
    QAction *act_opt = m_opt->addAction(LauncherRes::str(51008));
    connect(act_opt, &QAction::triggered, this, [this]() { showOptions(); });
    QAction *act_reg = m_opt->addAction(LauncherRes::str(51009));
    connect(act_reg, &QAction::triggered, this, &ClonkLauncher::showRegistration);

    QMenu *m_help = menu->addMenu(LauncherRes::str(51004));
    QAction *act_contents = m_help->addAction(LauncherRes::str(51001));
    connect(act_contents, &QAction::triggered, this, &ClonkLauncher::showHelp);
    QAction *act_credits = m_help->addAction(LauncherRes::str(51002));
    connect(act_credits, &QAction::triggered, this, &ClonkLauncher::showCredits);
    // ExplorerDlg::On4003Clicked: ShellExecute of string 52000
    QAction *act_web = m_help->addAction(LauncherRes::str(51010));
    connect(act_web, &QAction::triggered, this, []() { QDesktopServices::openUrl(QUrl(LauncherRes::str(52000))); });
    m_help->addSeparator();
    QAction *act_about = m_help->addAction(LauncherRes::str(51000));
    connect(act_about, &QAction::triggered, this, &ClonkLauncher::showAbout);
}

void ClonkLauncher::setup_main_ui() {
    ui_container->setStyleSheet(
        "* { color: black; }"
        "ClonkButton {"
        "    font-family: '" + comic_font_family + "';"
        "    font-size: 12px;"
        "}"
    );

    tree_frame = new ClonkArea(ui_container, "white");
    tree_frame->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2381));

    tree = new ExplorerTreeView(tree_frame);
    tree->setGeometry(tree_frame->rect().adjusted(2, 2, -2, -2));
    tree->setHeaderHidden(true);
    tree->setFrameShape(QFrame::NoFrame);

    tree_model = new QStandardItemModel(tree);
    tree->setModel(tree_model);
    tree->setItemDelegate(new PixelDelegate(tree, check_on_pix, check_off_pix, check_locked_on_pix, check_locked_off_pix));
    connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ClonkLauncher::onTreeSelection);
    connect(tree_model, &QStandardItemModel::itemChanged, this, [this](QStandardItem *item) {
        // checkbox clicked in the tree: same as the activate button
        if (updating_tree)
            return;
        if (renaming_item && item->data(Qt::UserRole + 1).toInt() == renaming_item) {
            finishRename(item);
            return;
        }
        ExplorerItem *e = explorerItem(item);
        if (e && e->activatable && !e->locked && (item->checkState() == Qt::Checked) != e->activated)
            setActivated(item, item->checkState() == Qt::Checked);
    });
    connect(tree, &QTreeView::expanded, this, &ClonkLauncher::onTreeExpanded);
    connect(tree, &ExplorerTreeView::itemDropped, this, &ClonkLauncher::onItemDropped);
    connect(tree, &ExplorerTreeView::filesDropped, this, &ClonkLauncher::onFilesDropped);
    connect(tree, &ExplorerTreeView::contextMenuAt, this, &ClonkLauncher::onTreeContextMenu);
    connect(tree, &ExplorerTreeView::keyAction, this, &ClonkLauncher::onTreeKey);
    // NM_DBLCLK: external editor for editable items, else expand / collapse (0x42d7b0)
    connect(tree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        ExplorerItem *e = explorerItem(index);
        if (e && explorerType(e->type).editor)
            openInEditor();
    });
    // Developer\AutoEditScan: rescan edited files every 2 seconds (timer 0x10932)
    edit_scan_timer = new QTimer(this);
    connect(edit_scan_timer, &QTimer::timeout, this, [this]() {
        if (!edited_items.empty() && get_cfg("Developer\\AutoEditScan", "0") == "1")
            scanEditedItems();
    });
    edit_scan_timer->start(2000);
    connect(tree, &QTreeView::collapsed, this, &ClonkLauncher::onTreeCollapsed);

    tree->setRootIsDecorated(false);
    tree->setIndentation(19);
    tree->setExpandsOnDoubleClick(true);
    tree->setEditTriggers(QTreeView::NoEditTriggers);
    tree->setIconSize(QSize(16, 16));
    tree->setUniformRowHeights(true);
    tree->setSelectionBehavior(QAbstractItemView::SelectItems);


    QPalette pal = tree->palette();
    pal.setColor(QPalette::Active, QPalette::Highlight, QColor("#3096fa"));
    pal.setColor(QPalette::Inactive, QPalette::Highlight, QColor("#3096fa"));
    tree->setPalette(pal);

    preview_frame = new ClonkArea(ui_container);
    preview_frame->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2016));

    preview = new ClonkPreviewLabel(preview_frame);
    preview->setGeometry(preview_frame->rect());
    preview->setAlignment(Qt::AlignCenter);

    desc_frame = new ClonkArea(ui_container, "white");
    desc_frame->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2221));

    desc = new QTextEdit(desc_frame);
    desc->setGeometry(desc_frame->rect().adjusted(2, 2, -2, -2));
    desc->setReadOnly(true);
    desc->setFrameShape(QFrame::NoFrame);
    desc->document()->setDocumentMargin(2); 
    desc->setStyleSheet("QTextEdit { padding-top: 1px; background: transparent; }");
    //QFont desc_font(comic_font_family, 9);
    QFont desc_font = desc->font();
    desc_font.setStyleStrategy(QFont::NoAntialias);
    desc_font.setHintingPreference(QFont::PreferFullHinting);
    desc->setFont(desc_font);

    connect(desc->document(), &QTextDocument::contentsChanged, this, [this]() {
        static bool in_update = false;
        if (in_update) return;
        in_update = true;
        
        QTextDocument *doc = desc->document();
        QTextCursor cursor(doc);
        cursor.beginEditBlock();
        for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
            for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
                QTextFragment fragment = it.fragment();
                if (fragment.isValid()) {
                    QTextCharFormat fmt = fragment.charFormat();
                    if (fmt.fontWeight() >= QFont::Bold || fmt.font().bold()) {
                        fmt.setFontStyleStrategy(QFont::PreferAntialias);
                    } else {
                        fmt.setFontStyleStrategy(QFont::NoAntialias);
                    }
                    QTextCursor fragCursor(doc);
                    fragCursor.setPosition(fragment.position());
                    fragCursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                    fragCursor.mergeCharFormat(fmt);
                }
            }
        }
        cursor.endEditBlock();
        
        in_update = false;
    });

    desc->setText("Select a scenario or player to begin.");

    QString btn_bg = LauncherRes::resPath("bitmap", 1006);
    QPoint offset(117, 14);

    auto create_btn = [&](int ctrl_id) -> ClonkButton* {
        ClonkButton *btn = new ClonkButton(LauncherRes::controlText(LauncherRes::IDD_EXPLORER, ctrl_id), ui_container, btn_bg, offset);
        btn->move(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, ctrl_id).topLeft());
        return btn;
    };

    btn_new = create_btn(2055);
    connect(btn_new, &QPushButton::clicked, this, &ClonkLauncher::onNewClicked);
    btn_activate = create_btn(2025);
    btn_activate->setEnabled(false);
    btn_rename = create_btn(2058);
    connect(btn_rename, &QPushButton::clicked, this, &ClonkLauncher::startRename);
    btn_rename->setEnabled(false);
    btn_delete = create_btn(2039);
    connect(btn_delete, &QPushButton::clicked, this, &ClonkLauncher::onDeleteClicked);
    btn_delete->setEnabled(false);
    btn_props = create_btn(2057);
    btn_props->setEnabled(false);
    connect(btn_props, &QPushButton::clicked, this, &ClonkLauncher::showProps);
    connect(btn_activate, &QPushButton::clicked, this, &ClonkLauncher::onActivateClicked);

    btn_start = create_btn(2070);
    connect(btn_start, &QPushButton::clicked, this, &ClonkLauncher::launchGame);

    btn_quit = create_btn(2049);
    connect(btn_quit, &QPushButton::clicked, this, &QWidget::close);

    // View: Player / Developer, only in developer mode (Explorer\Mode)
    view_label = new QLabel(LauncherRes::controlText(LauncherRes::IDD_EXPLORER, 2368), ui_container);
    view_label->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2368));
    view_label->setFont(LauncherRes::feFont());
    view_label->setStyleSheet("background: transparent;");
    radio_player = new QRadioButton(LauncherRes::str(50263), ui_container);
    radio_player->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2219));
    radio_developer = new QRadioButton(LauncherRes::str(50159), ui_container);
    radio_developer->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2220));
    for (QRadioButton *r : {radio_player, radio_developer}) {
        r->setFont(LauncherRes::feFont());
        r->setFocusPolicy(Qt::NoFocus);
    }
    developer_view = developerMode() && get_cfg("Explorer\\Mode", "0") == "1";
    radio_player->setChecked(!developer_view);
    radio_developer->setChecked(developer_view);
    connect(radio_developer, &QRadioButton::toggled, this, &ClonkLauncher::onViewModeChanged);
    for (QRadioButton *r : {radio_player, radio_developer})
        connect(r, &QRadioButton::clicked, this, []() { LauncherRes::playSound(7005); });
    const bool dev = developerMode();
    view_label->setVisible(dev);
    radio_player->setVisible(dev);
    radio_developer->setVisible(dev);

    author_label = new QLabel(ui_container);
    author_label->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2267));
    author_label->setStyleSheet("background: transparent;");

    status_frame = new ClonkArea(ui_container);
    status_frame->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2357));

    status_bar = new QLabel(status_frame);
    status_bar->setGeometry(status_frame->rect().adjusted(2, 2, -2, -2));
    logStatus(LauncherRes::str(50369)); // "Welcome to Clonk Planet." (ExplorerDlg::OnInitDialog)
    status_bar->setStyleSheet("background: transparent;");

    anim_frame = new ClonkArea(ui_container);
    anim_frame->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2000));

    animation = new AnimateCtrl(anim_frame);
    animation->setGeometry(anim_frame->rect().adjusted(2, 2, -2, -2));
    animation->setStyleSheet("background: transparent;");
    animation->play(6001); // network inactive

    separator = new QFrame(ui_container);
    separator->setGeometry(LauncherRes::controlRect(LauncherRes::IDD_EXPLORER, 2290));
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
}

QIcon ClonkLauncher::get_atlas_icon(int index) {
    if (icons_atlas.isNull()) return QIcon();
    QIcon icon;
    QRect rect(index * 16, 0, 16, 16);
    icon.addPixmap(icons_atlas.copy(rect), QIcon::Normal, QIcon::Off);
    if (index == 4) {
        QRect open_rect(22 * 16, 0, 16, 16);
        icon.addPixmap(icons_atlas.copy(open_rect), QIcon::Normal, QIcon::On);
    }
    return icon;
}

// ExplorerDlg::On4005Clicked (menu Options > Options...); page 5 = Network (planet symbol "Properties")
void ClonkLauncher::showOptions(int page) {
    if (net)
        net->pause();
    // "Properties" of the planet symbol opens the sheet on the Network page
    OptionsDialog dlg(this, page == 5);
    if (page >= 0 && page != 5)
        dlg.setActiveTab(page);
    const bool accepted = dlg.exec() == QDialog::Accepted;
    if (accepted) {
        saveConfig();
        // language, fonts and registration may have changed
        language = get_cfg("General\\Language", "US");
        LauncherRes::setLanguage(language);
        {
            int fe_size = 9;
            try { fe_size = std::stoi(get_cfg("General\\FEFontSize", "9")); } catch (...) {}
            const QString fe_name = QString::fromStdString(get_cfg("General\\FEFontName", comic_font_family.toStdString()));
            LauncherRes::setFonts(fe_name.isEmpty() ? comic_font_family : fe_name, fe_size, serif_font_family);
        }
        retranslate();
        applyRegistration();
        LauncherRes::setSoundsEnabled(get_cfg("Sound\\FESamples", "1") == "1");
        if (net)
            net->init();
        // developer mode / language may have changed
        const bool dev = developerMode();
        view_label->setVisible(dev);
        radio_player->setVisible(dev);
        radio_developer->setVisible(dev);
        if (!dev && developer_view)
            radio_player->setChecked(true);
        refreshTree();
    }
    if (net)
        net->resume();
}

// ExplorerDlg::LocalizeMenu (0x413e10) and the texts of OnInitDialog after a language change
void ClonkLauncher::retranslate() {
    menuBar()->clear();
    init_menu();
    for (auto [btn, id] : {std::pair<QPushButton *, int>{btn_new, 2055}, {btn_rename, 2058}, {btn_delete, 2039},
                           {btn_props, 2057}, {btn_quit, 2049}})
        btn->setText(LauncherRes::controlText(LauncherRes::IDD_EXPLORER, id));
    view_label->setText(LauncherRes::controlText(LauncherRes::IDD_EXPLORER, 2368));
    radio_player->setText(LauncherRes::str(50263));
    radio_developer->setText(LauncherRes::str(50159));
    for (QWidget *w : {static_cast<QWidget *>(view_label), static_cast<QWidget *>(radio_player),
                       static_cast<QWidget *>(radio_developer), static_cast<QWidget *>(author_label),
                       static_cast<QWidget *>(status_bar)})
        w->setFont(LauncherRes::feFont());
    for (QPushButton *b : {btn_new, btn_activate, btn_rename, btn_delete, btn_props, btn_start, btn_quit}) {
        QFont f = LauncherRes::feFont();
        b->setFont(f);
    }
}

// ExplorerDlg::OnPropertiesClicked (0x414150): player or scenario properties
void ClonkLauncher::showProps() {
    QStandardItem *it = selectedTreeItem();
    ExplorerItem *e = explorerItem(it);
    if (!e || !e->has_properties)
        return;
    if (e->type == T_Player) {
        C4Group grp;
        if (!GroupEdit::open(e->path, grp))
            return;
        C4TextDoc player(grp.getFile("Player.txt"));
        ClonkPlayerPropertiesDialog dlg(this);
        dlg.setWindowTitle(LauncherRes::str(50521).replace("%s", e->title)); // "Player Properties - %s"
        dlg.loadSettings(player.getInt("Preferences", "Color"), player.getInt("Preferences", "Control", 1),
                         player.getInt("Preferences", "Mouse", 1));
        if (dlg.exec() != QDialog::Accepted)
            return;
        for (const auto &pair : dlg.getSettings())
            player.setInt("Preferences", QString::fromStdString(pair.first), pair.second);
        if (!GroupEdit::writeFile(e->path, "Player.txt", player.toBytes()))
            clonkMessage(this, LauncherRes::str(50606)); // "File modification failure."
        refreshTree();
    } else if (e->type == T_Scenario) {
        editScenario(e);
    }
}

