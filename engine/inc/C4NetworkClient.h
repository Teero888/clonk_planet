/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Network interchange for one client (host or client side) */

const int C4NET_NoClient = -1, C4NET_AtServer = 0, C4NET_FirstClientNo = 1;

class C4NetworkClient {
  friend C4Network;
  friend C4NetworkClientList;
  friend C4Console;
  friend C4Game;

public:
  C4NetworkClient();
  ~C4NetworkClient();

public:
  BOOL Active;
  BOOL Deactivated;
  int Number;
  char Name[C4MaxTitle + 1];
  char Address[C4MaxTitle + 1];
  BOOL OutOfSync;
  BOOL NetReady, NetGo;

protected:
  C4Stream *pControlStream;
  C4Stream *pInputStream;
  C4NetworkClient *Next;

public:
  // Client side
  BOOL Join(const char *szServerName, const char *szServerAddress,
            BOOL fRetrieveNetworkGame);
  BOOL GetNetGo(C4Packet &rControl);
  BOOL SendNetReady(C4Packet &rControl);
  BOOL SendJoinReady();
  BOOL SendInput(C4Packet &rInput);
  BOOL SendSync();
  // Host side (client handler)
  BOOL HandleJoin(int iNumber, C4Stream *pStream, BOOL fRuntimeJoin,
                  BOOL fGetJoinReady);
  BOOL SendNetGo(C4Packet &rControl);
  BOOL GetNetReady(C4Packet &rControl);
  BOOL GetJoinReady();
  BOOL GetInput(C4Packet &rInput);
  BOOL GetSync();
  // General
  void Default();
  void Clear();
  void Activate();
  void Deactivate();
  void Draw(C4Facet &cgo);

protected:
  BOOL RetrieveNetworkGame();
  BOOL StreamOk(int iResult);
};

class C4NetworkClientList {
public:
  C4NetworkClientList();
  ~C4NetworkClientList();

public:
  C4NetworkClient *First;

public:
  int GetInput(C4Packet &rInput);
  void Draw(C4Facet &cgo);
  void RemoveDeactivated();
  void Clear();
  void Default();
  int GetCount();
  BOOL SendNetGo(C4Packet &rControl);
  BOOL GetJoinReady();
  BOOL GetNetReady(C4Packet &rControl);
  BOOL Add(C4NetworkClient *pClient);
  BOOL Remove(int iNetworkClient);
  int GetFreeNumber();
  C4NetworkClient *Get(int iNetworkClient);
};
