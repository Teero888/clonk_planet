/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles the sound bank and plays effects using DSoundX */

#include <C4Include.h>
#include <DSoundX.h>

C4SoundEffect::C4SoundEffect() { Default(); }

C4SoundEffect::~C4SoundEffect() { Clear(); }

void C4SoundEffect::Clear() {
  SetVolume(0);
  if (DSObj)
    DSndObjStop(DSObj);
  if (DSObj)
    DSndObjDestroy(DSObj);
  DSObj = NULL;
  if (WaveBuffer)
    delete[] WaveBuffer;
  WaveBuffer = NULL;
}

BOOL C4SoundEffect::Load(const char *szFileName, C4Group &hGroup, BOOL fStatic) {
  // Sound check
  if (!Config.Sound.RXSound)
    return FALSE;
  // Locate sound in file
  if (!hGroup.AccessEntry(szFileName, &WaveSize))
    return FALSE;
  // Allocate wave buffer, load sound, create sound object
  if (!(WaveBuffer = new BYTE[WaveSize]) || !hGroup.Read(WaveBuffer, WaveSize) || !(DSObj = DSndObjCreate(WaveBuffer, WaveSize))) {
    Clear();
    return FALSE;
  }
  // Set name
  SCopy(szFileName, Name, C4MaxSoundName);
  // Set usage time
  UsageTime = Game.Time;
  Static = fStatic;
  return TRUE;
}

void C4SoundEffect::Play(int iMode, int iVolume) {
  UsageTime = Game.Time;
  // Sound check
  if (!Config.Sound.RXSound)
    return;
  BOOL fPlaying;
  // Play sound
  switch (iMode) {
    // Single sound effect
  case 0:
    if (iVolume > 0)
      if (LoopLevel == 0) {
        SetVolume(iVolume);
        DSndObjPlay(DSObj, 0);
      }
    break;
    // Increase loop/volume Level
  case +1:
    if (!Config.Sound.RXSoundLoops)
      break;
    fPlaying = (LoopLevel && VolumeLevel);
    VolumeLevel += iVolume;
    LoopLevel++;
    if (LoopLevel && VolumeLevel) {
      SetVolume(VolumeLevel);
      if (!fPlaying)
        DSndObjPlay(DSObj, DSBPLAY_LOOPING);
    }
    break;
    // Decrease loop/volume Level
  case -1:
    if (!Config.Sound.RXSoundLoops)
      break;
    fPlaying = (LoopLevel && VolumeLevel);
    VolumeLevel = Max(VolumeLevel - iVolume, 0);
    if (LoopLevel > 0)
      LoopLevel--;
    if (LoopLevel && VolumeLevel) {
      SetVolume(VolumeLevel);
      if (!fPlaying)
        DSndObjPlay(DSObj, DSBPLAY_LOOPING);
    } else {
      if (fPlaying)
        DSndObjStop(DSObj);
    }
    break;
  // Adjust looping volume level
  case 2:
    if (!Config.Sound.RXSoundLoops)
      break;
    fPlaying = (LoopLevel && VolumeLevel);
    VolumeLevel = Max(VolumeLevel + iVolume, 0);
    if (LoopLevel && VolumeLevel) {
      SetVolume(VolumeLevel);
      if (!fPlaying)
        DSndObjPlay(DSObj, DSBPLAY_LOOPING);
    } else {
      if (fPlaying)
        DSndObjStop(DSObj);
    }
    break;
  }
}

BOOL C4SoundEffect::Stop() {
  // Sound check
  if (!Config.Sound.RXSound)
    return FALSE;
  // Stop sound
  LoopLevel = 0;
  return DSndObjStop(DSObj);
}

BOOL C4SoundEffect::Playing() { return DSndObjPlaying(DSObj); }

BOOL C4SoundEffect::SetVolume(int iVolume) {
  iVolume = BoundBy(iVolume, 0, 100);
  return DSndObjSetVolume(DSObj, (iVolume - 100) * 25); // 0 DB / -25 DB
}

BOOL DSndObjGetVolume(CSoundObject *pSO, long *lpVolume);

int C4SoundEffect::GetVolume() {
  long lVolume;
  if (DSndObjGetVolume(DSObj, &lVolume))
    return BoundBy(lVolume / 25 + 100, 0, 100);
  return 0;
}

C4SoundSystem::C4SoundSystem() { Default(); }

C4SoundSystem::~C4SoundSystem() {}

BOOL C4SoundSystem::Init() {
  // Open sound file
  if (!SoundFile.Open(Config.AtExePath(C4CFN_Sound)))
    return FALSE;
  return TRUE;
}

void C4SoundSystem::Default() { FirstSound = NULL; }

void C4SoundSystem::Clear() {
  // Clear sound bank
  C4SoundEffect *csfx, *next;
  for (csfx = FirstSound; csfx; csfx = next) {
    next = csfx->Next;
    delete csfx;
  }
  FirstSound = NULL;
  // Close sound file
  SoundFile.Close();
}

void C4SoundSystem::Execute() {
  // Sound effect statistics & unload check
  C4SoundEffect *csfx, *next = NULL, *prev = NULL;
  for (csfx = FirstSound; csfx; csfx = next) {
    next = csfx->Next;
    // Unload check
    if ((Game.Time - csfx->UsageTime > SoundUnloadTime - SoundUnloadTime * csfx->WaveSize / SoundMaxUnloadSize) && !csfx->Playing() && !csfx->Static) {
      if (prev)
        prev->Next = next;
      else
        FirstSound = next;
      delete csfx;
    } else
      prev = csfx;
  }
}

int C4SoundSystem::EffectInBank(const char *szSound) {
  int iResult = 0;
  C4SoundEffect *pSfx;
  char szName[C4MaxSoundName + 4 + 1];
  // Compose name (with extension)
  SCopy(szSound, szName, C4MaxSoundName);
  DefaultExtension(szName, "wav");
  // Count all matching sounds in bank
  for (pSfx = FirstSound; pSfx; pSfx = pSfx->Next)
    if (WildcardMatch(szName, pSfx->Name))
      iResult++;
  return iResult;
}

C4SoundEffect *C4SoundSystem::AddEffect(const char *szSoundName) {
  C4SoundEffect *nsfx;
  // Allocate new bank entry
  if (!(nsfx = new C4SoundEffect))
    return NULL;
  // Load sound to entry
  if (!nsfx->Load(szSoundName, SoundFile, FALSE))
    if (!nsfx->Load(szSoundName, Game.ScenarioFile, FALSE)) {
      delete nsfx;
      return NULL;
    }
  // Add sound to bank
  nsfx->Next = FirstSound;
  FirstSound = nsfx;
  return nsfx;
}

C4SoundEffect *C4SoundSystem::GetEffect(const char *szSndName) {
  C4SoundEffect *pSfx;
  char szName[C4MaxSoundName + 4 + 1];
  int iNumber;
  // Evaluate sound name
  SCopy(szSndName, szName, C4MaxSoundName);
  // Default extension
  DefaultExtension(szName, "wav");
  // Evaluate random sound
  if (SCharCount('*', szName)) {
    // Search global sound file
    if (!(iNumber = SoundFile.EntryCount(szName)))
      // Search scenario local files
      if (!(iNumber = Game.ScenarioFile.EntryCount(szName)))
        // Search bank loaded sounds
        if (!(iNumber = EffectInBank(szName)))
          // None found: failure
          return NULL;
    // Insert index to name
    iNumber = BoundBy(1 + SafeRandom(iNumber), 1, 9);
    SReplaceChar(szName, '*', '0' + iNumber);
  }
  // Find requested sound effect in bank
  for (pSfx = FirstSound; pSfx; pSfx = pSfx->Next)
    if (SEqualNoCase(szName, pSfx->Name))
      break;
  // Sound not in bank, try add
  if (!pSfx)
    if (!(pSfx = AddEffect(szName)))
      return NULL;
  return pSfx;
}

BOOL C4SoundSystem::PlayEffect(const char *szSndName, int iMode, int iVolume) {
  // Sound not active
  if (!Config.Sound.RXSound)
    return FALSE;
  // Get sound
  C4SoundEffect *csfx;
  if (!(csfx = GetEffect(szSndName)))
    return FALSE;
  // Play
  csfx->Play(iMode, iVolume);
  // Done
  return TRUE;
}

BOOL C4SoundSystem::SoundLevel(const char *szSndName, int iVolume) {
  C4SoundEffect *csfx;
  if (!Config.Sound.RXSound)
    return FALSE;
  iVolume = BoundBy(iVolume, 0, 100);
  // Stop sound if loaded and playing
  if (iVolume == 0) {
    if (EffectInBank(szSndName)) {
      if (!(csfx = GetEffect(szSndName)))
        return FALSE;
      csfx->Stop();
    }
    return TRUE;
  }
  // Set sound volume, loop sound if not playing.
  if (!(csfx = GetEffect(szSndName)))
    return FALSE;
  if (!csfx->Playing())
    csfx->Play(+1, iVolume);
  return TRUE;
}

void C4SoundEffect::Default() {
  Name[0] = 0;
  UsageTime = 0;
  WaveSize = 0;
  WaveBuffer = NULL;
  DSObj = NULL;
  LoopLevel = 0;
  VolumeLevel = 0;
  Static = FALSE;
  Next = NULL;
}

int C4SoundSystem::LoadEffects(C4Group &hGroup, BOOL fStatic) {
  int iNum = 0;
  char szFilename[_MAX_FNAME + 1];
  C4SoundEffect *nsfx;
  hGroup.ResetSearch();
  // Search all sound files in group
  while (hGroup.FindNextEntry(C4CFN_SoundFiles, szFilename))
    // Create and load effect
    if (nsfx = new C4SoundEffect)
      if (nsfx->Load(szFilename, hGroup, fStatic)) {
        // Overload same name effects
        RemoveEffect(szFilename);
        // Add effect
        nsfx->Next = FirstSound;
        FirstSound = nsfx;
        iNum++;
      } else
        delete nsfx;
  return iNum;
}

int C4SoundSystem::RemoveEffect(const char *szFilename) {
  int iResult = 0;
  C4SoundEffect *pNext, *pPrev = NULL;
  for (C4SoundEffect *pSfx = FirstSound; pSfx; pSfx = pNext) {
    pNext = pSfx->Next;
    if (WildcardMatch(szFilename, pSfx->Name)) {
      delete pSfx;
      if (pPrev)
        pPrev->Next = pNext;
      else
        FirstSound = pNext;
      iResult++;
    } else
      pPrev = pSfx;
  }
  return iResult;
}
