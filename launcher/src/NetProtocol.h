#pragma once

// Network protocol clients of the original launcher (Planet.exe), on plain POSIX sockets so the
// launcher does not need Qt6::Network. All calls block; they are used from the NetworkBrowser
// worker thread. Every wait is sliced and checks an optional abort flag, so a stopping thread
// never hangs in a socket call.
//
//   NetSocket - TCP connection (the skstream of the original, error codes as skstream: 3 = host
//               not found, 4 = connect failed)
//   NetStream - client side of the engine's C4Stream packet protocol (engine/src/C4Stream.cpp,
//               C4Packet.h): used to request a game reference from a host (port 11111) and to
//               send a message to a host
//   NetHttp   - the launcher's copy of CStdHttp (0x4376e0-0x438070, standard/src/StdHTTP.cpp):
//               master server status / list requests and reference downloads

#include <QByteArray>
#include <QString>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

class NetSocket {
public:
    // skstream::getlasterror values used by the launcher (0x436df0)
    enum Error { Ok = 0, NoSocket = 1, HostNotFound = 3, ConnectFailed = 4, IOError = 8, Aborted = 9 };

    explicit NetSocket(const std::atomic<bool> *abort = nullptr) : abort_(abort) {}
    ~NetSocket() { close(); }
    NetSocket(const NetSocket &) = delete;
    NetSocket &operator=(const NetSocket &) = delete;

    bool connectTo(const QString &host, int port, int timeout_ms = 10000);
    bool isOpen() const { return fd_ >= 0; }
    void close();
    int lastError() const { return error_; }

    bool sendAll(const void *data, size_t size);
    bool sendAll(const QByteArray &data) { return sendAll(data.constData(), data.size()); }
    // exactly size bytes
    bool recvAll(void *data, size_t size);
    // up to size bytes, 0 on orderly close, -1 on error
    int recvSome(void *data, size_t size);
    // one line without the line end (\n or \r\n)
    bool recvLine(std::string &line, size_t max_len = 4096);

    QString peerAddress() const;  // numeric address of the remote side


private:
    // waits until readable (POLLIN) / writeable (POLLOUT); false on timeout, abort or error
    bool waitFor(short events, int timeout_ms);
    bool aborted() const { return abort_ && abort_->load(); }

    intptr_t fd_ = -1; // socket handle (SOCKET on Windows)
    int error_ = Ok;
    int timeout_ms_ = 15000;
    const std::atomic<bool> *abort_;
};

// C4Packet types (engine/inc/C4Packet.h)
enum NetPacketType {
    C4PK_MyName = 2,
    C4PK_GoodBye = 3,
    C4PK_Filename = 4,
    C4PK_RequestNetworkReference = 8,
    C4PK_NetworkReference = 11,
    C4PK_Message = 21,
    C4PK_YourAddress = 27,
};

class NetStream {
public:
    // C4Stream result codes (engine/inc/C4Stream.h)
    enum Result {
        Ok = 0, NoGood = 1, NoPacket = 2, NoData = 3, NoMemory = 4, DataError = 5, SendError = 6,
        ReceiveError = 7, OutOfOrder = 8, NoOpen = 9, Break = 10, WrongPacket = 11, NoFile = 12,
    };

    explicit NetStream(const std::atomic<bool> *abort = nullptr) : sock_(abort) {}

    // C4Stream::Connect(szName, C4STRM_Client, szAddress, NULL, iPort)
    int connectClient(const QString &local_name, const QString &address, int port = 11111);
    int putPacket(int type, const QByteArray &data = {});
    int getPacket(int &type, QByteArray &data);
    int receivePacket(int type, QByteArray &data);
    // C4Stream::ReceiveFile: filename packet, then the data packet; stores it in dir.
    // path receives the full path of the written file.
    int receiveFile(const QString &dir, int type, QString &path);
    void close() { sock_.close(); }

    // skstream error of the connection (NetSocket::Error)
    int socketError() const { return sock_.lastError(); }
    QString peerName() const { return peer_name_; }
    QString peerAddress() const { return sock_.peerAddress(); }

    // C4Stream::ResultText
    static QString resultText(int result);

private:
    NetSocket sock_;
    QString peer_name_;
    int packet_send_ = 0;
    int packet_receive_ = 0;
};

struct NetHttpMessage {
    bool success = false;
    QString status = "000 No message";
    QString content_type = "Unknown";
    QByteArray data;
    bool text = false;
};

class NetHttp {
public:
    explicit NetHttp(const std::atomic<bool> *abort = nullptr) : abort_(abort) {}

    // CStdHttp::Init(agent, post target, request type, client version)
    void init(const QString &agent, const QString &post_target, const QString &request_type,
              const QString &version);
    // host[:port] (port 80 by default like the original; a port is accepted for local servers)
    bool connectTo(const QString &host);
    void disconnect();
    bool post(const QByteArray &text, const QByteArray &binary = {});
    bool receive(NetHttpMessage &msg);
    // GET <path> (absolute path on the server)
    bool getPath(const QString &path, NetHttpMessage &msg);
    // CStdHttp::GetFile: GET <DataPath>/<filename>, saved to target_dir/<filename>
    bool getFile(const QString &host, const QString &filename, const QString &target_dir, QString *saved = nullptr);
    // CStdHttp::ObtainStatus: POST version=..&action=status, reads status, motd and dataurl
    bool obtainStatus(const QString &host);

    QString dataPath() const { return data_path_; }
    void setDataPath(const QString &p) { data_path_ = p; }
    QString motd() const { return motd_; }

private:
    const std::atomic<bool> *abort_;
    std::unique_ptr<NetSocket> sock_;
    QString agent_ = "StdHttp Agent";
    QString host_name_ = "Not connected";
    QString post_target_;
    QString request_type_ = "text/plain";
    QString version_ = "1.00.0";
    QString motd_ = "Not obtained";
    QString data_path_;
};

// Value of a named segment in "a=1&b=2" (SCopyNamedSegment with '&' and '=')
QString netNamedSegment(const QString &text, const QString &name);
// Address inside <..> or (..) or the whole text (SCopyEnclosed as used for host list entries)
QString netEnclosedAddress(const QString &entry, bool angle_brackets = true);
