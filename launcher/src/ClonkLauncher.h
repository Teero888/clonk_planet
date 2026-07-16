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
#include <QItemSelection>
#include <map>
#include <string>
#include <vector>
#include "ConfigManager.h"
#include "Win3DWidgets.h"

class ClonkLauncher : public QMainWindow {
    Q_OBJECT
public:
    ClonkLauncher();
    ~ClonkLauncher() override;

    std::string get_cfg(const std::string &sub_key, const std::string &defaultValue) const;
    void set_cfg(const std::string &sub_key, const std::string &value);

    QString getDumpPath() const { return dump_path; }
    QString getResPath() const { return res_path; }
    QString getComicFontFamily() const { return comic_font_family; }
    std::string getLanguage() const { return language; }
    void setLanguage(const std::string &lang) { language = lang; }
    void saveConfig();

    void start_music();
    void stop_music();

private slots:
    void onTreeSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void showOptions();
    void showProps();
    void showAbout();
    void showCredits();
    void launchGame();
    void onGameFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updateMusic();

private:
    void init_ui();
    void init_menu();
    void setup_main_ui();
    void refresh_resources();
    void set_background(const QString &name);
    void extract_frontend_music();
    QIcon get_atlas_icon(int index);

    QString base_path;
    QString planet_data_path;
    QString res_path;
    QString res_music_path;
    QString wav_path;
    QString dump_path;
    QString config_path;

    std::string language = "US";
    QString comic_font_family = "Comic Sans MS";

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
    QTreeView *tree = nullptr;
    QStandardItemModel *tree_model = nullptr;

    ClonkArea *preview_frame = nullptr;
    QLabel *preview = nullptr;

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
    QLabel *animation = nullptr;
};
