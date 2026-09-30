#include "OptPage.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"

#include <QLineEdit>

OptPage::OptPage(int idd, QWidget *parent) : QWidget(parent), idd_(idd) {
    setFont(LauncherRes::sysFont());
    builder_ = std::make_unique<DialogBuilder>(idd, this, DialogBuilder::Standard);
}

OptPage::~OptPage() = default;

QWidget *OptPage::control(int id) const {
    auto it = replaced_.find(id);
    if (it != replaced_.end())
        return it->second;
    return builder_->widget(id);
}

void OptPage::replaceControl(int id, QWidget *w) {
    QWidget *old = control(id);
    w->setParent(this);
    if (old) {
        w->setGeometry(old->geometry());
        w->setFont(old->font());
        w->setEnabled(old->isEnabled());
        w->setVisible(old->isVisibleTo(this));
        w->setObjectName(old->objectName());
        w->stackUnder(old);
        old->hide();
        old->deleteLater();
    }
    replaced_[id] = w;
}

std::string OptPage::cfg(const std::string &key, const std::string &def) {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l ? l->get_cfg(key, def) : def;
}

int OptPage::cfgInt(const std::string &key, int def) {
    const std::string v = cfg(key, std::to_string(def));
    try {
        // registry DWORDs may be stored unsigned (4294967295 = -1)
        return static_cast<int>(static_cast<unsigned int>(std::stoll(v)));
    } catch (...) {
        return def;
    }
}

void OptPage::setCfg(const std::string &key, const std::string &value) {
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->set_cfg(key, value);
}

void OptPage::setCfgInt(const std::string &key, int value) {
    // stored as registry DWORD like the engine does (StdConfig: SetRegistryDWord)
    setCfg(key, std::to_string(static_cast<unsigned int>(value)));
}

void OptPage::setReadOnlyEdit(QWidget *edit) {
    if (auto *e = qobject_cast<QLineEdit *>(edit))
        e->setReadOnly(true);
    QPalette pal = edit->palette();
    pal.setColor(QPalette::Base, QColor(0xec, 0xe9, 0xd8));
    edit->setPalette(pal);
}
