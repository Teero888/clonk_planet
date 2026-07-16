#pragma once

#include <QDialog>
#include <QLabel>
#include <map>
#include <string>
#include "Win3DWidgets.h"

class ClonkPopupDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClonkPopupDialog(QWidget *parent = nullptr, const QString &text = "Sample Clonk Message", const QString &dump_path = "");
};

class ClonkPlayerPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClonkPlayerPropertiesDialog(QWidget *parent = nullptr, const QString &dump_path = "");
    void loadSettings(int color, int control, int mouse);
    std::map<std::string, int> getSettings() const;

private slots:
    void onColorLeft();
    void onColorRight();
    void onControlLeft();
    void onControlRight();
    void onMouseToggle();

private:
    void cycleAtlas(ClonkAtlasWidget *widget, int count, int delta);

    ClonkTexturedWidget *bg = nullptr;
    ClonkButton *btn_ok = nullptr;
    ClonkButton *btn_cancel = nullptr;
    ClonkArea *color_border = nullptr;
    ClonkArea *keyboard_border = nullptr;
    ClonkArea *mouse_border = nullptr;
    ClonkAtlasWidget *color_atlas = nullptr;
    ClonkAtlasWidget *keyboard_atlas = nullptr;
    ClonkAtlasWidget *mouse_atlas = nullptr;
    Win3DButton *btn_color_left = nullptr;
    Win3DButton *btn_color_right = nullptr;
    Win3DButton *btn_keyboard_left = nullptr;
    Win3DButton *btn_keyboard_right = nullptr;
    Win3DButton *btn_mouse_toggle = nullptr;
    QLabel *lbl_color = nullptr;
    QLabel *lbl_controls = nullptr;

    int pref_mouse_index = 0; // 0: ON, 1: OFF in atlas
};
