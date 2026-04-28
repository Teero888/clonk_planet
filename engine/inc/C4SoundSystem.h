/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles the sound bank and plays effects using DSoundX */

const int C4MaxSoundName = 100;

struct CSoundObject;

class C4SoundEffect {
public:
  C4SoundEffect();
  ~C4SoundEffect();

public:
  char Name[C4MaxSoundName + 1];
  int UsageTime;
  int WaveSize;
  BYTE *WaveBuffer;
  CSoundObject *DSObj;
  int LoopLevel;
  int VolumeLevel;
  BOOL Static;
  C4SoundEffect *Next;

public:
  void Default();
  void Clear();
  void Play(int iMode = 0, int iVolume = 100);
  int GetVolume();
  BOOL Load(const char *szFileName, C4Group &hGroup, BOOL fStatic);
  BOOL Stop();
  BOOL Playing();
  BOOL SetVolume(int iVolume);
};

const int SoundUnloadTime = 60, SoundMaxUnloadSize = 100000;

class C4SoundSystem {
public:
  C4SoundSystem();
  ~C4SoundSystem();

protected:
  C4SoundEffect *FirstSound;
  C4Group SoundFile;

public:
  void Default();
  void Clear();
  void Execute();
  int LoadEffects(C4Group &hGroup, BOOL fStatic = TRUE);
  BOOL SoundLevel(const char *szSound, int iVolume);
  BOOL PlayEffect(const char *szSound, int iMode, int iVolume);
  BOOL Init();

protected:
  C4SoundEffect *GetEffect(const char *szSound);
  C4SoundEffect *AddEffect(const char *szSound);
  int RemoveEffect(const char *szFilename);
  int EffectInBank(const char *szSound);
};
