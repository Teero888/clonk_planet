/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper to a tcp/ip stream that sends and receives packets */

const int C4STRM_Ok = 0, C4STRM_NoGood = 1, C4STRM_NoPacket = 2, C4STRM_NoData = 3, C4STRM_NoMemory = 4, C4STRM_DataError = 5, C4STRM_SendError = 6, C4STRM_ReceiveError = 7, C4STRM_OutOfOrder = 8,
          C4STRM_NoOpen = 9, C4STRM_Break = 10, C4STRM_WrongPacket = 11, C4STRM_NoFile = 12, C4STRM_DataMismatch = 13, C4STRM_NoSocket = 14, C4STRM_InvalidParameters = 15, C4STRM_NoListen = 16;

const int C4STRM_Server = 0, C4STRM_Client = 1, C4STRM_Listener = 2;

const int C4PORT_Control = 11111, C4PORT_Input = 11112;

const int C4STRM_MaxRecovery = 20;

class skstream;

class C4Stream {
public:
  C4Stream();
  C4Stream(skstream *pnStream);
  ~C4Stream();

protected:
  char LocalName[256 + 1];
  char LocalAddress[256 + 1];
  char PeerName[256 + 1];
  char PeerAddress[256 + 1];
  skstream *pStream;
  int PacketSend;
  int PacketReceive;

private:
  BYTE RecoveryBuffer[C4STRM_MaxRecovery];
  int iRecovery;

public:
  BOOL Writeable();
  BOOL Readable();
  BOOL Good();
  BOOL GetRecovery(char *sData);
  void Reset();
  void Close();
  int ReceiveInteger(int iFlagType, int &riValue);
  int SendInteger(int iFlagType, int iValue);
  int GetLastError();
  int ReceiveFile(char *sPath, int iType = C4PK_File);
  int SendFile(const char *szFilename, int iType = C4PK_File, void (*fnPrcs)(int) = NULL);
  int ReceivePacket(int iType, C4Packet &rPacket);
  int Connect(const char *szName, int iRole, const char *szAddress = NULL, C4Stream **ppAccept = NULL, int iPort = C4PORT_Control);
  int PutPacket(C4Packet &rPacket, void (*fnPrcs)(int) = NULL);
  int GetPacket(C4Packet &rPacket);
  int GetPort();
  const char *ResultText(int iResult);
  const char *GetPeerAddress();
  const char *GetPeerName();
  const char *GetLocalAddress();
  const char *GetLocalName();

protected:
  BOOL CrapRecovery(C4Packet &rPacket);
  void Default();
  void Clear();
};
