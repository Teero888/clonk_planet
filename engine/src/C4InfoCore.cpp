/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Structures for object and player info components */

#include <C4Include.h>

//------------------------------ Name File Handling -------------------------------------

char GetANameBuffer[C4MaxName + 1];

const char *GetAName(const char *szNameFile) {
  FILE *hNamefile;
  int iCnt, iName, iLoops;
  if (!szNameFile)
    return "Clonk";
  if (!(hNamefile = fopen(szNameFile, "r")))
    return "Clonk";

#ifdef C4ENGINE
  iName = SafeRandom(1000);
#else
  iName = Random(1000);
#endif

  for (iCnt = 0; iCnt < iName; iCnt++)
    AdvanceFileLine(hNamefile);
  GetANameBuffer[0] = 0;
  iLoops = 0;
  do {
    if (!ReadFileLine(hNamefile, GetANameBuffer, C4MaxName)) {
      rewind(hNamefile);
      iLoops++;
    }
  } while ((iLoops < 2) && (!GetANameBuffer[0] || (GetANameBuffer[0] == '#') || (GetANameBuffer[0] == ' ')));
  fclose(hNamefile);
  if (iLoops >= 2)
    return "Clonk";
  return GetANameBuffer;
}

//------------------------------- Player Info ----------------------------------------

#define offsC4P(x) (int)offsetof(C4PlayerInfoCore, x)

C4CompilerValue C4CR_PlayerInfoCore[] = {

    {"Player", C4CV_Section, 0, 0},
    {"Name", C4CV_String, offsC4P(Name), C4MaxName},
    {"Comment", C4CV_String, offsC4P(Comment), C4MaxComment},
    {"Rank", C4CV_Integer, offsC4P(Rank), 1},
    {"RankName", C4CV_String, offsC4P(RankName), C4MaxName},
    {"Score", C4CV_Integer, offsC4P(Score), 1},
    {"Rounds", C4CV_Integer, offsC4P(Rounds), 1},
    {"RoundsWon", C4CV_Integer, offsC4P(RoundsWon), 1},
    {"RoundsLost", C4CV_Integer, offsC4P(RoundsLost), 1},
    {"TotalPlayingTime", C4CV_Integer, offsC4P(TotalPlayingTime), 1},

    {"Preferences", C4CV_Section, 0, 0},
    {"Color", C4CV_Integer, offsC4P(PrefColor), 1},
    {"Control", C4CV_Integer, offsC4P(PrefControl), 1},
    {"Position", C4CV_Integer, offsC4P(PrefPosition), 1},
    {"Mouse", C4CV_Integer, offsC4P(PrefMouse), 1},

    {"LastRound", C4CV_Section, 0, 0},
    {"Title", C4CV_String, offsC4P(LastRound.Title), C4MaxTitle},
    {"Date", C4CV_Integer, offsC4P(LastRound.Date), 1},
    {"Duration", C4CV_Integer, offsC4P(LastRound.Duration), 1},
    {"Won", C4CV_Integer, offsC4P(LastRound.Won), 1},
    {"Score", C4CV_Integer, offsC4P(LastRound.Score), 1},
    {"FinalScore", C4CV_Integer, offsC4P(LastRound.FinalScore), 1},
    {"TotalScore", C4CV_Integer, offsC4P(LastRound.TotalScore), 1},
    {"Bonus", C4CV_Integer, offsC4P(LastRound.Bonus), 1},
    {"Level", C4CV_Integer, offsC4P(LastRound.Level), 1},

    {NULL, C4CV_End, 0, 0}};

C4PlayerInfoCore::C4PlayerInfoCore() {
  ZeroMem(this, offsetof(C4PlayerInfoCore, PrefPosition) + sizeof(PrefPosition));
  Ver1 = C4PVer1;
  Ver2 = C4PVer2;
  Default();
}

void C4PlayerInfoCore::Default(C4RankSystem *pRanks) {
  ZeroMem(this, offsetof(C4PlayerInfoCore, PrefPosition) + sizeof(PrefPosition));
  Ver1 = C4PVer1;
  Ver2 = C4PVer2;
  Rank = 0;
  SCopy("Neuling", Name);
  if (pRanks)
    SCopy(pRanks->Name(Rank), RankName);
  else
    SCopy("Rang", RankName);
  PrefColor = 0;
  PrefControl = C4P_Control_Keyboard2;
  PrefPosition = 0;
  PrefMouse = 1;
}

BOOL C4PlayerInfoCore::Load(C4Group &hGroup) {
  // New version
  BYTE *pSource;
  if (hGroup.LoadEntry(C4CFN_PlayerInfoCore, &pSource, NULL, 1)) {
    printf("C4PlayerInfoCore::Load: Using new version (Player.txt)\n");
    if (!Compile((char *)pSource)) {
      delete[] pSource;
      return FALSE;
    }
    delete[] pSource;
    return TRUE;
  }
  // Old version
  int size;
  if (!hGroup.AccessEntry("C4Player.c4b", &size))
    return FALSE;
  printf("C4PlayerInfoCore::Load: Using old version (C4Player.c4b), size=%d, sizeof(this)=%zu\n", size, sizeof(C4PlayerInfoCore));
  // Read into temporary buffer to avoid overwriting tail padding/Filename
  BYTE *pBuf = new BYTE[size];
  if (!hGroup.Read(pBuf, size)) {
    delete[] pBuf;
    return FALSE;
  }
  // If sizes match, we can try to copy, but it's still dangerous due to alignment
  if (size == sizeof(C4PlayerInfoCore)) {
     // Still only copy the part that doesn't include padding?
     // For now, let's just copy the whole thing but ONLY up to the last member.
     memcpy(this, pBuf, offsetof(C4PlayerInfoCore, PrefPosition) + sizeof(PrefPosition));
  } else {
     printf("C4PlayerInfoCore::Load: Size mismatch in old version!\n");
     delete[] pBuf;
     return FALSE;
  }
  delete[] pBuf;
  if (10 * Ver1 + Ver2 != 10 * C4PVer1 + C4PVer2)
    return FALSE;
  return TRUE;
}

BOOL C4PlayerInfoCore::Save(C4Group &hGroup) {
  char *Buffer;
  int BufferSize;
  if (!Decompile(&Buffer, &BufferSize))
    return FALSE;
  if (!hGroup.Add(C4CFN_PlayerInfoCore, Buffer, BufferSize, FALSE, TRUE)) {
    delete[] Buffer;
    return FALSE;
  }
  hGroup.Delete("C4Player.c4b");
  return TRUE;
}

void C4PlayerInfoCore::Promote(int iRank, C4RankSystem &rRanks) {
  Rank = iRank;
  SCopy(rRanks.Name(Rank), RankName, C4MaxName);
}

BOOL C4PlayerInfoCore::CheckPromotion(C4RankSystem &rRanks) {
  if (Score >= rRanks.Experience(Rank + 1)) {
    Promote(Rank + 1, rRanks);
    return TRUE;
  }
  return FALSE;
}

BOOL C4PlayerInfoCore::Compile(const char *szSource) {
  C4Compiler Compiler;
  Default();
  return Compiler.CompileStructure(C4CR_PlayerInfoCore, szSource, this);
}

BOOL C4PlayerInfoCore::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;
  C4PlayerInfoCore dC4P;
  *ppOutput = NULL;
  return Compiler.DecompileStructure(C4CR_PlayerInfoCore, this, &dC4P, ppOutput, ipSize);
}

//------------------------------- Physical Info ----------------------------------------

void C4PhysicalInfo::PromotionUpdate(int iRank) {
  if (iRank >= 0) {
    CanDig = 1;
    CanChop = 1;
    CanConstruct = 1;
  }
  if (iRank >= 4) {
    CanScale = 1;
  }
  if (iRank >= 6) {
    CanHangle = 1;
  }
  Energy = Max(Energy, (50 + 5 * BoundBy(iRank, 0, 10)) * C4MaxPhysical / 100);
}

C4PhysicalInfo::C4PhysicalInfo() { Default(); }

void C4PhysicalInfo::Default() { ZeroMem(this, sizeof(C4PhysicalInfo)); }

//------------------------------- Object Info ----------------------------------------

#define offsC4I(x) (int)offsetof(C4ObjectInfoCore, x)

C4CompilerValue C4CR_ObjectInfoCore[] = {

    {"ObjectInfo", C4CV_Section, 0, 0},
    {"id", C4CV_Id, offsC4I(id), 1},
    {"Name", C4CV_String, offsC4I(Name), C4MaxName},
    {"Rank", C4CV_Integer, offsC4I(Rank), 1},
    {"RankName", C4CV_String, offsC4I(RankName), C4MaxName},
    {"TypeName", C4CV_String, offsC4I(TypeName), C4MaxName},
    {"Participation", C4CV_Integer, offsC4I(Participation), 1},
    {"Experience", C4CV_Integer, offsC4I(Experience), 1},
    {"Rounds", C4CV_Integer, offsC4I(Rounds), 1},
    {"DeathCount", C4CV_Integer, offsC4I(DeathCount), 1},
    {"Birthday", C4CV_Integer, offsC4I(Birthday), 1},
    {"TotalPlayingTime", C4CV_Integer, offsC4I(TotalPlayingTime), 1},

    {"Physical", C4CV_Section, 0, 0},
    {"Energy", C4CV_Integer, offsC4I(Physical.Energy), 1},
    {"Breath", C4CV_Integer, offsC4I(Physical.Breath), 1},
    {"Walk", C4CV_Integer, offsC4I(Physical.Walk), 1},
    {"Jump", C4CV_Integer, offsC4I(Physical.Jump), 1},
    {"Scale", C4CV_Integer, offsC4I(Physical.Scale), 1},
    {"Hangle", C4CV_Integer, offsC4I(Physical.Hangle), 1},
    {"Dig", C4CV_Integer, offsC4I(Physical.Dig), 1},
    {"Swim", C4CV_Integer, offsC4I(Physical.Swim), 1},
    {"Throw", C4CV_Integer, offsC4I(Physical.Throw), 1},
    {"Push", C4CV_Integer, offsC4I(Physical.Push), 1},
    {"Fight", C4CV_Integer, offsC4I(Physical.Fight), 1},
    {"Magic", C4CV_Integer, offsC4I(Physical.Magic), 1},
    {"Float", C4CV_Integer, offsC4I(Physical.Float), 1},
    {"CanScale", C4CV_Integer, offsC4I(Physical.CanScale), 1},
    {"CanHangle", C4CV_Integer, offsC4I(Physical.CanHangle), 1},
    {"CanDig", C4CV_Integer, offsC4I(Physical.CanDig), 1},
    {"CanConstruct", C4CV_Integer, offsC4I(Physical.CanConstruct), 1},
    {"CanChop", C4CV_Integer, offsC4I(Physical.CanChop), 1},
    {"CanSwimDig", C4CV_Integer, offsC4I(Physical.CanSwimDig), 1},
    {"CorrosionResist", C4CV_Integer, offsC4I(Physical.CorrosionResist), 1},
    {"BreatheWater", C4CV_Integer, offsC4I(Physical.BreatheWater), 1},

    {NULL, C4CV_End, 0, 0}};

C4ObjectInfoCore::C4ObjectInfoCore() { Default(); }

void C4ObjectInfoCore::Default(C4ID n_id, C4DefList *pDefs, const char *cpNames) {

  // Def
  C4Def *pDef = NULL;
  if (pDefs)
    pDef = pDefs->ID2Def(n_id);

  // Defaults
  id = n_id;
  Participation = 1;
  Rank = 0;
  Experience = 0;
  Rounds = 0;
  DeathCount = 0;
  Birthday = 0;
  TotalPlayingTime = 0;
  SCopy("Clonk", Name, C4MaxName);
  SCopy("Clonk", TypeName, C4MaxName);
  SCopy("Clonk", RankName, C4MaxName);

  // Type
  if (pDef)
    SCopy(pDef->Name, TypeName, C4MaxName);

  // Name
  if (cpNames) {
    // Name file reference
    if (SSearchNoCase(cpNames, C4CFN_Names))
      SCopy(GetAName(cpNames), Name, C4MaxName);
    // Name list
    else {
      SCopySegment(cpNames, Random(SCharCount(0x0A, cpNames)), Name, 0x0A, C4MaxName + 1);
      SClearFrontBack(Name);
      SReplaceChar(Name, 0x0D, 0x00);
      if (!Name[0])
        SCopy("Clonk", Name, C4MaxName);
    }
  }

  // Physical
  Physical.Default();
  if (pDef)
    Physical = pDef->Physical;
  Physical.PromotionUpdate(Rank);

  // Old format
  Ver1 = C4OVer1;
  Ver2 = C4OVer2;
  ZeroMem(fbuf, 40 * sizeof(int));
}

BOOL C4ObjectInfoCore::LoadNext(C4Group &hGroup) // Old version
{
  int size;
  if (!hGroup.AccessNextEntry("*.c4o", &size))
    return FALSE;
  if (size != sizeof(C4ObjectInfoCore))
    return FALSE;
  if (!hGroup.Read(this, sizeof(C4ObjectInfoCore)))
    return FALSE;
  if (10 * Ver1 + Ver2 != 10 * C4OVer1 + C4OVer2)
    return FALSE;
  return TRUE;
}

void C4ObjectInfoCore::Promote(int iRank, C4RankSystem &rRanks) {
  Rank = iRank;
  Physical.PromotionUpdate(Rank);
  SCopy(rRanks.Name(Rank), RankName, C4MaxName);
}

BOOL C4ObjectInfoCore::Add(C4Group &hGroup) // Old version
{
  char fname[_MAX_FNAME];
  SCopy(Name, fname);
  SAppend(".c4o", fname);
  return hGroup.Add(fname, this, sizeof(C4ObjectInfoCore));
}

BOOL C4ObjectInfoCore::Load(C4Group &hGroup) {
  BYTE *pSource;
  if (hGroup.LoadEntry(C4CFN_ObjectInfoCore, &pSource, NULL, 1)) {
    if (!Compile((char *)pSource)) {
      delete[] pSource;
      return FALSE;
    }
    delete[] pSource;
    return TRUE;
  }
  return FALSE;
}

BOOL C4ObjectInfoCore::Save(C4Group &hGroup) {
  char *Buffer;
  int BufferSize;
  if (!Decompile(&Buffer, &BufferSize))
    return FALSE;
  if (!hGroup.Add(C4CFN_ObjectInfoCore, Buffer, BufferSize, FALSE, TRUE)) {
    delete[] Buffer;
    return FALSE;
  }
  return TRUE;
}

BOOL C4ObjectInfoCore::Compile(const char *szSource) {
  C4Compiler Compiler;
  Default();
  return Compiler.CompileStructure(C4CR_ObjectInfoCore, szSource, this);
}

BOOL C4ObjectInfoCore::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;
  C4ObjectInfoCore dC4O;
  *ppOutput = NULL;
  return Compiler.DecompileStructure(C4CR_ObjectInfoCore, this, &dC4O, ppOutput, ipSize);
}

//------------------------------- Round Info ------------------------------------------

C4RoundResult::C4RoundResult() { Default(); }

void C4RoundResult::Default() { ZeroMem(this, sizeof(C4RoundResult)); }
