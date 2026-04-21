/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A network packet */

#define C4PK_Head "C4PK"

const int C4PK_None											= 0,
					C4PK_Hello										= 1,
					C4PK_MyName										= 2,
					C4PK_GoodBye									= 3,
					C4PK_Filename									= 4,
					C4PK_File											= 5,									
					C4PK_RequestJoin							= 6,
					C4PK_RequestNetworkGame				= 7,
					C4PK_RequestNetworkReference	= 8,					
					C4PK_RequestDone							= 9,					
					C4PK_NetworkGame							= 10,
					C4PK_NetworkReference					= 11,
					C4PK_JoinClient								= 12,
					C4PK_RemoveClient							= 13,
					C4PK_JoinPlayer								= 14,
					C4PK_RemovePlayer							= 15,					
					C4PK_AcceptJoin								= 16,
					C4PK_JoinReady								= 17,
					C4PK_JoinGo										= 18,
					C4PK_SyncCheck								= 19,
					C4PK_Control									= 20,
					C4PK_Message									= 21,
					C4PK_PlayerControl						= 22,					
					C4PK_SurrenderPlayer					= 23,
					C4PK_SetHostility							= 24,
					C4PK_SetControlRate						= 25,
					C4PK_SetSyncRate							= 26,
					C4PK_YourAddress							= 27,
					C4PK_SignUp										= 28,
					C4PK_SignUpOkay								= 29,
					C4PK_JoinDenied								= 30,
					C4PK_PlayerSelection					= 31,
					C4PK_PlayerCommand						= 32,
					C4PK_ControlTime							= 33;

#pragma pack(1)

class C4PacketHeader
	{
	friend C4Packet;
	friend C4Stream;
	public:
		C4PacketHeader();
		~C4PacketHeader();
	protected:
		char Head[4+1];
	public:
		int Type;
		int Size;
		int Number;
	public:
		void Clear();
		void Default();
	};

#pragma pack()

class C4Packet: public C4PacketHeader
	{
	friend C4Stream;
	public:
		C4Packet();
		~C4Packet();
	public:
		BYTE *Data;
	protected:
		BOOL OwnData;
	public:
		void Default();
		void Clear();
		void Reset();
		void Set(int iType, void *pData=NULL, int iSize=0, BOOL fHoldData=FALSE);
		BOOL Copy(C4Packet &rPacket);
		BOOL GetStatic(int iIndex, C4Packet &rPacket);
		BOOL AddStatic(C4Packet &rPacket);
		BOOL AddStaticHead(C4Packet &rPacket);
		BOOL AddData(BYTE *bpData, int iSize);
		BOOL AddDataHead(BYTE *bpData , int iSize);
		BOOL Save(const char *szFilename);
		BOOL Load(int iType, const char *szFilename);
	};