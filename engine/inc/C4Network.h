/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles all network interchange (host or client side) */

class C4Network {
  friend C4Console;
  friend C4Game;

public:
  C4Network();
  ~C4Network();

public:
  BOOL Active;
  BOOL Host;
  BOOL Lobby, WaitingForLobby;
  char LocalName[C4MaxTitle + 1];
  char LocalAddress[C4MaxTitle + 1];

protected:
  // Host side
  HANDLE hHostThread;
  DWORD idHostThread;
  C4NetworkClientList Clients;
  // Client side
  C4NetworkClient Client;
  // Master server client (host side)
  C4MasterServerClient MasterServerClient;

public:
  void Default();
  void Clear();
  void SetComment(const char *szComment);
  void ToggleRuntimeJoin();
  void DrawClients(C4Facet &cgo);
  void DeactivateClient();
  void StoreClientNames(C4NameList &rNames);
  void ToggleReferenceRequest();
  BOOL Init(BOOL fHost, const char *szLocalName, const char *szLocalAddress);
  BOOL GetGameReference(const char *szAddress, char *sFilename);
  BOOL Command(const char *szCommand);
  BOOL CreateReference(const char *szLocalAddress);
  BOOL AdjustControlRate(int iChange);
  BOOL HandleReferenceRequest(C4Stream *pStream);
  BOOL DeactivateClient(int iClient);
  BOOL FinalInit();
  BOOL GetJoinReady();
  BOOL DoLobby();
  BOOL SetClientOutOfSync(int iClient);
  BOOL RemoveClient(int iClient, const char *szName);
  BOOL SendJoinReady();
  BOOL Join(const char *szServerName, const char *szServerAddress, BOOL fRetrieveNetworkGame);
  BOOL HandleJoin(C4Stream *pStrm);
  const char *GetClientName(int iClient);
  int GetClientCount();
  int GetClientNumber();
  // Synchronous control
  BOOL Execute();
  // Asynchronous control
  BOOL ExecuteControl(C4Control &rControl);
  BOOL ExecuteInput(C4Control &rInput);

protected:
  C4NetworkClient *GetClientByName(const char *szName);
  BOOL StreamOk(int iResult);

private:
  static DWORD WINAPI HostThread(LPVOID lpPar);
  int LobbyProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
