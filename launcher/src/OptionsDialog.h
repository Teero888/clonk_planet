#pragma once

// The options property sheet of the original (FUN_0041fc60 / FUN_0041ff00, a CPropertySheet with
// PSH_NOAPPLYNOW): pages Program, Graphics, Sound, Keyboard, Game Pad, Network and, in developer
// mode, Extern and Developer. Tab icons from bitmap 1037, window title "Options".
// exec() == Accepted: every page wrote its values into the config and the config was saved.

#include <QDialog>
#include <vector>

class ClonkLauncher;
class OptPage;
class Win3DTabWidget;

class OptionsDialog : public QDialog {
    Q_OBJECT
public:
    // show_network: open with the Network page (tree context menu "Properties" of the network
    // icon, DAT_0048fea0); otherwise the Program page is active
    explicit OptionsDialog(ClonkLauncher *parent = nullptr, bool show_network = false);

    void setActiveTab(int index);
    OptPage *page(int index) const;

private:
    void onOK();

    ClonkLauncher *launcher = nullptr;
    Win3DTabWidget *tab_widget = nullptr;
    std::vector<OptPage *> pages_;
};
