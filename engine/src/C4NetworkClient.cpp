/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Network interchange for one client (host or client side) */

#include <C4Include.h>

const int C4NET_Transfer_None					= 0,
					C4NET_Transfer_Cease				= 1,
				  C4NET_Transfer_GetNetReady	= 2,
					C4NET_Transfer_SendNetGo		= 3;

//--------------------------------- C4NetworkClientList --------------------------------------------

C4NetworkClientList::C4NetworkClientList()
	{
	Default();
	}

C4NetworkClientList::~C4NetworkClientList()
	{
	Clear();
	}

void C4NetworkClientList::Clear()
	{
	C4NetworkClient *pClient;
	while (pClient = First)	{	First = pClient->Next;	delete pClient; }
	First = NULL;
	}

void C4NetworkClientList::Default()
	{
	First=NULL;
	}

int C4NetworkClientList::GetCount()
	{
	int iCount = 0;
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next) iCount++;
	return iCount;
	}

BOOL C4NetworkClientList::Add(C4NetworkClient *pClient)
	{
	if (!pClient) return FALSE;
	pClient->Next = First;
	First = pClient;
	return TRUE;
	}

BOOL C4NetworkClientList::Remove(int iNetworkClient)
	{
	C4NetworkClient *pClient = Get(iNetworkClient); if (!pClient) return FALSE;
	C4NetworkClient *pPrev;
	for (pPrev=First; pPrev; pPrev=pPrev->Next)
		if (pPrev->Next==pClient) break;
	if (pPrev) pPrev->Next=pClient->Next;
	else First=pClient->Next;
	delete pClient;
	return TRUE;
	}

int C4NetworkClientList::GetFreeNumber()
	{
	int iNumber=C4NET_FirstClientNo-1;
	BOOL fFree; 
	do
		{
		iNumber++; fFree=TRUE;
	  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
			if (pClient->Number==iNumber)
				fFree=FALSE;
		}
	while (!fFree);
	return iNumber;
	}

C4NetworkClient* C4NetworkClientList::Get(int iNetworkClient)
	{
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		if (pClient->Number==iNetworkClient)
			return pClient;
	return NULL;
	}

int C4NetworkClientList::GetInput(C4Packet &rInput) // Nonblocking & conditional
	{
	int iResult=0;
	C4Packet pckInput;
	
	// Check for any incoming packets
	for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		if (pClient->GetInput(pckInput))
			{ rInput.AddData(pckInput.Data,pckInput.Size); iResult+=pckInput.Size; }

	return iResult;
	}

BOOL C4NetworkClientList::GetNetReady(C4Packet &rControl) // Nonblocking
	{
	C4Packet ClientControl;
	BOOL fAllReady=TRUE;

	// Check / receive NetReady from all clients
	for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		if (!pClient->NetReady)
			{
			// Client is ready and has sent client control packet
			if (pClient->GetNetReady(ClientControl))
				{
				rControl.AddData(ClientControl.Data,ClientControl.Size);
				pClient->NetReady=TRUE; // (double to Client::GetNetReady)
				}
			// Client not ready yet
			else
				{
				fAllReady=FALSE;
				}
			}
	
	// All ready: reset NetReady flags (formerly done by SendNetGo)
	if (fAllReady)
		for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
			pClient->NetReady=FALSE;

	// Return all ready
	return fAllReady;
	}

BOOL C4NetworkClientList::SendNetGo(C4Packet &rControl) // Blocking
	{
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		pClient->SendNetGo(rControl);
	return TRUE;
	}

void C4NetworkClientList::RemoveDeactivated()
	{
	// Post C4PK_RemoveClient for any deactivated client
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		if (!pClient->Active)
			if (!pClient->Deactivated)
				{
				Game.Input.AddRemoveClient( pClient->Number, Game.Network.GetClientName(pClient->Number) );
				pClient->Deactivated=TRUE;
				}
	}

BOOL C4NetworkClientList::GetJoinReady()
	{
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)		
		if (!pClient->GetJoinReady())
			return FALSE;
	return TRUE;
	}

void C4NetworkClientList::Draw(C4Facet &cgo)
	{
	C4Facet cgo2;
  for (C4NetworkClient *pClient=First; pClient; pClient=pClient->Next)
		{
		cgo2=cgo.TruncateSection(C4FCT_Right);
		pClient->Draw(cgo2);
		}
	}

//------------------------------------- C4NetworkClient ------------------------------------------------

C4NetworkClient::C4NetworkClient()
	{
	Default();
	}

C4NetworkClient::~C4NetworkClient()
	{
	Clear();
	}

void C4NetworkClient::Default()
	{
	Active=FALSE;
	Number = C4NET_NoClient;
	SCopy("Unknown",Name);
	SCopy("Unknown",Address);
	Next=NULL;
	pControlStream=NULL;
	pInputStream=NULL;
	NetReady=FALSE;
	NetGo=FALSE;
	OutOfSync=FALSE;
	Deactivated=FALSE;
	}

void C4NetworkClient::Clear()
	{	
	Active=FALSE;
	// Close streams
	if (pControlStream) delete pControlStream; pControlStream=NULL;
	if (pInputStream) delete pInputStream; pInputStream=NULL;
	}

struct C4ControlJoin
	{
	int Number;
	int Asynchronous;
	int ControlTime;
	int RandomSeed;
	};

BOOL C4NetworkClient::HandleJoin(int iNumber, C4Stream *pStream, 
																 BOOL fRuntimeJoin, BOOL fGetJoinReady)
	{
	C4Packet Packet;
	C4Group hGroup;

	// Init client (set number & stream pointer)
	if ((iNumber<0) || !pStream) return FALSE;
	Number = iNumber;
	pControlStream = pStream;
	
	// Set peer names
	SCopy(pControlStream->GetPeerName(),Name,C4MaxTitle);
	SCopy(pControlStream->GetPeerAddress(),Address,C4MaxTitle);
	
	// Block runtime join
	if (fRuntimeJoin && Config.Network.NoRuntimeJoin)
		{
		// Denial message
		sprintf(OSTR,LoadResStr(IDS_NET_NORUNTIMEJOIN),pControlStream->GetPeerName()); Log(OSTR);
		// Send deny & abort
		Packet.Set(C4PK_JoinDenied);
		StreamOk(pControlStream->PutPacket(Packet));
		// Stream destroyed by client clearance
		return FALSE;	
		}
		
	// Log join request
	sprintf(OSTR,LoadResStr(IDS_NET_REQUESTJOIN),pControlStream->GetPeerName()); Log(OSTR);
	Game.GraphicsSystem.MessageBoard.EnsureLastMessage();
	
	// Send accept, client number, synchronous mode, control time, random seed
	C4ControlJoin ControlJoin;
	ControlJoin.Number=Number;
	ControlJoin.Asynchronous=Game.AsynchronousControl;
	ControlJoin.ControlTime=Game.ControlTime();
	ControlJoin.RandomSeed=Game.RandomSeed;
	Packet.Set(C4PK_AcceptJoin,&ControlJoin,3*sizeof(C4ControlJoin));
	if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;

	// Handle requests
	BOOL fRequestDone = FALSE;
	while (!fRequestDone)
		{
		// Get packet
		if (!StreamOk(pControlStream->GetPacket(Packet))) return FALSE;
		// Handle packet
		switch (Packet.Type)
			{
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 						
			case C4PK_RequestDone:
				fRequestDone = TRUE;
				break;
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 						
			case C4PK_RequestNetworkGame:
				// Log network game request
				sprintf(OSTR,LoadResStr(IDS_NET_REQUESTGAME),pControlStream->GetPeerName()); Log(OSTR);
				Game.GraphicsSystem.MessageBoard.EnsureLastMessage();
				// Game data temp filename
				char szFilename[_MAX_PATH+1];
				SCopy( Config.AtTempPath(GetFilename(Game.ScenarioFilename)), szFilename);
				// Overwrite any duplicate temp file
				EraseItem(szFilename); 
				// Copy scenario file to temp file
				if (!C4Group_CopyItem(Game.ScenarioFilename,szFilename)) return FALSE;
				// Pack if necessary (unpacked files not eligible for network games anyway)
				if (FileAttributes(szFilename) & _A_SUBDIR) if (!C4Group_PackDirectory(szFilename)) return FALSE;
				// Save game to temp file (if running)
				if (fRuntimeJoin)
					{
					if (!hGroup.Open(szFilename)) return FALSE;
					if (!Game.Save(hGroup,TRUE,TRUE)) return FALSE;
					hGroup.Sort(C4FLS_Scenario);
					hGroup.Close();
					}
				// Send network game
				//NetLog("Sending network game...");
				if (!StreamOk(pControlStream->SendFile(szFilename,C4PK_NetworkGame,&LogProcess))) return FALSE;
				//NetLog("Ok");
				Log(LoadResStr(IDS_NET_TRANSFEROK));
				EraseItem(szFilename);
				break;
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 				
			default: 
				sprintf(OSTR,"Unhandled packet %i received",Packet.Type); NetLog(OSTR); 
				fRequestDone = TRUE;
				break;
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
			}
		}

	// Get join ready immediately / send join go / activate (if desired)
	if (fGetJoinReady) 
		if (!GetJoinReady()) 
			return FALSE;
	
	// Success
	return TRUE;
	}

BOOL C4NetworkClient::StreamOk(int iResult)
	{
	if (iResult==C4STRM_Ok) return TRUE;
	C4Stream bla;	char szMessage[256+1]; 
	sprintf(szMessage,bla.ResultText(iResult));
	NetLog(szMessage);
	return FALSE;
	}

BOOL C4NetworkClient::Join(const char *szServerName, 
													 const char *szServerAddress, 
													 BOOL fRetrieveNetworkGame)
	{
	C4Packet Packet;
	
	// Set local name and address
	SCopy(Game.Network.LocalName,Name,C4MaxTitle);
	SCopy(Game.Network.LocalAddress,Address,C4MaxTitle);
	
	// Create primary (control) stream
	pControlStream = new C4Stream;
	
	// Connect to server
	sprintf(OSTR,LoadResStr(IDS_NET_CONNECTING),szServerName,szServerAddress); Log(OSTR);
	if (!StreamOk(pControlStream->Connect(Name,C4STRM_Client,szServerAddress))) 
		{ Log(LoadResStr(IDS_NET_NOCONNECT)); return FALSE; }
	sprintf(OSTR,"Connected to %s on port %i",pControlStream->GetPeerAddress(),pControlStream->GetPort()); NetLog(OSTR);

	// Request join
	Log(LoadResStr(IDS_NET_REQUESTINGJOIN));
	Packet.Set(C4PK_RequestJoin);
	if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;
	
	// Receive accept, client number, control mode, control time, random seed
	if (!StreamOk(pControlStream->ReceivePacket(C4PK_AcceptJoin,Packet)))
		{ Log(LoadResStr(IDS_NET_NOACCEPT)); return FALSE; }
	Number = ((C4ControlJoin*)Packet.Data)->Number;
	Game.AsynchronousControl = ((C4ControlJoin*)Packet.Data)->Asynchronous;
	// Adjust control time start to match host start
	Game.ControlTimeStart = time_msecs() - ((C4ControlJoin*)Packet.Data)->ControlTime; 
	// Set random seed from host and refix random
	Game.RandomSeed = ((C4ControlJoin*)Packet.Data)->RandomSeed;
	Game.FixRandom(Game.RandomSeed);
	
	// Log
	sprintf(OSTR,"Joined as client %i %s",Number,Game.AsynchronousControl ? "(asynchronous)" : ""); NetLog(OSTR);
	
	// Retrieve network game (if desired)
	if (fRetrieveNetworkGame)	if (!RetrieveNetworkGame()) return FALSE;
	
	// Request done
	Packet.Set(C4PK_RequestDone); if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;
	
	// Connect secondary (input) stream
	if (Game.AsynchronousControl)
		{
		pInputStream = new C4Stream;
		if (!StreamOk(pInputStream->Connect(Name,C4STRM_Client,szServerAddress,NULL,C4PORT_Input))) 
			{ Log(LoadResStr(IDS_NET_NOINPUTSTREAM)); return FALSE; }
		}

	// Activate
	Activate();
	
	// Success
	return TRUE;
	}

BOOL C4NetworkClient::GetNetReady(C4Packet &rControl) // Nonblocking
	{
	// Clear packet
	rControl.Clear();
	// Deactivated client
	if (!Active) return TRUE;
	// No incoming data (Fractured packets may cause block anyway...)
	if (!pControlStream->Readable()) return FALSE;
	// Receive net ready
	if (!StreamOk(pControlStream->ReceivePacket(C4PK_Control,rControl))) 
		{ Deactivate(); return FALSE; } 
	NetReady=TRUE;
	// Success
	return NetReady;
	}

BOOL C4NetworkClient::GetInput(C4Packet &rInput) // Nonblocking
	{
	// Clear packet
	rInput.Clear();
	// Deactivated client
	if (!Active) return TRUE;
	// No incoming data (Fractured packets may cause block anyway...)
	if (!pInputStream->Readable()) return FALSE;
	// Receive input
	if (!StreamOk(pInputStream->ReceivePacket(C4PK_Control,rInput))) 
		{ Deactivate(); return FALSE; } 
	// Success
	return TRUE;
	}

BOOL C4NetworkClient::GetNetGo(C4Packet &rControl) // Nonblocking
	{
	// Clear packet
	rControl.Clear();
	// Deactivated client
	if (!Active) return TRUE;
	// No incoming data
	if (!pControlStream->Readable()) return FALSE;
	// Receive net go			
	if (!StreamOk(pControlStream->ReceivePacket(C4PK_Control,rControl))) { Deactivate(); return FALSE; } 
	NetGo=TRUE;
	// Net ready flag
	NetReady=FALSE;
	// Success
	return NetGo;
	}

BOOL C4NetworkClient::SendNetGo(C4Packet &rControl) // Blocking
	{
	// Deactivated client
	if (!Active) return TRUE;
	// Send net go, host control pack
	if (!StreamOk(pControlStream->PutPacket(rControl))) 
		{ Deactivate(); return FALSE; } 
	// Negate ready flag
	NetReady=FALSE;
	return TRUE;
	}

BOOL C4NetworkClient::SendNetReady(C4Packet &rControl) // Blocking
	{
	if (!Active) return TRUE;
	// Send net ready, client control pack
	// Hopefully doesn't block due to buffered socket output
	if (!StreamOk(pControlStream->PutPacket(rControl))) { Deactivate(); return FALSE; } 
	// Set net ready flag (abuse of host side flag for client side has sent)
	NetReady=TRUE;
	// Negate go flag
	NetGo=FALSE;
	// Success
	return TRUE;
	}

void C4NetworkClient::Activate() 
	{
	Active = TRUE;
	}

void C4NetworkClient::Deactivate()
	{
	sprintf(OSTR,"Client %s (%i) deactivated",Name,Number); NetLog(OSTR);
	Clear();
	}

BOOL C4NetworkClient::SendJoinReady() // and GetJoinGo
	{
	if (!Active) return TRUE;
	C4Packet Packet;
	// Send join ready
	Packet.Set(C4PK_JoinReady);
	if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;
	// Get join go
	Log(LoadResStr(IDS_NET_JOINREADY));
	// Message sensitive waiting
	Game.Network.WaitingForLobby=TRUE;
	Console.UpdateMenus();
	while (pControlStream && !pControlStream->Readable())
		if (Application.HandleMessage()==2)
			return FALSE;
	if (!pControlStream) return FALSE;
	Game.Network.WaitingForLobby=FALSE;
	// Receive join packet
	if (!StreamOk(pControlStream->ReceivePacket(C4PK_JoinGo,Packet))) return FALSE;
	// Success
	return TRUE;
	}

BOOL C4NetworkClient::GetJoinReady() // and SendJoinGo and Activate
	{
	C4Packet Packet;
	// Wait for join ready
	sprintf(OSTR,LoadResStr(IDS_NET_WAITFORJOIN),pControlStream->GetPeerName());	Log(OSTR);
	Game.GraphicsSystem.MessageBoard.EnsureLastMessage();
	if (!StreamOk(pControlStream->ReceivePacket(C4PK_JoinReady,Packet))) return FALSE;
	// Send join go
	Packet.Set(C4PK_JoinGo);
	if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;
	// Activate
	Activate();
	// Success
	return TRUE;
	}
	
BOOL C4NetworkClient::RetrieveNetworkGame() // Overwrites Game.ScenarioFilename
	{
	C4Packet Packet;
	// Request network game
	Log(LoadResStr(IDS_NET_RETRIEVING));
	NetLog("Requesting network game");
	Packet.Set(C4PK_RequestNetworkGame);
	if (!StreamOk(pControlStream->PutPacket(Packet))) return FALSE;
	// Receive network game 
	NetLog("Receiving network game");
	SCopy(Config.Network.WorkPath,Game.ScenarioFilename);
	if (!StreamOk(pControlStream->ReceiveFile(Game.ScenarioFilename,C4PK_NetworkGame))) return FALSE;
	// Success 
	return TRUE;
	}

void C4NetworkClient::Draw(C4Facet &cgo)
	{
	//GfxR->fctClient.Draw(cgo.Surface,cgo.X,cgo.Y);
	sprintf(OSTR,"%s",Name);
	Engine.DDraw.TextOut(OSTR,cgo.Surface,cgo.X,cgo.Y+30,FWhite,FBlack);
	}

BOOL C4NetworkClient::SendInput(C4Packet &rInput)
	{
	if (!Active) return TRUE;
	// Send packet
	if (!StreamOk(pInputStream->PutPacket(rInput))) { Deactivate(); return FALSE; } 
	// Success
	return TRUE;
	}

