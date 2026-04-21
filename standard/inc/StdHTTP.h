/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Basic http access */

#define httpVersion "HTTP/1.0"

const int httpMaxString = 1024;

class CStdHttpMessage
	{
	public:
		CStdHttpMessage();
		~CStdHttpMessage();
	public:
		bool Success;
		char Status[httpMaxString+1];
		char ContentType[httpMaxString+1];
		int ContentLength;
		char *Data;
		bool Text;
	public:
		void Default();
		void Clear();
	};

class CStdHttp  
	{
	public:
		CStdHttp();
		~CStdHttp();
	protected:
		bool ReceiveBody(CStdHttpMessage &rMsg);
		bool ReceiveLine(char *sBuf, int iBufSize);
		bool ReceiveHeader(CStdHttpMessage &rMsg);
		HOSTENT *pHost;
		SOCKET Socket;
		char UserAgent[httpMaxString+1];
		char HostName[httpMaxString+1];
		char PostTarget[httpMaxString+1];
		char RequestType[httpMaxString+1];
		char ServerStatus[httpMaxString+1];
		char MessageOfTheDay[httpMaxString+1];
		char DataPath[httpMaxString+1];
		char ClientVersion[httpMaxString+1];
	public:
		bool ObtainStatus(const char *szHost);
		bool Get(const char *szFilename, const char *szTargetPath);
		bool GetFile(const char *szHost, const char *szFilename, const char *szTargetPath);
		bool GetLocalAddress(const char *szTargetConnect, char *sBuffer);
		bool Receive(CStdHttpMessage &rMsg);
		bool Post(const char *szText, BYTE *bpBinary=NULL, int iBinarySize=0);
		void Disconnect();
		bool Connect(const char *szHost);
		void Clear();
		void Default();
		bool Init(const char *szUserAgent, const char *szPostTarget, const char *szRequestType, const char *szClientVersion);
	};

