/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles landscape and sky */

const BYTE GBM = 128,
           GBM_ColNum = 64,
           IFT = GBM_ColNum;

const BYTE CSkyDef1=104,CSkyDef2=123;

const int C4MaxMaterial = GBM_ColNum / C4M_ColsPerMat;

const int C4LSC_Dynamic = 0,
					C4LSC_Static = 1,
					C4LSC_Exact = 2;

class C4Landscape
  {
  public: 
    C4Landscape();
		~C4Landscape();
  public:
		int Mode;
		int Width,Height;
	  SURFACE Surface;
		int MapWidth,MapHeight,MapZoom;
		SURFACE Map;
    DWORD MatCount[C4MaxMaterial];
		BOOL MatCountValid;
		int BlastMatCount[C4MaxMaterial];
		C4Sky Sky;
  public:
		BYTE *QBAPointer;	int QBAPitch;
    int ScanX,ScanSpeed;
    int ScanMatCount[C4MaxMaterial];
		int LeftOpen,RightOpen,TopOpen,BottomOpen;
		FIXED Gravity;
	public:
		void Default();
	  void Clear();
    void Execute();
		void Synchronize();
		void Draw(C4FacetEx &cgo);
    void ScenarioInit();
		void ClearRect(int iTx, int iTy, int iWdt, int iHgt);
		void ClearMatCount();
		void ClearBlastMatCount();
    void ClearScanMatCount();
    void ScanSideOpen();
		void UnLockQBA();
		void CheckInstabilityRange(int tx, int ty);
		void ShakeFree(int tx, int ty, int rad);
		void DigFree(int tx, int ty, int rad, BOOL fRequest=FALSE, C4Object *pByObj=NULL);
		void DigFreeRect(int tx, int ty, int wdt, int hgt, BOOL fRequest=FALSE, C4Object *pByObj=NULL);
		void DigFreeMat(int tx, int ty, int wdt, int hgt, int mat);
		void BlastFree(int tx, int ty, int rad, int grade);
		void DrawMaterialRect(int mat, int tx, int ty, int wdt, int hgt);
		void RaiseTerrain(int tx, int ty, int wdt);
		void FindMatTop(int mat, int &x, int &y);
		BYTE GetMapIndex(int iX, int iY);
		BOOL Load(C4Group &hGroup);
		BOOL Save(C4Group &hGroup);
		BOOL SaveMap(C4Group &hGroup);
		BOOL Init();
		BOOL MapToLandscape();
		BOOL LockQBA();
		BOOL IsQBALocked();
		BOOL SetMode(int iMode);
		BOOL SetPix(int x, int y, BYTE npix) ;
		BOOL CheckInstability(int tx, int ty);
		BOOL ClearPix(int tx, int ty);
		BOOL InsertMaterial(int mat, int tx, int ty);
		BOOL FindMatPath(int &fx, int &fy, int ydir, int mdens, int mslide);
		BOOL FindMatSlide(int &fx, int &fy, int ydir, int mdens, int mslide);
		BOOL Incinerate(int x, int y);
		BOOL DrawBrush(int iX, int iY, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT);
		BOOL DrawLine(int iX1, int iY1, int iX2, int iY2, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT);
		BOOL DrawBox(int iX1, int iY1, int iX2, int iY2, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT);
		BYTE GetPix(int x, int y);
		int DigFreePix(int tx, int ty);
		int ShakeFreePix(int tx, int ty);
		int BlastFreePix(int tx, int ty, int grade);
		int AreaSolidCount(int x, int y, int wdt, int hgt);
		int ExtractMaterial(int fx, int fy);
		int GetMaterialCount(int iMaterial);
  protected:
	  void ClearColorPattern();
    void ExecuteScan();
		void DrawChunk(SURFACE sfcTarget, int tx, int ty, int wdt, int hgt, int mcol, BYTE top_rough, BYTE side_rough);
		void DrawSmoothOChunk(SURFACE sfcTarget, int tx, int ty, int wdt, int hgt, int mcol, BYTE flip);
		void ChunkOZoom(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, int iTexture, int iMaterial);
		BOOL SetColorPattern(const char *szMaterial, const char *szTexture, BOOL fIFT, BYTE &rbyCol);
		BOOL GetTexUsage(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, DWORD *dwpTextureUsage);
		BOOL TexOZoom(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, DWORD *dwpTextureUsage, C4TextureMap &rTextureMap);
		BOOL MapToLandscape(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, C4TextureMap &rTextureMap, BOOL fStaticChunks=FALSE);
	  BOOL GetMapColorIndex(const char *szMaterial, const char *szTexture, BOOL fIFT, BYTE &rbyCol);
	  BOOL SkyToLandscape(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape);
		SURFACE CreateMap(C4TextureMap &rTextureMap);
  };

/* Some global landscape functions */

BOOL AboveSolid(int &rx, int &ry);
BOOL AboveSemiSolid(int &rx, int &ry);
BOOL SemiAboveSolid(int &rx, int &ry);
BOOL FindSolidGround(int &rx, int &ry, int width);
BOOL FindLiquid(int &rx, int &ry, int width, int height);
BOOL FindSurfaceLiquid(int &rx, int &ry, int width, int height);
BOOL FindLevelGround(int &rx, int &ry, int width, int hrange);
BOOL FindConSiteSpot(int &rx, int &ry, int wdt, int hgt, DWORD category, int hrange=-1);
BOOL FindThrowingPosition(int iTx, int iTy, FIXED fXDir, FIXED fYDir, int iHeight, int &rX, int &rY);
BOOL PathFree(int x1, int y1, int x2, int y2, int *ix=NULL, int *iy=NULL);
BOOL FindClosestFree(int &rX, int &rY, int iAngle1, int iAngle2, int iExcludeAngle1, int iExcludeAngle2);
BOOL ConstructionCheck(C4ID id, int iX, int iY, C4Object *pByObj=NULL);


