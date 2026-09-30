#pragma once

// The pages of the options sheet (see OptPage.h). Order in the sheet: Program, Graphics, Sound,
// Keyboard, Game Pad, Network (+ Extern and Developer in developer mode).

#include "OptPage.h"

#include <QLineEdit>
#include <array>
#include <vector>

class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSlider;
class QSpinBox;
class QTimer;
class OptJoystick;

// PageProgram (IDD 3020)
class OptProgramPage : public OptPage {
    Q_OBJECT
public:
    explicit OptProgramPage(QWidget *parent = nullptr);
    void apply() override;

private:
    void onDeveloperActiveClicked();
    QRadioButton *german_ = nullptr, *english_ = nullptr;
    QComboBox *font_ = nullptr;
    QSpinBox *fe_size_ = nullptr, *rx_size_ = nullptr;
    QCheckBox *quick_start_ = nullptr, *developer_ = nullptr;
};

// PageGraphics (IDD 3015)
class OptGraphicsPage : public OptPage {
    Q_OBJECT
public:
    explicit OptGraphicsPage(QWidget *parent = nullptr);
    void apply() override;
};

// PageSound (IDD 3021)
class OptSoundPage : public OptPage {
    Q_OBJECT
public:
    explicit OptSoundPage(QWidget *parent = nullptr);
    void apply() override;
};

// msctls_hotkey32 replacement: shows the key name, captures the next key press (no modifiers,
// HKM_SETRULES(0xfe)). The value is a Windows virtual key code as used by the engine.
class OptHotKeyEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit OptHotKeyEdit(QWidget *parent = nullptr);
    int hotKey() const { return vk_; }
    void setHotKey(int vk); // HKM_SETHOTKEY
    // name of a virtual key as the hotkey control shows it (GetKeyNameText, US layout)
    static QString keyName(int vk, bool extended = false);
    // Qt key event -> virtual key (0 if unknown), extended key flag
    static int virtualKey(const QKeyEvent *event, bool *extended = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    int vk_ = 0;
};

// PageKeyboard (IDD 3016)
class OptKeyboardPage : public OptPage {
    Q_OBJECT
public:
    explicit OptKeyboardPage(QWidget *parent = nullptr);
    void onOK() override;
    void apply() override;
    // BLOCK1..BLOCK4 radio (1..4)
    void selectBlock(int block);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void onBlockClicked(int block);
    void onResetClicked();
    void storeBlock();
    void showBlock();
    std::array<std::array<int, 12>, 4> keys_{}; // Config.Keyboard
    std::array<OptHotKeyEdit *, 12> hotkeys_{};
    int block_ = 1;
    bool loading_ = true; // +0x90: do not store the controls on the first block switch
};

// PageGamepad (IDD 3014)
class OptGamepadPage : public OptPage {
    Q_OBJECT
public:
    explicit OptGamepadPage(QWidget *parent = nullptr);
    ~OptGamepadPage() override;
    void apply() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void poll();
    void onJoystick(unsigned pos, unsigned buttons);
    void setState(int ctrl, bool on);
    void assign(int ctrl, int button_id);
    std::array<int, 12> buttons_{}; // Config.Gamepad.Button
    std::array<int, 12> state_{};   // highlighted icons
    OptJoystick *joystick_ = nullptr;
    QTimer *timer_ = nullptr;
    unsigned last_pos_ = 0, last_buttons_ = 0;
    int assigning_ = -1, assign_button_ = 0;
};

// PageNetwork (IDD 3018)
class OptNetworkPage : public OptPage {
    Q_OBJECT
public:
    explicit OptNetworkPage(QWidget *parent = nullptr);
    void apply() override;

private:
    void enableControls(bool enable);
    void onNew();
    void onEdit();
    void onDelete();
    QString hostList() const;
    QCheckBox *active_ = nullptr;
    QListWidget *hosts_ = nullptr;
};

// PageEditor (IDD 3011, tab "Extern")
class OptEditorPage : public OptPage {
    Q_OBJECT
public:
    explicit OptEditorPage(QWidget *parent = nullptr);
    void apply() override;

private:
    void enableEditors(bool enable);
    void browse(int edit_id);
};

// PageDeveloper (IDD 3010)
class OptDeveloperPage : public OptPage {
    Q_OBJECT
public:
    explicit OptDeveloperPage(QWidget *parent = nullptr);
    void apply() override;
};
