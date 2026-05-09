/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles all network interchange (host or client side) */

#include <C4Include.h>

#define C4NET_Command_Comment "[Comment]"
#define C4NET_Command_Kick "[Kick]"

#define NetStatus(x) SendMessage(hAppWnd, WM_USER_NETLOG, 0, (LPARAM)x);

C4Network::C4Network() { Default(); }

C4Network::~C4Network() { Clear(); }

void C4Network::Default() {
  Active = FALSE;
  Host = FALSE;
  SCopy("Unknown", LocalName);
  SCopy("Unknown", LocalAddress);
  Lobby = WaitingForLobby = FALSE;
  Client.Default();
  Clients.Default();
  hHostThread = NULL;
  idHostThread = 0;
  MasterServerClient.Default();
}

void C4Network::Clear() {
  // Flag
  Active = FALSE;
  // Clear members & data
  Client.Clear();
  Clients.Clear();
  Config.Network.ClearWorkPath();
  // Clear master server client
  MasterServerClient.Clear();
  // Terminate host thread
  // (Cannot close nicely, because host thread is usually blocked by listening
  // call.)
  if (hHostThread)
    TerminateThread(hHostThread, 999);
  hHostThread = NULL;
}

BOOL C4Network::ExecuteInput(C4Control &rInput) {
  // Not active
  if (!Active)
    return TRUE;
  // Host: check for any input from clients, add to own input
  if (Host) {
    // Done by input handler
  }
  // Client: send input to host, clear own input
  else {
    if (rInput.Data) {
      Client.SendInput(rInput);
      rInput.Clear();
    }
  }
  // Done
  return TRUE;
}

BOOL C4Network::ExecuteControl(C4Control &rControl) {
  // Not active
  if (!Active)
    return TRUE;
  // Host: send control to clients
  if (Host) {
    // Check for errorneous client deactivation
    Clients.RemoveDeactivated();
    // Send control (slow client might block host)
    Clients.SendNetGo(rControl);
  }
  // Client: get control from host
  else {
    // Get control
    if (!Client.GetNetGo(rControl))
      return FALSE;
    // Check errorneous deactivation
    if (!Client.Active) {
      Log(LoadResStr(IDS_NET_LOSTHOST));
      Game.Players.RemoveAtRemoteClient();
      Clear();
    }
  }
  // Done
  return TRUE;
}

BOOL C4Network::Execute() {

  // Not active
  if (!Active) {
    // Full input to full control
    Game.Control.Copy(Game.Input);
    Game.Input.Clear();
    return TRUE;
  }

  // Host
  if (Host) {
    // Add all clients' input (NetReady) to own input
    if (!Clients.GetNetReady(Game.Input))
      return FALSE;
    // Check for errorneous client deactivation
    Clients.RemoveDeactivated();
    // Full input to full control
    Game.Control.Copy(Game.Input);
    Game.Input.Clear();
    // Send full control (NetGo)
    Clients.SendNetGo(Game.Control);
  }

  // Client
  else {
    // Send input control (NetReady) if not already done so
    if (!Client.NetReady) {
      Client.SendNetReady(Game.Input);
      Game.Input.Clear();
    }
    // Get full control (NetGo)
    if (!Client.GetNetGo(Game.Control))
      return FALSE;
    // Check errorneous deactivation
    if (!Client.Active) {
      Log(LoadResStr(IDS_NET_LOSTHOST));
      Game.Players.RemoveAtRemoteClient();
      Clear();
    }
  }

  // Success: go
  return TRUE;
}

BOOL C4Network::Init(BOOL fHost, const char *szLocalName, const char *szLocalAddress) {

  printf("C4Network::Init: Active=%d, fHost=%d, Host=%d\n", Active, fHost, Host);
  // Already initialized
  if (Active) {
    // No side changes allowed
    if (fHost != Host) {
      printf("C4Network::Init: Failed because fHost != Host\n");
      return FALSE;
    }
    // Keep earlier initialization
    return TRUE;
  }

  // Init
  Active = TRUE;
  Host = fHost;
  SCopy(szLocalName, LocalName, C4MaxTitle);
  SCopy(szLocalAddress, LocalAddress,
        C4MaxTitle); // Subject to change by reference request

  printf("C4Network::Init: Calling CreateWorkPath()\n");
  // Work path
  if (!Config.Network.CreateWorkPath()) {
    printf("C4Network::Init: CreateWorkPath failed\n");
    Log(LoadResStr(IDS_NET_NOWORKPATH));
    return FALSE;
  }

  // Host
  if (Host) {
    // Log
    sprintf(OSTR, LoadResStr(IDS_NET_HOST), LocalName);
    Log(OSTR);

    // Init master server client
    C4ConfigNetwork *pCfg = &Config.Network;
    printf("Network.Init: As Host. MasterServerSignUp=%d, Addr=%s, Dir=%s\n", pCfg->MasterServerSignUp, pCfg->MasterServerAddress, pCfg->MasterServerPath);
    if (pCfg->MasterServerSignUp && Game.fLobby) {
      BOOL msInit = MasterServerClient.Init(Application.hWindow, pCfg->MasterServerAddress, pCfg->MasterServerPath, pCfg->MasterKeepPeriod, pCfg->MasterReferencePeriod);
      printf("MasterServerClient.Init returned %d\n", msInit);
    }

    printf("C4Network::Init: Calling CreateThread()\n");
    // Create host thread
    if (!(hHostThread = CreateThread(NULL, 0, &HostThread, Application.hWindow, 0, &idHostThread))) {
      printf("C4Network::Init: CreateThread failed\n");
      Log(LoadResStr(IDS_NET_NOTHREAD));
      return FALSE;
    }
    // Set client member number
    Client.Number = C4NET_AtServer;
    // Set control rate (post to control queue)
    Game.Input.AddSetControlRate(Config.Network.ControlRate - 1);
  }

  // Client
  else {
    // Log
    sprintf(OSTR, LoadResStr(IDS_NET_CLIENT), LocalName);
    Log(OSTR);
  }

  // Update console
  Console.UpdateMenus();

  printf("C4Network::Init: Success\n");
  return TRUE;
}

BOOL C4Network::StreamOk(int iResult) {
  if (iResult == C4STRM_Ok)
    return TRUE;
  C4Stream bla;
  char szMessage[256 + 1];
  sprintf(szMessage, bla.ResultText(iResult));
  NetLog(szMessage);
  return FALSE;
}

BOOL C4Network::HandleJoin(C4Stream *pStrm) {

  BOOL fRuntimeJoin = !Lobby;
  BOOL fGetJoinReady = !Lobby;

  // Create client handler
  C4NetworkClient *pClient = new C4NetworkClient;

  // Join client
  if (!(pClient->HandleJoin(Clients.GetFreeNumber(), pStrm, fRuntimeJoin, fGetJoinReady))) {
    delete pClient;
    return FALSE;
  }

  // Add to client list
  if (!Clients.Add(pClient)) {
    delete pClient;
    return FALSE;
  }

  // Update console
  Console.UpdateMenus();

  return TRUE;
}

BOOL C4Network::Join(const char *szServerName, const char *szServerAddress, BOOL fRetrieveNetworkGame) { return Client.Join(szServerName, szServerAddress, fRetrieveNetworkGame); }

int C4Network::GetClientNumber() { return Client.Number; }

const char *C4Network::GetClientName(int iClient) {
  // Local
  if (iClient == GetClientNumber())
    return LocalName;
  // Invalid
  if (!Active || (iClient == C4NET_NoClient))
    return "No client";
  // Search client (host only for now)
  C4NetworkClient *pClient = NULL;
  if (Host)
    pClient = Clients.Get(iClient);

  if (pClient)
    return pClient->Name;

  return "?";
}

BOOL C4Network::SendJoinReady() {
  if (!Active || Host)
    return FALSE;
  return Client.SendJoinReady();
}

BOOL C4Network::RemoveClient(int iClient, const char *szName) {
  if (!Active)
    return FALSE;
  // No killin host
  if (iClient == C4NET_AtServer) {
    Console.Out("No killin' host");
    return FALSE;
  }
  // Local machine is the client: kicked from game
  if (iClient == Client.Number) {
    Log(LoadResStr(IDS_NET_LOCALREMOVED));
    Game.Players.RemoveAtRemoteClient();
    Clear();
    return TRUE;
  }
  // Log
  sprintf(OSTR, LoadResStr(IDS_NET_CLIENTREMOVED), szName, iClient);
  Log(OSTR);
  // Remove all client's players
  Game.Players.RemoveAtClient(szName);
  // Network host: remove client from list
  Clients.Remove(iClient);
  // Update console
  Console.UpdateMenus();
  // Success
  return TRUE;
}

BOOL C4Network::SetClientOutOfSync(int iClient) {
  // Get client
  C4NetworkClient *pClient;
  if (!(pClient = Clients.Get(iClient)))
    return FALSE;
  // Out of sync already
  if (pClient->OutOfSync)
    return FALSE;
  // Set out of sync
  pClient->OutOfSync = TRUE;
  sprintf(OSTR, LoadResStr(IDS_NET_SYNCLOSS), pClient->Name);
  Log(OSTR);
  SoundEffect("SyncError");
  // Update console
  Console.UpdateMenus();
  // Success
  return TRUE;
}

BOOL C4Network::DoLobby() {
  // Instructions
  Log(LoadResStr(IDS_NET_LOBBYINSTRUCTIONS));

  // Expecting participants
  if (Game.C4S.Head.NetworkClients.IsEmpty())
    Log(LoadResStr(IDS_NET_LOBBYWAITING));
  // Expecting specific participants
  else {
    SCopy(LoadResStr(IDS_NET_LOBBYEXPECTING), OSTR);
    for (int cnt = 0; cnt < C4MaxNameList; cnt++)
      if (Game.C4S.Head.NetworkClients.Name[cnt][0]) {
        if (cnt > 0)
          SAppend(", ", OSTR);
        SAppend(Game.C4S.Head.NetworkClients.Name[cnt], OSTR);
      }
    Log(OSTR);
  }

  // Flag lobby active
  Lobby = TRUE;

  // Update console menus
  Console.UpdateMenus();

  // Message Execution Loop
  while (Lobby) {
    if (Application.HandleMessage() == 2)
      return FALSE;
    // Update and draw startup graphics
    if (Application.Fullscreen) {
      C4Facet cgo;
      cgo.Set(Engine.DDraw.lpBack, 0, 0, Config.Graphics.ResX, Config.Graphics.ResY);
      Game.GraphicsSystem.MessageBoard.Execute();
      Game.GraphicsSystem.MessageBoard.Draw(cgo);
      Engine.DDraw.PageFlip();
    }
    Sleep(10);
  }

  // Success
  return TRUE;
}

BOOL C4Network::GetJoinReady() {
  if (!Active || !Host)
    return FALSE;
  if (!Clients.GetJoinReady())
    return FALSE;
  Console.UpdateMenus();
  return TRUE;
}

BOOL C4Network::FinalInit() {
  // Not active
  if (!Active)
    return TRUE;
  // Lobby host: get join ready from all clients / activate
  if (Host && Game.fLobby) {
    // Get join ready from and activate all clients
    if (!GetJoinReady()) {
      Log(LoadResStr(IDS_NET_NOJOINREADY));
      return FALSE;
    }
  }
  // Client: send join ready / get join go
  if (!Host) {
    if (!SendJoinReady()) {
      Log(LoadResStr(IDS_NET_NOJOINGO));
      return FALSE;
    }
  }
  // Synchronize
  Game.Synchronize();
  // Advance master server reference
  MasterServerClient.ForceReference();
  // Success
  return TRUE;
}

int C4Network::GetClientCount() { return Clients.GetCount(); }

BOOL C4Network::DeactivateClient(int iClient) {
  // Get client
  C4NetworkClient *pClient;
  if (!(pClient = Clients.Get(iClient)))
    return FALSE;
  // Deactivate
  pClient->Deactivate();
  // Update console
  Console.UpdateMenus();
  // Success
  return TRUE;
}

BOOL C4Network::HandleReferenceRequest(C4Stream *pStream) {
  C4Packet Packet;
  if (!Active)
    return FALSE;
  if (!pStream)
    return FALSE;
  // Denied: we will send an empty scenario file only
  BOOL fDenied = Config.Network.NoReferenceRequest;
  // Set reference filename
  char szReferenceFilename[_MAX_PATH + 1];
  SCopy(Config.AtNetworkPath(GetFilename(Game.ScenarioFilename)), szReferenceFilename);
  // Remove any old duplicates
  EraseItem(szReferenceFilename);
  // Overwrite Network.LocalAddress with value from current stream for reference
  SCopy(pStream->GetLocalAddress(), LocalAddress, C4MaxTitle);
  // Create group and save info
  C4Group hGroup;
  if (!hGroup.Open(szReferenceFilename, TRUE))
    return FALSE;
  if (!Game.SaveNetworkReference(hGroup, fDenied))
    return FALSE;
  hGroup.Sort(C4FLS_Scenario);
  hGroup.Close();
  // Send reference
  StreamOk(pStream->SendFile(szReferenceFilename, C4PK_NetworkReference, &LogProcess));
  EraseItem(szReferenceFilename);
  Log(LoadResStr(fDenied ? IDS_NET_DENIED : IDS_NET_TRANSFEROK));
  // Receive goodbye
  StreamOk(pStream->ReceivePacket(C4PK_GoodBye, Packet));
  // Destroy stream
  delete pStream;
  // Done
  return TRUE;
}

BOOL C4Network::AdjustControlRate(int iChange) {
  if (!Active || !Host)
    return FALSE;
  // Add to control queue
  Game.Input.AddSetControlRate(iChange);
  return TRUE;
}

BOOL C4Network::CreateReference(const char *szLocalAddress) {
  if (!Active)
    return FALSE;
  printf("C4Network::CreateReference: Start\n");
  // Set reference filename (using temp path instead of NetworkPath to avoid
  // conflict with HandleReferenceRequest)
  char szReferenceFilename[_MAX_PATH + 1];
  SCopy(Config.AtTempPath(GetFilename(Game.ScenarioFilename)), szReferenceFilename);
  printf("C4Network::CreateReference: File path %s\n", szReferenceFilename);
  // Overwrite any duplicate files
  EraseItem(szReferenceFilename);
  // Overwrite Network.LocalAddress with specified value for reference
  SCopy(szLocalAddress, LocalAddress, C4MaxTitle);
  // Create group and save info
  C4Group hGroup;
  if (!hGroup.Open(szReferenceFilename, TRUE)) {
    printf("C4Network::CreateReference: Failed to open C4Group\n");
    return FALSE;
  }
  if (!Game.SaveNetworkReference(hGroup)) {
    printf("C4Network::CreateReference: SaveNetworkReference failed\n");
    return FALSE;
  }
  printf("C4Network::CreateReference: Sorting and closing\n");
  hGroup.Sort(C4FLS_Scenario);
  if (!hGroup.Close()) {
    printf("C4Network::CreateReference: Failed to close group file\n");
    return FALSE;
  }
  // Hardcoded notify to MasterServerClient
  SCopy(szReferenceFilename, MasterServerClient.ReferenceFilename);
  MasterServerClient.Reference = true;
  printf("C4Network::CreateReference: Done\n");

  if(!FileExists(szReferenceFilename)) {
    printf("C4Network::CreateReference: Reference file does not exist after creation: %s\n", szReferenceFilename);
    return FALSE;
  }
  // Done
  return TRUE;
}

DWORD WINAPI C4Network::HostThread(void *lpPar) {
  char ostr[500];
  C4Stream *pListener, *pStrm = NULL;
  C4Packet Packet;
  int iResult;
  BOOL fKeepStream;
  BOOL fTerminate = FALSE;
  HWND hAppWnd = (HWND)lpPar;
  // Create listener stream
  pListener = new C4Stream;
  // Host thread loop
  while (!fTerminate) {
    // Await client & connect
    printf("Host: Awaiting client connection...\n");
    iResult = pListener->Connect(Config.Network.LocalName, C4STRM_Listener, NULL, &pStrm);
    if (iResult == C4STRM_Ok)
      printf("Host: Client connected from %s on port %i\n", pStrm->GetPeerAddress(), pStrm->GetPort());
    else
      printf("Host: Client connection failure: %d (%s)\n", iResult, pListener->ResultText(iResult));
    NetStatus(ostr);
    fKeepStream = FALSE;
    // Handle stream
    while (iResult == C4STRM_Ok) {
      iResult = pStrm->GetPacket(Packet);
      switch (iResult) {
      case C4STRM_Ok:
        switch (Packet.Type) {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case C4PK_RequestJoin:
          // Pass connected stream to main thread by message
          SendMessage(hAppWnd, WM_NET_REQUESTJOIN, 0, (LPARAM)pStrm);
          iResult = C4STRM_Break;
          fKeepStream = TRUE;
          break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case C4PK_RequestNetworkReference:
          // Pass connected stream to main thread by message
          SendMessage(hAppWnd, WM_NET_REQUESTREFERENCE, 0, (LPARAM)pStrm);
          iResult = C4STRM_Break;
          fKeepStream = TRUE;
          break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case C4PK_Message:
          // Send message to main thread (log)
          char szMessage[1024 + 1];
          if (Packet.Size > 256)
            break;
          sprintf(szMessage, "%s (%s): %s", pStrm->GetPeerName(), pStrm->GetPeerAddress(), Packet.Data);
          SendMessage(hAppWnd, WM_USER_LOG, 0, (LPARAM)szMessage);
          break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case C4PK_GoodBye:
          iResult = C4STRM_Break;
          break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        default:
          sprintf(ostr, "Host: Unhandled packet %i received", Packet.Type);
          NetStatus(ostr);
          break;
          // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        }
        break;
      default:
        NetStatus(pStrm->ResultText(iResult));
        break;
      }
    }
    // Thread handling done, close stream if desired
    if (!fKeepStream) {
      delete pStrm;
      pStrm = NULL;
    }
  }
  // Host thread loop terminated
  delete pListener;
  return 0;
}

void C4Network::DrawClients(C4Facet &cgo) {
  if (!Active || !Host)
    return;
  Clients.Draw(cgo);
}

void C4Network::ToggleReferenceRequest() {
  if (!Active || !Host)
    return;
  Toggle(Config.Network.NoReferenceRequest);
  Game.GraphicsSystem.FlashMessage(LoadResStr(Config.Network.NoReferenceRequest ? IDS_NET_REFREQUESTBARRED : IDS_NET_REFREQUESTFREE));
}

void C4Network::ToggleRuntimeJoin() {
  if (!Active || !Host)
    return;
  Toggle(Config.Network.NoRuntimeJoin);
  Game.GraphicsSystem.FlashMessage(LoadResStr(Config.Network.NoRuntimeJoin ? IDS_NET_RUNTIMEJOINBARRED : IDS_NET_RUNTIMEJOINFREE));
  MasterServerClient.ForceReference();
}

void C4Network::SetComment(const char *szComment) {
  if (!Active || !Host)
    return;
  SCopy(szComment, Config.Network.Comment, CFG_MaxString);
  Game.GraphicsSystem.FlashMessage(LoadResStr(IDS_NET_COMMENTCHANGED));
  MasterServerClient.ForceReference();
}

BOOL C4Network::Command(const char *szCommand) {
  // Set comment
  if (SEqual2(szCommand, C4NET_Command_Comment)) {
    SetComment(szCommand + SLen(C4NET_Command_Comment));
    return TRUE;
  }
  // No command
  return FALSE;
}

BOOL C4Network::GetGameReference(const char *szAddress, char *sFilename) {
  C4Stream Strm;
  C4Packet Packet;

  printf("C4Network::GetGameReference: Connecting to %s...\n", szAddress);
  // Get address
  char szHostAddress[C4MaxTitle + 1];
  if (!SCopyEnclosed(szAddress, '<', '>', szHostAddress, C4MaxTitle))
    if (!SCopyEnclosed(szAddress, '(', ')', szHostAddress, C4MaxTitle))
      SCopy(szAddress, szHostAddress, C4MaxTitle);

  // Connect to remote server
  if (Strm.Connect(LocalName, C4STRM_Client, szHostAddress) != C4STRM_Ok) {
    printf("C4Network::GetGameReference: Connection failed\n");
    return FALSE;
  }
  printf("C4Network::GetGameReference: Connected!\n");

  // Get remote peer name
  char szPeerName[C4MaxTitle + 1];
  SCopy(Strm.GetPeerName(), szPeerName);

  // Request network reference
  Packet.Set(C4PK_RequestNetworkReference);
  printf("C4Network::GetGameReference: Requesting reference...\n");
  StreamOk(Strm.PutPacket(Packet));

  // Receive network reference
  char szReferenceFilename[_MAX_PATH + 1];
  SCopy(sFilename, szReferenceFilename);
  if (ItemAttributes(szReferenceFilename) & _A_SUBDIR)
    SAppend("Reference.c4s", szReferenceFilename);
  printf("C4Network::GetGameReference: Receiving file to %s...\n", szReferenceFilename);
  BOOL fResult = StreamOk(Strm.ReceiveFile(szReferenceFilename, C4PK_NetworkReference));
  if (fResult) {
    printf("C4Network::GetGameReference: File received successfully\n");
    SCopy(szReferenceFilename, sFilename);
  } else {
    printf("C4Network::GetGameReference: File reception failed\n");
  }

  // GoodBye
  Packet.Set(C4PK_GoodBye);
  StreamOk(Strm.PutPacket(Packet));

  // Close stream
  Strm.Close();

  // Return result
  return fResult;
}

void C4Network::DeactivateClient() { Client.Deactivate(); }

void C4Network::StoreClientNames(C4NameList &rNames) // Including host name
{
  rNames.Clear();
  rNames.Add(LocalName);
  for (C4NetworkClient *pClient = Clients.First; pClient; pClient = pClient->Next)
    rNames.Add(pClient->Name, pClient->Number);
}

C4NetworkClient *C4Network::GetClientByName(const char *szName) {
  // Host can do this only
  if (!Active || !Host)
    return NULL;
  // Check client list
  for (C4NetworkClient *pClient = Clients.First; pClient; pClient = pClient->Next)
    if (SEqualNoCase(pClient->Name, szName))
      return pClient;
  // No match
  return NULL;
}
