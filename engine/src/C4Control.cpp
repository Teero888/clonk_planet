/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Control packets contain all player input in the message queue */

#include <C4Include.h>

//=============================== C4ControlJoinClient ================================================

C4ControlJoinClient::C4ControlJoinClient() { Default(); }

void C4ControlJoinClient::Default() {
  Name[0] = 0;
  pStream = NULL;
}

void C4ControlJoinClient::Clear() { Default(); }

void C4ControlJoinClient::Execute() {
  // Empty: no client to join
  if (!Name[0])
    return;
  // Sync clearance (all)
  Game.SyncClearance();
  // Host: handle join
  if (Game.Network.Active && Game.Network.Host) {
    // Notice the stream pointer is valid to the host only
    Game.Network.HandleJoin(pStream);
  }
  // Client: notice join
  else {
    sprintf(OSTR, LoadResStr(IDS_NET_REQUESTJOIN), Name);
    Log(OSTR);
    Game.GraphicsSystem.MessageBoard.EnsureLastMessage();
  }
  // Synchronize
  Game.Synchronize();
  // Clear this control
  Clear();
}

//===================================== C4ControlPacket ================================================

int iPacketDelay = 0;

void C4ControlPacket::Execute() {
  C4Player *pPlr;
  C4ObjectList olList;

  switch (Type) {
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_SetControlRate:
    Game.ControlRate = BoundBy(Game.ControlRate + *((int *)Data), 1, C4MaxControlRate);
    if (Game.Network.Host) {
      sprintf(OSTR, LoadResStr(IDS_NET_CONTROLRATE), Game.ControlRate, Game.FrameCounter);
      Game.GraphicsSystem.FlashMessage(OSTR);
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_SyncCheck:
    ((C4ControlSyncCheck *)Data)->Execute();
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_JoinClient:
    // Store client join data in Game.ControlJoinClient buffer (for later
    // execution)
    Game.ControlJoinClient = *((C4ControlJoinClient *)Data);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_Message:
    // Quoted player message: reroute as cursor object message
    if (pPlr = Game.GraphicsSystem.MessageBoard.GetMessagePlayer((const char *)Data))
      if (*((const char *)Data + SLen(pPlr->Name) + 2) == '"')
        if (pPlr->Cursor) {
          SCopy((const char *)Data + SLen(pPlr->Name) + 2, OSTR);
          if (OSTR[SLen(OSTR) - 1] != '"')
            SAppendChar('"', OSTR);
          GameMsgObject(OSTR, pPlr->Cursor, FPlayer + pPlr->Color);
          break;
        }
    // Message string to log
    Log((const char *)Data);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_PlayerControl:
    // Get player control data
    C4ControlPlayerControl *pPlayerControl;
    pPlayerControl = (C4ControlPlayerControl *)Data;
    // Player incom
    if (pPlr = Game.Players.Get(pPlayerControl->Player))
      pPlr->InCom(pPlayerControl->Com, pPlayerControl->Data);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_JoinPlayer:
    // Get player join data
    C4ControlJoinPlayer *pJoinPlayer;
    pJoinPlayer = (C4ControlJoinPlayer *)Data;
    // Local player (join from local file)
    if (pJoinPlayer->AtClient == Game.Network.GetClientNumber()) {
      Game.JoinPlayer(pJoinPlayer->Filename, pJoinPlayer->AtClient, pJoinPlayer->AtClientName);
    }
    // Remote player (join from temp saved file)
    else {
      char szPlayerFilename[_MAX_PATH + 1];
      sprintf(szPlayerFilename, "%s-%s", Config.AtNetworkPath(pJoinPlayer->AtClientName), GetFilename(pJoinPlayer->Filename));
      EraseItem(szPlayerFilename);
      CStdFile hFile;
      if (!hFile.Save(szPlayerFilename, Data + sizeof(C4ControlJoinPlayer), Size - sizeof(C4ControlJoinPlayer)))
        break;
      Game.JoinPlayer(szPlayerFilename, pJoinPlayer->AtClient, pJoinPlayer->AtClientName);
    }
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_SurrenderPlayer:
    // Surrender player
    if (pPlr = Game.Players.Get(*((int *)Data)))
      pPlr->Surrender();
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_RemovePlayer:
    // Log message
    if (pPlr = Game.Players.Get(*((int *)Data))) {
      sprintf(OSTR, LoadResStr(IDS_PRC_REMOVEPLR), pPlr->Name);
      Log(OSTR);
    }
    // Remove player
    Game.Players.Remove(*((int *)Data));
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_RemoveClient:
    // Remove client
    Game.Network.RemoveClient(*((int *)Data), (char *)(Data + sizeof(int)));
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_SetHostility:
    // Get set-hostility data
    C4ControlSetHostility *pSetHostility;
    pSetHostility = (C4ControlSetHostility *)Data;
    // Set hostility
    if (pPlr = Game.Players.Get(pSetHostility->Player))
      pPlr->SetHostility(pSetHostility->Opponent, pSetHostility->Hostility);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_PlayerSelection:
    // Get player selection data
    int iCnt, iPlayer, iCount;
    iPlayer = ((int *)Data)[0];
    iCount = ((int *)Data)[1];
    for (iCnt = 0; iCnt < iCount; iCnt++)
      olList.Add(Game.Objects.ObjectPointer(((int *)Data)[2 + iCnt]));
    // Player selection
    if (pPlr = Game.Players.Get(iPlayer))
      pPlr->SelectCrew(olList);
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  case C4PK_PlayerCommand:
    ((C4ControlPlayerCommand *)Data)->Execute();
    break;
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // - - - - - - - - - - -
  default:
    sprintf(OSTR, "Undefined packet %i in control packet", Type);
    NetLog(OSTR);
    break;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - -
  }
}

void C4ControlPlayerCommand::Execute() {
  // sprintf(OSTR,"Player %d, Command %s %d/%d %d
  // %d",Player,CommandName(Command),X,Y,Target,Target2); Log(OSTR);
  C4Player *pPlr;
  if (pPlr = Game.Players.Get(Player))
    pPlr->ObjectCommand(Command, Game.Objects.ObjectPointer(Target), X, Y, Game.Objects.ObjectPointer(Target2), Data, AddMode);
}

// extern int ScreenRate;

void C4ControlSyncCheck::Execute() // And determine control time difference
{
  // Network not active
  if (!Game.Network.Active)
    return;
  // Hosts need not
  if (Game.Network.Host)
    return;

  // Create own sync value (set on construction)
  C4ControlSyncCheck SyncCheck;

  // Compare and reset control time
  Game.ControlTimeBehind = SyncCheck.ControlTime - ControlTime;
  ControlTime = SyncCheck.ControlTime = 0;
  // Adjust screen rate by behind value
  /*if (Game.ControlTimeBehind>250)
          ScreenRate = BoundBy( 1 + Game.ControlTimeBehind/250, 1, 25 );
  else
          ScreenRate = BoundBy( ScreenRate-1, 1, 15 );*/

  // Not equal
  if (!MemEqual(this, &SyncCheck, sizeof(C4ControlSyncCheck))) {
    // Message
    NetLog("Synchronization loss!");
    sprintf(OSTR, "Host Frm %i Rnc %i Rn3 %i Cpx %i PXS %i MMi %i Obc %i Oei %i", Frame, RandomCount, Random3, AllCrewPosX, PXSCount, MassMoverIndex, ObjectCount, ObjectEnumerationIndex);
    NetLog(OSTR);
    sprintf(OSTR, "Clnt Frm %i Rnc %i Rn3 %i Cpx %i PXS %i MMi %i Obc %i Oei %i", SyncCheck.Frame, SyncCheck.RandomCount, SyncCheck.Random3, SyncCheck.AllCrewPosX, SyncCheck.PXSCount,
            SyncCheck.MassMoverIndex, SyncCheck.ObjectCount, SyncCheck.ObjectEnumerationIndex);
    NetLog(OSTR);
    SoundEffect("SyncError");
    // Deactivate
    Game.Network.DeactivateClient();
  }
}

extern int FRndPtr3;

int GetAllCrewPosX() {
  int cpx = 0;
  for (C4Player *pPlr = Game.Players.First; pPlr; pPlr = pPlr->Next)
    for (C4ObjectLink *clnk = pPlr->Crew.First; clnk; clnk = clnk->Next)
      cpx += clnk->Obj->x;
  return cpx;
}

void C4ControlSyncCheck::Set() {
  ControlTime = Game.ControlTime();
  Frame = Game.FrameCounter;
  Random3 = FRndPtr3;
  RandomCount = ::RandomCount;
  AllCrewPosX = GetAllCrewPosX();
  PXSCount = Game.PXS.Count;
  MassMoverIndex = Game.MassMover.CreatePtr;
  ObjectCount = Game.Objects.ObjectCount();
  ObjectEnumerationIndex = Game.ObjectEnumerationIndex;
}

C4ControlSyncCheck::C4ControlSyncCheck() { Set(); }

//=========================================== C4Control ================================================

C4Control::C4Control() { Default(); }

C4Control::~C4Control() { Clear(); }

BOOL C4Control::AddPlayerControl(int iPlayer, int iCom, int iData) {
  // Set packet data
  C4ControlPlayerControl PlayerControl;
  PlayerControl.Player = iPlayer;
  PlayerControl.Com = iCom;
  PlayerControl.Data = iData;
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_PlayerControl, &PlayerControl, sizeof(C4ControlPlayerControl));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddMessage(const char *szMessage) {
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_Message, (void *)szMessage, SLen(szMessage) + 1);
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddJoinPlayer(const char *szFilename) {
  // Load player file
  BYTE *bypPlayerFile;
  int iPlayerFileSize;
  CStdFile hFile;
  if (!hFile.Load(szFilename, &bypPlayerFile, &iPlayerFileSize))
    return FALSE;
  // Set packet data
  C4ControlJoinPlayer JoinPlayer;
  SCopy(szFilename, JoinPlayer.Filename, _MAX_PATH);
  JoinPlayer.AtClient = Game.Network.GetClientNumber();
  SCopy(Game.Network.LocalName, JoinPlayer.AtClientName, C4MaxTitle);
  // Add packet (with player file added)
  C4Packet Packet;
  Packet.Set(C4PK_JoinPlayer, &JoinPlayer, sizeof(C4ControlJoinPlayer));
  Packet.AddData(bypPlayerFile, iPlayerFileSize);
  AddStatic(Packet);
  // Free player file buffer
  delete[] bypPlayerFile;
  // Success
  return TRUE;
}

void C4Control::Execute() {
  // Execute child packets
  C4ControlPacket Packet;
  for (int iPacket = 0; GetStatic(iPacket, Packet); iPacket++)
    Packet.Execute();
  Clear();
}

void C4Control::Default() {
  C4Packet::Default();
  Set(C4PK_Control);
}

void C4Control::Clear() {
  C4Packet::Clear();
  Set(C4PK_Control);
}

BOOL C4Control::AddSetHostility(int iPlayer, int iOpponent, int iHostility) {
  // Set packet data
  C4ControlSetHostility SetHostility;
  SetHostility.Player = iPlayer;
  SetHostility.Opponent = iOpponent;
  SetHostility.Hostility = iHostility;
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_SetHostility, &SetHostility, sizeof(C4ControlSetHostility));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddSurrenderPlayer(int iPlayer) {
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_SurrenderPlayer, &iPlayer, sizeof(int));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddRemovePlayer(int iPlayer) {
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_RemovePlayer, &iPlayer, sizeof(int));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddRemoveClient(int iClient, const char *szName) {
  // Add packet
  C4Packet Packet;
  int iSize = sizeof(int) + SLen(szName) + 1;
  BYTE *bpBuf = new BYTE[iSize];
  MemCopy(&iClient, bpBuf, sizeof(int));
  SCopy(szName, (char *)bpBuf + sizeof(int));
  Packet.Set(C4PK_RemoveClient, bpBuf, iSize);
  AddStatic(Packet);
  delete[] bpBuf;
  // Success
  return TRUE;
}

BOOL C4Control::AddSyncCheck() {
  // Set sync check (on construction)
  C4ControlSyncCheck SyncCheck;
  // Add packet (to head of queue)
  C4Packet Packet;
  Packet.Set(C4PK_SyncCheck, &SyncCheck, sizeof(C4ControlSyncCheck));
  AddStaticHead(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddJoinClient(C4Stream *pStream) {
  // Safety
  if (!pStream)
    return FALSE;
  // Set packet data
  C4ControlJoinClient JoinClient;
  SCopy(pStream->GetPeerName(), JoinClient.Name, C4MaxTitle);
  // Notice stream pointer will be valid for host only
  JoinClient.pStream = pStream;
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_JoinClient, &JoinClient, sizeof(C4ControlJoinClient));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddSetControlRate(int iRate) {
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_SetControlRate, &iRate, sizeof(int));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddSetSyncRate(int iRate) {
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_SetSyncRate, &iRate, sizeof(int));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddPlayerSelection(int iPlayer, C4ObjectList &rList) {
  // Create data (player, count, object numbers)
  int iSize = 1 + 1 + rList.ObjectCount();
  int *ipData = new int[iSize];
  ipData[0] = iPlayer;
  ipData[1] = rList.ObjectCount();
  C4ObjectLink *cLnk;
  int iIndex;
  for (cLnk = rList.First, iIndex = 2; cLnk; cLnk = cLnk->Next, iIndex++)
    ipData[iIndex] = cLnk->Obj->Number;
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_PlayerSelection, ipData, iSize * sizeof(int));
  AddStatic(Packet);
  // Free data buffer
  delete[] ipData;
  // Success
  return TRUE;
}

BOOL C4Control::AddPlayerCommand(int iPlayer, int iCommand, int iX, int iY, C4Object *pTarget, C4Object *pTarget2, int iData, int iAddMode) {
  // Set packet data
  C4ControlPlayerCommand PlayerCommand;
  PlayerCommand.Player = iPlayer;
  PlayerCommand.Command = iCommand;
  PlayerCommand.X = iX;
  PlayerCommand.Y = iY;
  PlayerCommand.Target = 0;
  if (pTarget)
    PlayerCommand.Target = pTarget->Number;
  PlayerCommand.Target2 = 0;
  if (pTarget2)
    PlayerCommand.Target2 = pTarget2->Number;
  PlayerCommand.Data = iData;
  PlayerCommand.AddMode = iAddMode;
  // Add packet
  C4Packet Packet;
  Packet.Set(C4PK_PlayerCommand, &PlayerCommand, sizeof(PlayerCommand));
  AddStatic(Packet);
  // Success
  return TRUE;
}

BOOL C4Control::AddControlTime() {
  // Add packet
  C4Packet Packet;
  DWORD dwTime = Game.ControlTime();
  Packet.Set(C4PK_ControlTime, &dwTime, sizeof(DWORD));
  AddStaticHead(Packet);
  // Success
  return TRUE;
}

C4ControlQueue::C4ControlQueue() {}

C4ControlQueue::~C4ControlQueue() {}

BOOL C4ControlQueue::Save(C4Group &hGroup) {
  // Save to temp file
  if (!C4Packet::Save(Config.AtTempPath(C4CFN_ControlQueue)))
    return FALSE;
  // Move temp file to group
  if (!hGroup.Move(Config.AtTempPath(C4CFN_ControlQueue)))
    return FALSE;
  // Success
  return TRUE;
}

BOOL C4ControlQueue::Load(C4Group &hGroup) { return TRUE; }
