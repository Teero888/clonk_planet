#pragma once

#include "LauncherCompat.h"
#include <QMainWindow>
#include <QTreeView>
#include <QTextEdit>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProcess>
#include <QSoundEffect>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>
#include <QDateTime>
#include <QItemSelection>
#include <map>
#include <string>
#include <vector>
#include "ConfigManager.h"
#include "Win3DWidgets.h"
#include "LauncherRes.h"
#include "ExplorerModel.h"
#include "AnimateCtrl.h"
#include "ExplorerTreeView.h"

class ClonkLauncher : public QMainWindow {
    Q_OBJECT
public:
    ClonkLauncher();
    ~ClonkLauncher() override;

    // the single main window (ExplorerDlg of the original)
    static ClonkLauncher *instance();

    // directory with the game data (clonk.ini, *.c4f, *.c4d, ...)
    QString planetDataPath() const { return planet_data_path; }
    // object definition packs that are activated in the tree (checked .c4d items)
    QList<ItemPath> activeDefinitions() const;
    // status bar message (the original logs every action there)
    void logStatus(const QString &text);
    // selects a scenario in the tree and starts it like the start button (used by the quick start screen)
    void startScenario(const ItemPath &scenario);
    // developer mode enabled in the options (Developer\Active)
    bool developerMode() const { return get_cfg("Developer\\Active", "0") == "1"; }


    std::string get_cfg(const std::string &sub_key, const std::string &defaultValue) const;
    void set_cfg(const std::string &sub_key, const std::string &value);

    QString getSerifFontFamily() const { return serif_font_family; }
    std::string getLanguage() const { return language; }
    void setLanguage(const std::string &lang) { language = lang; LauncherRes::setLanguage(lang); }
    void saveConfig();

    // registration name / code validated like the original (Registration.cpp)
    bool isRegistered() const;
    // "Clonk Planet" / "Clonk Planet - NOT REGISTERED"
    QString windowTitleText() const;
    void applyRegistration();
    void retranslate();

    void start_music();
    void stop_music();

    // tree (ExplorerTree.cpp)
    ExplorerContext explorerContext() const;
    ExplorerItem *selectedExplorerItem();
    // reloads the tree, keeping the selection if possible
    void refreshTree();

protected:
    // WM_ACTIVATEAPP: rescan externally edited files
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // ExplorerDlg::OnHelp (0x415570): F1 shows the help popup of the control under the focus
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTreeSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void onTreeExpanded(const QModelIndex &index);
    void onTreeCollapsed(const QModelIndex &index);
    void onActivateClicked();
    void onViewModeChanged(bool developer);
    void onNetworkSymbolClicked(Qt::MouseButton button);
    // ExplorerActions.cpp
    void onNewClicked();
    void onDeleteClicked();
    void startRename();
    void onItemDropped(const QModelIndex &source, const QModelIndex &target, bool move);
    void onFilesDropped(const QStringList &files, const QModelIndex &target);
    void onTreeContextMenu(const QPoint &pos);
    void onTreeKey(int key);
    void showScenarioPresets();
    void showOptions(int page = -1);
    void showProps();
    void showAbout();
    void showCredits();
    void showHelp();          // Help > Contents... (ExplorerDlg::On4001Clicked)
    void showRegistration();  // Options > Registration... (ExplorerDlg::On4006Clicked)
    void launchGame();
    void onGameFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateMusic();

private:
    void init_ui();
    void init_menu();
    void setup_main_ui();
    void refresh_resources();
    // tree (ExplorerTree.cpp)
    QStandardItem *makeTreeItem(const ExplorerItem &item);
    void loadChildren(QStandardItem *parent);
    ExplorerItem *explorerItem(const QModelIndex &index);
    ExplorerItem *explorerItem(QStandardItem *item);
    QStandardItem *findTreeItem(const ItemPath &path);
    void showItemInfo(ExplorerItem *item);
    void updatePackageLocks(ExplorerItem *selected);
    void setActivated(QStandardItem *tree_item, bool activated);
    void updateCheckState(QStandardItem *tree_item);
    bool validateStart(ExplorerItem *scenario);
    void editScenario(ExplorerItem *scenario);
    // ExplorerActions.cpp
    QString itemLocation(QStandardItem *item);
    QStandardItem *selectedTreeItem();
    bool isOriginalItem(QStandardItem *tree_item);
    bool canModify(QStandardItem *tree_item, bool check_parent);
    void finishRename(QStandardItem *item);
    bool dropItem(const ItemPath &src, QStandardItem *target_item, bool move, bool confirm, bool is_new);
    void showRoundLog();
    // ExplorerNetwork.cpp
    void initNetwork();
    // ExplorerLayout.cpp: control geometry (anchors of 0x415cb0), resizable developer view
    void layoutControls();
    void applyViewMode();
    void toggleNetwork();
    bool isNetworkReference(ExplorerItem *item);
    void sendNetworkMessage();
    // ExplorerEdit.cpp (developer mode)
    bool openInEditor();
    void scanEditedItems();
    void duplicateItem();
    void packItem(bool pack, bool recursive);
    void evaluatePlayers();
    QString playerRankName(int rank);
    void extract_frontend_music();
    QIcon get_atlas_icon(int index);

    QString base_path;
    QString planet_data_path;
    QString res_music_path;
    QString config_path;

    std::string language = "US";
    QString comic_font_family = "Comic Sans MS";
    QString serif_font_family = "wewa";

    ConfigManager config;

    QPixmap icons_atlas;
    QPixmap check_atlas;
    QPixmap check_on_pix;
    QPixmap check_off_pix;
    QPixmap check_locked_on_pix;
    QPixmap check_locked_off_pix;

    QSoundEffect *sound_start = nullptr;
    QSoundEffect *sound_click = nullptr;

    QProcess *midi_process = nullptr;
    QString current_music_path;
    QTimer *music_timer = nullptr;

    QWidget *central_widget = nullptr;
    ClonkTexturedWidget *ui_container = nullptr;
    QLabel *bg_label = nullptr;

    ClonkArea *tree_frame = nullptr;
    ExplorerTreeView *tree = nullptr;
    QStandardItemModel *tree_model = nullptr;

    ClonkArea *preview_frame = nullptr;
    ClonkPreviewLabel *preview = nullptr;

    ClonkArea *desc_frame = nullptr;
    QTextEdit *desc = nullptr;

    ClonkButton *btn_new = nullptr;
    ClonkButton *btn_activate = nullptr;
    ClonkButton *btn_rename = nullptr;
    ClonkButton *btn_delete = nullptr;
    ClonkButton *btn_props = nullptr;
    ClonkButton *btn_start = nullptr;
    ClonkButton *btn_quit = nullptr;

    QLabel *author_label = nullptr;

    ClonkArea *status_frame = nullptr;
    QLabel *status_bar = nullptr;
    ClonkArea *anim_frame = nullptr;
    AnimateCtrl *animation = nullptr;
    QFrame *separator = nullptr;
    bool desc_full_area = false; // description over the picture area (no picture)

    QRadioButton *radio_player = nullptr;
    QRadioButton *radio_developer = nullptr;
    QLabel *view_label = nullptr;

    std::map<int, ExplorerItem> explorer_items;
    int next_explorer_item = 1;
    bool developer_view = false;
    bool updating_tree = false;
    int renaming_item = 0;
    struct EditedItem {
        ItemPath path;
        QString temp_file;
        QDateTime mtime;
        bool original = false;
    };
    std::vector<EditedItem> edited_items;
    QTimer *edit_scan_timer = nullptr;
    class NetworkBrowser *net = nullptr;
    QTimer *status_timer = nullptr;
    bool status_fresh = false;
};
