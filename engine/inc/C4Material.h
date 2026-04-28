/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Material definitions used by the landscape */

class C4MaterialCore
  {
	public:
		C4MaterialCore();
  public:
    int  Ver1,Ver2;
    char Name[C4M_MaxName+1];
    int  Color[C4M_ColsPerMat*3];
    int  MapChunkType;
    int  Density;
    int  Friction;
    int  DigFree;
    int  BlastFree;
    C4ID Dig2Object;
    int  Dig2ObjectRatio;
    int  Dig2ObjectOnRequestOnly;
    C4ID Blast2Object;
    int  Blast2ObjectRatio;
    int  Blast2PXSRatio;
    int  Instable;
    int  MaxAirSpeed;
    int  MaxSlide;
    int  WindDrift;
    int  Inflammable;
    int  Incindiary;
    int  Extinguisher;
		int  Corrosive;
		int  Corrode;
    int  Soil;
    char szBlastShiftTo[C4M_MaxName+1];
    char szInMatConvert[C4M_MaxName+1];
    char szInMatConvertTo[C4M_MaxName+1];
    int  BelowTempConvert;
    char szBelowTempConvertTo[C4M_MaxName+1];
    int  AboveTempConvert;
    char szAboveTempConvertTo[C4M_MaxName+1];
		int  ColorAnimation;
		int  TempConvStrength;
		int fbuf[98];
	public:
		void Clear();
		void Default();
		BOOL Load(C4Group &hGroup, const char *szEntryName);
	protected:
		BOOL Compile(const char *szSource);
		BOOL Decompile(char **ppOutput, int *ipSize);
  };

class C4Material: public C4MaterialCore
  {
  public:
    C4Material();
  public:
    int BlastShiftTo;    // Cross-mapped material values
    int InMatConvert;    // mapped by C4MaterialMap::Load()
    int InMatConvertTo;
    int BelowTempConvertTo;
    int AboveTempConvertTo;
  };

class C4MaterialMap
  {
  public:
    C4MaterialMap();
    ~C4MaterialMap();
  public:
    int Num;
    C4Material *Map;
  public:
	  void Default();
    void Clear();
    int Load(C4Group &hGroup);
    int Get(const char *szMaterial);
    BOOL SaveEnumeration(C4Group &hGroup);
	  BOOL LoadEnumeration(C4Group &hGroup);
  protected:
	  BOOL SortEnumeration(int iMat, const char *szMatName);
    void CrossMapMaterials();
  };

const int C4M_Flat    = 0,
          C4M_TopFlat = 1,
          C4M_Smooth  = 2,
          C4M_Rough   = 3,

          // Material Density Levels

          C4M_Solid     = 50,
          C4M_SemiSolid = 25,
          C4M_Liquid    = 25;
          
const int MNone = -1;

