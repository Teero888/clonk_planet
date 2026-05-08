
#include <C4Include.h>
#include <StdHTTP.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

CStdHttpMessage::CStdHttpMessage() { Default(); }
CStdHttpMessage::~CStdHttpMessage() { Clear(); }

void CStdHttpMessage::Default() {
  Success = false;
  SCopy("000 No message", Status);
  SCopy("Unknown", ContentType);
  ContentLength = 0;
  Data = NULL;
  Text = false;
}

void CStdHttpMessage::Clear() {
  if (Data) delete[] Data;
  Data = NULL;
  Default();
}

CStdHttp::CStdHttp() { Default(); }
CStdHttp::~CStdHttp() { Clear(); }

void CStdHttp::Default() {
  pHost = NULL;
  Socket = -1;
  SCopy("StdHttp Agent", UserAgent);
  SCopy("Not connected", HostName);
  PostTarget[0] = 0;
  SCopy("text/plain", RequestType);
  SCopy("NOT OBTAINED", ServerStatus);
  SCopy("Not obtained", MessageOfTheDay);
  DataPath[0] = 0;
  SCopy("1.00.0", ClientVersion);
}

void CStdHttp::Clear() { Disconnect(); }

bool CStdHttp::Init(const char *szUserAgent, const char *szPostTarget, const char *szRequestType, const char *szClientVersion) {
  SCopy(szUserAgent, UserAgent, httpMaxString);
  SCopy(szPostTarget, PostTarget, httpMaxString);
  SCopy(szRequestType, RequestType, httpMaxString);
  SCopy(szClientVersion, ClientVersion, httpMaxString);
  return true;
}

bool CStdHttp::Connect(const char *szHost) {
  char szHostName[256];
  SCopy(szHost, szHostName, 255);
  int port = 80;
  
  char* colon = strchr(szHostName, ':');
  if (colon) {
    *colon = 0;
    port = atoi(colon + 1);
  }

  printf("CStdHttp::Connect: Connecting to %s:%d\n", szHostName, port);

  struct hostent *pHostInfo = gethostbyname(szHostName);
  if (!pHostInfo) {
    printf("CStdHttp::Connect: gethostbyname failed for %s\n", szHostName);
    return false;
  }

  SCopy(szHostName, HostName, httpMaxString);

  Socket = socket(AF_INET, SOCK_STREAM, 0);
  if (Socket < 0) {
    printf("CStdHttp::Connect: socket creation failed\n");
    return false;
  }

  struct sockaddr_in remoteAddr;
  memset(&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_port = htons(port);
  memcpy(&remoteAddr.sin_addr, pHostInfo->h_addr_list[0], pHostInfo->h_length);

  if (connect(Socket, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) != 0) {
    printf("CStdHttp::Connect: connect() failed to %s:%d\n", szHostName, port);
    Disconnect();
    return false;
  }
  printf("CStdHttp::Connect: Connected!\n");
  return true;
}

void CStdHttp::Disconnect() {
  if (Socket != -1) close(Socket);
  Socket = -1;
  pHost = NULL;
  SCopy("Not connected", HostName);
}

bool CStdHttp::Get(const char *szFilename, const char *szTarget) {
  if (!szFilename || !szTarget || !szFilename[0] || !szTarget[0]) return false;
  
  char szTargetFilename[_MAX_PATH+1]; SCopy(szTarget,szTargetFilename);
  if (szTargetFilename[SLen(szTargetFilename)-1]=='/') SAppend(GetFilename(szFilename),szTargetFilename);
  
  char szIn[1024 + 1];
  sprintf(szIn, "GET %s/%s %s\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n\r\n", DataPath, szFilename, httpVersion, HostName, UserAgent);
  if (send(Socket, szIn, SLen(szIn), 0) == -1) return false;
  
  CStdHttpMessage Msg;
  if (!Receive(Msg) || !Msg.Success) return false;
  
  CStdFile hFile;
  if (!hFile.Save(szTargetFilename, (BYTE*)Msg.Data, Msg.ContentLength)) return false;
  return true;
}

bool CStdHttp::ReceiveHeader(CStdHttpMessage &rMsg) {
  char szLine[1024 + 1];
  if (!ReceiveLine(szLine, 1024)) return false;
  SCopy(szLine, rMsg.Status, httpMaxString);
  printf("CStdHttp::ReceiveHeader: Status: %s\n", szLine);
  
  rMsg.Success = false;
  char szCode[4]; SCopySegment(szLine, 1, szCode, ' ', 3); 
  if (szCode[0] == '2') rMsg.Success = true;
  
  do {
    if (!ReceiveLine(szLine, 1024)) return false;
    if (SEqual2(szLine, "Content-Length:")) sscanf(szLine, "Content-Length: %d", &rMsg.ContentLength);
    if (SEqual2(szLine, "Content-Type:")) { SCopy(szLine + 13, rMsg.ContentType); SClearFrontBack(rMsg.ContentType); }
  } while (szLine[0]);
  
  if (SEqual2(rMsg.ContentType, "text")) rMsg.Text = true;
  return true;
}

bool CStdHttp::ReceiveLine(char *sBuf, int iBufSize) {
  char bBuf;
  int iResult, iIndex = 0;
  while ((iResult = recv(Socket, &bBuf, 1, 0)) > 0) {
    sBuf[iIndex] = 0;
    if (bBuf == '\n') {
      if (iIndex > 0 && sBuf[iIndex - 1] == '\r') sBuf[iIndex - 1] = 0;
      return true;
    }
    if (iIndex + 1 > iBufSize) return false;
    sBuf[iIndex++] = bBuf; sBuf[iIndex] = 0;
  }
  return false;
}

bool CStdHttp::Post(const char *szText, BYTE *bpBinary, int iBinarySize) {
  char szHeader[1024 + 1];
  sprintf(szHeader, "POST %s %s\r\nHost: %s\r\nUser-Agent: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", 
          PostTarget, httpVersion, HostName, UserAgent, RequestType, SLen(szText) + iBinarySize);
  
  printf("CStdHttp::Post: Sending header (%d bytes)\n", (int)strlen(szHeader));
  if (send(Socket, szHeader, SLen(szHeader), 0) == -1) return false;
  printf("CStdHttp::Post: Sending text (%d bytes)\n", (int)strlen(szText));
  if (send(Socket, szText, SLen(szText), 0) == -1) return false;
  if (bpBinary && iBinarySize > 0) {
    printf("CStdHttp::Post: Sending binary (%d bytes)\n", iBinarySize);
    if (send(Socket, bpBinary, iBinarySize, 0) == -1) return false;
  }
  return true;
}

bool CStdHttp::Receive(CStdHttpMessage &rMsg) {
  rMsg.Clear();
  if (!ReceiveHeader(rMsg)) return false;
  if (!ReceiveBody(rMsg)) return false;
  return true;
}

bool CStdHttp::ReceiveBody(CStdHttpMessage &rMsg) {
  int iResult;
  if (rMsg.ContentLength == 0) {
    // No body expected – success
    rMsg.Data = NULL;
    return true;
  } else {
    rMsg.Data = new char[rMsg.ContentLength + rMsg.Text];
    const int iMaxChunk = 1024;
    int iChunk;
    for (int iReceived = 0; iReceived < rMsg.ContentLength; iReceived += iChunk) {
      iChunk = Min(iMaxChunk, rMsg.ContentLength - iReceived);
      iResult = recv(Socket, rMsg.Data + iReceived, iChunk, 0);
      if (iResult <= 0) return false;
      iChunk = iResult;
    }
    if (rMsg.Text) rMsg.Data[rMsg.ContentLength] = 0;
  }
  return true;
}

bool CStdHttp::GetLocalAddress(const char *szTargetConnect, char *sBuffer) {
  if (!Connect(szTargetConnect)) return false;
  struct sockaddr_in localAddr;
  socklen_t sockaddrinSize = sizeof(localAddr);
  getsockname(Socket, (struct sockaddr *)&localAddr, &sockaddrinSize);
  SCopy(inet_ntoa(localAddr.sin_addr), sBuffer);
  Disconnect();
  return true;
}

bool CStdHttp::GetFile(const char *szHost, const char *szFilename, const char *szTargetPath) {
  if (!Connect(szHost)) return false;
  if (!Get(szFilename, szTargetPath)) return false;
  Disconnect();
  return true;
}

bool CStdHttp::ObtainStatus(const char *szHost) {
  if (!Connect(szHost)) return false;
  char szMessage[1024 + 1];
  sprintf(szMessage, "version=%s&action=status", ClientVersion);
  if (!Post(szMessage)) return false;
  CStdHttpMessage Msg;
  if (!Receive(Msg) || !Msg.Success || !Msg.Text) return false;
  SCopyNamedSegment(Msg.Data, "status", ServerStatus, '&', '=', httpMaxString);
  SCopyNamedSegment(Msg.Data, "motd", MessageOfTheDay, '&', '=', httpMaxString);
  SCopyNamedSegment(Msg.Data, "dataurl", DataPath, '&', '=', httpMaxString);
  Disconnect();
  return true;
}
