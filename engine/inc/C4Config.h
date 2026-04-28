/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Game configuration as stored in registry */

class C4ConfigGeneral {
public:
  char Name[CFG_MaxString + 1];
  char Code[CFG_MaxString + 1];
  char UserNameOld[CFG_MaxString + 1];
  char RegCodeOld[CFG_MaxString + 1];
  char Language[CFG_MaxString + 1];
  char Definitions[CFG_MaxString + 1];
  int DebugMode;
  char RXFontName[CFG_MaxString + 1];
  int RXFontSize;
  char FEFontName[CFG_MaxString + 1];
  int FEFontSize;
  char PlayerPath[CFG_MaxString + 1];
  char RoundPath[CFG_MaxString + 1];
  char SaveGameFolder[CFG_MaxString + 1];
  char SaveScenarioFolder[CFG_MaxString + 1];
  char MissionAccess[CFG_MaxString + 1];
  // Determined at run-time
  char ExePath[CFG_MaxString + 1];
  char TempPath[CFG_MaxString + 1];
  char LogPath[CFG_MaxString + 1];

public:
  void DefaultLanguage();
  void DefaultName();
  void CheckSaveFolders(WORD wSaveGame, WORD wSaveScenario);
  void DeterminePaths();
};

class C4ConfigDeveloper {
public:
  int Active;
  int SendDefReload;
  int AutoEditScan;
};

class C4ConfigGraphics {
public:
  int Resolution;
  int SplitscreenDividers;
  int ShowPlayerInfoAlways;
  int ShowCommands;
  int ShowCommandKeys;
  int ShowPortraits;
  int ShowStartupMessages;
  int VerboseObjectLoading;
  int ColorAnimation;
  int SmokeLevel;
  int DDrawAccel;
  int VideoModule;
  // Determined at run-time
  int ResX, ResY;

public:
  void DetermineResolution();
};

class C4ConfigSound {
public:
  int RXSound;
  int RXSoundLoops;
  int RXMusic;
  int FEMusic;
  int FESamples;
};

class C4ConfigNetwork {
public:
  int Active;
  int ControlRate;
  int Lobby;
  int NoRuntimeJoin;
  int NoReferenceRequest;
  int AsynchronousControl;
  char LocalName[CFG_MaxString + 1];
  char LocalAddress[CFG_MaxString + 1];
  char HostList[CFG_MaxString + 1];
  char WorkPath[CFG_MaxString + 1];
  char Comment[CFG_MaxString + 1];
  int MasterServerSignUp;
  int MasterServerActive;
  char MasterServerAddress[CFG_MaxString + 1];
  char MasterServerPath[CFG_MaxString + 1];
  int MasterKeepPeriod;
  int MasterReferencePeriod;

public:
  void ClearWorkPath();
  BOOL CreateWorkPath();
  void CheckPath();
};

class C4ConfigExplorer {
public:
  int Mode;
  int Run;
  char Participants[CFG_MaxString + 1];
  char Definitions[CFG_MaxString + 1];
  char EditorBitmap[CFG_MaxString + 1];
  char EditorMusic[CFG_MaxString + 1];
  char EditorRichText[CFG_MaxString + 1];
  char EditorScript[CFG_MaxString + 1];
  char EditorText[CFG_MaxString + 1];
  char EditorSound[CFG_MaxString + 1];
  char EditorZip[CFG_MaxString + 1];
  char EditorDefinition[CFG_MaxString + 1];
  char EditorHtml[CFG_MaxString + 1];
  int EditorUseShell;
  int Kindersicherung;
  int ShowQuickStart;
  char Reload[CFG_MaxString + 1];
};

class C4ConfigGamepad {
public:
  int Button[C4MaxKey];
  int MaxX, MinX, MaxY, MinY;
};

class C4Config : protected CStdConfig {
public:
  C4Config();
  ~C4Config();

public:
  C4ConfigGeneral General;
  C4ConfigDeveloper Developer;
  C4ConfigGraphics Graphics;
  C4ConfigSound Sound;
  C4ConfigNetwork Network;
  C4ConfigExplorer Explorer;
  C4ConfigGamepad Gamepad;
  int Keyboard[C4MaxKeyboardSet][C4MaxKey];

public:
  void Default();
  void ResetControls();
  BOOL Save();
  BOOL Load();
  BOOL Init();
  BOOL Registered();
  BOOL TestRegistered(const char *szName, const char *szCode);
  BOOL TestRegisteredOld(const char *szName, const char *szCode);
  BOOL ValidRegistrationName(const char *szName);
  BOOL IsFreeFolder(const char *szFoldername, const char *szMaker);
  const char *AtExePath(const char *szFilename);
  const char *AtTempPath(const char *szFilename);
  const char *AtLogPath(const char *szFilename);
  const char *AtNetworkPath(const char *szFilename);
  const char *AtExeRelativePath(const char *szFilename);

protected:
  BOOL IsRegistered(const char *szName, const char *szCode, const char *szSecKey, BOOL fOld);
};
