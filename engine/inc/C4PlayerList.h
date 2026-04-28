/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Dynamic list to hold runtime player data */

class C4PlayerList {
public:
  C4PlayerList();
  ~C4PlayerList();

public:
  C4Player *First;

public:
  void Default();
  void Clear();
  void Execute();
  void DenumeratePointers();
  void EnumeratePointers();
  void ClearPointers(C4Object *pObj);
  int GetCount();
  int GetIndex(C4Player *pPlr);
  int GetCountNotEliminated();
  int ControlTakenBy(int iControl);
  int AverageValueGain();
  C4Player *Get(int iPlayer);
  C4Player *GetByIndex(int iIndex);
  C4Player *GetByName(const char *szName, int iExcluding = NO_OWNER);
  C4Player *GetLocalByKbdSet(int iKbdSet);
  C4Player *GetLocalByIndex(int iIndex);
  C4Player *GetAtClient(int iClient, int iIndex = 0);
  C4Player *GetAtClient(const char *szName, int iIndex = 0);
  C4Player *GetAtRemoteClient(int iIndex = 0);
  C4Player *Join(const char *szFilename, BOOL fScenarioInit, int iAtClient, const char *szAtClientName);
  BOOL FileInUse(const char *szFilename);
  BOOL Retire(C4Player *pPlr);
  BOOL Evaluate();
  BOOL Save();
  BOOL Save(C4Group &hGroup, BOOL fNetwork = FALSE);
  BOOL Remove(int iPlayer);
  BOOL Remove(C4Player *pPlr);
  BOOL RemoveAtRemoteClient();
  BOOL MouseControlTaken();
  BOOL RemoveAtClient(int iClient);
  BOOL RemoveAtClient(const char *szName);
  BOOL Valid(int iPlayer);
  BOOL Hostile(int iPlayer1, int iPlayer2);
  BOOL PositionTaken(int iPosition);
  BOOL ColorTaken(int iColor);
  BOOL ControlTaken(int iControl);

protected:
  int GetFreeNumber();
};
