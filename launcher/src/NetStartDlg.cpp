#include "NetStartDlg.h"

#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "NetworkBrowser.h"

#include <QAbstractButton>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>

namespace {
// control ids (DoDataExchange 0x41dd40)
enum {
    IDC_WAIT = 2087,       // "Wait for clients"          -> 0x2f8 Lobby
    IDC_NOREFREQUEST = 2089, // "Deny game info requests" -> 0x310 NoReferenceRequest
    IDC_NORUNTIMEJOIN = 2090, // "No runtime join"        -> 0x30c NoRuntimeJoin
    IDC_SIGNUP = 2100,     // "Sign up at master server"  -> 0x308 MasterServerSignUp
    IDC_COMMENT = 2110,    // multi line edit             -> 0x2fc Comment
    IDC_RATE = 2111,       // edit + up-down 2260         -> 0x304 ControlRate (DDV 1..10)
    IDC_ADDRESS = 2154,    // edit                        -> 0x300 MasterServerAddress
    IDC_RATE_SPIN = 2260,
    IDC_RATE_LABEL = 2276,
    IDC_COMMENT_LABEL = 2274,
    IDC_ADDRESS_LABEL = 2350,
};

std::string cfg(const char *key, const char *def) {
    ClonkLauncher *l = ClonkLauncher::instance();
    return l ? l->get_cfg(key, def) : std::string(def);
}
bool cfgBool(const char *key) { return cfg(key, "0") != "0" && !cfg(key, "0").empty(); }
void setCfg(const char *key, const QString &value) {
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->set_cfg(key, value.toStdString());
}
} // namespace

NetStartDlg::NetStartDlg(QWidget *parent) : ClonkDialog(LauncherRes::IDD_NETSTART, parent) {
    // The shared skin style sheet ("QCheckBox { background: transparent }") hides the check box
    // indicators with the Fusion style and keeps disabled labels black; override it for this dialog.
    body()->setStyleSheet("QLabel { background: transparent; color: black; }"
                          "QLabel:disabled { color: #aca899; }");

    // OnInitDialog -> 0x41df10: texts
    setWindowTitle(LauncherRes::str(50517)); // "Start Network Game"
    if (auto *b = get<QAbstractButton>(IDC_WAIT))
        b->setText(LauncherRes::str(50216));
    if (auto *b = get<QAbstractButton>(IDC_NORUNTIMEJOIN))
        b->setText(LauncherRes::str(50254));
    if (auto *b = get<QAbstractButton>(IDC_NOREFREQUEST))
        b->setText(LauncherRes::str(50251));
    if (auto *b = get<QAbstractButton>(IDC_SIGNUP))
        b->setText(LauncherRes::str(50322));
    if (auto *l = get<QLabel>(IDC_ADDRESS_LABEL))
        l->setText(LauncherRes::str(50321));
    if (auto *l = get<QLabel>(IDC_COMMENT_LABEL))
        l->setText(LauncherRes::str(50140));
    if (auto *l = get<QLabel>(IDC_RATE_LABEL))
        l->setText(LauncherRes::str(50148));
    if (auto *b = get<QAbstractButton>(1))
        b->setText(LauncherRes::str(50033)); // "Start"
    if (auto *b = get<QAbstractButton>(2))
        b->setText(LauncherRes::str(50006)); // "Cancel"

    // DoModal -> 0x41e090: values from the config, then DDX
    if (auto *b = get<QAbstractButton>(IDC_WAIT))
        b->setChecked(cfgBool("Network\\Lobby"));
    if (auto *b = get<QAbstractButton>(IDC_NORUNTIMEJOIN))
        b->setChecked(cfgBool("Network\\NoRuntimeJoin"));
    if (auto *b = get<QAbstractButton>(IDC_NOREFREQUEST))
        b->setChecked(cfgBool("Network\\NoReferenceRequest"));
    if (auto *b = get<QAbstractButton>(IDC_SIGNUP))
        b->setChecked(cfgBool("Network\\MasterServerSignUp"));
    // UDM_SETRANGE 0x1000a: 1..10
    QSpinBox *rate = get<QSpinBox>(IDC_RATE);
    if (!rate)
        rate = get<QSpinBox>(IDC_RATE_SPIN);
    if (rate) {
        rate->setRange(1, 10);
        int value = 1;
        try {
            value = std::stoi(cfg("Network\\ControlRate", "1"));
        } catch (...) {
        }
        rate->setValue(value);
    }
    if (auto *e = get<QLineEdit>(IDC_ADDRESS)) {
        e->setText(QString::fromStdString(cfg("Network\\MasterServerAddress", "www.clonk.de")));
        // disabled edit of Windows XP: beige background, grey text
        QPalette pal = e->palette();
        pal.setColor(QPalette::Disabled, QPalette::Base, QColor(0xec, 0xe9, 0xd8));
        pal.setColor(QPalette::Disabled, QPalette::Text, QColor(0xac, 0xa8, 0x99));
        e->setPalette(pal);
    }
    QString comment = QString::fromStdString(cfg("Network\\Comment", ""));
    if (auto *e = get<QPlainTextEdit>(IDC_COMMENT))
        e->setPlainText(comment);
    else if (auto *e = get<QLineEdit>(IDC_COMMENT))
        e->setText(comment);

    // master server address enabled by the sign up flag
    updateSignUp();
    if (auto *b = get<QAbstractButton>(IDC_SIGNUP))
        connect(b, &QAbstractButton::clicked, this, &NetStartDlg::updateSignUp);
    // OnWaitForParticipantsClicked (0x421000) does nothing
}

void NetStartDlg::updateSignUp() {
    // OnInitDialog / OnSignUpAtServerClicked: EnableWindow(check == 1) on the edit and its label
    auto *b = get<QAbstractButton>(IDC_SIGNUP);
    bool on = b && b->isChecked();
    if (QWidget *w = control(IDC_ADDRESS))
        w->setEnabled(on);
    if (QWidget *w = control(IDC_ADDRESS_LABEL))
        w->setEnabled(on);
}

void NetStartDlg::onOK() {
    // DDX (UpdateData TRUE) + 0x41dff0: values to the config
    auto checked = [this](int id) {
        auto *b = get<QAbstractButton>(id);
        return QString(b && b->isChecked() ? "1" : "0");
    };
    setCfg("Network\\Lobby", checked(IDC_WAIT));
    setCfg("Network\\NoRuntimeJoin", checked(IDC_NORUNTIMEJOIN));
    setCfg("Network\\NoReferenceRequest", checked(IDC_NOREFREQUEST));
    QSpinBox *rate = get<QSpinBox>(IDC_RATE);
    if (!rate)
        rate = get<QSpinBox>(IDC_RATE_SPIN);
    if (rate)
        setCfg("Network\\ControlRate", QString::number(rate->value()));
    setCfg("Network\\MasterServerSignUp", checked(IDC_SIGNUP));
    if (auto *e = get<QLineEdit>(IDC_ADDRESS))
        setCfg("Network\\MasterServerAddress", e->text());
    QString comment;
    if (auto *e = get<QPlainTextEdit>(IDC_COMMENT))
        comment = e->toPlainText();
    else if (auto *e = get<QLineEdit>(IDC_COMMENT))
        comment = e->text();
    // the config file stores one line per value (C4Config Comment is a single string)
    comment.replace("\r\n", " ").replace('\n', ' ');
    setCfg("Network\\Comment", comment);
    accept();
}

bool runNetStartDialog(QWidget *parent, QStringList &engine_args) {
    // ExplorerDlg::OnStartClicked: NetStartDlg::DoModal (0x41dfc0) != IDOK -> no start
    NetStartDlg dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    // the engine reads the network settings from the config file (saved before CreateProcess)
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->saveConfig();
    engine_args << NetworkBrowser::engineArgs();
    return true;
}
