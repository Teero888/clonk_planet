/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Material definitions used by the landscape */

#include <C4Include.h>

#define offsC4M(x) offsetof(C4MaterialCore, x)

C4CompilerValue C4CR_Material[] = {

    {"Material", C4CV_Section, 0, 0},
    {"Name", C4CV_String, offsC4M(Name), C4M_MaxName},
    {"Color", C4CV_Integer, offsC4M(Color), C4M_ColsPerMat * 3},
    {"ColorAnimation", C4CV_Integer, offsC4M(ColorAnimation), 1},
    {"Shape", C4CV_Integer, offsC4M(MapChunkType), 1},
    {"Density", C4CV_Integer, offsC4M(Density), 1},
    {"Friction", C4CV_Integer, offsC4M(Friction), 1},
    {"DigFree", C4CV_Integer, offsC4M(DigFree), 1},
    {"BlastFree", C4CV_Integer, offsC4M(BlastFree), 1},
    {"Blast2Object", C4CV_Id, offsC4M(Blast2Object), 1},
    {"Dig2Object", C4CV_Id, offsC4M(Dig2Object), 1},
    {"Dig2ObjectRatio", C4CV_Integer, offsC4M(Dig2ObjectRatio), 1},
    {"Dig2ObjectRequest", C4CV_Integer, offsC4M(Dig2ObjectOnRequestOnly), 1},
    {"Blast2ObjectRatio", C4CV_Integer, offsC4M(Blast2ObjectRatio), 1},
    {"Blast2PXSRatio", C4CV_Integer, offsC4M(Blast2PXSRatio), 1},
    {"Instable", C4CV_Integer, offsC4M(Instable), 1},
    {"MaxAirSpeed", C4CV_Integer, offsC4M(MaxAirSpeed), 1},
    {"MaxSlide", C4CV_Integer, offsC4M(MaxSlide), 1},
    {"WindDrift", C4CV_Integer, offsC4M(WindDrift), 1},
    {"Inflammable", C4CV_Integer, offsC4M(Inflammable), 1},
    {"Incindiary", C4CV_Integer, offsC4M(Incindiary), 1},
    {"Corrode", C4CV_Integer, offsC4M(Corrode), 1},
    {"Corrosive", C4CV_Integer, offsC4M(Corrosive), 1},
    {"Extinguisher", C4CV_Integer, offsC4M(Extinguisher), 1},
    {"Soil", C4CV_Integer, offsC4M(Soil), 1},
    {"TempConvStrength", C4CV_Integer, offsC4M(TempConvStrength), 1},
    {"BlastShiftTo", C4CV_String, offsC4M(szBlastShiftTo), C4M_MaxName},
    {"InMatConvert", C4CV_String, offsC4M(szInMatConvert), C4M_MaxName},
    {"InMatConvertTo", C4CV_String, offsC4M(szInMatConvertTo), C4M_MaxName},
    {"AboveTempConvert", C4CV_Integer, offsC4M(AboveTempConvert), 1},
    {"AboveTempConvertTo", C4CV_String, offsC4M(szAboveTempConvertTo), C4M_MaxName},
    {"BelowTempConvert", C4CV_Integer, offsC4M(BelowTempConvert), 1},
    {"BelowTempConvertTo", C4CV_String, offsC4M(szBelowTempConvertTo), C4M_MaxName},

    {NULL, C4CV_End, 0, 0}};

C4MaterialCore::C4MaterialCore() { Clear(); }

void C4MaterialCore::Clear() { ZeroMem(this, sizeof(C4MaterialCore)); }

void C4MaterialCore::Default() { Clear(); }

BOOL C4MaterialCore::Load(C4Group &hGroup, const char *szEntryName) {
  BYTE *pSource;
  if (!hGroup.LoadEntry(szEntryName, &pSource, NULL, 1))
    return FALSE;
  if (!Compile((char *)pSource)) {
    delete[] pSource;
    return FALSE;
  }
  delete[] pSource;
  return TRUE;
}

C4Material::C4Material() {
  BlastShiftTo = MNone;
  InMatConvert = MNone;
  InMatConvertTo = MNone;
  BelowTempConvertTo = MNone;
  AboveTempConvertTo = MNone;
}

C4MaterialMap::C4MaterialMap() { Default(); }

C4MaterialMap::~C4MaterialMap() { Clear(); }

void C4MaterialMap::Clear() {
  if (Map)
    delete[] Map;
  Map = NULL;
}

int C4MaterialMap::Load(C4Group &hGroup) {
  int cnt, mat_num;
  char entryname[256 + 1];

  // Determine number of materials in file
  mat_num = hGroup.EntryCount(C4CFN_MaterialFiles);

  // Delete any old map
  Clear();

  // Allocate new map
  if (!(Map = new C4Material[mat_num]))
    return 0;

  // Load material cores to map
  hGroup.ResetSearch();

  for (cnt = 0; cnt < mat_num; cnt++)
    if (!hGroup.FindNextEntry(C4CFN_MaterialFiles, entryname) || !Map[cnt].Load(hGroup, entryname)) {
      Clear();
      return 0;
    }

  Num = mat_num;

  // Cross map mats
  CrossMapMaterials();

  return mat_num;
}

int C4MaterialMap::Get(const char *szMaterial) {
  int cnt;
  for (cnt = 0; cnt < Num; cnt++)
    if (SEqualNoCase(szMaterial, Map[cnt].Name))
      return cnt;
  return MNone;
}

void C4MaterialMap::CrossMapMaterials() // Called after load
{
  for (int cnt = 0; cnt < Num; cnt++) {
    if (Map[cnt].szBlastShiftTo[0])
      Map[cnt].BlastShiftTo = Get(Map[cnt].szBlastShiftTo);
    if (Map[cnt].szInMatConvert[0])
      Map[cnt].InMatConvert = Get(Map[cnt].szInMatConvert);
    if (Map[cnt].szInMatConvertTo[0])
      Map[cnt].InMatConvertTo = Get(Map[cnt].szInMatConvertTo);
    if (Map[cnt].szBelowTempConvertTo[0])
      Map[cnt].BelowTempConvertTo = Get(Map[cnt].szBelowTempConvertTo);
    if (Map[cnt].szAboveTempConvertTo[0])
      Map[cnt].AboveTempConvertTo = Get(Map[cnt].szAboveTempConvertTo);
  }
}

BOOL C4MaterialMap::SaveEnumeration(C4Group &hGroup) {
  char *mapbuf = new char[1000];
  mapbuf[0] = 0;
  SAppend("[Enumeration]", mapbuf);
  SAppend(LineFeed, mapbuf);
  for (int cnt = 0; cnt < Num; cnt++) {
    SAppend(Map[cnt].Name, mapbuf);
    SAppend(LineFeed, mapbuf);
  }
  SAppend(EndOfFile, mapbuf);
  return hGroup.Add(C4CFN_MatMap, mapbuf, SLen(mapbuf), FALSE, TRUE);
}

BOOL C4MaterialMap::LoadEnumeration(C4Group &hGroup) {
  // Load enumeration map (from savegame), succeed if not present
  char *mapbuf;
  if (!hGroup.LoadEntry(C4CFN_MatMap, (BYTE **)&mapbuf, NULL, 1))
    return TRUE;

  // Sort material array by enumeration map, fail if some missing
  const char *csearch;
  char cmatname[C4M_MaxName + 1];
  int cmat = 0;
  if (!(csearch = SSearch(mapbuf, "[Enumeration]"))) {
    delete[] mapbuf;
    return FALSE;
  }
  csearch = SAdvanceSpace(csearch);
  while (IsIdentifier(*csearch)) {
    SCopyIdentifier(csearch, cmatname, C4M_MaxName);
    if (!SortEnumeration(cmat, cmatname)) {
      // Output error message!
      delete[] mapbuf;
      return FALSE;
    }
    cmat++;
    csearch += SLen(cmatname);
    csearch = SAdvanceSpace(csearch);
  }

  delete[] mapbuf;
  return TRUE;
}

BOOL C4MaterialMap::SortEnumeration(int iMat, const char *szMatName) {

  // Not enough materials loaded
  if (iMat >= Num)
    return FALSE;

  // Find requested mat
  int cmat;
  for (cmat = iMat; cmat < Num; cmat++)
    if (SEqual(szMatName, Map[cmat].Name))
      break;
  // Not found
  if (cmat >= Num)
    return FALSE;

  // Move requested mat to indexed position
  C4Material mswap;
  mswap = Map[iMat];
  Map[iMat] = Map[cmat];
  Map[cmat] = mswap;

  return TRUE;
}

void C4MaterialMap::Default() {
  Num = 0;
  Map = NULL;
}

BOOL C4MaterialCore::Compile(const char *szSource) {
  C4Compiler Compiler;
  Default();
  return Compiler.CompileStructure(C4CR_Material, szSource, this);
}

BOOL C4MaterialCore::Decompile(char **ppOutput, int *ipSize) {
  C4Compiler Compiler;
  C4MaterialCore dC4M;
  *ppOutput = NULL;
  return Compiler.DecompileStructure(C4CR_Material, this, &dC4M, ppOutput, ipSize);
}
