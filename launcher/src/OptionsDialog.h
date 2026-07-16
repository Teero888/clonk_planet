#pragma once

#include "LauncherCompat.h"
#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <map>
#include <string>
#include "Win3DWidgets.h"

class ClonkLauncher;

class KeyBindInput : public QLineEdit {
    Q_OBJECT
public:
    explicit KeyBindInput(const QString &action_name, int key_code = 0, QWidget *parent = nullptr);
    int getKeyCode() const { return key_code; }
    void setKeyCode(int code);
protected:
    void keyPressEvent(QKeyEvent *event) override;
signals:
    void keyChanged(const QString &action_name, int code);
private:
    QString action_name;
    int key_code;
};

class HostDialog : public QDialog {
    Q_OBJECT
public:
    explicit HostDialog(QWidget *parent = nullptr, const QString &title = "New host address", const QString &initial_text = "");
    QString getAddress() const;
private:
    QLineEdit *input = nullptr;
};

class OptionsDialog : public QDialog {
    Q_OBJECT
public:
    explicit OptionsDialog(ClonkLauncher *parent = nullptr);
    
private slots:
    void onKeyInputChanged(const QString &action_name, int code);
    void loadKeyProfile(int block_num);
    void promptResetControls();
    void addHost();
    void editHost();
    void deleteHost();
    void onOK();
    void updateNetworkState(bool is_active);

private:
    QWidget* createProgramPage();
    QWidget* createGraphicsPage();
    QWidget* createSoundPage();
    QWidget* createKeyboardPage();
    QWidget* createNetworkPage();

    ClonkLauncher *launcher = nullptr;

    std::map<std::string, int> key_codes;
    int current_block = 1;

    Win3DTabWidget *tab_widget = nullptr;
    
    // Program page
    QRadioButton *radio_german = nullptr;
    QRadioButton *radio_english = nullptr;
    QComboBox *combo_font = nullptr;
    QSpinBox *spin_menus = nullptr;
    QSpinBox *spin_game = nullptr;
    QCheckBox *check_quick = nullptr;
    QCheckBox *check_dev = nullptr;

    // Graphics page
    QRadioButton *radio_640 = nullptr;
    QRadioButton *radio_800 = nullptr;
    QRadioButton *radio_1024 = nullptr;
    QSlider *slider_smoke = nullptr;
    QCheckBox *chk_viewport = nullptr;
    QCheckBox *chk_ext_info = nullptr;
    QCheckBox *chk_color_anim = nullptr;
    QCheckBox *chk_startup = nullptr;
    QCheckBox *chk_obj_cmds = nullptr;
    QCheckBox *chk_portraits = nullptr;
    QCheckBox *chk_ddraw = nullptr;

    // Sound page
    QCheckBox *chk_game_sfx = nullptr;
    QCheckBox *chk_game_loops = nullptr;
    QCheckBox *chk_game_music = nullptr;
    QCheckBox *chk_front_sfx = nullptr;
    QCheckBox *chk_front_music = nullptr;
    Win3DButton *btn_volume = nullptr;

    // Keyboard page
    QRadioButton *radio_block1 = nullptr;
    QRadioButton *radio_block2 = nullptr;
    QRadioButton *radio_block3 = nullptr;
    QRadioButton *radio_block4 = nullptr;
    Win3DButton *btn_reset = nullptr;
    std::map<std::string, KeyBindInput*> key_inputs;

    // Network page
    QCheckBox *chk_net_active = nullptr;
    Win3DButton *btn_ip_config = nullptr;
    QLineEdit *inp_hostname = nullptr;
    QListWidget *list_hosts = nullptr;
    Win3DButton *btn_host_new = nullptr;
    Win3DButton *btn_host_del = nullptr;
    Win3DButton *btn_host_edit = nullptr;
    QLineEdit *inp_master = nullptr;
    Win3DGroupBox *local_group = nullptr;
    Win3DGroupBox *hosts_group = nullptr;
    Win3DGroupBox *master_group = nullptr;
};
