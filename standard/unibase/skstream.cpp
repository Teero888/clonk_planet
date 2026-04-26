#include "skstream.h"

#ifndef _WIN32

int skstream::init() { return 1; }
int skstream::shutdown() { return 1; }

skstream::skstream() : iostream(&_sockbuf), _socket(0), _sockbuf(_socket) { }
skstream::skstream(const char *addr, service svc, role side) : iostream(&_sockbuf), _socket(0), _sockbuf(_socket) { }
skstream::skstream(SOCKET sock) : iostream(&_sockbuf), _socket(sock), _sockbuf(_socket) { }
skstream::~skstream() {}

int skstream::is_open() const { return 0; }
void skstream::open(const char *addr, int port, role side) {}
void skstream::create(int port) {}
void skstream::listen(skstream ** skNewConn) {}
bool skstream::has_socket() { return false; }
void skstream::close() {}
char* skstream::getpeername(char *buf, int size) const { return buf; }
unsigned short skstream::getport() const { return 0; }
int skstream::getlasterror() { return 0; }
BOOL skstream::is_readable() { return FALSE; }
BOOL skstream::is_writeable() { return FALSE; }
void skstream::attach(SOCKET sock) { _socket = sock; }
SOCKET skstream::getsocket() const { return _socket; }

sockbuf::sockbuf(SOCKET &sock) : _socket(sock), streambuf() { _buffer = nullptr; }
sockbuf::sockbuf(SOCKET &sock, char *buf, int length) : _socket(sock), streambuf() { _buffer = nullptr; }
sockbuf::~sockbuf() {}
int sockbuf::overflow(int ch) { return EOF; }
int sockbuf::underflow() { return EOF; }
int sockbuf::sync() { return 0; }

#endif
