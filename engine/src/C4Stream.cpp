/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* A wrapper to a tcp/ip stream that sends and receives packets */

#include <C4Include.h>

#include <..\unibase\skstream.h>

C4Stream::C4Stream()
	{
	Default();
	}

C4Stream::C4Stream(skstream *pnStream)
	{
	pStream = pnStream;	
	SCopy("LocalUnknown",LocalName);
	SCopy("LocalAddressUnknown",LocalAddress);
	SCopy("PeerUnknown",PeerName);
	SCopy("PeerAddressUnknown",PeerAddress);
	PacketSend=0;
	PacketReceive=0;
	iRecovery=0;
	}

C4Stream::~C4Stream()
	{
	Clear();
	}

void C4Stream::Default()
	{
	pStream = new skstream;	
	SCopy("LocalUnknown",LocalName);
	SCopy("LocalAddressUnknown",LocalAddress);
	SCopy("PeerUnknown",PeerName);
	SCopy("PeerAddressUnknown",PeerAddress);
	PacketSend=0;
	PacketReceive=0;
	iRecovery=0;
	}

void C4Stream::Clear()
	{
	if (pStream) delete pStream; pStream=NULL;
	}

int C4Stream::Connect(const char *szName, int iRole, const char *szAddress, C4Stream **ppAccept, int iPort)
	{
	C4Packet Packet;
	
	// Set local name (local address is told by client)
	SCopy(szName,LocalName,256);
	
	// Connect
	switch (iRole)
		{
		
		case C4STRM_Server:
			// Connect
			pStream->open(szAddress, iPort, skstream::server);	
			if (!pStream->is_open()) return C4STRM_NoOpen;
			// Receive the client's name
			if (ReceivePacket(C4PK_MyName,Packet)!=C4STRM_Ok) return C4STRM_ReceiveError;
			SCopy((const char*)Packet.Data,PeerName,256);
			// Receive used local address
			if (ReceivePacket(C4PK_YourAddress,Packet)!=C4STRM_Ok) return C4STRM_ReceiveError;
			SCopy((const char*)Packet.Data,LocalAddress,256);
			// Send name
			Packet.Set(C4PK_MyName,(BYTE*)LocalName,SLen(LocalName));
			if (PutPacket(Packet)!=C4STRM_Ok) return C4STRM_SendError;
			break;
		
		case C4STRM_Client:								
			// Connect
			pStream->open(szAddress, iPort, skstream::client);
			if (!pStream->is_open()) return C4STRM_NoOpen;
			// Send name
			Packet.Set(C4PK_MyName,(BYTE*)LocalName,SLen(LocalName));
			if (PutPacket(Packet)!=C4STRM_Ok) return C4STRM_SendError;
			// Tell host his own address
			Packet.Set(C4PK_YourAddress,(BYTE*)GetPeerAddress(),SLen(GetPeerAddress()));
			if (PutPacket(Packet)!=C4STRM_Ok) return C4STRM_SendError;
			// Receive the host's name
			if (ReceivePacket(C4PK_MyName,Packet)!=C4STRM_Ok) return C4STRM_ReceiveError;
			SCopy((const char*)Packet.Data,PeerName,256);
			break;

		case C4STRM_Listener:
			// Need pointer
			if (!ppAccept) return C4STRM_InvalidParameters;
			*ppAccept=NULL;
			// Create stream socket if necessary
			if (!pStream->has_socket()) pStream->create(iPort);
			if (!pStream->has_socket()) return C4STRM_NoSocket;
			// Listen and accept new stream
			skstream *pNewStream;
			pStream->listen(&pNewStream);
			if (!pNewStream) return C4STRM_NoListen;
			// Create accepted stream
			*ppAccept = new C4Stream(pNewStream);
			SCopy(szName,(*ppAccept)->LocalName,256);
			// Accepted stream: Receive the client's name
			if ((*ppAccept)->ReceivePacket(C4PK_MyName,Packet)!=C4STRM_Ok) return C4STRM_ReceiveError;
			SCopy((const char*)Packet.Data,(*ppAccept)->PeerName,256);
			// Accepted stream: Receive used local address
			if ((*ppAccept)->ReceivePacket(C4PK_YourAddress,Packet)!=C4STRM_Ok) return C4STRM_ReceiveError;
			SCopy((const char*)Packet.Data,(*ppAccept)->LocalAddress,256);
			// Accepted stream: Send name
			Packet.Set(C4PK_MyName,(BYTE*)((*ppAccept)->LocalName),SLen((*ppAccept)->LocalName));
			if ((*ppAccept)->PutPacket(Packet)!=C4STRM_Ok) return C4STRM_SendError;
			break;

		}

	// Success
	return C4STRM_Ok;
	}

const char *C4Stream::GetPeerAddress()
	{
	return pStream->getpeername(PeerAddress,256);
	}

const char *C4Stream::GetPeerName()
	{
	return PeerName;
	}

const char *C4Stream::GetLocalName()
	{
	return LocalName;
	}

const char *C4Stream::GetLocalAddress()
	{
	return LocalAddress;
	}

int C4Stream::GetPort()
	{
	return pStream->getport();
	}

void C4Stream::Close()
	{
	pStream->close();
	}

void C4Stream::Reset()
	{
	Clear(); Default();
	}

int C4Stream::GetPacket(C4Packet &rPacket)
	{
	rPacket.Reset();
	// Get header
	for (int cnt=0; cnt<sizeof(C4PacketHeader); cnt++)
		{
		if (!pStream->good()) return C4STRM_NoGood;
		char ch; pStream->get(ch);
		((BYTE*)&rPacket)[cnt] = ch;
		}
	// Check header
	iRecovery = 0; // Reset recovery flag
	if (!SEqual(rPacket.Head,C4PK_Head))
		if (!CrapRecovery(rPacket))
			return C4STRM_NoPacket;
	// Get data
	if (rPacket.Size)
		{
		if (!(rPacket.Data = new BYTE [rPacket.Size+1])) return C4STRM_NoMemory;
		rPacket.OwnData=TRUE;
		for (cnt=0; cnt<rPacket.Size; cnt++)
			{
			if (!pStream->good()) return C4STRM_DataError;
			char ch; pStream->get(ch);
			rPacket.Data[cnt]=ch;
			}
		// Append zero
		rPacket.Data[rPacket.Size]=0;
		}
	// Check reception order
	if (rPacket.Number != PacketReceive++) return C4STRM_OutOfOrder;
	// Success
	return C4STRM_Ok;
	}

int C4Stream::PutPacket(C4Packet &rPacket, void (*fnPrcs)(int))
	{
	// Stamp packet
	rPacket.Number = PacketSend++;
	// Put packet header
	for (int cnt=0; cnt<sizeof(C4PacketHeader); cnt++)
		{
		if (!pStream->good()) return C4STRM_NoGood;
		pStream->put(((BYTE*)&rPacket)[cnt]);
		}
	// Put data
	int iPrcBlock=0;
	for (cnt=0; cnt<rPacket.Size; cnt++)
		{
		if (!rPacket.Data) return C4STRM_NoData;
		if (!pStream->good()) return C4STRM_NoGood;
		pStream->put(rPacket.Data[cnt]);
		if (iPrcBlock==0) { iPrcBlock=100; if (fnPrcs) fnPrcs(100*cnt/rPacket.Size+1); } iPrcBlock--;
		}
	// Success
	pStream->flush();
	return C4STRM_Ok;
	}

const char *C4Stream::ResultText(int iResult)
	{
	switch (iResult)
		{
		case C4STRM_Ok: return "No error";
		case C4STRM_NoGood: return "No good";
		case C4STRM_NoPacket: return "No packet";
		case C4STRM_NoData: return "No data";
		case C4STRM_NoMemory: return "No memory";
		case C4STRM_DataError: return "Data error";
		case C4STRM_SendError: return "Send error";
		case C4STRM_ReceiveError: return "Receive error";
		case C4STRM_OutOfOrder: return "Packet out of order";
		case C4STRM_NoOpen: return "Stream not open";
		case C4STRM_Break: return "Break";
		case C4STRM_WrongPacket: return "Incorrect packet type received";
		case C4STRM_NoFile: return "File error";
		case C4STRM_DataMismatch: return "Data mismatch";
		case C4STRM_NoSocket: return "No socket";
		case C4STRM_InvalidParameters: return "Invalid parameters";
		case C4STRM_NoListen: return "No listen";
		}
	return "Undefined result";
	}

int C4Stream::ReceivePacket(int iType, C4Packet &rPacket)
	{
	int iResult;
	// Get packet
	if ((iResult=GetPacket(rPacket))!=C4STRM_Ok) return iResult;
	// Check packet type
	if (rPacket.Type!=iType) return C4STRM_WrongPacket;
	// Success
	return C4STRM_Ok;
	}

int C4Stream::SendFile(const char *szFilename, int iType, void (*fnPrcs)(int))
	{
	int iResult;
	C4Packet Packet;
	// Send filename
	Packet.Set(C4PK_Filename,(BYTE*)GetFilename(szFilename),SLen(GetFilename(szFilename)));
	if ((iResult=PutPacket(Packet))!=C4STRM_Ok) return iResult;
	// Send file
	if (!Packet.Load(iType,szFilename)) return C4STRM_NoFile;
	if ((iResult=PutPacket(Packet,fnPrcs))!=C4STRM_Ok) return iResult;
	// Success
	return C4STRM_Ok;
	}

int C4Stream::ReceiveFile(char *sPath, int iType)
	{
	int iResult;
	C4Packet Packet;
	if (!sPath) return C4STRM_NoData;
	// Receive filename
	if ((iResult=ReceivePacket(C4PK_Filename,Packet))!=C4STRM_Ok) return iResult;
	if (sPath[0]) AppendBackslash(sPath); SAppend((const char *)Packet.Data,sPath);
	// Receive file
	if ((iResult=ReceivePacket(iType,Packet))!=C4STRM_Ok) return iResult;
	EraseItem(sPath); // Overwrite any old duplicate
	if (!Packet.Save(sPath)) return C4STRM_NoFile;
	// Success
	return C4STRM_Ok;
	}

int C4Stream::GetLastError()
	{
	return pStream->getlasterror();
	}

int C4Stream::SendInteger(int iFlagType, int iValue)
	{
	int iResult;
	C4Packet Packet;
	Packet.Set(iFlagType, (BYTE*) &iValue, sizeof (int));
	if ((iResult=PutPacket(Packet))!=C4STRM_Ok) return iResult;
	return C4STRM_Ok;
	}

int C4Stream::ReceiveInteger(int iFlagType, int &riValue)
	{
	int iResult;
	C4Packet Packet;
	if ((iResult=ReceivePacket(iFlagType,Packet))!=C4STRM_Ok) return iResult;
	if (!Packet.Data || (Packet.Size!=sizeof (int))) return C4STRM_DataMismatch;
	riValue = *((int*) Packet.Data);
	return C4STRM_Ok;
	}

BOOL C4Stream::CrapRecovery(C4Packet &rPacket)
	{
	// Header was received and is incorrect.
	// Try to advance bytes until header id is matched.
	for (iRecovery=0; iRecovery<C4STRM_MaxRecovery; iRecovery++)
		{
		// Get next byte from stream
		if (!pStream->good()) return FALSE;
		char ch; pStream->get(ch);
		// Move first header byte to recovery buffer
		RecoveryBuffer[iRecovery] = ((BYTE*)&rPacket)[0];
		// Shift header data by one byte
		MemCopy( ((BYTE*)&rPacket)+1, ((BYTE*)&rPacket), sizeof C4PacketHeader - 1 );
		// Put next byte to end of header
		((BYTE*)&rPacket)[sizeof C4PacketHeader - 1] = ch;
		// Check match
		if (SEqual(rPacket.Head,C4PK_Head)) { iRecovery++; return TRUE; }
		}
	// Unable to match
	return FALSE;
	}

BOOL C4Stream::GetRecovery(char *sData)
	{
	// Write recovery data to string 
	sData[0]=0;
	for (int cnt=0; cnt<iRecovery; cnt++)
		sprintf(sData+SLen(sData),"%02X ",RecoveryBuffer[cnt]);
	// Return signal
	return (iRecovery!=0);
	}

BOOL C4Stream::Readable()
	{
	if (!pStream) return FALSE;
	return pStream->is_readable();
	}

BOOL C4Stream::Writeable()
	{
	if (!pStream) return FALSE;
	return pStream->is_writeable();
	}

BOOL C4Stream::Good()
	{
	if (!pStream) return FALSE;
	return pStream->is_open();
	}