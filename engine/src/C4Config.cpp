/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Game configuration as stored in registry */

#include <C4Include.h>

char AtPathFilename[_MAX_PATH + 1];

#define ofC4Cfg(x) offsetof(C4Config, x)

CStdConfigValue C4ConfigMap[] =

    {

        {CFG_Company, "RedWolf Design", 0, 0},
        {CFG_Product, "Clonk 4", 0, 0},

        {CFG_Section, "General", 0, 0},
        {CFG_String, "Name", ofC4Cfg(General.Name), 0},
        {CFG_String, "Code", ofC4Cfg(General.Code), 0},
        {CFG_String, "UserName", ofC4Cfg(General.UserNameOld), 0},
        {CFG_String, "RegCode", ofC4Cfg(General.RegCodeOld), 0},
        {CFG_String, "Language", ofC4Cfg(General.Language), 0},
        {CFG_String, "Definitions", ofC4Cfg(General.Definitions), 0},
        {CFG_Integer, "DebugMode", ofC4Cfg(General.DebugMode), 0},
        {CFG_String, "RXFontName", ofC4Cfg(General.RXFontName), (long)"Comic Sans MS"},
        {CFG_Integer, "RXFontSize", ofC4Cfg(General.RXFontSize), 10},
        {CFG_String, "FEFontName", ofC4Cfg(General.FEFontName), (long)"Comic Sans MS"},
        {CFG_Integer, "FEFontSize", ofC4Cfg(General.FEFontSize), 9},
        {CFG_String, "TempPath", ofC4Cfg(General.TempPath), 0},
        {CFG_String, "LogPath", ofC4Cfg(General.LogPath), 0},
        {CFG_String, "PlayerPath", ofC4Cfg(General.PlayerPath), 0},
        {CFG_String, "RoundPath", ofC4Cfg(General.RoundPath), 0},
        {CFG_String, "SaveGameFolder", ofC4Cfg(General.SaveGameFolder), 0},
        {CFG_String, "SaveScenarioFolder", ofC4Cfg(General.SaveScenarioFolder), 0},
        {CFG_String, "MissionAccess", ofC4Cfg(General.MissionAccess), 0},

        {CFG_Section, "Developer", 0, 0},
        {CFG_Integer, "Active", ofC4Cfg(Developer.Active), 0},
        {CFG_Integer, "SendDefReload", ofC4Cfg(Developer.SendDefReload), 0},
        {CFG_Integer, "AutoEditScan", ofC4Cfg(Developer.AutoEditScan), 0},

        {CFG_Section, "Graphics", 0, 0},
        {CFG_Integer, "Resolution", ofC4Cfg(Graphics.Resolution), 1},
        {CFG_Integer, "SplitscreenDividers", ofC4Cfg(Graphics.SplitscreenDividers), 1},
        {CFG_Integer, "ShowPlayerInfoAlways", ofC4Cfg(Graphics.ShowPlayerInfoAlways), 1},
        {CFG_Integer, "ShowPortraits", ofC4Cfg(Graphics.ShowPortraits), 1},
        {CFG_Integer, "ShowCommands", ofC4Cfg(Graphics.ShowCommands), 1},
        {CFG_Integer, "ShowCommandKeys", ofC4Cfg(Graphics.ShowCommandKeys), 1},
        {CFG_Integer, "ShowStartupMessages", ofC4Cfg(Graphics.ShowStartupMessages), 1},
        {CFG_Integer, "ColorAnimation", ofC4Cfg(Graphics.ColorAnimation), 1},
        {CFG_Integer, "SmokeLevel", ofC4Cfg(Graphics.SmokeLevel), 200},
        {CFG_Integer, "DDrawAccel", ofC4Cfg(Graphics.DDrawAccel), 1},
        {CFG_Integer, "VerboseObjectLoading", ofC4Cfg(Graphics.VerboseObjectLoading), 0},
        {CFG_Integer, "VideoModule", ofC4Cfg(Graphics.VideoModule), 0},

        {CFG_Section, "Sound", 0, 0},
        {CFG_Integer, "RXSound", ofC4Cfg(Sound.RXSound), 1},
        {CFG_Integer, "RXSoundLoops", ofC4Cfg(Sound.RXSoundLoops), 1},
        {CFG_Integer, "RXMusic", ofC4Cfg(Sound.RXMusic), 1},
        {CFG_Integer, "FEMusic", ofC4Cfg(Sound.FEMusic), 1},
        {CFG_Integer, "FESamples", ofC4Cfg(Sound.FESamples), 1},

        {CFG_Section, "Network", 0, 0},
        {CFG_Integer, "Active", ofC4Cfg(Network.Active), 0},
        {CFG_String, "LocalName", ofC4Cfg(Network.LocalName), (long)"Unknown"},
        {CFG_String, "LocalAddress", ofC4Cfg(Network.LocalAddress), (long)"Unknown"},
        {CFG_String, "HostList", ofC4Cfg(Network.HostList), 0},
        {CFG_Integer, "ControlRate", ofC4Cfg(Network.ControlRate), 1},
        {CFG_String, "WorkPath", ofC4Cfg(Network.WorkPath), (long)"Network"},
        {CFG_Integer, "Lobby", ofC4Cfg(Network.Lobby), 0},
        {CFG_Integer, "NoRuntimeJoin", ofC4Cfg(Network.NoRuntimeJoin), 0},
        {CFG_Integer, "NoReferenceRequest", ofC4Cfg(Network.NoReferenceRequest), 0},
        {CFG_Integer, "AsynchronousControl", ofC4Cfg(Network.AsynchronousControl), 0},
        {CFG_String, "Comment", ofC4Cfg(Network.Comment), 0},

        {CFG_Integer, "MasterServerSignUp", ofC4Cfg(Network.MasterServerSignUp), 0},
        {CFG_Integer, "MasterServerActive", ofC4Cfg(Network.MasterServerActive), 0},
        {CFG_String, "MasterServerAddress", ofC4Cfg(Network.MasterServerAddress), (long)"www.clonk.de"},
        {CFG_String, "MasterServerDirectory", ofC4Cfg(Network.MasterServerPath), (long)"/cgi-bin/cpmaster.pl"},
        {CFG_Integer, "MasterKeepPeriod", ofC4Cfg(Network.MasterKeepPeriod), 60},
        {CFG_Integer, "MasterReferencePeriod", ofC4Cfg(Network.MasterReferencePeriod), 120},

        {CFG_Section, "Explorer", 0, 0},
        {CFG_Integer, "Mode", ofC4Cfg(Explorer.Mode), 0},
        {CFG_Integer, "Run", ofC4Cfg(Explorer.Run), 0},
        {CFG_Integer, "EditorUseShell", ofC4Cfg(Explorer.EditorUseShell), 1},
        {CFG_Integer, "Kindersicherung", ofC4Cfg(Explorer.Kindersicherung), 1},
        {CFG_Integer, "ShowQuickStart", ofC4Cfg(Explorer.ShowQuickStart), 1},
        {CFG_String, "Participants", ofC4Cfg(Explorer.Participants), 0},
        {CFG_String, "Definitions", ofC4Cfg(Explorer.Definitions), 0},
        {CFG_String, "EditorBitmap", ofC4Cfg(Explorer.EditorBitmap), (long)"MSPaint.exe"},
        {CFG_String, "EditorMusic", ofC4Cfg(Explorer.EditorMusic), (long)"MPlayer.exe"},
        {CFG_String, "EditorRichText", ofC4Cfg(Explorer.EditorRichText), (long)"Wordpad.exe"},
        {CFG_String, "EditorScript", ofC4Cfg(Explorer.EditorScript), (long)"Notepad.exe"},
        {CFG_String, "EditorText", ofC4Cfg(Explorer.EditorText), (long)"Notepad.exe"},
        {CFG_String, "EditorSound", ofC4Cfg(Explorer.EditorSound), (long)"MPlayer.exe"},
        {CFG_String, "EditorZip", ofC4Cfg(Explorer.EditorZip), (long)"WinZip32.exe"},
        {CFG_String, "EditorDefinition", ofC4Cfg(Explorer.EditorDefinition), 0},
        {CFG_String, "EditorHtml", ofC4Cfg(Explorer.EditorHtml), (long)"iexplore.exe"},
        {CFG_String, "Reload", ofC4Cfg(Explorer.Reload), 0},

        {CFG_Section, "Controls", 0, 0},

        {CFG_Integer, "Kbd1Key1", ofC4Cfg(Keyboard[0][0]), 'Q'},
        {CFG_Integer, "Kbd1Key2", ofC4Cfg(Keyboard[0][1]), 'W'},
        {CFG_Integer, "Kbd1Key3", ofC4Cfg(Keyboard[0][2]), 'E'},
        {CFG_Integer, "Kbd1Key4", ofC4Cfg(Keyboard[0][3]), 'A'},
        {CFG_Integer, "Kbd1Key5", ofC4Cfg(Keyboard[0][4]), 'S'},
        {CFG_Integer, "Kbd1Key6", ofC4Cfg(Keyboard[0][5]), 'D'},
        {CFG_Integer, "Kbd1Key7", ofC4Cfg(Keyboard[0][6]), 'Y'},
        {CFG_Integer, "Kbd1Key8", ofC4Cfg(Keyboard[0][7]), 'X'},
        {CFG_Integer, "Kbd1Key9", ofC4Cfg(Keyboard[0][8]), 'C'},
        {CFG_Integer, "Kbd1Key10", ofC4Cfg(Keyboard[0][9]), 226},
        {CFG_Integer, "Kbd1Key11", ofC4Cfg(Keyboard[0][10]), 'V'},
        {CFG_Integer, "Kbd1Key12", ofC4Cfg(Keyboard[0][11]), 'F'},

        {CFG_Integer, "Kbd2Key1", ofC4Cfg(Keyboard[1][0]), 103},
        {CFG_Integer, "Kbd2Key2", ofC4Cfg(Keyboard[1][1]), 104},
        {CFG_Integer, "Kbd2Key3", ofC4Cfg(Keyboard[1][2]), 105},
        {CFG_Integer, "Kbd2Key4", ofC4Cfg(Keyboard[1][3]), 100},
        {CFG_Integer, "Kbd2Key5", ofC4Cfg(Keyboard[1][4]), 101},
        {CFG_Integer, "Kbd2Key6", ofC4Cfg(Keyboard[1][5]), 102},
        {CFG_Integer, "Kbd2Key7", ofC4Cfg(Keyboard[1][6]), 97},
        {CFG_Integer, "Kbd2Key8", ofC4Cfg(Keyboard[1][7]), 98},
        {CFG_Integer, "Kbd2Key9", ofC4Cfg(Keyboard[1][8]), 99},
        {CFG_Integer, "Kbd2Key10", ofC4Cfg(Keyboard[1][9]), 96},
        {CFG_Integer, "Kbd2Key11", ofC4Cfg(Keyboard[1][10]), 110},
        {CFG_Integer, "Kbd2Key12", ofC4Cfg(Keyboard[1][11]), 107},

        {CFG_Integer, "Kbd3Key1", ofC4Cfg(Keyboard[2][0]), 'I'},
        {CFG_Integer, "Kbd3Key2", ofC4Cfg(Keyboard[2][1]), 'O'},
        {CFG_Integer, "Kbd3Key3", ofC4Cfg(Keyboard[2][2]), 'P'},
        {CFG_Integer, "Kbd3Key4", ofC4Cfg(Keyboard[2][3]), 'K'},
        {CFG_Integer, "Kbd3Key5", ofC4Cfg(Keyboard[2][4]), 'L'},
        {CFG_Integer, "Kbd3Key6", ofC4Cfg(Keyboard[2][5]), 192},
        {CFG_Integer, "Kbd3Key7", ofC4Cfg(Keyboard[2][6]), 188},
        {CFG_Integer, "Kbd3Key8", ofC4Cfg(Keyboard[2][7]), 190},
        {CFG_Integer, "Kbd3Key9", ofC4Cfg(Keyboard[2][8]), 189},
        {CFG_Integer, "Kbd3Key10", ofC4Cfg(Keyboard[2][9]), 'M'},
        {CFG_Integer, "Kbd3Key11", ofC4Cfg(Keyboard[2][10]), 222},
        {CFG_Integer, "Kbd3Key12", ofC4Cfg(Keyboard[2][11]), 186},

        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][0]), VK_INSERT},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][1]), VK_HOME},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][2]), VK_PRIOR},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][3]), VK_DELETE},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][4]), VK_UP},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][5]), VK_NEXT},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][6]), VK_LEFT},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][7]), VK_DOWN},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][8]), VK_RIGHT},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][9]), VK_END},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][10]), VK_RETURN},
        {CFG_Integer, NULL, ofC4Cfg(Keyboard[3][11]), VK_BACK},

        {CFG_Section, "Gamepad", 0, 0},

        {CFG_Integer, "MaxX", ofC4Cfg(Gamepad.MaxX), 0},
        {CFG_Integer, "MinX", ofC4Cfg(Gamepad.MinX), 0},
        {CFG_Integer, "MaxY", ofC4Cfg(Gamepad.MaxY), 0},
        {CFG_Integer, "MinY", ofC4Cfg(Gamepad.MinY), 0},

        {CFG_Integer, "Button1", ofC4Cfg(Gamepad.Button[0]), -1},
        {CFG_Integer, "Button2", ofC4Cfg(Gamepad.Button[1]), -1},
        {CFG_Integer, "Button3", ofC4Cfg(Gamepad.Button[2]), -1},
        {CFG_Integer, "Button4", ofC4Cfg(Gamepad.Button[3]), -1},
        {CFG_Integer, "Button5", ofC4Cfg(Gamepad.Button[4]), -1},
        {CFG_Integer, "Button6", ofC4Cfg(Gamepad.Button[5]), -1},
        {CFG_Integer, "Button7", ofC4Cfg(Gamepad.Button[6]), -1},
        {CFG_Integer, "Button8", ofC4Cfg(Gamepad.Button[7]), -1},
        {CFG_Integer, "Button9", ofC4Cfg(Gamepad.Button[8]), -1},
        {CFG_Integer, "Button10", ofC4Cfg(Gamepad.Button[9]), -1},
        {CFG_Integer, "Button11", ofC4Cfg(Gamepad.Button[10]), -1},
        {CFG_Integer, "Button12", ofC4Cfg(Gamepad.Button[11]), -1},

        {CFG_End, 0, 0, 0}

};

C4Config::C4Config() { Default(); }

C4Config::~C4Config() {}

void C4Config::Default() { ZeroMem(this, sizeof(C4Config)); }

BOOL C4Config::Load() {
  if (!CStdConfig::Load(C4ConfigMap, this))
    return FALSE;
  General.DeterminePaths();
  Graphics.DetermineResolution();
  Network.CheckPath();
  General.DefaultName();
  General.DefaultLanguage();
  return TRUE;
}

BOOL C4Config::Save() { return CStdConfig::Save(C4ConfigMap, this); }

void C4ConfigGeneral::DeterminePaths() {
  // Exe path
  if (GetModuleFileName(NULL, ExePath, CFG_MaxString)) {
    char dir[CFG_MaxString];
    if (GetParentPath(ExePath, dir)) {
      SCopy(dir, ExePath);
    }
  }
  // WorkPath (force working directory to exe path)
  BOOL fOkay = SetCurrentDirectory(ExePath);
  printf("DeterminePaths: SetCurrentDirectory(%s): %d\n", ExePath, fOkay);
#ifndef _WIN32
  AppendBackslash(ExePath); // Clonk expects trailing backslash for its own path
                            // joins
#endif Temp path
  GetTempPath(CFG_MaxString, TempPath);
  if (TempPath[0])
    AppendBackslash(TempPath);
  // Log path
  SCopy(ExePath, LogPath);
#ifdef _WIN32
  if (LogPath[0])
    AppendBackslash(LogPath);
#endif
  // Player path
  if (PlayerPath[0])
    AppendBackslash(PlayerPath);
  // Round path
  if (RoundPath[0])
    AppendBackslash(RoundPath);
}

void C4ConfigGraphics::DetermineResolution() {
  switch (Resolution) {
  case 0:
    ResX = 320;
    ResY = 200;
    break;
  case 1:
    ResX = 640;
    ResY = 480;
    break;
  case 2:
    ResX = 800;
    ResY = 600;
    break;
  case 3:
    ResX = 1024;
    ResY = 768;
    break;
  }
}

const char *C4Config::AtExePath(const char *szFilename) {
  SCopy(General.ExePath, AtPathFilename);
  SAppend(szFilename, AtPathFilename);
  return AtPathFilename;
}

const char *C4Config::AtTempPath(const char *szFilename) {
  SCopy(General.TempPath, AtPathFilename);
  SAppend(szFilename, AtPathFilename);
  return AtPathFilename;
}

const char *C4Config::AtLogPath(const char *szFilename) {
  SCopy(General.LogPath, AtPathFilename);
  SAppend(szFilename, AtPathFilename);
  return AtPathFilename;
}

const char *C4Config::AtNetworkPath(const char *szFilename) {
  SCopy(Network.WorkPath, AtPathFilename);
  SAppend(szFilename, AtPathFilename);
  return AtPathFilename;
}

void C4ConfigGeneral::CheckSaveFolders(WORD wSaveGame, WORD wSaveScenario) {
  // Save game folder default
  if (!SaveGameFolder[0])
    SCopy(LoadResStr(wSaveGame), SaveGameFolder);
  // Save scenario folder default
  if (!SaveScenarioFolder[0])
    SCopy(LoadResStr(wSaveScenario), SaveScenarioFolder);
}

BOOL C4Config::Init() {
  Graphics.DetermineResolution();
  return TRUE;
}

void C4ConfigNetwork::CheckPath() {
  // Work path
  if (WorkPath[0])
    AppendBackslash(WorkPath);
}

BOOL C4ConfigNetwork::CreateWorkPath() {
  TruncateBackslash(WorkPath);
  EraseItem(WorkPath);
  if (!CreateDirectory(WorkPath, NULL))
    return FALSE;
  AppendBackslash(WorkPath);
  return TRUE;
}

void C4ConfigNetwork::ClearWorkPath() {
  TruncateBackslash(WorkPath);
  EraseItem(WorkPath);
  AppendBackslash(WorkPath);
}

void C4Config::ResetControls() { LoadDefault(C4ConfigMap, this, "Controls"); }

const char *C4Config::AtExeRelativePath(const char *szFilename) {
  // Specified file is located in ExePath: return relative path
  if (SEqualNoCase(szFilename, General.ExePath, SLen(General.ExePath)))
    return szFilename + SLen(General.ExePath);
  // Else keep full path
  return szFilename;
}

void C4ConfigGeneral::DefaultName() {
  // Use old user name for new default
  if (!Name[0])
    if (UserNameOld[0])
      SCopy(UserNameOld, Name);
}

void C4ConfigGeneral::DefaultLanguage() {
  if (!Language[0]) {
    if (PRIMARYLANGID(GetUserDefaultLangID()) == LANG_GERMAN)
      SCopy("DE", Language);
    else
      SCopy("US", Language);
  }
}

BOOL C4Config::IsRegistered(const char *szName, const char *szCode, const char *szSecKey, BOOL fOld) {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}

BOOL C4Config::Registered() {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}

BOOL C4Config::TestRegistered(const char *szName, const char *szCode) {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}

BOOL C4Config::TestRegisteredOld(const char *szName, const char *szCode) {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}

BOOL C4Config::ValidRegistrationName(const char *szName) {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}

BOOL C4Config::IsFreeFolder(const char *szFoldername, const char *szMaker) {
  // Dummy function: registration is checked in shareware compile only
  return TRUE;
}
