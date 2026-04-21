/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Basic http access */

#include <Windows.h>
#include <Winsock.h>
#include <StdIO.h>
#include <StdLib.h>

#include <Standard.h>
#include <StdFile.h>
#include <CStdFile.h>
#include <StdHttp.h>

CStdHttpMessage::CStdHttpMessage()
	{
	Default();
	}

CStdHttpMessage::~CStdHttpMessage()
	{
	Clear();
	}

void CStdHttpMessage::Default()
	{
	Success=false;
	SCopy("000 No message",Status);
	SCopy("Unknown",ContentType);
	ContentLength=0;
	Data=NULL;
	Text=false;
	}

void CStdHttpMessage::Clear()
	{
	if (Data) delete [] Data; Data=NULL;
	Default();
	}

CStdHttp::CStdHttp()
	{
	Default();
	}

CStdHttp::~CStdHttp()
	{
	Clear();
	}

bool CStdHttp::Init(const char *szUserAgent, const char *szPostTarget, const char *szRequestType, const char *szClientVersion)
	{
	// Init winsock
	WSADATA wsadata; ZeroMem(&wsadata,sizeof wsadata);
	if (WSAStartup(MAKEWORD(1,1), &wsadata)!=0) return false;
	// Set data
	SCopy(szUserAgent,UserAgent,httpMaxString);
	SCopy(szPostTarget,PostTarget,httpMaxString);
	SCopy(szRequestType,RequestType,httpMaxString);
	SCopy(szClientVersion,ClientVersion,httpMaxString);
	// Success
	return true;
	}

void CStdHttp::Default()
	{
	pHost=NULL;
	Socket=INVALID_SOCKET;
	SCopy("StdHttp Agent",UserAgent);
	SCopy("Not connected",HostName);
	PostTarget[0]=0;
	SCopy("text/plain",RequestType);
	SCopy("NOT OBTAINED",ServerStatus);
	SCopy("Not obtained",MessageOfTheDay);
	DataPath[0]=0;
	SCopy("1.00.0",ClientVersion);
	}

void CStdHttp::Clear()
	{
	Disconnect();
	}

bool CStdHttp::Connect(const char *szHost)
	{
	// Get host (by name only)
	HOSTENT* pHost = gethostbyname(szHost);
	if (!pHost) return false;
	// Store host name
	SCopy(szHost,HostName,httpMaxString);
	// Create socket
	Socket = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN remoteAddr; ZeroMem(&remoteAddr,sizeof remoteAddr);
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(80);
	memcpy((char*) &(remoteAddr.sin_addr), pHost->h_addr, pHost->h_length);
	// Connect
	int iResult = connect(Socket, (SOCKADDR*) &remoteAddr, sizeof remoteAddr);
	if (iResult != 0) { Disconnect();	return false; }
	// Success
	return true;
	}

void CStdHttp::Disconnect()
	{
	if (Socket!=INVALID_SOCKET) closesocket(Socket); Socket=INVALID_SOCKET;
	pHost=NULL;
	SCopy("Not connected",HostName);
	}

bool CStdHttp::Get(const char *szFilename, const char *szTarget)
	{	
	// Safety
	if (!szFilename || !szTarget || !szFilename[0] || !szTarget[0]) return false;
	// Target is path only: compose target filename
	char szTargetFilename[_MAX_PATH+1]; SCopy(szTarget,szTargetFilename);
	if (szTargetFilename[SLen(szTargetFilename)-1]==Backslash) SAppend(GetFilename(szFilename),szTargetFilename);
	// Send request
	char szIn[1024+1];
	sprintf(szIn,"GET %s/%s %s\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n\r\n",DataPath,szFilename,httpVersion,HostName,UserAgent);
	if ( send(Socket, szIn, SLen(szIn), 0) == SOCKET_ERROR ) return false;
	// Receive message
	CStdHttpMessage Msg;
	if (!Receive(Msg) || !Msg.Success) 
		return false;
	// Save file
	CStdFile hFile;
	if (!hFile.Save(szTargetFilename,(BYTE*)Msg.Data,Msg.ContentLength)) return false;
	// Success
	return true;
	}

bool CStdHttp::ReceiveHeader(CStdHttpMessage &rMsg)
	{
	char szLine[1024+1];
	// Receive status
	if (!ReceiveLine(szLine,1024)) return false;
	SCopy(szLine,rMsg.Status,httpMaxString);
	// Check status
	rMsg.Success=false;
	char szCode[4]; SCopySegment(szLine,1,szCode,' ',3); if (szCode[0]=='2') rMsg.Success=true;
	// Receive lines until empty line
	do	
		{
		// Receive next line
		if (!ReceiveLine(szLine,1024)) return false;
		// Evaluate line
		if (SEqual2(szLine,"Content-Length:")) sscanf(szLine,"Content-Length: %d",&rMsg.ContentLength);
		if (SEqual2(szLine,"Content-Type:")) { SCopy(szLine+13,rMsg.ContentType); SClearFrontBack(rMsg.ContentType); }
		}
	while (szLine[0]);
	// Check content type
	if (SEqual2(rMsg.ContentType,"text")) rMsg.Text=true;
	// Success
	return true;
	}

bool CStdHttp::ReceiveLine(char *sBuf, int iBufSize)
	{
	char bBuf;
	int iResult,iIndex=0;
	// Receive characters
	while ( (iResult = recv(Socket,&bBuf,1,0)) && (iResult != SOCKET_ERROR) )
		{
		sBuf[iIndex]=0;
		// Line feed
		if (bBuf=='\n')
			{ 
			// Remove preceding carriage return
			if (iIndex>0) if (sBuf[iIndex-1]=='\r') sBuf[iIndex-1]=0; 
			// Success
			return true;
			}
		// Line buffer overflow
		if (iIndex+1>iBufSize) return false;
		// Append character
		sBuf[iIndex++]=bBuf; sBuf[iIndex]=0;
		}
	// Failure
	return false;
	}

bool CStdHttp::Post(const char *szText, BYTE *bpBinary, int iBinarySize)
	{
	// Compose header
	char szHeader[1024+1];
	sprintf(szHeader, "POST %s %s\nHost: %s\nUser-Agent: %s\nContent-Type: %s\nContent-Length: %d\n\n",
										PostTarget,httpVersion,HostName,UserAgent,RequestType,SLen(szText)+iBinarySize);
	// Send header
	if ( send(Socket, szHeader, SLen(szHeader), 0) == SOCKET_ERROR ) return false;
	// Send text body
	if ( send(Socket, szText, SLen(szText), 0) == SOCKET_ERROR ) return false;
	// Send binary body
	if (bpBinary && (iBinarySize>0))
		if ( send(Socket, (const char *) bpBinary, iBinarySize, 0) == SOCKET_ERROR ) return false;
	// Success
	return true;
	}

bool CStdHttp::Receive(CStdHttpMessage &rMsg)
	{
	// Clear any old message
	rMsg.Clear();
	// Receive header
	if (!ReceiveHeader(rMsg)) 
		return false;
	// Receive body
	if (!ReceiveBody(rMsg)) 
		return false;
	// Success
	return true;
	}

bool CStdHttp::ReceiveBody(CStdHttpMessage &rMsg)
	{
	int iResult;
	// Size unknown
	if (rMsg.ContentLength==0)
		{
		// Receive to constant size temp buffer
		const int const_buf=64000;
		char buf[const_buf+1];
		iResult = recv(Socket,buf,const_buf,0);
		if (iResult == SOCKET_ERROR) return false;
		// Text
		if (rMsg.Text)
			{
			rMsg.Data = new char [iResult+1];
			buf[iResult]=0;
			SCopy(buf,rMsg.Data);
			rMsg.ContentLength = SLen(rMsg.Data);
			}
		// Binary
		else
			{
			if (iResult)
				{
				rMsg.Data = new char [iResult];
				MemCopy(buf,rMsg.Data,iResult);
				rMsg.ContentLength=iResult;
				}
			}
		}
	// Size known
	else
		{
		// Allocate buffer
		rMsg.Data = new char [rMsg.ContentLength + rMsg.Text];
		// Receive data
		const int iMaxChunk = 1024; int iChunk;
		for (int iReceived=0; iReceived<rMsg.ContentLength; iReceived+=iChunk)
			{
			iChunk = Min( iMaxChunk, rMsg.ContentLength-iReceived );
			iResult = recv(Socket,rMsg.Data+iReceived,iChunk,0);
			// Error
			if (iResult == SOCKET_ERROR) return false;
			if (iResult == 0) return false; 
			// Be happy with what you get
			iChunk = iResult;
			}
		// Zero terminate text
		if (rMsg.Text) rMsg.Data[rMsg.ContentLength]=0;
		}
	// Success
	return true;
	}

bool CStdHttp::GetLocalAddress(const char *szTargetConnect, char *sBuffer)
	{
	// Init winsock
	WSADATA wsadata; ZeroMem(&wsadata,sizeof wsadata);
	if (WSAStartup(MAKEWORD(1,1), &wsadata)!=0) return false;
	// Connect
	if (!Connect(szTargetConnect)) return false;
	// Get address from socket
	SOCKADDR_IN localAddr;
	int sockaddrinSize = sizeof(SOCKADDR_IN);
	getsockname(Socket, (SOCKADDR*) &localAddr, &sockaddrinSize);
	SCopy( inet_ntoa(localAddr.sin_addr), sBuffer );
	// Disconnect
	Disconnect();
	// Done
	return true;
	}

bool CStdHttp::GetFile(const char *szHost, const char *szFilename, const char *szTargetPath)
	{
	// Connect							
	if (!Connect(szHost)) return false;
	// Get file
	if (!Get(szFilename,szTargetPath)) return false;
	// Disconnect
	Disconnect();
	// Done
	return true;
	}

bool CStdHttp::ObtainStatus(const char *szHost)
	{
	// Connect							
	if (!Connect(szHost)) return false;
	// Compose request
	char szMessage[1024+1];
	sprintf(szMessage,"version=%s&action=status",ClientVersion);
	// Post request
	if (!Post(szMessage)) return false;
	// Receive message
	CStdHttpMessage Msg;
	if (!Receive(Msg) || !Msg.Success || !Msg.Text) 
		return false;
	// Store status data
	SCopyNamedSegment( Msg.Data, "status", ServerStatus, '&', '=', httpMaxString );
	SCopyNamedSegment( Msg.Data, "motd", MessageOfTheDay, '&', '=', httpMaxString );
	SCopyNamedSegment( Msg.Data, "dataurl", DataPath, '&', '=', httpMaxString );	
	// Disconnect
	Disconnect();
	// Success
	return true;
	}
