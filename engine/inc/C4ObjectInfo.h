/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Holds crew member information */

class C4ObjectInfo : public C4ObjectInfoCore {
public:
  C4ObjectInfo();
  ~C4ObjectInfo();

public:
  BOOL WasInAction;
  BOOL InAction;
  int InActionTime;
  BOOL HasDied;
  int ControlCount;
#ifdef C4ENGINE
  C4Surface Portrait;
#endif
  char Filename[_MAX_PATH + 1];
  C4ObjectInfo *Next;

public:
  void Default();
  void Clear();
  void Evaluate();
  void Retire();
  void Recruit();
  void SetBirthday();
  void Draw(C4Facet &cgo, BOOL fShowPortrait, BOOL fShowCaptain);
  BOOL LoadOld(C4Group &hMother, const char *szEntryname);
  BOOL Save(C4Group &hGroup);
  BOOL Load(C4Group &hGroup);
  BOOL Load(C4Group &hMother, const char *szEntryname);
};
