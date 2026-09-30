#pragma once

// IDSelectDlg (IDD 3003): "Object selection: <caption>" - all definitions of a category as
// symbols or details (name, value, description). The selected objects are returned as an ID list
// with count 1 each (IDSelectDlg::OnOK, Planet.exe 0x41ac50).

#include "ClonkDialog.h"
#include "ScenData.h"

class DefinitionDB;
class QListWidget;
class QTreeWidget;

class IDSelectDlg : public ClonkDialog {
    Q_OBJECT
public:
    explicit IDSelectDlg(QWidget *parent = nullptr);

    // members the caller sets before DoModal (FUN_00419d40)
    void setup(uint32_t category, const QString &caption, const DefinitionDB *defs);
    const Scen::IDList &selected() const { return selected_; }

    // 0 = symbols, 1 = details (IDSelectDlg::OnSymbolsClicked / OnDetailsClicked)
    void setViewMode(int mode);

protected:
    void onOK() override;

private:
    void fill();
    void sort(int column); // LVM_SORTITEMS with the compare function 0x41ae30
    QStringList selectedIds() const;

    uint32_t category_ = 0;
    QString caption_;
    const DefinitionDB *defs_ = nullptr;
    Scen::IDList selected_;
    int view_mode_ = 0;
    int sort_column_ = 0;
    int sort_dir_ = 1;
    QStringList order_; // ids in list order
    QListWidget *symbols_ = nullptr;
    QTreeWidget *details_ = nullptr;
};
