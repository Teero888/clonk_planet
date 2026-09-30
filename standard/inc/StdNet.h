/* Blocking TCP client sockets for StdHTTP. The header has no system includes: the socket headers
   of Windows (<winsock2.h>) clash with Compat.h, so they are only included by StdNet.cpp. */

#pragma once

#include <stdint.h>

typedef intptr_t StdNetSocket;
const StdNetSocket StdNetInvalid = -1;

// Connects to host:port (host name or numeric address). StdNetInvalid on failure.
StdNetSocket StdNetConnect(const char *szHost, int iPort);
void StdNetClose(StdNetSocket hSocket);
// Returns the number of bytes sent / received, -1 on error, 0 if the connection was closed.
int StdNetSend(StdNetSocket hSocket, const void *pData, int iSize);
int StdNetRecv(StdNetSocket hSocket, void *pData, int iSize);
// Numeric address of our side of the connection.
bool StdNetLocalAddress(StdNetSocket hSocket, char *szBuffer, int iBufferSize);
