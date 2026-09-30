#pragma once

// The launcher's network watch (Planet.exe 0x41e180-0x41f210 "C4NetWatch" plus the ExplorerDlg
// network handlers 0x414fe0, 0x415820-0x415a00, OnUserMsg2/16/17).
//
// While the network is active (config Network\Active), a worker thread polls every two seconds:
//   - one entry of the host list (Network\HostList, "; " separated, "Name (address)" allowed):
//     connects with the engine's C4Stream protocol on port 11111 and requests the game reference
//     (each host at most once per 60 s),
//   - every 30th cycle the master server (Network\MasterServerAddress): action=status once for the
//     data url, then action=list&filename=*.c4s, then GET <dataurl>/<filename> of every listed
//     reference.
// Received references (scenario groups with [Head] NetworkReference=1, written by the engine's
// C4Game::SaveNetworkReference) are stored in the "Network" folder <data>/Network.c4f, which is
// created from the BINARY template 5004 (Title.txt, Title.bmp, DescDE.rtf, DescUS.rtf) when the
// network is activated and deleted when it is deactivated or the launcher closes. References older
// than 70 s (entry time in the group) are removed again.
//
// The status bar texts (string ids 51200-51220) and the AVI clip of the planet symbol are reported
// by signals: 6000 network error, 6001 network inactive, 6002 receiving data, 6003 searching,
// 6004 idle/active (clips launcher/data/res/avi/<id>.png, always played looped - ACS_AUTOPLAY).

#include "GroupEdit.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <memory>

class QWidget;
class NetWatchThread;

// [Head] network data of a reference scenario (C4Scenario: Network*)
struct NetReference {
    bool reference = false;   // NetworkReference=1
    bool lobby = false;       // NetworkLobby
    bool developer_mode = false; // NetworkDeveloperMode
    bool no_runtime_join = false; // NetworkNoRuntimeJoin
    QString title;            // Title
    QString host_name;        // NetworkHostName
    QString host_address;     // NetworkHostAddress
    QString filename;         // NetworkFilename (scenario path on the host)
};

// Config the watch works with (copied into the thread at init)
struct NetWatchSettings {
    bool active = false;
    QString local_name;       // Network\LocalName
    QString host_list;        // Network\HostList ("; " separated)
    QString master_address;   // Network\MasterServerAddress without a directory part
    QString master_directory; // directory part of the address or Network\MasterServerDirectory
    QString temp_dir;         // where received files are stored before they go into Network.c4f
};

class NetworkBrowser : public QObject {
    Q_OBJECT
public:
    // AVI ids of the planet symbol (SysAnimate32 control 2000 of the main window)
    enum Avi { AviError = 6000, AviInactive = 6001, AviReceiving = 6002, AviSearching = 6003, AviActive = 6004 };

    explicit NetworkBrowser(QObject *parent = nullptr);
    ~NetworkBrowser() override;

    // ExplorerDlg::NetworkInit (0x414fe0): applies the config (Network\Active, host list, master
    // server, local name). Creates or deletes the Network folder and starts the watch thread.
    // Call at startup (OnInitDialog) and after the options dialog was closed with OK.
    void init();
    // Sets Network\Active (in memory, not saved) and calls init()
    void activate();
    void deactivate();
    // Left click on the planet symbol (0x415a00): flips Network\Active and calls init().
    // (the original first plays wave 7002 if Sound\FESamples - done by the caller)
    void toggle();
    bool isActive() const { return active_; }
    // OnUserMsg7: re-init when active and log "Network reset."
    void reset();

    // The watch pauses while the engine or the options dialog runs (0x41e540 / 0x41e530)
    void pause();
    void resume();

    // ExplorerDlg::OnClose: deletes the Network folder; stops the thread
    void shutdown();

    // <data>/Network.c4f
    QString networkFolder() const;
    // last AVI id reported by animationChanged

    // Scenario.txt [Head] of a scenario item; false if it can't be read
    static bool readReference(const ItemPath &scenario, NetReference &out);
    // Network part of the start checks (ExplorerDlg 0x4152c0) for a reference: 0 if it may be
    // joined, else the string id of the message to show (51134, 51135, 51119)
    static int joinCheck(const NetReference &ref, bool developer_mode);
    // Extra engine arguments of a network start (OnStartClicked): "/Lobby" if Network\Active and
    // Network\Lobby. runNetStartDialog already appends these; call it yourself only when joining a
    // reference (no NetStartDlg).
    static QStringList engineArgs();

    // "Message" of the tree context menu of a reference (0x431000): prompts for a text (51106) and
    // sends it to the host (C4PK_Message). Returns false if cancelled or not a reference.
    bool sendMessage(QWidget *parent, const NetReference &ref);

signals:
    // status bar text (ExplorerDlg::OnUserMsg2 -> SetStatus)
    void statusMessage(const QString &text);
    // AVI clip of the planet symbol changed
    void animationChanged(int aviId, bool loop);
    // Network.c4f was created, deleted or its contents changed: refresh that tree item
    void folderChanged();

private slots:
    void onWorkerStatus(int id, const QString &arg);
    void onWorkerAvi(int id);
    void onReferenceReceived(const QString &file);
    void onPurge();

private:
    friend class NetWatchThread;
    bool ensureFolder();              // 0x415820 / 0x415900(1)
    void removeFolder();              // 0x415900(0)
    void setAvi(int id);
    void status(int id, const QString &arg = {});
    void startThread();
    void stopThread();
    NetWatchSettings readSettings() const;

    bool active_ = false;
    int avi_ = 0;
    QString temp_dir_;
    std::unique_ptr<NetWatchThread> thread_;
};
