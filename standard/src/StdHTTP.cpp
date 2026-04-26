
#include <C4Include.h>
#include <StdHTTP.h>
CStdHttp::CStdHttp() {}
CStdHttp::~CStdHttp() {}
bool CStdHttp::Init(const char *szUserAgent, const char *szPostTarget, const char *szRequestType, const char *szClientVersion) { return FALSE; }
bool CStdHttp::Connect(const char *szHost) { return FALSE; }
bool CStdHttp::Post(const char *szText, BYTE *bpBinary, int iBinarySize) { return FALSE; }
void CStdHttp::Disconnect() {}
bool CStdHttp::Receive(CStdHttpMessage &rMsg) { return FALSE; }
bool CStdHttp::GetLocalAddress(const char *szTargetConnect, char *sBuffer) { return FALSE; }
CStdHttpMessage::CStdHttpMessage() {}
CStdHttpMessage::~CStdHttpMessage() {}
