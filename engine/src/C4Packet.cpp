/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A network packet */

#include <C4Include.h>

C4PacketHeader::C4PacketHeader() { Default(); }

C4PacketHeader::~C4PacketHeader() { Clear(); }

void C4PacketHeader::Default() {
  SCopy(C4PK_Head, Head);
  Type = C4PK_None;
  Size = 0;
  Number = 0;
}

void C4PacketHeader::Clear() {}

C4Packet::C4Packet() { Default(); }

C4Packet::~C4Packet() { Clear(); }

void C4Packet::Default() {
  C4PacketHeader::Default();
  Data = NULL;
  OwnData = FALSE;
}

void C4Packet::Clear() {
  C4PacketHeader::Clear();
  if (OwnData)
    if (Data)
      delete[] Data;
  Data = NULL;
}

void C4Packet::Reset() {
  Clear();
  Default();
}

void C4Packet::Set(int iType, void *pData, int iSize, BOOL fHoldData) {
  Reset();
  Type = iType;
  Data = (BYTE *)pData;
  Size = iSize;
  OwnData = fHoldData;
}

BOOL C4Packet::Load(int iType, const char *szFilename) {
  CStdFile hFile;
  BYTE *bpBuf;
  int iSize;
  if (!hFile.Load(szFilename, &bpBuf, &iSize))
    return FALSE;
  Set(iType, bpBuf, iSize, TRUE);
  return TRUE;
}

BOOL C4Packet::Save(const char *szFilename) {
  CStdFile hFile;
  if (!hFile.Save(szFilename, Data, Size))
    return FALSE;
  return TRUE;
}

// Appends data to the existing packet data.

BOOL C4Packet::AddData(BYTE *bpData, int iSize) {
  // Nothing to add
  if (!bpData || !iSize)
    return TRUE;
  // Allocate new buffer
  BYTE *pBuffer = new BYTE[Size + iSize];
  if (!pBuffer)
    return FALSE;
  // Fill buffer
  if (Data)
    MemCopy(Data, pBuffer, Size);
  MemCopy(bpData, pBuffer + Size, iSize);
  // Set new packet
  Set(Type, pBuffer, Size + iSize, TRUE);
  // Success
  return TRUE;
}

// Prepends data to the existing packet data.

BOOL C4Packet::AddDataHead(BYTE *bpData, int iSize) {
  // Nothing to add
  if (!bpData || !iSize)
    return TRUE;
  // Allocate new buffer
  BYTE *pBuffer = new BYTE[Size + iSize];
  if (!pBuffer)
    return FALSE;
  // Fill buffer
  if (Data)
    MemCopy(Data, pBuffer + iSize, Size);
  MemCopy(bpData, pBuffer, iSize);
  // Set new packet
  Set(Type, pBuffer, Size + iSize, TRUE);
  // Success
  return TRUE;
}

// Appends a packet (header + data) as data to the existing packet data.

BOOL C4Packet::AddStatic(C4Packet &rPacket) {
  // Add header
  if (!AddData((BYTE *)&rPacket, sizeof(C4PacketHeader)))
    return FALSE;
  // Add data
  if (rPacket.Data && rPacket.Size)
    if (!AddData(rPacket.Data, rPacket.Size))
      return FALSE;
  // Success
  return TRUE;
}

// Prepends a packet (header + data) as data to the existing packet data.

BOOL C4Packet::AddStaticHead(C4Packet &rPacket) {
  // Add data
  if (rPacket.Data && rPacket.Size)
    if (!AddDataHead(rPacket.Data, rPacket.Size))
      return FALSE;
  // Add header
  if (!AddDataHead((BYTE *)&rPacket, sizeof(C4PacketHeader)))
    return FALSE;
  // Success
  return TRUE;
}

// Retrieves the indexed packet (header + data) from the packet data.

BOOL C4Packet::GetStatic(int iIndex, C4Packet &rPacket) {
  C4PacketHeader *pHeader;
  int cIndex = 0, iOffset = 0;
  BYTE *pData = Data + iOffset;
  while (pData) {
    // Point to header
    pHeader = (C4PacketHeader *)pData;
    // Set static packet & return
    if (cIndex == iIndex) {
      rPacket.Set(pHeader->Type, pHeader->Size ? pData + sizeof(C4PacketHeader) : NULL, pHeader->Size);
      return TRUE;
    }
    // Advance to next packet
    cIndex++;
    iOffset += sizeof(C4PacketHeader) + pHeader->Size;
    if (iOffset < Size)
      pData = Data + iOffset;
    else
      pData = NULL;
  }
  return FALSE;
}

BOOL C4Packet::Copy(C4Packet &rPacket) {
  // Reset
  Reset();
  // Duplicate source data
  BYTE *bpData = NULL;
  if (rPacket.Data && rPacket.Size) {
    if (!(bpData = new BYTE[rPacket.Size]))
      return FALSE;
    MemCopy(rPacket.Data, bpData, rPacket.Size);
  }
  // Set packet
  Set(rPacket.Type, bpData, rPacket.Size, TRUE);
  // Success
  return TRUE;
}
