/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A special packet that statically contains other packets */

#include <C4Include.h>

C4PacketQueue::C4PacketQueue()
	{
	Default();
	}

C4PacketQueue::~C4PacketQueue()
	{
	Clear();
	}

void C4PacketQueue::Default()
	{
	C4Packet::Default();
	DataSize=0;
	ChunkSize=1024;
	}

void C4PacketQueue::Clear()
	{
	C4Packet::Clear();
	}

void C4PacketQueue::SetChunkSize(int iChunkSize)
	{
	if (iChunkSize) ChunkSize=iChunkSize;
	}

void C4PacketQueue::Set(int iType, void *pData, int iSize, BOOL fHoldData, int iDataSize)
	{
	C4Packet::Set(iType,pData,iSize,fHoldData);
	if (iDataSize>0) DataSize=iDataSize;
	else DataSize=Size;
	}

// Appends data to the existing packet data.

BOOL C4PacketQueue::AddData(BYTE *bpData, int iSize)
	{
	// Nothing to add
	if (!bpData || !iSize) return TRUE;
	// Append to existing buffer
	if (OwnData && (Size+iSize<=DataSize))
		{
		MemCopy(bpData,Data+Size,iSize);
		Size+=iSize;
		}
	// Append to new buffer
	else
		{
		// Allocate new buffer	
		while (DataSize<Size+iSize) DataSize+=ChunkSize;
		BYTE *pBuffer = new BYTE [DataSize]; if (!pBuffer) return FALSE;
		// Fill buffer
		if (Data) MemCopy(Data,pBuffer,Size);
		MemCopy(bpData,pBuffer+Size,iSize);
		// Set new packet
		Set(Type,pBuffer,Size+iSize,TRUE,DataSize);
		}
	// Success
	return TRUE;
	}

// Prepends data to the existing packet data.

BOOL C4PacketQueue::AddDataHead(BYTE *bpData, int iSize)
	{
	// Nothing to add
	if (!bpData || !iSize) return TRUE;
	// Prepend to existing buffer
	if (OwnData && (Size+iSize<=DataSize))
		{
		MemCopy(Data,Data+iSize,Size);
		MemCopy(bpData,Data,iSize);
		Size+=iSize;
		}
	// Prepend to new buffer
	else
		{
		// Allocate new buffer	
		while (DataSize<Size+iSize) DataSize+=ChunkSize;
		BYTE *pBuffer = new BYTE [DataSize]; if (!pBuffer) return FALSE;
		// Fill buffer
		if (Data) MemCopy(Data,pBuffer+iSize,Size);
		MemCopy(bpData,pBuffer,iSize);
		// Set new packet
		Set(Type,pBuffer,Size+iSize,TRUE,DataSize);
		}
	// Success
	return TRUE;
	}
	
// Appends a packet (header + data) as data to the existing packet data.

int ControlQueueSize,ControlQueueDataSize;

BOOL C4PacketQueue::AddStatic(C4Packet &rPacket)
	{
	// Add header
	if (!AddData( (BYTE*) &rPacket, sizeof C4PacketHeader )) return FALSE;
	// Add data
	if (rPacket.Data && rPacket.Size) 
		if (!AddData( rPacket.Data, rPacket.Size )) return FALSE;

	ControlQueueSize=Size; ControlQueueDataSize=DataSize;

	// Success
	return TRUE;	
	}

// Prepends a packet (header + data) as data to the existing packet data.

BOOL C4PacketQueue::AddStaticHead(C4Packet &rPacket)
	{
	// Add data
	if (rPacket.Data && rPacket.Size) 
		if (!AddDataHead( rPacket.Data, rPacket.Size )) return FALSE;
	// Add header
	if (!AddDataHead( (BYTE*) &rPacket, sizeof C4PacketHeader )) return FALSE;
	// Success
	return TRUE;	
	}
