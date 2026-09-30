#pragma once

// ScenarioPropertiesDlg (IDD 3027) and ScenarioDefinitionsDlg (IDD 3028) of Planet.exe.

#include "ClonkDialog.h"
#include "GroupEdit.h"
#include "ScenData.h"
#include "ScenPages.h"

#include <QWidget>
#include <memory>
#include <vector>

// Tab control of the scenario properties (SysTabControl32 "MainTab" with the frontend font,
// classic look over the marble background)
class ScenTabCtrl : public QWidget {
    Q_OBJECT
public:
    explicit ScenTabCtrl(QWidget *placeholder);
    void addTab(const QString &text, int param);
    int current() const { return current_; }
    int param(int index) const { return tabs_[index].param; }
    void setCurrent(int index);
    // TCM_ADJUSTRECT(FALSE) + the page adjustment of FUN_00428b20, in parent coordinates
    QRect pageRect() const;

signals:
    void changed(int index); // TCN_SELCHANGE

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *) override { update(); }
    void focusOutEvent(QFocusEvent *) override { update(); }

private:
    struct Tab {
        QString text;
        int param;
    };
    QRect tabRect(int index) const;
    int tabHeight() const;
    std::vector<Tab> tabs_;
    int current_ = 0;
};

class ScenarioPropertiesDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit ScenarioPropertiesDlg(QWidget *parent = nullptr);
    ~ScenarioPropertiesDlg() override;

    // FUN_00428570: definitions, Scenario.txt, title, map. False if the scenario can't be read.
    bool load(const ItemPath &scenario, bool original, const QString &title = QString());
    // FUN_00428d40: writes Scenario.txt (and removes the description of foreign scenarios)
    bool save();

    const ItemPath &path() const { return path_; }
    void showPage(int tab); // TCN_SELCHANGE handler
    void storePages();      // FUN_00428c40: data out of every page into the core
    ScenPage *page(int param) const { return pages_[param]; }
    ScenTabCtrl *tabs() const { return tabs_; }

protected:
    void onOK() override; // ScenarioPropertiesDlg::OnOK

private:
    ItemPath path_;
    bool original_ = false;
    QString maker_;
    QString title_;
    Scen::Core core_;
    ScenContext ctx_;
    ScenTabCtrl *tabs_ = nullptr;
    ScenPage *pages_[5] = {};
};

class QTreeWidget;

class ScenarioDefinitionsDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit ScenarioDefinitionsDlg(QWidget *parent = nullptr);

    // FUN_00427580
    bool load(const ItemPath &scenario, bool original, const QString &title = QString());
    // FUN_004278d0
    bool save();

protected:
    void onOK() override; // ScenarioDefinitionsDlg::OnOK

private:
    void addItem(const QString &module); // FUN_00427b30
    void updateEnable();                 // FUN_00427890
    int mode() const;

    ItemPath path_;
    bool original_ = false;
    QString title_;
    Scen::Core core_;
    QTreeWidget *list_ = nullptr;
};
