#include "NetProtocol.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <cerrno>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// Platform differences of the BSD socket API (winsock on Windows)
namespace {
#ifdef _WIN32
struct WinsockStartup {
    WinsockStartup() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
} winsock_startup;
using socklen_type = int;
int socketError() { return WSAGetLastError(); }
bool isRetry(int e) { return e == WSAEWOULDBLOCK || e == WSAEINTR; }
bool isConnectPending(int e) { return e == WSAEWOULDBLOCK || e == WSAEINPROGRESS; }
bool isInterrupted(int e) { return e == WSAEINTR; }
int pollSocket(pollfd *p, int timeout_ms) { return WSAPoll(p, 1, timeout_ms); }
void setNonBlocking(intptr_t fd) {
    u_long on = 1;
    ioctlsocket(static_cast<SOCKET>(fd), FIONBIO, &on);
}
void closeSocket(intptr_t fd) { closesocket(static_cast<SOCKET>(fd)); }
constexpr int kSendFlags = 0;
#else
using socklen_type = socklen_t;
int socketError() { return errno; }
bool isRetry(int e) { return e == EAGAIN || e == EINTR; }
bool isConnectPending(int e) { return e == EINPROGRESS; }
bool isInterrupted(int e) { return e == EINTR; }
int pollSocket(pollfd *p, int timeout_ms) { return ::poll(p, 1, timeout_ms); }
void setNonBlocking(intptr_t fd) { ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK); }
void closeSocket(intptr_t fd) { ::close(fd); }
constexpr int kSendFlags = MSG_NOSIGNAL;
#endif
} // namespace

// ---------------------------------------------------------------------------------------------
// NetSocket

namespace {
constexpr int kSlice = 200; // ms per wait slice (abort flag check)
}

bool NetSocket::waitFor(short events, int timeout_ms) {
    int waited = 0;
    while (true) {
        if (aborted()) {
            error_ = Aborted;
            return false;
        }
        pollfd p{};
        p.fd = fd_;
        p.events = events;
        int r = pollSocket(&p, kSlice);
        if (r > 0)
            return true;
        if (r < 0 && !isInterrupted(socketError())) {
            error_ = IOError;
            return false;
        }
        waited += kSlice;
        if (timeout_ms >= 0 && waited >= timeout_ms) {
            error_ = IOError;
            return false;
        }
    }
}

bool NetSocket::connectTo(const QString &host, int port, int timeout_ms) {
    close();
    error_ = Ok;
    // resolve (skstream: inet_addr, else gethostbyname -> error 3)
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *res = nullptr;
    QByteArray h = host.trimmed().toLocal8Bit();
    if (h.isEmpty() || ::getaddrinfo(h.constData(), nullptr, &hints, &res) != 0 || !res) {
        error_ = HostNotFound;
        return false;
    }
    sockaddr_in addr{};
    std::memcpy(&addr, res->ai_addr, sizeof(addr));
    ::freeaddrinfo(res);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    fd_ = static_cast<intptr_t>(::socket(AF_INET, SOCK_STREAM, 0));
    if (fd_ < 0) {
        fd_ = -1;
        error_ = NoSocket;
        return false;
    }
    setNonBlocking(fd_);
    int r = ::connect(fd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (r != 0 && !isConnectPending(socketError())) {
        close();
        error_ = ConnectFailed;
        return false;
    }
    if (r != 0) {
        if (!waitFor(POLLOUT, timeout_ms)) {
            int e = error_;
            close();
            error_ = e == Aborted ? Aborted : ConnectFailed;
            return false;
        }
        int so_error = 0;
        socklen_type len = sizeof(so_error);
        ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&so_error), &len);
        if (so_error != 0) {
            close();
            error_ = ConnectFailed;
            return false;
        }
    }
    int one = 1;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&one), sizeof(one));
    return true;
}

void NetSocket::close() {
    if (fd_ >= 0)
        closeSocket(fd_);
    fd_ = -1;
}

bool NetSocket::sendAll(const void *data, size_t size) {
    const char *p = static_cast<const char *>(data);
    while (size > 0) {
        if (fd_ < 0 || !waitFor(POLLOUT, timeout_ms_))
            return false;
        const int n = ::send(fd_, p, static_cast<int>(size), kSendFlags);
        if (n < 0) {
            if (isRetry(socketError()))
                continue;
            error_ = IOError;
            return false;
        }
        p += n;
        size -= static_cast<size_t>(n);
    }
    return true;
}

int NetSocket::recvSome(void *data, size_t size) {
    while (true) {
        if (fd_ < 0 || !waitFor(POLLIN, timeout_ms_))
            return -1;
        const int n = ::recv(fd_, static_cast<char *>(data), static_cast<int>(size), 0);
        if (n < 0) {
            if (isRetry(socketError()))
                continue;
            error_ = IOError;
            return -1;
        }
        return static_cast<int>(n);
    }
}

bool NetSocket::recvAll(void *data, size_t size) {
    char *p = static_cast<char *>(data);
    while (size > 0) {
        int n = recvSome(p, size);
        if (n <= 0) {
            if (n == 0)
                error_ = IOError;
            return false;
        }
        p += n;
        size -= static_cast<size_t>(n);
    }
    return true;
}

bool NetSocket::recvLine(std::string &line, size_t max_len) {
    line.clear();
    char c;
    while (true) {
        if (recvSome(&c, 1) != 1)
            return false;
        if (c == '\n') {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            return true;
        }
        if (line.size() >= max_len)
            return false;
        line.push_back(c);
    }
}

static QString sockAddrString(intptr_t fd, bool peer) {
    if (fd < 0)
        return {};
    sockaddr_in a{};
    socklen_type len = sizeof(a);
    int r = peer ? ::getpeername(fd, reinterpret_cast<sockaddr *>(&a), &len)
                 : ::getsockname(fd, reinterpret_cast<sockaddr *>(&a), &len);
    if (r != 0)
        return {};
    char buf[INET_ADDRSTRLEN] = {};
    ::inet_ntop(AF_INET, &a.sin_addr, buf, sizeof(buf));
    return QString::fromLatin1(buf);
}

QString NetSocket::peerAddress() const { return sockAddrString(fd_, true); }
// ---------------------------------------------------------------------------------------------
// NetStream (C4Stream, client role)

namespace {
// C4PacketHeader, #pragma pack(1): char Head[5] = "C4PK", int Type, int Size, int Number
constexpr int kHeaderSize = 17;

void putInt(char *p, int32_t v) {
    uint32_t u = static_cast<uint32_t>(v);
    p[0] = static_cast<char>(u & 0xff);
    p[1] = static_cast<char>((u >> 8) & 0xff);
    p[2] = static_cast<char>((u >> 16) & 0xff);
    p[3] = static_cast<char>((u >> 24) & 0xff);
}
int32_t getInt(const char *p) {
    const auto *u = reinterpret_cast<const unsigned char *>(p);
    return static_cast<int32_t>(u[0] | (u[1] << 8) | (u[2] << 16) | (static_cast<uint32_t>(u[3]) << 24));
}
} // namespace

int NetStream::connectClient(const QString &local_name, const QString &address, int port) {
    packet_send_ = packet_receive_ = 0;
    if (!sock_.connectTo(address, port))
        return NoOpen;
    // Send name
    if (putPacket(C4PK_MyName, local_name.toLocal8Bit()) != Ok)
        return SendError;
    // Tell host his own address
    if (putPacket(C4PK_YourAddress, sock_.peerAddress().toLatin1()) != Ok)
        return SendError;
    // Receive the host's name
    QByteArray name;
    if (receivePacket(C4PK_MyName, name) != Ok)
        return ReceiveError;
    peer_name_ = QString::fromLocal8Bit(name);
    return Ok;
}

int NetStream::putPacket(int type, const QByteArray &data) {
    char head[kHeaderSize] = {'C', '4', 'P', 'K', 0};
    putInt(head + 5, type);
    putInt(head + 9, data.size());
    putInt(head + 13, packet_send_++);
    if (!sock_.isOpen())
        return NoGood;
    if (!sock_.sendAll(head, sizeof(head)))
        return NoGood;
    if (!data.isEmpty() && !sock_.sendAll(data))
        return NoGood;
    return Ok;
}

int NetStream::getPacket(int &type, QByteArray &data) {
    data.clear();
    char head[kHeaderSize];
    if (!sock_.isOpen() || !sock_.recvAll(head, sizeof(head)))
        return NoGood;
    // (the engine's CrapRecovery resynchronisation is not needed for our short sessions)
    if (std::memcmp(head, "C4PK", 5) != 0)
        return NoPacket;
    type = getInt(head + 5);
    int32_t size = getInt(head + 9);
    int32_t number = getInt(head + 13);
    if (size < 0 || size > 64 * 1024 * 1024)
        return DataError;
    if (size > 0) {
        data.resize(size);
        if (!sock_.recvAll(data.data(), static_cast<size_t>(size)))
            return DataError;
    }
    if (number != packet_receive_++)
        return OutOfOrder;
    return Ok;
}

int NetStream::receivePacket(int type, QByteArray &data) {
    int got = 0;
    int r = getPacket(got, data);
    if (r != Ok)
        return r;
    if (got != type)
        return WrongPacket;
    return Ok;
}

int NetStream::receiveFile(const QString &dir, int type, QString &path) {
    QByteArray name;
    int r = receivePacket(C4PK_Filename, name);
    if (r != Ok)
        return r;
    // only the file name part (the host sends GetFilename(ScenarioFilename))
    QString file = QFileInfo(QString::fromLocal8Bit(name).replace('\\', '/')).fileName();
    if (file.isEmpty() || file == "." || file == "..")
        return NoFile;
    path = QDir(dir).filePath(file);
    QByteArray data;
    r = receivePacket(type, data);
    if (r != Ok)
        return r;
    QFile::remove(path); // Overwrite any old duplicate
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(data) != data.size())
        return NoFile;
    return Ok;
}

QString NetStream::resultText(int result) {
    switch (result) {
    case Ok: return "No error";
    case NoGood: return "No good";
    case NoPacket: return "No packet";
    case NoData: return "No data";
    case NoMemory: return "No memory";
    case DataError: return "Data error";
    case SendError: return "Send error";
    case ReceiveError: return "Receive error";
    case OutOfOrder: return "Packet out of order";
    case NoOpen: return "Stream not open";
    case Break: return "Break";
    case WrongPacket: return "Incorrect packet type received";
    case NoFile: return "File error";
    }
    return "Undefined result";
}

// ---------------------------------------------------------------------------------------------
// NetHttp (CStdHttp of the launcher)

void NetHttp::init(const QString &agent, const QString &post_target, const QString &request_type,
                   const QString &version) {
    agent_ = agent;
    post_target_ = post_target;
    request_type_ = request_type;
    version_ = version;
}

bool NetHttp::connectTo(const QString &host_in) {
    disconnect();
    QString host = host_in.trimmed();
    int port = 80; // htons(0x50) in the original; host:port is an extension for local servers
    int colon = host.lastIndexOf(':');
    if (colon > 0) {
        bool ok = false;
        int p = host.mid(colon + 1).toInt(&ok);
        if (ok) {
            port = p;
            host = host.left(colon);
        }
    }
    sock_ = std::make_unique<NetSocket>(abort_);
    if (!sock_->connectTo(host, port)) {
        sock_.reset();
        return false;
    }
    host_name_ = host_in.trimmed();
    return true;
}

void NetHttp::disconnect() {
    sock_.reset();
    host_name_ = "Not connected";
}

bool NetHttp::post(const QByteArray &text, const QByteArray &binary) {
    if (!sock_)
        return false;
    // Original header (0x47d5dc) uses bare \n line ends; \r\n is sent instead, which every HTTP
    // server (and the repo's masterserver, which only splits at \r\n) accepts.
    QByteArray head = QString("POST %1 %2\r\nHost: %3\r\nUser-Agent: %4\r\nContent-Type: %5\r\nContent-Length: %6\r\n\r\n")
                          .arg(post_target_, "HTTP/1.0", host_name_, agent_, request_type_)
                          .arg(text.size() + binary.size())
                          .toLatin1();
    if (!sock_->sendAll(head) || !sock_->sendAll(text))
        return false;
    if (!binary.isEmpty() && !sock_->sendAll(binary))
        return false;
    return true;
}

bool NetHttp::receive(NetHttpMessage &msg) {
    msg = NetHttpMessage();
    if (!sock_)
        return false;
    std::string line;
    if (!sock_->recvLine(line, 1024))
        return false;
    msg.status = QString::fromLatin1(line.c_str());
    // "HTTP/1.0 200 OK": success if the code starts with 2
    QStringList parts = msg.status.split(' ');
    msg.success = parts.size() > 1 && parts[1].startsWith('2');
    long content_length = -1;
    do {
        if (!sock_->recvLine(line, 1024))
            return false;
        QString l = QString::fromLatin1(line.c_str());
        if (l.startsWith("Content-Length:", Qt::CaseInsensitive))
            content_length = l.mid(15).trimmed().toLong();
        if (l.startsWith("Content-Type:", Qt::CaseInsensitive))
            msg.content_type = l.mid(13).trimmed();
    } while (!line.empty());
    msg.text = msg.content_type.startsWith("text");
    if (content_length > 0) {
        if (content_length > 64L * 1024 * 1024)
            return false;
        msg.data.resize(static_cast<int>(content_length));
        if (!sock_->recvAll(msg.data.data(), static_cast<size_t>(content_length)))
            return false;
    } else if (content_length < 0) {
        // no length: read until the server closes the connection
        char buf[4096];
        int n;
        while ((n = sock_->recvSome(buf, sizeof(buf))) > 0) {
            msg.data.append(buf, n);
            if (msg.data.size() > 64 * 1024 * 1024)
                return false;
        }
    }
    return true;
}

bool NetHttp::getPath(const QString &path, NetHttpMessage &msg) {
    if (!sock_)
        return false;
    // GET format of the original (0x47d564)
    QByteArray req = QString("GET %1 %2\r\nHost: %3\r\nUser-Agent: %4\r\n\r\n\r\n")
                         .arg(path, "HTTP/1.0", host_name_, agent_)
                         .toLatin1();
    if (!sock_->sendAll(req))
        return false;
    return receive(msg);
}

bool NetHttp::getFile(const QString &host, const QString &filename, const QString &target_dir, QString *saved) {
    // 0x438030: Connect + Get + Disconnect
    if (filename.isEmpty() || target_dir.isEmpty())
        return false;
    if (!connectTo(host))
        return false;
    NetHttpMessage msg;
    QString path = filename.startsWith('/') ? filename : data_path_ + "/" + filename;
    bool ok = getPath(path, msg) && msg.success;
    disconnect();
    if (!ok)
        return false;
    QString name = QFileInfo(filename).fileName();
    QString target = QDir(target_dir).filePath(name);
    QFile::remove(target);
    QFile f(target);
    if (!f.open(QIODevice::WriteOnly) || f.write(msg.data) != msg.data.size())
        return false;
    if (saved)
        *saved = target;
    return true;
}

bool NetHttp::obtainStatus(const QString &host) {
    // 0x438070
    if (!connectTo(host))
        return false;
    // "\r\n" like the engine's master server client (the repo's masterserver needs a line end)
    QByteArray text = QString("version=%1&action=status\r\n").arg(version_).toLatin1();
    NetHttpMessage msg;
    bool ok = post(text) && receive(msg) && msg.success && msg.text;
    disconnect();
    if (!ok)
        return false;
    QString data = QString::fromLatin1(msg.data);
    motd_ = netNamedSegment(data, "motd");
    data_path_ = netNamedSegment(data, "dataurl");
    return true;
}

QString netNamedSegment(const QString &text, const QString &name) {
    for (const QString &seg : text.split('&')) {
        int eq = seg.indexOf('=');
        if (eq > 0 && seg.left(eq).trimmed() == name)
            return seg.mid(eq + 1).trimmed();
    }
    return {};
}

QString netEnclosedAddress(const QString &entry, bool angle_brackets) {
    auto enclosed = [&](QChar open, QChar close) -> QString {
        int a = entry.indexOf(open);
        if (a < 0)
            return {};
        int b = entry.indexOf(close, a + 1);
        if (b < 0)
            return {};
        return entry.mid(a + 1, b - a - 1);
    };
    QString r;
    if (angle_brackets)
        r = enclosed('<', '>');
    if (r.isEmpty())
        r = enclosed('(', ')');
    if (r.isEmpty())
        r = entry;
    return r.trimmed();
}
