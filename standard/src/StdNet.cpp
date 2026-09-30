/* Blocking TCP client sockets for StdHTTP, see StdNet.h. Does not include Compat.h. */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <StdNet.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
static bool NetStartup() {
  static bool started = false;
  if (!started) {
    WSADATA data;
    started = WSAStartup(MAKEWORD(2, 2), &data) == 0;
  }
  return started;
}
#define CloseSocket closesocket
#else
static bool NetStartup() { return true; }
#define CloseSocket close
#endif

StdNetSocket StdNetConnect(const char *szHost, int iPort) {
  if (!NetStartup()) return StdNetInvalid;
  char szPort[16];
  snprintf(szPort, sizeof(szPort), "%d", iPort);
  addrinfo hints, *pResult = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(szHost, szPort, &hints, &pResult) != 0 || !pResult) return StdNetInvalid;
  StdNetSocket hSocket = StdNetInvalid;
  for (addrinfo *pAddr = pResult; pAddr; pAddr = pAddr->ai_next) {
    auto s = socket(pAddr->ai_family, pAddr->ai_socktype, pAddr->ai_protocol);
#ifdef _WIN32
    if (s == INVALID_SOCKET) continue;
#else
    if (s < 0) continue;
#endif
    if (connect(s, pAddr->ai_addr, (int)pAddr->ai_addrlen) == 0) {
      hSocket = (StdNetSocket)s;
      break;
    }
    CloseSocket(s);
  }
  freeaddrinfo(pResult);
  return hSocket;
}

void StdNetClose(StdNetSocket hSocket) {
  if (hSocket != StdNetInvalid) CloseSocket(hSocket);
}

int StdNetSend(StdNetSocket hSocket, const void *pData, int iSize) {
  return send(hSocket, (const char *)pData, iSize, 0);
}

int StdNetRecv(StdNetSocket hSocket, void *pData, int iSize) {
  return recv(hSocket, (char *)pData, iSize, 0);
}

bool StdNetLocalAddress(StdNetSocket hSocket, char *szBuffer, int iBufferSize) {
  sockaddr_in localAddr;
  socklen_t iSize = sizeof(localAddr);
  if (getsockname(hSocket, (sockaddr *)&localAddr, &iSize) != 0) return false;
  return inet_ntop(AF_INET, &localAddr.sin_addr, szBuffer, iBufferSize) != NULL;
}
