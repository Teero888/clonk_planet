// Options page Network (PageNetwork, IDD 3018)

#include "ClonkDialog.h"
#include "LauncherRes.h"
#include "OptPages.h"

#include <QCheckBox>
#include <QHostInfo>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>

namespace {
enum {
    IDC_ACTIVE = 2075,
    IDC_LOCAL = 2314,
    IDC_NAME_LABEL = 2315,
    IDC_NAME = 2144,
    IDC_HOST = 2305,
    IDC_ADDRESS = 2306,
    IDC_HOSTS = 2197,
    IDC_NEW = 2056,
    IDC_DELETE = 2040,
    IDC_EDIT = 2041,
    IDC_IPCONFIG = 2052,
    IDC_SERVER = 2348,
    IDC_SERVER_ADDRESS = 2153,
};

// FUN_0041f9d0: everything but the Active check box and IP configuration
const int NETWORK_CONTROLS[] = {IDC_LOCAL, IDC_NAME_LABEL, IDC_ADDRESS, IDC_NAME, IDC_HOST, IDC_HOSTS,
                                IDC_NEW, IDC_DELETE, IDC_EDIT, IDC_SERVER, IDC_SERVER_ADDRESS};

// characters removed from the local name (FUN_0042b5e0 with 0x47d090)
const QString FORBIDDEN_NAME_CHARS = QString::fromLatin1("!\"\xa7%&/=?+*#:;");
} // namespace

OptNetworkPage::OptNetworkPage(QWidget *parent) : OptPage(LauncherRes::IDD_PAGE_NETWORK, parent) {
    // FUN_0041f710: texts
    get<QPushButton>(IDC_IPCONFIG)->setText(LauncherRes::str(50020));
    get<QCheckBox>(IDC_ACTIVE)->setText(LauncherRes::str(50101));
    get<QLabel>(IDC_NAME_LABEL)->setText(LauncherRes::str(50241));
    get<QPushButton>(IDC_NEW)->setText(LauncherRes::str(50023));
    get<QPushButton>(IDC_DELETE)->setText(LauncherRes::str(50012));
    get<QPushButton>(IDC_EDIT)->setText(LauncherRes::str(50015));
    // "Address: %s" with Network.LocalAddress (the host name when not determined yet)
    QString address = QString::fromStdString(cfg("Network\\LocalAddress", ""));
    if (address.isEmpty() || address == "Unknown")
        address = QHostInfo::localHostName();
    get<QLabel>(IDC_ADDRESS)->setText(LauncherRes::str(50202).replace("%s", address));

    // FUN_0041f900 (ctor): Active, LocalName (DDV_MaxChars 30), HostList, MasterServerAddress
    active_ = get<QCheckBox>(IDC_ACTIVE);
    active_->setChecked(cfgInt("Network\\Active", 0) != 0);
    QString name = QString::fromStdString(cfg("Network\\LocalName", "Unknown"));
    if (name.isEmpty() || name == "Unknown")
        name = QHostInfo::localHostName().toUpper(); // the computer name
    get<QLineEdit>(IDC_NAME)->setMaxLength(30);
    get<QLineEdit>(IDC_NAME)->setText(name);
    get<QLineEdit>(IDC_SERVER_ADDRESS)->setText(QString::fromStdString(cfg("Network\\MasterServerAddress", "www.clonk.de")));

    // FUN_00402790: host list, ';' separated, blanks trimmed, empty entries skipped
    hosts_ = get<QListWidget>(IDC_HOSTS);
    for (QString host : QString::fromStdString(cfg("Network\\HostList", "")).split(';')) {
        host = host.trimmed();
        if (!host.isEmpty())
            hosts_->addItem(host);
    }

    connect(active_, &QCheckBox::clicked, this, [this]() { enableControls(active_->isChecked()); });
    connect(get<QPushButton>(IDC_NEW), &QPushButton::clicked, this, [this]() { onNew(); });
    connect(get<QPushButton>(IDC_EDIT), &QPushButton::clicked, this, [this]() { onEdit(); });
    connect(get<QPushButton>(IDC_DELETE), &QPushButton::clicked, this, [this]() { onDelete(); });
    connect(hosts_, &QListWidget::itemDoubleClicked, this, [this]() { onEdit(); });
    // PageNetwork::OnIpConfigClicked: winipcfg.exe
    connect(get<QPushButton>(IDC_IPCONFIG), &QPushButton::clicked, this, []() {
#if defined(_WIN32)
        QProcess::startDetached("control.exe", {"ncpa.cpl"});
#else
        QProcess::startDetached("nm-connection-editor", {});
#endif
    });

    // PageNetwork::OnInitDialog
    enableControls(active_->isChecked());
}

void OptNetworkPage::enableControls(bool enable) {
    for (int id : NETWORK_CONTROLS)
        if (QWidget *w = control(id))
            w->setEnabled(enable);
}

// PageNetwork::OnNewClicked
void OptNetworkPage::onNew() {
    QString address;
    if (clonkPrompt(this, LauncherRes::str(51113), address)) // "New host address:"
        hosts_->addItem(address);
}

// PageNetwork::OnEditClicked / LBN_DBLCLK
void OptNetworkPage::onEdit() {
    const int row = hosts_->currentRow();
    if (row < 0)
        return;
    QString address = hosts_->item(row)->text();
    if (clonkPrompt(this, LauncherRes::str(51103), address)) { // "Edit host address:"
        delete hosts_->takeItem(row);
        hosts_->insertItem(row, address);
    }
}

// PageNetwork::OnDeleteClicked
void OptNetworkPage::onDelete() {
    const int row = hosts_->currentRow();
    if (row >= 0)
        delete hosts_->takeItem(row);
}

// FUN_00402830: "a; b; c"
QString OptNetworkPage::hostList() const {
    QStringList list;
    for (int i = 0; i < hosts_->count(); ++i)
        list << hosts_->item(i)->text();
    return list.join("; ");
}

// FUN_0041f870
void OptNetworkPage::apply() {
    setCfgInt("Network\\Active", active_->isChecked() ? 1 : 0);
    QString name = get<QLineEdit>(IDC_NAME)->text();
    for (QChar c : FORBIDDEN_NAME_CHARS)
        name.remove(c);
    setCfg("Network\\LocalName", name.toStdString());
    setCfg("Network\\HostList", hostList().toStdString());
    setCfg("Network\\MasterServerAddress", get<QLineEdit>(IDC_SERVER_ADDRESS)->text().toStdString());
}
