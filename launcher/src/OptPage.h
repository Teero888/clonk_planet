#pragma once

// Property pages of the options sheet (OptionsDialog). Each page is built from its dialog template
// (DialogBuilder), reads the config in its constructor (the original page constructors copy the
// C4Config values into members) and writes them back in apply() (the functions the sheet calls
// after DoModal() == IDOK, FUN_00420030). Config keys are those of engine/src/C4Config.cpp.

#include "DialogBuilder.h"

#include <QWidget>
#include <map>
#include <memory>
#include <string>

class OptPage : public QWidget {
    Q_OBJECT
public:
    OptPage(int idd, QWidget *parent = nullptr);
    ~OptPage() override;

    int idd() const { return idd_; }
    QWidget *control(int id) const;
    template <class T> T *get(int id) const { return qobject_cast<T *>(control(id)); }

    // CPropertyPage::OnOK (DDX of the page) - called for every page when the sheet closes with OK
    virtual void onOK() {}
    // write the page's values into the config
    virtual void apply() = 0;

    // config access (ClonkLauncher::get_cfg / set_cfg, keys like "Graphics\\Resolution")
    static std::string cfg(const std::string &key, const std::string &def = "");
    static int cfgInt(const std::string &key, int def);
    static void setCfg(const std::string &key, const std::string &value);
    static void setCfgInt(const std::string &key, int value);

    // ES_READONLY edit: face colored background like the Windows edit control
    static void setReadOnlyEdit(QWidget *edit);

protected:
    // replaces a template control by another widget with the same geometry, font and state
    void replaceControl(int id, QWidget *w);

private:
    int idd_;
    std::unique_ptr<DialogBuilder> builder_;
    std::map<int, QWidget *> replaced_;
};
