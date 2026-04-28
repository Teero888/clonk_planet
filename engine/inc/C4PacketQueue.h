/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A special packet that statically contains other packets */

class C4PacketQueue: public C4Packet  
	{
	public:
		C4PacketQueue();
		~C4PacketQueue();
	protected:
		int ChunkSize;
		int DataSize;
	public:
		void Default();
		void Clear();
		void Set(int iType, void *pData=NULL, int iSize=0, BOOL fHoldData=FALSE, int iDataSize=0);
		void SetChunkSize(int iChunkSize);
		BOOL AddStatic(C4Packet &rPacket);
		BOOL AddStaticHead(C4Packet &rPacket);
		BOOL AddData(BYTE *bpData, int iSize);
		BOOL AddDataHead(BYTE *bpData , int iSize);
	};

