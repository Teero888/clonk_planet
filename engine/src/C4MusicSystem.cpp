/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles Music.c4g and randomly plays songs */

#include <C4Include.h>
#include <Midi.h>

C4MusicSystem::C4MusicSystem() { Default(); }

C4MusicSystem::~C4MusicSystem() {}

BOOL C4MusicSystem::Init() {
  // Local music file
  if (Game.ScenarioFile.FindEntry(C4CFN_Music)) {
    if (!MusicFile.OpenAsChild(&Game.ScenarioFile, C4CFN_Music))
      return FALSE;
    sprintf(OSTR, LoadResStr(IDS_PRC_LOCALMUSIC), GetFilename(Game.ScenarioFile.GetName()), GetFilename(MusicFile.GetName()));
    Log(OSTR);
    pMusicFile = &MusicFile;
    return TRUE;
  }
  // Local midi files
  if (Game.ScenarioFile.FindEntry(C4CFN_MusicFiles)) {
    pMusicFile = &Game.ScenarioFile;
    return TRUE;
  }
  // Global music file
  if (MusicFile.Open(Config.AtExePath(C4CFN_Music))) {
    pMusicFile = &MusicFile;
    return TRUE;
  }
  // No music
  return FALSE;
}

void C4MusicSystem::Default() {
  pMusicFile = NULL;
  fPlaying = FALSE;
}

void C4MusicSystem::Clear() {
  StopMidi();
  MusicFile.Close();
  pMusicFile = NULL;
  EraseFile(Config.AtTempPath(C4CFN_TempMusic2));
}

void C4MusicSystem::Execute() {
  if (!Tick35)
    if (Config.Sound.RXMusic)
      if (!fPlaying)
        Play();
}

BOOL C4MusicSystem::Play(const char *szSongname) {
  char songname[_MAX_FNAME + 1];

  // No music file: init
  if (!pMusicFile)
    if (!Init())
      return FALSE;

  // Specific song
  if (szSongname && szSongname[0]) {
    SCopy(szSongname, songname);
    DefaultExtension(songname, "mid");
  }

  // Random song
  else {
    int song_num = pMusicFile->EntryCount(C4CFN_MusicFiles);
    if (!song_num)
      return FALSE;
    pMusicFile->ResetSearch();
    for (int cnt = SafeRandom(song_num); cnt >= 0; cnt--)
      if (!pMusicFile->FindNextEntry(C4CFN_MusicFiles, songname))
        return FALSE;
  }

  // Extract to temp file
  if (!pMusicFile->ExtractEntry(songname, Config.AtTempPath(C4CFN_TempMusic2)))
    return FALSE;

  // Play song
  if (PlayMidi(Config.AtTempPath(C4CFN_TempMusic2), Application.hWindow)) {
    fPlaying = TRUE;
    return TRUE;
  }

  return FALSE;
}

void C4MusicSystem::NotifySuccess() { fPlaying = FALSE; }

BOOL C4MusicSystem::Stop() {
  StopMidi();
  return TRUE;
}
