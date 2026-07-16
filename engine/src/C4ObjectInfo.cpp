/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Holds crew member information */

#include <C4Include.h>

C4ObjectInfo::C4ObjectInfo() { Default(); }

C4ObjectInfo::~C4ObjectInfo() { Clear(); }

void C4ObjectInfo::Default() {
  WasInAction = FALSE;
  InAction = FALSE;
  InActionTime = 0;
  HasDied = FALSE;
  ControlCount = 0;
  Filename[0] = 0;
  Next = NULL;
}

BOOL C4ObjectInfo::Load(C4Group &hMother, const char *szEntryname) {

  // New version
  if (SEqualNoCase(GetExtension(szEntryname), "c4i")) {
    C4Group hChild;
    if (hChild.OpenAsChild(&hMother, szEntryname)) {
      if (!C4ObjectInfo::Load(hChild)) {
        hChild.Close();
        return FALSE;
      }
      hChild.Close();
      return TRUE;
    }
  }

  // Old version
  if (SEqualNoCase(GetExtension(szEntryname), "c4o"))
    if (LoadOld(hMother, szEntryname))
      return TRUE;

  return FALSE;
}

BOOL C4ObjectInfo::Load(C4Group &hGroup) {
  // Store group file name
  SCopy(GetFilename(hGroup.GetName()), Filename, _MAX_FNAME);
  // Load core
  if (!C4ObjectInfoCore::Load(hGroup))
    return FALSE;
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - -
  // Load portrait
  if (Portrait.Load(hGroup, C4CFN_Portrait))
    Portrait.SetPalette(Game.GraphicsResource.GamePalette, TRUE);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - -
  return TRUE;
}

BOOL C4ObjectInfo::Save(C4Group &hGroup) {
  // Set group file name
  if (!Filename[0]) {
    SCopy(Name, Filename);
    SAppend(".c4i", Filename);
  }
  // Set temp group file name
  char szTempGroup[_MAX_PATH + 1];
  SCopy(Filename, szTempGroup);
  MakeTempFilename(szTempGroup);
  // If object info group exists, copy to temp group file
  hGroup.Extract(Filename, szTempGroup);
  // Open temp group
  C4Group hTemp;
  if (!hTemp.Open(szTempGroup, TRUE))
    return FALSE;
  // Save to temp group
  if (!C4ObjectInfoCore::Save(hTemp)) {
    hTemp.Close();
    return FALSE;
  }
  // Close temp group
  hTemp.Sort(C4FLS_Object);
  if (!hTemp.Close())
    return FALSE;
  // Move temp group to mother group
  if (!hGroup.Move(szTempGroup, Filename))
    return FALSE;
  // Success
  return TRUE;
}

BOOL C4ObjectInfo::LoadOld(C4Group &hMother, const char *szEntryname) {
  if (hMother.EntrySize(szEntryname) != sizeof(C4ObjectInfoCore))
    return FALSE;
  if (!hMother.AccessEntry(szEntryname))
    return FALSE;
  if (!hMother.Read(this, sizeof(C4ObjectInfoCore)))
    return FALSE;
  if (10 * Ver1 + Ver2 != 10 * C4OVer1 + C4OVer2)
    return FALSE;
  return TRUE;
}

void C4ObjectInfo::Evaluate() {
  Retire();
  if (WasInAction)
    Rounds++;
  if (HasDied)
    DeathCount++;
}

void C4ObjectInfo::Clear() {
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - -
  Portrait.Clear();
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - -
}

void C4ObjectInfo::Draw(C4Facet &cgo, BOOL fShowPortrait, BOOL fCaptain) {
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - -

  int iX = 0;

  // Portrait
  if (fShowPortrait)
    if (Portrait.Surface) {
      C4Facet fctPortrait;
      fctPortrait.Set(Portrait);
      C4Facet ccgo;
      ccgo.Set(cgo.Surface, cgo.X + iX, cgo.Y, 4 * cgo.Hgt / 3 + 10, cgo.Hgt + 10);
      fctPortrait.Draw(ccgo);
      iX += 4 * cgo.Hgt / 3;
    }

  // Captain symbol
  if (fCaptain) {
    Game.GraphicsResource.fctCaptain.Draw(cgo.Surface, cgo.X + iX, cgo.Y, 0, 0);
    iX += Game.GraphicsResource.fctCaptain.Wdt;
  }

  // Rank symbol
  Game.GraphicsResource.fctRank.Draw(cgo.Surface, cgo.X + iX, cgo.Y, BoundBy(Rank, 0, 23), 0);
  iX += Game.GraphicsResource.fctRank.Wdt;

  // Rank & Name
  if (Rank > 0)
    sprintf(OSTR, "%s|%s", RankName, Name);
  else
    sprintf(OSTR, "%s", Name);
  Engine.DDraw.TextOut(OSTR, cgo.Surface, cgo.X + iX, cgo.Y, FWhite, FBlack, ALeft);

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - -
}

void C4ObjectInfo::Recruit() {
  WasInAction = TRUE;
  InAction = TRUE;
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - -
  InActionTime = Game.Time;
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - -
}

void C4ObjectInfo::Retire() {
  InAction = FALSE;
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                // - - - -
  TotalPlayingTime += (Game.Time - InActionTime);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       // - - - - -
}

void C4ObjectInfo::SetBirthday() { Birthday = time(NULL); }
