// Main window side of the network (ExplorerDlg 0x414fe0, 0x415a00, OnLbuttonup / OnRbuttonup on
// the planet symbol, network references in the tree). The network watch itself is NetworkBrowser.

#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "NetworkBrowser.h"

#include <QCoreApplication>
#include <QMenu>
#include <QSoundEffect>

void ClonkLauncher::initNetwork() {
    net = new NetworkBrowser(this);
    connect(net, &NetworkBrowser::statusMessage, this, &ClonkLauncher::logStatus);
    connect(net, &NetworkBrowser::animationChanged, this, [this](int avi, bool loop) { animation->play(avi, loop); });
    connect(net, &NetworkBrowser::folderChanged, this, [this]() { refreshTree(); });
    connect(animation, &AnimateCtrl::clicked, this, &ClonkLauncher::onNetworkSymbolClicked);
    // ExplorerDlg::OnClose: the Network folder is removed (the main window is never deleted)
    connect(qApp, &QCoreApplication::aboutToQuit, net, &NetworkBrowser::shutdown);
    net->init();
}

// ExplorerDlg::OnLbuttonup / OnRbuttonup on the planet symbol (control 2000)
void ClonkLauncher::onNetworkSymbolClicked(Qt::MouseButton button) {
    if (button == Qt::LeftButton) {
        toggleNetwork();
        return;
    }
    if (button != Qt::RightButton)
        return;
    QMenu menu(this);
    menu.setFont(LauncherRes::sysFont());
    QAction *toggle = menu.addAction(LauncherRes::str(net->isActive() ? 50010 : 50002)); // (De)activate network
    menu.addSeparator();
    QAction *props = menu.addAction(LauncherRes::str(50029)); // Properties
    QAction *chosen = menu.exec(QCursor::pos());
    if (chosen == toggle)
        toggleNetwork();
    else if (chosen == props)
        showOptions(5); // Network page
}

// 0x415a00
void ClonkLauncher::toggleNetwork() {
    if (get_cfg("Sound\\FESamples", "1") == "1" && sound_click)
        sound_click->play(); // wave 7002
    net->toggle();
    set_cfg("Network\\Active", net->isActive() ? "1" : "0");
    saveConfig();
}

// network reference: scenario with [Head] NetworkReference=1 (in the Network folder)
bool ClonkLauncher::isNetworkReference(ExplorerItem *item) {
    if (!item || item->type != T_Scenario)
        return false;
    NetReference ref;
    return NetworkBrowser::readReference(item->path, ref) && ref.reference;
}

// ExplorerTree 0x431000: "Message" to the host of a reference
void ClonkLauncher::sendNetworkMessage() {
    ExplorerItem *e = selectedExplorerItem();
    NetReference ref;
    if (e && NetworkBrowser::readReference(e->path, ref))
        net->sendMessage(this, ref);
}
