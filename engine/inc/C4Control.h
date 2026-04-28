/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Control packets contain all player input in the message queue */

struct C4ControlPlayerControl {
  int Player;
  int Com;
  int Data;
};

class C4ControlPlayerCommand {
public:
  int Player;
  int Command;
  int X, Y;
  int Target;
  int Target2;
  int Data;
  int AddMode;

public:
  void Execute();
};

struct C4ControlSetHostility {
  int Player;
  int Opponent;
  int Hostility;
};

class C4ControlSyncCheck {
public:
  C4ControlSyncCheck();

public:
  int ControlTime; // (do not compare)
  int Frame;
  int Random3;
  int RandomCount;
  int AllCrewPosX;
  int PXSCount;
  int MassMoverIndex;
  int ObjectCount;
  int ObjectEnumerationIndex;

public:
  void Set();
  void Execute();
};

struct C4ControlJoinPlayer {
  char Filename[_MAX_PATH + 1];
  int AtClient;
  char AtClientName[C4MaxTitle + 1];
};

class C4ControlJoinClient {
public:
  C4ControlJoinClient();

public:
  char Name[C4MaxTitle + 1];
  C4Stream *pStream;

public:
  void Default();
  void Clear();
  void Execute();
};

class C4ControlPacket : public C4Packet {
public:
  void Execute();
};

class C4Control : public C4Packet {
public:
  C4Control();
  ~C4Control();

public:
  void Clear();
  void Default();
  void Execute();
  BOOL AddJoinPlayer(const char *szFilename);
  BOOL AddMessage(const char *szMessage);
  BOOL AddPlayerControl(int iPlayer, int iCom, int iData = 0);
  BOOL AddPlayerSelection(int iPlayer, C4ObjectList &rList);
  BOOL AddSetSyncRate(int iRate);
  BOOL AddSetControlRate(int iRate);
  BOOL AddJoinClient(C4Stream *pStream);
  BOOL AddSyncCheck();
  BOOL AddRemoveClient(int iClient, const char *szName);
  BOOL AddRemovePlayer(int iPlayer);
  BOOL AddSurrenderPlayer(int iPlayer);
  BOOL AddSetHostility(int iPlayer, int iOpponent, int iHostility);
  BOOL AddPlayerCommand(int iPlayer, int iCommand, int iX, int iY,
                        C4Object *pTarget, C4Object *pTarget2 = NULL,
                        int iData = 0, int iAddMode = C4P_Command_Set);
  BOOL AddControlTime();
};

class C4ControlQueue : public C4PacketQueue {
public:
  C4ControlQueue();
  ~C4ControlQueue();

public:
  BOOL Load(C4Group &hGroup);
  BOOL Save(C4Group &hGroup);
};
