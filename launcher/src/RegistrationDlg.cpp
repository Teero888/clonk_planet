#include "RegistrationDlg.h"
#include "HelpViewer.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "Registration.h"

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>

namespace {
enum {
    IDC_HELP = 2051,
    IDC_NAME = 2151,
    IDC_CODE = 2152,
    IDC_REGCODE_LABEL = 2338,
    IDC_NAME_LABEL = 2329,
};
}

RegistrationDlg::RegistrationDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_REGISTRATION, parent) {
    // RegistrationDlg::OnInitDialog
    setWindowTitle(LauncherRes::str(50524)); // "Registration"
    get<QAbstractButton>(1)->setText(LauncherRes::str(50026));        // "OK"
    get<QAbstractButton>(2)->setText(LauncherRes::str(50006));        // "Cancel"
    get<QAbstractButton>(IDC_HELP)->setText(LauncherRes::str(50019)); // "Help"
    get<QLabel>(IDC_NAME_LABEL)->setText(LauncherRes::str(50270));    // "Player name"
    get<QLabel>(IDC_REGCODE_LABEL)->setText(LauncherRes::str(50287)); // "Registration code"

    // RegistrationDlg::ctor: name = Config.General.Name, code empty; DDV_MaxChars 100 / 10
    auto *name = get<QLineEdit>(IDC_NAME);
    auto *code = get<QLineEdit>(IDC_CODE);
    name->setMaxLength(100);
    code->setMaxLength(10);
    if (ClonkLauncher *l = ClonkLauncher::instance())
        name->setText(QString::fromStdString(l->get_cfg("General\\Name", "")));
    get<QAbstractButton>(1)->setFocus(); // first control of the template

    // RegistrationDlg::OnHelpClicked: WinHelp(HELP_CONTEXT, HID_BASE_RESOURCE + 3030)
    connect(get<QAbstractButton>(IDC_HELP), &QAbstractButton::clicked, this, [this]() { showHelpContext(this, 0x20bd6); });
}

QString RegistrationDlg::name() const { return get<QLineEdit>(IDC_NAME)->text(); }

QString RegistrationDlg::code() const { return get<QLineEdit>(IDC_CODE)->text(); }
