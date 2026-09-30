#include "NetworkBrowser.h"

#include "C4Group.h"
#include "C4TextDoc.h"
#include "ClonkDialog.h"
#include "ClonkLauncher.h"
#include "LauncherRes.h"
#include "NetProtocol.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QRegularExpression>
#include <QThread>

#include <condition_variable>
#include <ctime>
#include <functional>
#include <mutex>
#include <utime.h>

// Engine version the launcher reports to the master server (C4XVer 4.6.5.0)
static const char *kClientVersion = "4.65.0";                       // "%i.%i%i.%i"
static const char *kUserAgent = "Clonk Planet/4.65 (Frontend)";    // "Clonk Planet/%i.%i%i (Frontend)"
static const char *kRequestType = "application/x-cpmaster-request";
static const int kHostPort = 11111;       // 0x2b67, C4PORT_Control
static const int kMaxHosts = 50;          // 0x32 time stamps
static const int kHostPeriod = 60;        // s between two requests to the same host
static const int kMasterCycles = 30;      // master server every 30th cycle
static const int kReferenceLifetime = 70; // s until a reference is removed from Network.c4f

// ---------------------------------------------------------------------------------------------
// NetWatchCore: the request logic of C4NetWatch, independent of threading. Callbacks deliver
// status texts (string id + argument), AVI ids and received reference files.

class NetWatchCore {
public:
    std::function<void(int, const QString &)> onStatus;
    std::function<void(int)> onAvi;
    std::function<void(const QString &)> onReference;

    explicit NetWatchCore(const std::atomic<bool> *abort) : abort_(abort) {}

    NetWatchSettings settings;
    bool error = false;           // 0x8e6: a transfer failed in this cycle
    bool status_obtained = false; // 0x2b02
    QString data_path;            // dataurl of the master server status

    // 0x41f0a0: AVI, 6001 while inactive
    void avi(int id) {
        if (onAvi)
            onAvi(settings.active ? id : NetworkBrowser::AviInactive);
    }
    void status(int id, const QString &arg = {}) {
        if (onStatus)
            onStatus(id, arg);
    }
    // 0x41e5e0: logs "Network error: %s" for a failed stream call
    int check(int result) {
        if (result != NetStream::Ok)
            status(51220, NetStream::resultText(result));
        return result;
    }

    QString newTempDir() {
        static std::atomic<int> counter{0};
        QString dir = QDir(settings.temp_dir).filePath(QString::number(++counter));
        QDir().mkpath(dir);
        return dir;
    }

    // 0x41e820: requests the game reference of a host
    bool queryHost(const QString &entry) {
        QString address = netEnclosedAddress(entry, true);
        status(51216, entry); // "Searching %s"
        avi(NetworkBrowser::AviSearching);
        NetStream strm(abort_);
        if (strm.connectClient(settings.local_name, address, kHostPort) != NetStream::Ok) {
            int e = strm.socketError();
            status(e == NetSocket::HostNotFound ? 51219 : e == NetSocket::ConnectFailed ? 51218 : 51204, entry);
            return false;
        }
        status(51200, strm.peerAddress()); // "Connect to %s."
        QString peer = strm.peerName();
        status(51214, peer); // "Requesting game information..."
        check(strm.putPacket(C4PK_RequestNetworkReference));
        status(51211); // "Receiving game information..."
        avi(NetworkBrowser::AviReceiving);
        QString path;
        int received = check(strm.receiveFile(newTempDir(), C4PK_NetworkReference, path));
        check(strm.putPacket(C4PK_GoodBye));
        strm.close();
        if (received != NetStream::Ok) {
            error = true;
            if (!path.isEmpty())
                QDir(QFileInfo(path).absolutePath()).removeRecursively();
            return false;
        }
        status(51212, peer); // "Game information received."
        // the file time becomes the entry time in Network.c4f (reference age)
        ::utime(QFile::encodeName(path).constData(), nullptr);
        if (onReference)
            onReference(path);
        return true;
    }

    // 0x41ef60: downloads one reference from the master server
    bool retrieveReference(NetHttp &http, const QString &name, const QString &path_on_server = {}) {
        status(51201, name); // "Retrieving reference %s..."
        avi(NetworkBrowser::AviReceiving);
        QString dir = newTempDir();
        QString saved;
        if (!http.getFile(settings.master_address, path_on_server.isEmpty() ? name : path_on_server, dir, &saved)) {
            QDir(dir).removeRecursively();
            error = true;
            return false;
        }
        if (onReference)
            onReference(saved);
        return true;
    }

    // 0x41f000: every line with "filename=<name>&..." of the list reply
    int retrieveList(NetHttp &http, const QByteArray &data) {
        int count = 0;
        for (const QByteArray &line : data.split('\n')) {
            int pos = line.indexOf("filename=");
            if (pos < 0)
                continue;
            QByteArray name = line.mid(pos + 9);
            int amp = name.indexOf('&');
            if (amp >= 0)
                name = name.left(amp);
            name = name.left(256).trimmed();
            if (name.isEmpty())
                continue;
            if (retrieveReference(http, QString::fromLatin1(name)))
                count++;
        }
        return count;
    }

    // Compatibility with the repo's masterserver/masterserver.cpp, which does not implement
    // action=status / action=list: its game list is an HTML page at the script path with links
    // /file/<ip>/<filename>. Returns -1 if that page is not available.
    int retrieveHtmlList(NetHttp &http) {
        NetHttpMessage page;
        if (!http.connectTo(settings.master_address))
            return -1;
        bool ok = http.getPath(settings.master_directory, page) && page.success;
        http.disconnect();
        if (!ok)
            return -1;
        int count = 0;
        static const QRegularExpression link("href=\"(/file/[^\"]+)\"");
        auto it = link.globalMatch(QString::fromLatin1(page.data));
        while (it.hasNext()) {
            QString path = it.next().captured(1);
            QString name = path.section('/', -1);
            if (retrieveReference(http, name, path))
                count++;
        }
        return count;
    }

    // 0x41ed90: game list of the master server
    void queryMaster() {
        const QString address = settings.master_address;
        avi(NetworkBrowser::AviSearching);
        status(51202, address); // "Retrieving information from server %s..."
        NetHttp http(abort_);
        http.init(kUserAgent, settings.master_directory, kRequestType, kClientVersion);
        if (!status_obtained) {
            status_obtained = http.obtainStatus(address);
            if (status_obtained)
                data_path = http.dataPath();
        }
        http.setDataPath(data_path);
        if (!http.connectTo(address)) {
            status(51207); // "Cannot connect to master server."
            return;
        }
        // "version=%i.%i%i.%i&action=list&filename=*.c4s" (+ line end like the engine's client)
        QByteArray request = QString("version=%1&action=list&filename=*.c4s\r\n").arg(kClientVersion).toLatin1();
        NetHttpMessage msg;
        if (!http.post(request) || !http.receive(msg)) {
            status(51210); // "Data transfer failure."
            error = true;
            return;
        }
        http.disconnect();
        int count = -1;
        if (msg.success && msg.text)
            count = retrieveList(http, msg.data);
        else if (msg.success && msg.data.isEmpty())
            count = retrieveHtmlList(http);
        if (count >= 0) {
            if (count == 0)
                status(51205); // "No game references found on master server."
            else
                status(51213, QString::number(count)); // "%s game references found..."
            return;
        }
        // "Master server request failure (%s).": server error text (first line) or status line
        QString what = msg.status;
        if (msg.text)
            what = QString::fromLatin1(msg.data.left(msg.data.indexOf('\r') >= 0 ? msg.data.indexOf('\r') : msg.data.size()));
        status(51209, what);
        error = true;
    }

private:
    const std::atomic<bool> *abort_;
};

// ---------------------------------------------------------------------------------------------
// NetWatchThread: the C4NetWatch thread loop (0x41e580 / 0x41e670)

class NetWatchThread : public QThread {
public:
    explicit NetWatchThread(NetworkBrowser *browser) : browser_(browser), core_(&terminate_) {
        QPointer<NetworkBrowser> b(browser);
        core_.onStatus = [b](int id, const QString &arg) {
            QMetaObject::invokeMethod(b, [b, id, arg] { if (b) b->onWorkerStatus(id, arg); }, Qt::QueuedConnection);
        };
        core_.onAvi = [b](int id) {
            QMetaObject::invokeMethod(b, [b, id] { if (b) b->onWorkerAvi(id); }, Qt::QueuedConnection);
        };
        core_.onReference = [b](const QString &file) {
            QMetaObject::invokeMethod(b, [b, file] { if (b) b->onReferenceReceived(file); }, Qt::QueuedConnection);
        };
    }

    // NetWatch::Init (0x41e330)
    void configure(const NetWatchSettings &s) {
        std::lock_guard<std::mutex> lock(mutex_);
        bool master_changed = s.master_address != pending_.master_address || s.master_directory != pending_.master_directory;
        pending_ = s;
        reinit_ = true;
        reset_status_ = reset_status_ || master_changed;
    }
    void setPaused(bool p) { paused_ = p; }
    void stop() {
        terminate_ = true;
        cv_.notify_all();
    }

protected:
    void run() override {
        while (cycle()) {
        }
    }

private:
    // one pass of 0x41e670; false ends the thread
    bool cycle() {
        core_.error = false;
        if (terminate_)
            return false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(2000), [this] { return terminate_.load(); });
            if (terminate_)
                return false;
            if (reinit_) {
                core_.settings = pending_;
                reinit_ = false;
                master_countdown_ = 0;
                for (auto &t : host_time_)
                    t = 0;
                if (reset_status_) {
                    core_.status_obtained = false;
                    core_.data_path.clear();
                    reset_status_ = false;
                }
            }
        }
        if (paused_)
            return true;
        if (!core_.settings.active) {
            core_.avi(NetworkBrowser::AviInactive);
            return true;
        }
        // next host of the list (SCopySegment ';', trimmed); wraps to the first one
        QStringList hosts = core_.settings.host_list.split(';');
        if (host_index_ < 0 || host_index_ >= hosts.size())
            host_index_ = 0;
        QString host = hosts.value(host_index_).trimmed();
        if (host.isEmpty()) {
            core_.status(51206); // "No network host specified."
        } else if (host_index_ < kMaxHosts) {
            qint64 now = QDateTime::currentSecsSinceEpoch();
            qint64 &last = host_time_[host_index_];
            if ((last == 0 || now - last > kHostPeriod) && core_.queryHost(host))
                last = QDateTime::currentSecsSinceEpoch();
        }
        host_index_++;
        if (terminate_)
            return false;
        if (core_.settings.active && --master_countdown_ < 1) {
            master_countdown_ = kMasterCycles;
            if (!core_.settings.master_address.isEmpty())
                core_.queryMaster();
        }
        // 0x41eb30: remove old references (done by the main thread, it owns Network.c4f)
        QPointer<NetworkBrowser> b(browser_);
        QMetaObject::invokeMethod(b, [b] { if (b) b->onPurge(); }, Qt::QueuedConnection);
        core_.avi(core_.error ? NetworkBrowser::AviError : NetworkBrowser::AviActive);
        return true;
    }

    NetworkBrowser *browser_;
    std::atomic<bool> terminate_{false};
    std::atomic<bool> paused_{false};
    NetWatchCore core_;
    std::mutex mutex_;
    std::condition_variable cv_;
    NetWatchSettings pending_;
    bool reinit_ = false;
    bool reset_status_ = true;
    int master_countdown_ = 0; // 0x2b04
    int host_index_ = 0;       // DAT_0048fc94
    qint64 host_time_[kMaxHosts] = {};
};

// ---------------------------------------------------------------------------------------------
// NetworkBrowser

namespace {
QString cfg(const char *key, const QString &def) {
    ClonkLauncher *l = ClonkLauncher::instance();
    if (!l)
        return def;
    return QString::fromStdString(l->get_cfg(key, def.toStdString()));
}
} // namespace

NetworkBrowser::NetworkBrowser(QObject *parent) : QObject(parent) {
    temp_dir_ = QDir(QDir::tempPath()).filePath(QString("clonk_netwatch_%1").arg(QCoreApplication::applicationPid()));
}

NetworkBrowser::~NetworkBrowser() {
    stopThread();
    QDir(temp_dir_).removeRecursively();
}

QString NetworkBrowser::networkFolder() const {
    ClonkLauncher *l = ClonkLauncher::instance();
    QString data = l ? l->planetDataPath() : QDir::currentPath();
    return QDir(data).filePath("Network.c4f"); // "%s%s", ExePath, "Network.c4f"
}

NetWatchSettings NetworkBrowser::readSettings() const {
    NetWatchSettings s;
    s.active = cfg("Network\\Active", "0").toInt() != 0;
    s.local_name = cfg("Network\\LocalName", "Unknown");
    // engine key is HostList; older launcher builds wrote Hosts
    s.host_list = cfg("Network\\HostList", cfg("Network\\Hosts", ""));
    QString address = cfg("Network\\MasterServerAddress", "www.clonk.de").trimmed();
    QString directory = cfg("Network\\MasterServerDirectory", "/cgi-bin/cpmaster.pl");
    // NetWatch::Init: an address with a '/' carries the script directory
    int slash = address.indexOf('/');
    if (slash >= 0) {
        directory = address.mid(slash);
        address = address.left(slash);
    }
    s.master_address = address;
    s.master_directory = directory;
    s.temp_dir = temp_dir_;
    return s;
}

void NetworkBrowser::init() {
    // ExplorerDlg::NetworkInit (0x414fe0)
    NetWatchSettings s = readSettings();
    active_ = s.active;
    if (active_)
        ensureFolder();
    else
        removeFolder();
    // NetWatch::Init (0x41e330)
    setAvi(active_ ? AviActive : AviInactive);
    QDir().mkpath(temp_dir_);
    if (active_ && !thread_)
        startThread();
    if (thread_)
        thread_->configure(s);
}

void NetworkBrowser::activate() {
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->set_cfg("Network\\Active", "1");
    init();
}

void NetworkBrowser::deactivate() {
    if (ClonkLauncher *l = ClonkLauncher::instance())
        l->set_cfg("Network\\Active", "0");
    init();
}

void NetworkBrowser::toggle() {
    // 0x415a00
    if (active_)
        deactivate();
    else
        activate();
}

void NetworkBrowser::reset() {
    // ExplorerDlg::OnUserMsg7
    if (cfg("Network\\Active", "0").toInt() != 0) {
        init();
        status(51215); // "Network reset."
    }
}

void NetworkBrowser::pause() {
    if (thread_)
        thread_->setPaused(true);
}

void NetworkBrowser::resume() {
    if (thread_)
        thread_->setPaused(false);
}

void NetworkBrowser::shutdown() {
    // ExplorerDlg::OnClose: 0x415900(0); the NetWatch destructor stops the thread
    removeFolder();
    stopThread();
}

void NetworkBrowser::startThread() {
    thread_ = std::make_unique<NetWatchThread>(this);
    thread_->start();
}

void NetworkBrowser::stopThread() {
    if (!thread_)
        return;
    thread_->stop();
    // the original waits 5 s and then terminates the thread; our socket waits check the flag
    if (!thread_->wait(8000))
        thread_->terminate(), thread_->wait();
    thread_.reset();
}

bool NetworkBrowser::ensureFolder() {
    // 0x415820 / 0x415900(1): create Network.c4f from the BINARY template 5004
    QString folder = networkFolder();
    if (!QFileInfo::exists(folder)) {
        std::vector<uint8_t> tmpl = LauncherRes::binary(5004);
        QFile f(folder);
        if (tmpl.empty() || !f.open(QIODevice::WriteOnly) ||
            f.write(reinterpret_cast<const char *>(tmpl.data()), tmpl.size()) != static_cast<qint64>(tmpl.size())) {
            f.close();
            QFile::remove(folder);
            return false;
        }
        f.close();
        emit folderChanged();
    }
    C4Group grp;
    return GroupEdit::open(ItemPath(folder), grp);
}

void NetworkBrowser::removeFolder() {
    // 0x415900(0): EraseItem(Network.c4f)
    QString folder = networkFolder();
    if (!QFileInfo::exists(folder))
        return;
    if (GroupEdit::remove(ItemPath(folder)))
        emit folderChanged();
}

void NetworkBrowser::setAvi(int id) {
    // ExplorerDlg::OnUserMsg2: the clip is only reopened when it changes
    if (id == avi_)
        return;
    avi_ = id;
    emit animationChanged(id, true);
}

void NetworkBrowser::status(int id, const QString &arg) {
    // ExplorerDlg::OnUserMsg2: Format(LoadResStr(id), arg)
    QString text = LauncherRes::str(id);
    text.replace("%s", arg);
    emit statusMessage(text);
}

void NetworkBrowser::onWorkerStatus(int id, const QString &arg) {
    status(id, arg);
}

void NetworkBrowser::onWorkerAvi(int id) {
    if (!active_)
        id = AviInactive;
    setAvi(id);
}

void NetworkBrowser::onReferenceReceived(const QString &file) {
    // ExplorerDlg::OnUserMsg16: move the received file into Network.c4f
    QString tmp_dir = QFileInfo(file).absolutePath();
    auto cleanup = [&] {
        QFile::remove(file);
        if (tmp_dir.startsWith(temp_dir_))
            QDir(tmp_dir).removeRecursively();
    };
    // a reply that arrives after the network was deactivated must not recreate the folder
    if (!active_) {
        cleanup();
        return;
    }
    // only scenario groups (the master server may answer with an error page)
    C4Group ref;
    QString name = QFileInfo(file).fileName();
    if (!ref.loadFromFile(file.toStdString()) || !ref.hasEntry("Scenario.txt") || !ensureFolder()) {
        cleanup();
        return;
    }
    // C4Group::Move of the original. The engine writes the reference with the plain gzip magic
    // (1f 8b), which GroupEdit::copy does not unpack; groups inside groups must be stored unpacked.
    bool ok;
    QString folder = networkFolder();
    if (QFileInfo(folder).isDir()) {
        C4GroupWriter w;
        w.setHeaderFrom(ref);
        w.addFromGroup(ref);
        ok = w.writeToFile(QDir(folder).filePath(name).toStdString(), true);
    } else {
        ok = GroupEdit::writeFile(ItemPath(folder), name.toStdString(), ref.getRawData());
    }
    cleanup();
    if (ok)
        emit folderChanged();
}

void NetworkBrowser::onPurge() {
    // 0x41eb30 + OnUserMsg17: remove *.c4s / *.c4f references older than 70 s
    QString folder = networkFolder();
    if (!active_ || !QFileInfo::exists(folder))
        return;
    qint64 now = QDateTime::currentSecsSinceEpoch();
    QStringList old;
    auto isRef = [](const QString &n) {
        return n.endsWith(".c4s", Qt::CaseInsensitive) || n.endsWith(".c4f", Qt::CaseInsensitive);
    };
    if (QFileInfo(folder).isDir()) {
        for (const QFileInfo &fi : QDir(folder).entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
            if (isRef(fi.fileName()) && now - fi.lastModified().toSecsSinceEpoch() > kReferenceLifetime)
                old << fi.fileName();
    } else {
        C4Group grp;
        if (!GroupEdit::open(ItemPath(folder), grp))
            return;
        for (const C4GroupEntry &e : grp.getEntries()) {
            QString n = QString::fromStdString(e.name);
            if (isRef(n) && now - static_cast<qint64>(e.time) > kReferenceLifetime)
                old << n;
        }
    }
    bool changed = false;
    for (const QString &n : old)
        changed |= GroupEdit::remove(ItemPath(folder, {n}));
    if (changed)
        emit folderChanged();
}

bool NetworkBrowser::readReference(const ItemPath &scenario, NetReference &out) {
    out = NetReference();
    std::vector<uint8_t> data = GroupEdit::readFile(scenario, "Scenario.txt");
    if (data.empty())
        return false;
    C4TextDoc doc(data);
    out.reference = doc.getInt("Head", "NetworkReference") != 0;
    out.lobby = doc.getInt("Head", "NetworkLobby") != 0;
    out.developer_mode = doc.getInt("Head", "NetworkDeveloperMode") != 0;
    out.no_runtime_join = doc.getInt("Head", "NetworkNoRuntimeJoin") != 0;
    out.title = doc.get("Head", "Title");
    out.host_name = doc.get("Head", "NetworkHostName");
    out.host_address = doc.get("Head", "NetworkHostAddress");
    out.filename = doc.get("Head", "NetworkFilename");
    for (QString *s : {&out.title, &out.host_name, &out.host_address, &out.filename})
        if (s->size() >= 2 && s->startsWith('"') && s->endsWith('"'))
            *s = s->mid(1, s->size() - 2);
    return true;
}

int NetworkBrowser::joinCheck(const NetReference &ref, bool developer_mode) {
    // ExplorerDlg 0x4152c0 (item flag 0x16 = NetworkReference)
    if (!ref.reference)
        return 0;
    if (ref.developer_mode && !developer_mode)
        return 51134; // "This scenario can only be started in developer mode."
    if (!ref.developer_mode && developer_mode)
        return 51135; // "This scenario can only be started in player mode."
    if (ref.no_runtime_join && !ref.lobby)
        return 51119; // "Runtime join for this game denied."
    return 0;
}

QStringList NetworkBrowser::engineArgs() {
    // ExplorerDlg::OnStartClicked: " /Lobby" if Network.Active && Network.Lobby
    QStringList args;
    if (cfg("Network\\Active", "0").toInt() != 0 && cfg("Network\\Lobby", "0").toInt() != 0)
        args << "/Lobby";
    return args;
}

bool NetworkBrowser::sendMessage(QWidget *parent, const NetReference &ref) {
    // 0x431000
    if (ref.host_name.isEmpty() || ref.host_address.isEmpty())
        return false;
    QString text;
    if (!clonkPrompt(parent, LauncherRes::str(51106).replace("%s", ref.host_name), text))
        return false;
    setAvi(AviSearching);
    status(51217, ref.host_name); // "Sending message to %s..."
    QString local_name = cfg("Network\\LocalName", "Unknown");
    QString address = ref.host_address;
    QPointer<NetworkBrowser> b(this);
    QThread *t = QThread::create([b, local_name, address, text] {
        NetStream strm;
        bool ok = strm.connectClient(local_name, address, kHostPort) == NetStream::Ok;
        if (ok) {
            // C4PK_Message, at most 120 characters
            ok = strm.putPacket(C4PK_Message, text.toLocal8Bit().left(120)) == NetStream::Ok;
            if (ok)
                strm.putPacket(C4PK_GoodBye);
            strm.close();
        }
        if (!ok)
            QMetaObject::invokeMethod(b, [b] { if (b) b->setAvi(AviError); }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
    return true;
}

