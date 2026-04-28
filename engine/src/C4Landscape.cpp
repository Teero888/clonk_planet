/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Handles landscape and sky */

#include <C4Include.h>

int MVehic = MNone, MTunnel = MNone, MWater = MNone, MSnow = MNone, MEarth = MNone, MGranite = MNone;

C4Landscape::C4Landscape() { Default(); }

C4Landscape::~C4Landscape() { Clear(); }

void C4Landscape::ScenarioInit() {
  // Gravity
  Gravity = ftofix(0.2 * (float)Game.C4S.Landscape.Gravity.Evaluate() / 100.0);
  // Scan settings
  ScanSpeed = BoundBy(Width / 500, 2, 15);
  // Opens
  LeftOpen = Game.C4S.Landscape.LeftOpen;
  RightOpen = Game.C4S.Landscape.RightOpen;
  TopOpen = Game.C4S.Landscape.TopOpen;
  BottomOpen = Game.C4S.Landscape.BottomOpen;
  // Side open scan
  if (Game.C4S.Landscape.AutoScanSideOpen)
    ScanSideOpen();
}

void C4Landscape::Execute() {
  // Landscape scan
  for (int cnt = 0; cnt < ScanSpeed; cnt++)
    ExecuteScan();
  // Side open scan
  /*if (!Tick255)
    if (Game.C4S.LScape.AutoScanSideOpen)
      ScanSideOpen(); */
}

void C4Landscape::ClearScanMatCount() {
  for (int cnt = 0; cnt < C4MaxMaterial; cnt++)
    ScanMatCount[cnt] = 0;
}

void C4Landscape::ExecuteScan() {
  int cy, mat;
  BYTE temp_conv_done[C4MaxMaterial];
  ZeroMem(temp_conv_done, C4MaxMaterial);

  // Scan landscape column
  for (cy = 0; cy < Height; cy++)
    if (MatValid(mat = GBackMat(ScanX, cy))) {
      // ScanMatCount
      ScanMatCount[mat]++;
      // Temperature conversion
      if (temp_conv_done[mat] < Game.Material.Map[mat].TempConvStrength) {
        int conv_to = MNone;
        // Check below conv
        if (Game.Material.Map[mat].BelowTempConvertTo != MNone)
          if (Game.Weather.GetTemperature() < Game.Material.Map[mat].BelowTempConvert)
            conv_to = Game.Material.Map[mat].BelowTempConvertTo;
        // Check above conv
        if (Game.Material.Map[mat].AboveTempConvertTo != MNone)
          if (Game.Weather.GetTemperature() > Game.Material.Map[mat].AboveTempConvert)
            conv_to = Game.Material.Map[mat].AboveTempConvertTo;
        // Conversion
        if (conv_to != MNone) {
          SBackPix(ScanX, cy, Mat2PixColRnd(conv_to) + GBackIFT(ScanX, cy));
          CheckInstabilityRange(ScanX, cy);
          temp_conv_done[mat]++;
        }
      }
    }

  // Scan advance & rewind
  ScanX++;
  if (ScanX == Width) {
    ScanX = 0;
    // Set valid matcount
    for (int cnt = 0; cnt < C4MaxMaterial; cnt++)
      MatCount[cnt] = ScanMatCount[cnt];
    MatCountValid = TRUE;
    // New count
    ClearScanMatCount();
  }
}

void C4Landscape::ScanSideOpen() {
  int cy;
  for (cy = 0; (cy < Height) && !GBackSemiSolid(0, cy); cy++)
    ;
  LeftOpen = cy;
  for (cy = 0; (cy < Height) && !GBackSemiSolid(Width - 1, cy); cy++)
    ;
  RightOpen = cy;
}

void C4Landscape::Clear() {
  Sky.Clear();
  if (Surface)
    DestroySurface(Surface);
  Surface = NULL;
  if (Map)
    DestroySurface(Map);
  Map = NULL;
}

BOOL ChunkyRandomStatic = FALSE;

int ChunkyRandom(int &iOffset, int iRange) {
  if (ChunkyRandomStatic && iRange) {
    iOffset += 3;
    return iOffset % iRange;
  }
  return Random(iRange);
}

void C4Landscape::DrawChunk(SURFACE sfcTarget, int tx, int ty, int wdt, int hgt, int mcol, BYTE top_rough, BYTE side_rough) {
  int vtcs[16];
  int rx = Max(wdt / 2, 1);
  int cro = (tx + ty) / 7;

  vtcs[0] = tx - ChunkyRandom(cro, rx / 2);
  vtcs[1] = ty - ChunkyRandom(cro, rx / 2 * top_rough);
  vtcs[2] = tx - ChunkyRandom(cro, rx * side_rough);
  vtcs[3] = ty + hgt / 2;
  vtcs[4] = tx - ChunkyRandom(cro, rx);
  vtcs[5] = ty + hgt + ChunkyRandom(cro, rx);
  vtcs[6] = tx + wdt / 2;
  vtcs[7] = ty + hgt + ChunkyRandom(cro, 2 * rx);
  vtcs[8] = tx + wdt + ChunkyRandom(cro, rx);
  vtcs[9] = ty + hgt + ChunkyRandom(cro, rx);
  vtcs[10] = tx + wdt + ChunkyRandom(cro, rx * side_rough);
  vtcs[11] = ty + hgt / 2;
  vtcs[12] = tx + wdt + ChunkyRandom(cro, rx / 2);
  vtcs[13] = ty - ChunkyRandom(cro, rx / 2 * top_rough);
  vtcs[14] = tx + wdt / 2;
  vtcs[15] = ty - ChunkyRandom(cro, rx * top_rough);

  Engine.DDraw.DrawPolygon(sfcTarget, 8, vtcs, mcol);
}

void C4Landscape::DrawSmoothOChunk(SURFACE sfcTarget, int tx, int ty, int wdt, int hgt, int mcol, BYTE flip) {
  int vtcs[8];
  int rx = Max(wdt / 2, 1);
  int cro = (tx + ty) / 3;

  vtcs[0] = tx;
  vtcs[1] = ty - ChunkyRandom(cro, rx / 2);
  vtcs[2] = tx;
  vtcs[3] = ty + hgt;
  vtcs[4] = tx + wdt;
  vtcs[5] = ty + hgt;
  vtcs[6] = tx + wdt;
  vtcs[7] = ty - ChunkyRandom(cro, rx / 2);

  if (flip) {
    vtcs[0] = tx + wdt / 2;
    vtcs[1] = ty + hgt / 3;
  } else {
    vtcs[6] = tx + wdt / 2;
    vtcs[7] = ty + hgt / 3;
  }

  Engine.DDraw.DrawPolygon(sfcTarget, 4, vtcs, mcol);
}

void C4Landscape::ChunkOZoom(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, int iTexture, int iMaterial) {
  int iX, iY, iChunkWidth, iChunkHeight, iToX, iToY;
  int iIFT;
  BYTE *bypMapPixel, *bypMapPixelBelow, *bypMap;
  int iMapPitch, iMapWidth, iMapHeight, iLandscapeWidth, iLandscapeHeight;
  int iChunkType = Game.Material.Map[iMaterial].MapChunkType;
  BYTE byColor = Mat2PixCol(iMaterial);
  // Get map & landscape size
  if (!GetSurfaceSize(sfcMap, iMapWidth, iMapHeight) || !GetSurfaceSize(sfcLandscape, iLandscapeWidth, iLandscapeHeight))
    return;
  // Clip desired map segment to map size
  iMapX = BoundBy(iMapX, 0, iMapWidth - 1);
  iMapY = BoundBy(iMapY, 0, iMapHeight - 1);
  iMapWdt = BoundBy(iMapWdt, 0, iMapWidth - iMapX);
  iMapHgt = BoundBy(iMapHgt, 0, iMapHeight - iMapY);
  // Calculate chunk size
  iChunkWidth = Max(iLandscapeWidth / iMapWidth, 1);
  iChunkHeight = Max(iLandscapeHeight / iMapHeight, 1) + 1;
  // Lock map surface
  if (!(bypMap = LockSurface(sfcMap, iMapPitch)))
    return;
  // Scan map lines
  for (iY = iMapY; iY < iMapY + iMapHgt; iY++) {
    // Map scan line start
    bypMapPixel = bypMap + iMapPitch * iY + iMapX;
    // Map scan line pixel below
    if (iY < iMapHeight - 1)
      bypMapPixelBelow = bypMapPixel + iMapPitch;
    else
      bypMapPixelBelow = NULL;
    // Landscape target coordinate vertical
    iToY = iY * iLandscapeHeight / iMapHeight;
    // Scan map line
    for (iX = iMapX; iX < iMapX + iMapWdt; iX++) {
      // Landscape target coordinate horizontal
      iToX = iX * iLandscapeWidth / iMapWidth;
      // Here's a chunk of the texture-material to zoom
      if (((*bypMapPixel) & 127) == iTexture) {
        // Determine IFT
        iIFT = 0;
        if (*bypMapPixel >= 128)
          iIFT = IFT;
        // Draw chunk
        switch (iChunkType) {
        case C4M_Flat:
          Engine.DDraw.DrawBox(sfcLandscape, iToX, iToY, iToX + iChunkWidth, iToY + iChunkHeight, byColor + iIFT);
          break;
        case C4M_TopFlat:
          DrawChunk(sfcLandscape, iToX, iToY, iChunkWidth, iChunkHeight, byColor + iIFT, 0, 1);
          break;
        case C4M_Smooth:
          DrawChunk(sfcLandscape, iToX, iToY, iChunkWidth, iChunkHeight, byColor + iIFT, 1, 1);
          break;
        case C4M_Rough:
          DrawChunk(sfcLandscape, iToX, iToY, iChunkWidth, iChunkHeight, byColor + iIFT, 1, 2);
          break;
        }
      }
      // Other chunk, check for slope smoothers
      else
        // Smooth chunk & same texture-material below
        if ((iChunkType == C4M_Smooth) && bypMapPixelBelow && (((*bypMapPixelBelow) & 127) == iTexture)) {
          // Same texture-material on left
          if ((iX > 0) && (((*(bypMapPixel - 1)) & 127) == iTexture)) {
            // Determine IFT
            iIFT = 0;
            if (*(bypMapPixel - 1) >= 128)
              iIFT = IFT;
            // Draw smoother
            DrawSmoothOChunk(sfcLandscape, iToX, iToY, iChunkWidth, iChunkHeight, byColor + iIFT, 0);
          }
          // Same texture-material on right
          if ((iX < iMapWidth - 1) && (((*(bypMapPixel + 1)) & 127) == iTexture)) {
            // Determine IFT
            iIFT = 0;
            if (*(bypMapPixel + 1) >= 128)
              iIFT = IFT;
            // Draw smoother
            DrawSmoothOChunk(sfcLandscape, iToX, iToY, iChunkWidth, iChunkHeight, byColor + iIFT, 1);
          }
        }
      // Advance to next scan line pixel
      bypMapPixel++;
      if (bypMapPixelBelow)
        bypMapPixelBelow++;
    }
  }

  // Unlock map surface
  UnLockSurface(sfcMap);
}

BOOL C4Landscape::GetTexUsage(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, DWORD *dwpTextureUsage) {
  BYTE *bypMap, *bypPixel;
  int iPitch, iWidth, iHeight;
  int iX, iY;
  // No good parameters
  if (!sfcMap || !dwpTextureUsage)
    return FALSE;
  // Lock map surface
  if (!(bypMap = LockSurface(sfcMap, iPitch, &iWidth, &iHeight)))
    return FALSE;
  // Clip desired map segment to map size
  iMapX = BoundBy(iMapX, 0, iWidth - 1);
  iMapY = BoundBy(iMapY, 0, iHeight - 1);
  iMapWdt = BoundBy(iMapWdt, 0, iWidth - iMapX);
  iMapHgt = BoundBy(iMapHgt, 0, iHeight - iMapY);
  // Zero texture usage list
  for (int cnt = 0; cnt < C4M_MaxTexIndex; cnt++)
    dwpTextureUsage[cnt] = 0;
  // Scan map pixels
  for (iY = iMapY; iY < iMapY + iMapHgt; iY++)
    for (iX = iMapX, bypPixel = bypMap + iPitch * iY + iX; iX < iMapX + iMapWdt; iX++, bypPixel++)
      // Count texture map index only (no IFT)
      dwpTextureUsage[(*bypPixel) & (C4M_MaxTexIndex - 1)]++;
  // Unlock map surface
  UnLockSurface(sfcMap);
  // Done
  return TRUE;
}

BOOL C4Landscape::TexOZoom(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, DWORD *dwpTextureUsage, C4TextureMap &rTextureMap) {
  int iIndex, iMaterial;
  const char *szMaterialTexture;
  char szMaterial[C4M_MaxName + 1];
  char szTexture[C4M_MaxName + 1 + 4];
  SURFACE sfcTexture;

  // ChunkOZoom all used textures
  for (iIndex = 1; iIndex < C4M_MaxTexIndex; iIndex++)
    if (dwpTextureUsage[iIndex] > 0) {
      // Find material-texture in texture map by index
      if (!(szMaterialTexture = rTextureMap.GetMaterialTexture(iIndex))) {
        // Map index used but not defined
        sprintf(OSTR, LoadResStr(IDS_PRC_NOTEXMAP), iIndex);
        Log(OSTR);
        return FALSE;
      }
      // Find material in material map by material-texture
      SCopySegment(szMaterialTexture, 0, szMaterial, '-', C4M_MaxName);
      if ((iMaterial = Game.Material.Get(szMaterial)) == MNone) {
        // Material not available
        sprintf(OSTR, LoadResStr(IDS_PRC_NOMATMAP), szMaterial);
        Log(OSTR);
        return FALSE;
      }
      // Get texture pattern
      sfcTexture = NULL;
      if (SCopySegment(szMaterialTexture, 1, szTexture, '-', C4M_MaxName))
        if (!(sfcTexture = rTextureMap.GetTexture(szTexture))) {
          // Texture not available
          sprintf(OSTR, LoadResStr(IDS_PRC_NOTEXPAT), szTexture);
          Log(OSTR);
          return FALSE;
        }
      // Set drawing pattern
      if (sfcTexture)
        if (!Engine.DDraw.DefinePattern(sfcTexture)) {
          Log("Pattern error");
          return FALSE;
        }

      // ChunkOZoom map to landscape
      ChunkOZoom(sfcMap, iMapX, iMapY, iMapWdt, iMapHgt, sfcLandscape, iIndex, iMaterial);

      // Clear drawing pattern
      Engine.DDraw.NoPattern();
    }

  // Done
  return TRUE;
}

BOOL C4Landscape::SkyToLandscape(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape) {
  // Get map & landscape size
  int iMapWidth, iMapHeight, iLandscapeWidth, iLandscapeHeight;
  if (!GetSurfaceSize(sfcMap, iMapWidth, iMapHeight) || !GetSurfaceSize(sfcLandscape, iLandscapeWidth, iLandscapeHeight))
    return FALSE;
  // Clip map segment to map size
  iMapX = BoundBy(iMapX, 0, iMapWidth - 1);
  iMapY = BoundBy(iMapY, 0, iMapHeight - 1);
  iMapWdt = BoundBy(iMapWdt, 0, iMapWidth - iMapX);
  iMapHgt = BoundBy(iMapHgt, 0, iMapHeight - iMapY);
  // No segment
  if (!iMapWdt || !iMapHgt)
    return TRUE;
  // Clear landscape segment to sky background
  int iToX, iToY, iToWdt, iToHgt;
  iToX = iMapX * iLandscapeWidth / iMapWidth;
  iToY = iMapY * iLandscapeHeight / iMapHeight;
  iToWdt = iMapWdt * iLandscapeWidth / iMapWidth;
  iToHgt = iMapHgt * iLandscapeHeight / iMapHeight;
  Engine.DDraw.BlitSurfaceTile(Sky.Surface, Surface, iToX, iToY, iToWdt, iToHgt, -iToX, -iToY);
  // Done
  return TRUE;
}

BOOL C4Landscape::MapToLandscape(SURFACE sfcMap, int iMapX, int iMapY, int iMapWdt, int iMapHgt, SURFACE sfcLandscape, C4TextureMap &rTextureMap, BOOL fStaticChunks) {
  // Sky background segment
  SkyToLandscape(sfcMap, iMapX, iMapY, iMapWdt, iMapHgt, sfcLandscape);

  // Clip to map/landscape segment
  int iMapWidth, iMapHeight, iLandscapeWidth, iLandscapeHeight;
  // Get map & landscape size
  if (!GetSurfaceSize(sfcMap, iMapWidth, iMapHeight) || !GetSurfaceSize(sfcLandscape, iLandscapeWidth, iLandscapeHeight))
    return FALSE;
  // Clip map segment to map size
  iMapX = BoundBy(iMapX, 0, iMapWidth - 1);
  iMapY = BoundBy(iMapY, 0, iMapHeight - 1);
  iMapWdt = BoundBy(iMapWdt, 0, iMapWidth - iMapX);
  iMapHgt = BoundBy(iMapHgt, 0, iMapHeight - iMapY);
  // No segment
  if (!iMapWdt || !iMapHgt)
    return TRUE;
  // Clear landscape segment to sky background
  int iToX, iToY, iToWdt, iToHgt;
  iToX = iMapX * iLandscapeWidth / iMapWidth;
  iToY = iMapY * iLandscapeHeight / iMapHeight;
  iToWdt = iMapWdt * iLandscapeWidth / iMapWidth;
  iToHgt = iMapHgt * iLandscapeHeight / iMapHeight;
  Engine.DDraw.SetPrimaryClipper(iToX, iToY, iToX + iToWdt - 1, iToY + iToHgt - 1);

  // Set chunk random mode
  ChunkyRandomStatic = fStaticChunks;

  // Enlarge map segment for chunky rim
  iMapX -= 2;
  iMapY -= 2;
  iMapWdt += 4;
  iMapHgt += 4;
  // Determine texture usage in map segment
  DWORD dwTexUsage[C4M_MaxTexIndex];
  if (!GetTexUsage(sfcMap, iMapX, iMapY, iMapWdt, iMapHgt, dwTexUsage))
    return FALSE;
  // Texture zoom map to landscape
  if (!TexOZoom(sfcMap, iMapX, iMapY, iMapWdt, iMapHgt, sfcLandscape, dwTexUsage, rTextureMap))
    return FALSE;

  Engine.DDraw.NoPrimaryClipper();

  // Done
  return TRUE;
}

SURFACE C4Landscape::CreateMap(C4TextureMap &rTextureMap) {
  SURFACE sfcMap;
  int iWidth = 0, iHeight = 0;
  BYTE *bpBuffer;
  int iPitch;

  // Create map surface
  Game.C4S.Landscape.GetMapSize(iWidth, iHeight, Game.StartupPlayerCount);
  if (!(sfcMap = CreateSurface(iWidth, iHeight)))
    return NULL;

  // Fill sfcMap
  if (bpBuffer = LockSurface(sfcMap, iPitch)) {
    C4MapCreator MapCreator;
    MapCreator.Create(bpBuffer, iPitch, Game.C4S.Landscape, rTextureMap, TRUE, Game.StartupPlayerCount);
    UnLockSurface(sfcMap);
  }

  return sfcMap;
}

BOOL C4Landscape::Init() {

  // Exact landscape from scenario (no map)
  if (Game.C4S.Landscape.ExactLandscape) {
    Mode = C4LSC_Exact;
    return Load(Game.ScenarioFile);
  }

  // Static map from scenario
  if (Game.ScenarioFile.AccessEntry(C4CFN_Landscape))
    if (Map = GroupReadSurface(Game.ScenarioFile))
      Mode = C4LSC_Static;

  // Dynamic map by scenario
  if (!Map)
    if (Map = CreateMap(Game.TextureMap))
      Mode = C4LSC_Dynamic;

  // No map failure
  if (!Map)
    return FALSE;

  // Store map size and calculate map zoom
  GetSurfaceSize(Map, MapWidth, MapHeight);
  MapZoom = Game.C4S.Landscape.MapZoom.Evaluate();

  // Calculate landscape size
  Width = MapZoom * MapWidth;
  Height = MapZoom * MapHeight;
  Width = Max(Width, 100);
  Height = Max(Height, 100);
  Width = (Width / 8) * 8;

  // Sky (might need to know lanscape height)
  if (!Sky.Init())
    return FALSE;

  // Create landscape surface
  if (!(Surface = CreateSurface(Width, Height)))
    return FALSE;

  // Map to landscape
  if (!MapToLandscape())
    return FALSE;

  // Success
  return TRUE;
}

BOOL C4Landscape::LockQBA() {
  if (QBAPointer)
    return TRUE;
  if (!(QBAPointer = LockSurface(Surface, QBAPitch)))
    return FALSE;
  return TRUE;
}

void C4Landscape::UnLockQBA() {
  if (QBAPointer)
    UnLockSurface(Surface);
  QBAPointer = NULL;
}

BOOL C4Landscape::IsQBALocked() {
  if (QBAPointer)
    return TRUE;
  return FALSE;
}

BYTE C4Landscape::GetPix(int x, int y) {
  // Border checks
  if (x < 0)
    if (y < LeftOpen)
      return 0;
    else
      return Mat2PixCol(MVehic) + 1;
  if (x >= Width)
    if (y < RightOpen)
      return 0;
    else
      return Mat2PixCol(MVehic) + 1;
  if (y < 0)
    if (TopOpen)
      return 0;
    else
      return Mat2PixCol(MVehic) + 1;
  if (y >= Height)
    if (BottomOpen)
      return 0;
    else
      return Mat2PixCol(MVehic) + 1;
  // Evaluate gback
  if (QBAPointer)
    return QBAPointer[QBAPitch * y + x];
  return Engine.DDraw.GetPixel(Surface, x, y);
}

BOOL C4Landscape::SetPix(int x, int y, BYTE npix) {
  // QBA
  if (QBAPointer) {
    if ((x < 0) || (y < 0) || (x >= Width) || (y >= Height))
      return TRUE;
    QBAPointer[QBAPitch * y + x] = npix;
    return TRUE;
  }
  // Non-QBA
  return Engine.DDraw.SetPixel(Surface, x, y, npix);
}

BOOL C4Landscape::CheckInstability(int tx, int ty) {
  int mat = GBackMat(tx, ty);
  if (MatValid(mat))
    if (Game.Material.Map[mat].Instable)
      return Game.MassMover.Create(tx, ty);
  return FALSE;
}

void C4Landscape::CheckInstabilityRange(int tx, int ty) {
  if (!CheckInstability(tx, ty)) {
    CheckInstability(tx, ty - 1);
    CheckInstability(tx - 1, ty);
    CheckInstability(tx + 1, ty);
  }
}

BOOL C4Landscape::ClearPix(int tx, int ty) {
  BYTE bcol;
  if (GBackIFT(tx, ty))
    bcol = Mat2PixColRnd(MTunnel) + IFT;
  else
    bcol = (BYTE)BoundBy(Engine.DDraw.GetPixel(Sky.Surface, tx % Sky.Width, ty % Sky.Height), CSkyDef1, CSkyDef2);
  return SetPix(tx, ty, bcol);
}

int C4Landscape::DigFreePix(int tx, int ty) {
  int mat = GBackMat(tx, ty);
  if (mat != MNone)
    if (Game.Material.Map[mat].DigFree)
      ClearPix(tx, ty);
  CheckInstabilityRange(tx, ty);
  return mat;
}

int C4Landscape::ShakeFreePix(int tx, int ty) {
  int mat = GBackMat(tx, ty);
  if (mat != MNone)
    if (Game.Material.Map[mat].DigFree) {
      ClearPix(tx, ty);
      Game.PXS.Create(mat, tx, ty);
    }
  CheckInstabilityRange(tx, ty);
  return mat;
}

int C4Landscape::BlastFreePix(int tx, int ty, int grade) {
  int mat = GBackMat(tx, ty);
  if (MatValid(mat)) {
    // Blast Shift
    if (Game.Material.Map[mat].BlastShiftTo != MNone)
      if (GBackMatColIndex(tx, ty) + grade >= C4M_ColsPerMat)
        SetPix(tx, ty, Mat2PixColRnd(Game.Material.Map[mat].BlastShiftTo) + GBackIFT(tx, ty));
      else
        SetPix(tx, ty, GBackPix(tx, ty) + grade);
    // Blast Free
    if (Game.Material.Map[mat].BlastFree)
      ClearPix(tx, ty);
  }

  CheckInstabilityRange(tx, ty);

  return mat;
}

void C4Landscape::DigFree(int tx, int ty, int rad, BOOL fRequest, C4Object *pByObj) {
  int ycnt, xcnt, iLineWidth, iLineY, iMaterial;
  // Dig free
  for (ycnt = -rad; ycnt < rad; ycnt++) {
    iLineWidth = (int)sqrt(rad * rad - ycnt * ycnt);
    iLineY = ty + ycnt;
    for (xcnt = -iLineWidth; xcnt < iLineWidth + (iLineWidth == 0); xcnt++)
      if (MatValid(iMaterial = DigFreePix(tx + xcnt, iLineY)))
        if (pByObj)
          pByObj->AddMaterialContents(iMaterial, 1);
  }
  // Dig out material cast
  if (!Tick5)
    if (pByObj)
      pByObj->DigOutMaterialCast(fRequest);
}

void C4Landscape::DigFreeRect(int tx, int ty, int wdt, int hgt, BOOL fRequest, C4Object *pByObj) {
  // Dig free pixels
  int cx, cy, iMaterial;
  for (cx = tx; cx < tx + wdt; cx++)
    for (cy = ty; cy < ty + hgt; cy++)
      if (MatValid(iMaterial = DigFreePix(cx, cy)))
        if (pByObj)
          pByObj->AddMaterialContents(iMaterial, 1);
  // Dig out material cast
  if (!Tick5)
    if (pByObj)
      pByObj->DigOutMaterialCast(fRequest);
}

void C4Landscape::ShakeFree(int tx, int ty, int rad) {
  int ycnt, xcnt, lwdt, dpy;
  // Shake free pixels
  for (ycnt = -rad; ycnt < rad; ycnt++) {
    lwdt = (int)sqrt(rad * rad - ycnt * ycnt);
    dpy = ty + ycnt;
    for (xcnt = -lwdt; xcnt < lwdt + (lwdt == 0); xcnt++)
      ShakeFreePix(tx + xcnt, dpy);
  }
}

void C4Landscape::DigFreeMat(int tx, int ty, int wdt, int hgt, int mat) {
  int cx, cy;
  if (MatValid(mat))
    for (cx = tx; cx < tx + wdt; cx++)
      for (cy = ty; cy < ty + hgt; cy++)
        if (GBackMat(cx, cy) == mat)
          DigFreePix(cx, cy);
}

void C4Landscape::BlastFree(int tx, int ty, int rad, int grade) {
  int ycnt, xcnt, lwdt, dpy, mat, cnt;

  // Reset material count
  ClearBlastMatCount();

  // Blast free pixels
  for (ycnt = -rad; ycnt <= rad; ycnt++) {
    lwdt = (int)sqrt(rad * rad - ycnt * ycnt);
    dpy = ty + ycnt;
    for (xcnt = -lwdt; xcnt < lwdt + (lwdt == 0); xcnt++)
      if (MatValid(mat = BlastFreePix(tx + xcnt, dpy, grade)))
        BlastMatCount[mat]++;
  }

  // Evaluate material count
  for (cnt = 0; cnt < Game.Material.Num; cnt++)
    if (BlastMatCount[cnt]) {

      if (Game.Material.Map[cnt].Blast2Object != C4ID_None)
        if (Game.Material.Map[cnt].Blast2ObjectRatio != 0)
          Game.BlastCastObjects(Game.Material.Map[cnt].Blast2Object, BlastMatCount[cnt] / Game.Material.Map[cnt].Blast2ObjectRatio, tx, ty);

      if (Game.Material.Map[cnt].Blast2PXSRatio != 0)
        Game.PXS.Cast(cnt, BlastMatCount[cnt] / Game.Material.Map[cnt].Blast2PXSRatio, tx, ty, 60);
    }
}

void C4Landscape::DrawMaterialRect(int mat, int tx, int ty, int wdt, int hgt) {
  int cx, cy;
  for (cy = ty; cy < ty + hgt; cy++)
    for (cx = tx; cx < tx + wdt; cx++)
      if ((MatDensity(mat) > GBackDensity(cx, cy)) || ((MatDensity(mat) == GBackDensity(cx, cy)) && (MatDigFree(mat) <= MatDigFree(GBackMat(cx, cy)))))
        SetPix(cx, cy, Mat2PixCol(mat) + GBackIFT(cx, cy));
}

void C4Landscape::RaiseTerrain(int tx, int ty, int wdt) {
  int cx, cy;
  BYTE cpix;
  for (cx = tx; cx < tx + wdt; cx++) {
    for (cy = ty; (cy + 1 < GBackHgt) && !GBackSolid(cx, cy + 1); cy++)
      ;
    if (cy + 1 < GBackHgt)
      if (cy - ty < 20) {
        cpix = GBackPix(cx, cy + 1);
        if (PixCol2Mat(cpix) != MVehic)
          while (cy >= ty) {
            SetPix(cx, cy, cpix);
            cy--;
          }
      }
  }
}

int C4Landscape::AreaSolidCount(int x, int y, int wdt, int hgt) {
  int cx, cy, ascnt = 0;
  for (cy = y; cy < y + hgt; cy++)
    for (cx = x; cx < x + wdt; cx++)
      if (GBackSolid(cx, cy))
        ascnt++;
  return ascnt;
}

void C4Landscape::FindMatTop(int mat, int &x, int &y) {
  int mslide, cslide, tslide; // tslide 0 none 1 left 2 right
  BOOL fLeft, fRight;

  if (!MatValid(mat))
    return;
  mslide = Game.Material.Map[mat].MaxSlide;

  do {

    // Find upwards slide
    fLeft = TRUE;
    fRight = TRUE;
    tslide = 0;
    for (cslide = 0; (cslide <= mslide) && (fLeft || fRight); cslide++) {
      // Left
      if (fLeft)
        if (GBackMat(x - cslide, y) != mat)
          fLeft = FALSE;
        else if (GBackMat(x - cslide, y - 1) == mat) {
          tslide = 1;
          break;
        }
      // Right
      if (fRight)
        if (GBackMat(x + cslide, y) != mat)
          fRight = FALSE;
        else if (GBackMat(x + cslide, y - 1) == mat) {
          tslide = 2;
          break;
        }
    }

    // Slide
    if (tslide == 1) {
      x -= cslide;
      y--;
    }
    if (tslide == 2) {
      x += cslide;
      y--;
    }

  } while (tslide);
}

int C4Landscape::ExtractMaterial(int fx, int fy) {
  int mat = GBackMat(fx, fy);
  if (mat == MNone)
    return MNone;
  FindMatTop(mat, fx, fy);
  ClearPix(fx, fy);
  CheckInstabilityRange(fx, fy);
  return mat;
}

BOOL C4Landscape::InsertMaterial(int mat, int tx, int ty) {
  int mdens;
  if (!MatValid(mat))
    return FALSE;
  mdens = MatDensity(mat);

  // Bounds
  if (!Inside(tx, 0, Width - 1) || !Inside(ty, 0, Height))
    return FALSE;

  // Move up above same density
  while (mdens == GBackDensity(tx, ty)) {
    ty--;
    if (ty < 0)
      return FALSE;
    // Primitive slide (1)
    if (GBackDensity(tx - 1, ty) < mdens)
      tx--;
    if (GBackDensity(tx + 1, ty) < mdens)
      tx++;
  }

  // Stuck in higher density
  if (GBackDensity(tx, ty) > mdens)
    return FALSE;

  // Try slide
  while (FindMatSlide(tx, ty, +1, mdens, Game.Material.Map[mat].MaxSlide))
    if (GBackDensity(tx, ty + 1) < mdens) {
      Game.PXS.Create(mat, tx, ty);
      return TRUE;
    }

  // Insert dead material
  SetPix(tx, ty, Mat2PixColRnd(mat) + GBackIFT(tx, ty));
  return TRUE;
}

// Finds the next pixel position moving to desired slide.

BOOL C4Landscape::FindMatPath(int &fx, int &fy, int ydir, int mdens, int mslide) {
  int cslide;
  BOOL fLeft = TRUE, fRight = TRUE;

  // One downwards
  if (GBackDensity(fx, fy + ydir) < mdens) {
    fy += ydir;
    return TRUE;
  }

  // Find downwards slide path
  for (cslide = 1; (cslide <= mslide) && (fLeft || fRight); cslide++) {
    // Check left
    if (fLeft)
      if (GBackDensity(fx - cslide, fy) >= mdens) // Left clogged
        fLeft = FALSE;
      else if (GBackDensity(fx - cslide, fy + ydir) < mdens) // Left slide okay
      {
        fx--;
        return TRUE;
      }
    // Check right
    if (fRight)
      if (GBackDensity(fx + cslide, fy) >= mdens) // Right clogged
        fRight = FALSE;
      else if (GBackDensity(fx + cslide, fy + ydir) < mdens) // Right slide okay
      {
        fx++;
        return TRUE;
      }
  }

  return FALSE;
}

// Finds the closest immediate slide position.

BOOL C4Landscape::FindMatSlide(int &fx, int &fy, int ydir, int mdens, int mslide) {
  int cslide;
  BOOL fLeft = TRUE, fRight = TRUE;

  // One downwards
  if (GBackDensity(fx, fy + ydir) < mdens) {
    fy += ydir;
    return TRUE;
  }

  // Find downwards slide path
  for (cslide = 1; (cslide <= mslide) && (fLeft || fRight); cslide++) {
    // Check left
    if (fLeft)
      if (GBackDensity(fx - cslide, fy) >= mdens) // Left clogged
        fLeft = FALSE;
      else if (GBackDensity(fx - cslide, fy + ydir) < mdens) // Left slide okay
      {
        fx -= cslide;
        fy += ydir;
        return TRUE;
      }
    // Check right
    if (fRight)
      if (GBackDensity(fx + cslide, fy) >= mdens) // Right clogged
        fRight = FALSE;
      else if (GBackDensity(fx + cslide, fy + ydir) < mdens) // Right slide okay
      {
        fx += cslide;
        fy += ydir;
        return TRUE;
      }
  }

  return FALSE;
}

BOOL C4Landscape::Incinerate(int x, int y) {
  int mat = GBackMat(x, y);
  if (MatValid(mat))
    if (Game.Material.Map[mat].Inflammable)
      if (Game.CreateObject(C4Id("FLAM"), NO_OWNER, x, y))
        return TRUE;
  return FALSE;
}

BOOL C4Landscape::Save(C4Group &hGroup) {

  // Save members
  if (!Sky.Save(hGroup))
    return FALSE;

  // Check QBA
  BOOL fQBA = IsQBALocked();
  if (fQBA)
    UnLockQBA();

  // Save landscape surface
  if (!SaveSurface(Config.AtTempPath(C4CFN_TempLandscape), Surface, Game.GraphicsResource.GamePalette))
    return FALSE;

  // Check QBA
  if (fQBA)
    LockQBA();

  // Move temp file to group
  if (!hGroup.Move(Config.AtTempPath(C4CFN_TempLandscape), C4CFN_Landscape))
    return FALSE;

  return TRUE;
}

BOOL C4Landscape::Load(C4Group &hGroup) {
  // Load exact landscape from group
  if (!hGroup.AccessEntry(C4CFN_Landscape))
    return FALSE;
  if (!(Surface = GroupReadSurface(hGroup)))
    return FALSE;
  if (!GetSurfaceSize(Surface, Width, Height))
    return FALSE;
  // Init sky
  if (!Sky.Init())
    return FALSE;
  // Success
  return TRUE;
}

void C4Landscape::Default() {
  Mode = C4LSC_Dynamic;
  Surface = NULL;
  Map = NULL;
  Width = Height = 0;
  MapWidth = MapHeight = MapZoom = 0;
  ClearMatCount();
  ClearScanMatCount();
  ClearBlastMatCount();
  ScanX = 0;
  ScanSpeed = 2;
  MatCountValid = FALSE;
  QBAPointer = NULL;
  LeftOpen = RightOpen = TopOpen = BottomOpen = 0;
  Gravity = ftofix(0.2);
}

void C4Landscape::Draw(C4FacetEx &cgo) { Engine.DDraw.BlitFast(Surface, cgo.TargetX, cgo.TargetY, cgo.Surface, cgo.X, cgo.Y, cgo.Wdt, cgo.Hgt); }

void C4Landscape::ClearBlastMatCount() {
  for (int cnt = 0; cnt < C4MaxMaterial; cnt++)
    BlastMatCount[cnt] = 0;
}

void C4Landscape::ClearMatCount() {
  for (int cnt = 0; cnt < C4MaxMaterial; cnt++)
    MatCount[cnt] = 0;
}

void C4Landscape::Synchronize() {
  ClearMatCount();
  ClearScanMatCount();
  MatCountValid = FALSE;
  ScanX = 0;
  ClearBlastMatCount();
}

BOOL AboveSemiSolid(long &rx, long &ry) // Nearest free above semi solid
{
  int cy1 = ry, cy2 = ry;
  BOOL UseUpwardsNextFree = FALSE, UseDownwardsNextSolid = FALSE;

  while ((cy1 >= 0) || (cy2 < GBackHgt)) {
    // Check upwards
    if (cy1 >= 0)
      if (GBackSemiSolid(rx, cy1))
        UseUpwardsNextFree = TRUE;
      else if (UseUpwardsNextFree) {
        ry = cy1;
        return TRUE;
      }
    // Check downwards
    if (cy2 < GBackHgt)
      if (!GBackSemiSolid(rx, cy2))
        UseDownwardsNextSolid = TRUE;
      else if (UseDownwardsNextSolid) {
        ry = cy2;
        return TRUE;
      }
    // Advance
    cy1--;
    cy2++;
  }

  return FALSE;
}

BOOL AboveSolid(long &rx, long &ry) // Nearest free directly above solid
{
  int cy1 = ry, cy2 = ry;

  while ((cy1 >= 0) || (cy2 < GBackHgt)) {
    // Check upwards
    if (cy1 >= 0)
      if (!GBackSemiSolid(rx, cy1))
        if (GBackSolid(rx, cy1 + 1)) {
          ry = cy1;
          return TRUE;
        }
    // Check downwards
    if (cy2 + 1 < GBackHgt)
      if (!GBackSemiSolid(rx, cy2))
        if (GBackSolid(rx, cy2 + 1)) {
          ry = cy2;
          return TRUE;
        }
    // Advance
    cy1--;
    cy2++;
  }

  return FALSE;
}

BOOL SemiAboveSolid(long &rx, long &ry) // Nearest free/semi above solid
{
  int cy1 = ry, cy2 = ry;

  while ((cy1 >= 0) || (cy2 < GBackHgt)) {
    // Check upwards
    if (cy1 >= 0)
      if (!GBackSolid(rx, cy1))
        if (GBackSolid(rx, cy1 + 1)) {
          ry = cy1;
          return TRUE;
        }
    // Check downwards
    if (cy2 + 1 < GBackHgt)
      if (!GBackSolid(rx, cy2))
        if (GBackSolid(rx, cy2 + 1)) {
          ry = cy2;
          return TRUE;
        }
    // Advance
    cy1--;
    cy2++;
  }

  return FALSE;
}

BOOL FindLiquidHeight(int cx, long &ry, int hgt) {
  int cy1 = ry, cy2 = ry, rl1 = 0, rl2 = 0;

  while ((cy1 >= 0) || (cy2 < GBackHgt)) {
    // Check upwards
    if (cy1 >= 0)
      if (GBackLiquid(cx, cy1)) {
        rl1++;
        if (rl1 >= hgt) {
          ry = cy1 + hgt / 2;
          return TRUE;
        }
      } else
        rl1 = 0;
    // Check downwards
    if (cy2 + 1 < GBackHgt)
      if (GBackLiquid(cx, cy2)) {
        rl2++;
        if (rl2 >= hgt) {
          ry = cy2 - hgt / 2;
          return TRUE;
        }
      } else
        rl2 = 0;
    // Advance
    cy1--;
    cy2++;
  }

  return FALSE;
}

// Starting from rx/ry, searches for a width
// of solid ground. Returns bottom center
// of surface space found.

BOOL FindSolidGround(long &rx, long &ry, int width) {
  BOOL fFound = FALSE;

  int cx1, cx2, cy1, cy2, rl1 = 0, rl2 = 0;

  for (cx1 = cx2 = rx, cy1 = cy2 = ry; (cx1 > 0) || (cx2 < GBackWdt); cx1--, cx2++) {
    // Left search
    if (cx1 >= 0) // Still going
    {
      long lcx1 = cx1, lcy1 = cy1;
      if (AboveSolid(lcx1, lcy1)) {
        cy1 = (int)lcy1;
        rl1++;
      } // Run okay
      else
        rl1 = 0; // No run
    }
    // Right search
    if (cx2 < GBackWdt) // Still going
    {
      long lcx2 = cx2, lcy2 = cy2;
      if (AboveSolid(lcx2, lcy2)) {
        cy2 = (int)lcy2;
        rl2++;
      } // Run okay
      else
        rl2 = 0; // No run
    }
    // Check runs
    if (rl1 >= width) {
      rx = cx1 + rl1 / 2;
      ry = cy1;
      fFound = TRUE;
      break;
    }
    if (rl2 >= width) {
      rx = cx2 - rl2 / 2;
      ry = cy2;
      fFound = TRUE;
      break;
    }
  }

  if (fFound)
    AboveSemiSolid(rx, ry);

  return fFound;
}

BOOL FindSurfaceLiquid(long &rx, long &ry, int width, int height) {
  BOOL fFound = FALSE;

  int cx1, cx2, cy1, cy2, rl1 = 0, rl2 = 0, cnt;
  BOOL lokay;
  for (cx1 = cx2 = rx, cy1 = cy2 = ry; (cx1 > 0) || (cx2 < GBackWdt); cx1--, cx2++) {
    // Left search
    if (cx1 > 0) // Still going
    {
      long lcx1 = cx1, lcy1 = cy1;
      if (!AboveSemiSolid(lcx1, lcy1))
        cx1 = -1; // Abort left
      else {
        cy1 = (int)lcy1;
        for (lokay = TRUE, cnt = 0; cnt < height; cnt++)
          if (!GBackLiquid(cx1, cy1 + 1 + cnt))
            lokay = FALSE;
        if (lokay)
          rl1++; // Run okay
        else
          rl1 = 0; // No run
      }
    }
    // Right search
    if (cx2 < GBackWdt) // Still going
    {
      long lcx2 = cx2, lcy2 = cy2;
      if (!AboveSemiSolid(lcx2, lcy2))
        cx2 = GBackWdt; // Abort right
      else {
        cy2 = (int)lcy2;
        for (lokay = TRUE, cnt = 0; cnt < height; cnt++)
          if (!GBackLiquid(cx2, cy2 + 1 + cnt))
            lokay = FALSE;
        if (lokay)
          rl2++; // Run okay
        else
          rl2 = 0; // No run
      }
    }
    // Check runs
    if (rl1 >= width) {
      rx = cx1 + rl1 / 2;
      ry = cy1;
      fFound = TRUE;
      break;
    }
    if (rl2 >= width) {
      rx = cx2 - rl2 / 2;
      ry = cy2;
      fFound = TRUE;
      break;
    }
  }

  if (fFound)
    AboveSemiSolid(rx, ry);

  return fFound;
}

BOOL FindLiquid(long &rx, long &ry, int width, int height) {
  int cx1, cx2, cy1, cy2, rl1 = 0, rl2 = 0;

  for (cx1 = cx2 = rx, cy1 = cy2 = ry; (cx1 > 0) || (cx2 < GBackWdt); cx1--, cx2++) {
    // Left search
    if (cx1 > 0) {
      long lcy1 = cy1;
      if (FindLiquidHeight(cx1, lcy1, height)) {
        cy1 = (int)lcy1;
        rl1++;
      } else
        rl1 = 0;
    }
    // Right search
    if (cx2 < GBackWdt) {
      long lcy2 = cy2;
      if (FindLiquidHeight(cx2, lcy2, height)) {
        cy2 = (int)lcy2;
        rl2++;
      } else
        rl2 = 0;
    }
    // Check runs
    if (rl1 >= width) {
      rx = cx1 + rl1 / 2;
      ry = cy1;
      return TRUE;
    }
    if (rl2 >= width) {
      rx = cx2 - rl2 / 2;
      ry = cy2;
      return TRUE;
    }
  }

  return FALSE;
}

// FindLevelGround: Starting from rx/ry, searches for a width
//                  of solid ground. Extreme distances may not
//                  exceed hrange.
//                  Returns bottom center of surface found.

BOOL FindLevelGround(long &rx, long &ry, int width, int hrange) {
  BOOL fFound = FALSE;

  int cx1, cx2, cy1, cy2, rh1, rh2, rl1, rl2;

  cx1 = cx2 = rx;
  cy1 = cy2 = ry;
  rh1 = cy1;
  rh2 = cy2;
  rl1 = rl2 = 0;

  for (cx1--, cx2++; (cx1 > 0) || (cx2 < GBackWdt); cx1--, cx2++) {
    // Left search
    if (cx1 > 0) // Still going
    {
      long lcx1 = cx1, lcy1 = cy1;
      if (!AboveSemiSolid(lcx1, lcy1))
        cx1 = -1; // Abort left
      else {
        cy1 = (int)lcy1;
        if (GBackSolid(cx1, cy1 + 1) && (Abs(cy1 - rh1) < hrange))
          rl1++; // Run okay
        else {
          rl1 = 0;
          rh1 = cy1;
        } // No run
      }
    }

    // Right search
    if (cx2 < GBackWdt) // Still going
    {
      long lcx2 = cx2, lcy2 = cy2;
      if (!AboveSemiSolid(lcx2, lcy2))
        cx2 = GBackWdt; // Abort right
      else {
        cy2 = (int)lcy2;
        if (GBackSolid(cx2, cy2 + 1) && (Abs(cy2 - rh2) < hrange))
          rl2++; // Run okay
        else {
          rl2 = 0;
          rh2 = cy2;
        } // No run
      }
    }

    // Check runs
    if (rl1 >= width) {
      rx = cx1 + rl1 / 2;
      ry = cy1;
      fFound = TRUE;
      break;
    }
    if (rl2 >= width) {
      rx = cx2 - rl2 / 2;
      ry = cy2;
      fFound = TRUE;
      break;
    }
  }

  if (fFound)
    AboveSemiSolid(rx, ry);

  return fFound;
}

// Starting from rx/ry, searches for a width of solid level
// ground with structure clearance (category).
// Returns bottom center of surface found.

BOOL FindConSiteSpot(long &rx, long &ry, int wdt, int hgt, DWORD category, int hrange) {
  BOOL fFound = FALSE;

  // No hrange limit, use standard smooth surface limit
  if (hrange == -1)
    hrange = Max(wdt / 4, 5);

  int cx1, cx2, cy1, cy2, rh1, rh2, rl1, rl2;

  // Left offset starting position
  cx1 = Min((int)rx + wdt / 2, GBackWdt - 1);
  cy1 = ry;
  // No good: use centered starting position
  {
    long lcx1 = cx1, lcy1 = cy1;
    if (!AboveSemiSolid(lcx1, lcy1)) {
      cx1 = Min((int)rx, GBackWdt - 1);
      cy1 = ry;
    }
  }
  // Right offset starting position
  cx2 = Max((int)rx - wdt / 2, 0);
  cy2 = ry;
  // No good: use centered starting position
  {
    long lcx2 = cx2, lcy2 = cy2;
    if (!AboveSemiSolid(lcx2, lcy2)) {
      cx2 = Min((int)rx, GBackWdt - 1);
      cy2 = ry;
    }
  }

  rh1 = cy1;
  rh2 = cy2;
  rl1 = rl2 = 0;

  for (cx1--, cx2++; (cx1 > 0) || (cx2 < GBackWdt); cx1--, cx2++) {
    // Left search
    if (cx1 > 0) // Still going
    {
      long lcx1 = cx1, lcy1 = cy1;
      if (!AboveSemiSolid(lcx1, lcy1))
        cx1 = -1; // Abort left
      else {
        cy1 = (int)lcy1;
        if (GBackSolid(cx1, cy1 + 1) && (Abs(cy1 - rh1) < hrange))
          rl1++; // Run okay
        else {
          rl1 = 0;
          rh1 = cy1;
        } // No run
      }
    }

    // Right search
    if (cx2 < GBackWdt) // Still going
    {
      long lcx2 = cx2, lcy2 = cy2;
      if (!AboveSemiSolid(lcx2, lcy2))
        cx2 = GBackWdt; // Abort right
      else {
        cy2 = (int)lcy2;
        if (GBackSolid(cx2, cy2 + 1) && (Abs(cy2 - rh2) < hrange))
          rl2++; // Run okay
        else {
          rl2 = 0;
          rh2 = cy2;
        } // No run
      }
    }

    // Check runs & object overlap
    if (rl1 >= wdt)
      if (cx1 > 0)
        if (!Game.OverlapObject(cx1, cy1 - hgt - 10, wdt, hgt + 40, category)) {
          rx = cx1 + wdt / 2;
          ry = cy1;
          fFound = TRUE;
          break;
        }
    if (rl2 >= wdt)
      if (cx2 < GBackWdt)
        if (!Game.OverlapObject(cx2 - wdt, cy2 - hgt - 10, wdt, hgt + 40, category)) {
          rx = cx2 - wdt / 2;
          ry = cy2;
          fFound = TRUE;
          break;
        }
  }

  if (fFound)
    AboveSemiSolid(rx, ry);

  return fFound;
}

// Returns FALSE on any solid pix in path.

BOOL PathFreePix(int x, int y, int par) { return !GBackSolid(x, y); }

BOOL PathFree(int x1, int y1, int x2, int y2, int *ix, int *iy) { return ForLine(x1, y1, x2, y2, &PathFreePix, 0, ix, iy); }

int TrajectoryDistance(int iFx, int iFy, FIXED iXDir, FIXED iYDir, int iTx, int iTy) {
  int iClosest = Distance(iFx, iFy, iTx, iTy);
  // Follow free trajectory, take closest point distance
  FIXED cx = itofix(iFx), cy = itofix(iFy);
  int cdis;
  while (Inside(fixtoi(cx), 0, GBackWdt - 1) && Inside(fixtoi(cy), 0, GBackHgt - 1) && !GBackSolid(fixtoi(cx), fixtoi(cy))) {
    cdis = Distance(fixtoi(cx), fixtoi(cy), iTx, iTy);
    if (cdis < iClosest)
      iClosest = cdis;
    cx += iXDir;
    cy += iYDir;
    iYDir += GravAccel;
  }
  return iClosest;
}

const int C4LSC_Throwing_MaxVertical = 50, C4LSC_Throwing_MaxHorizontal = 60;

BOOL FindThrowingPosition(int iTx, int iTy, FIXED fXDir, FIXED fYDir, int iHeight, long &rX, long &rY) {

  // Start underneath throwing target
  rX = iTx;
  rY = iTy; // improve: check from overhanging cliff
  if (!SemiAboveSolid(rX, rY))
    return FALSE;

  // Target too far above surface
  if (!Inside((int)rY - iTy, -C4LSC_Throwing_MaxVertical, +C4LSC_Throwing_MaxVertical))
    return FALSE;

  // Search in direction according to launch fXDir
  int iDir = +1;
  if (fXDir > 0)
    iDir = -1;

  // Move along surface
  for (int cnt = 0; Inside((int)rX, 0, GBackWdt - 1) && (cnt <= C4LSC_Throwing_MaxHorizontal); rX += iDir, cnt++) {
    // Adjust to surface
    if (!SemiAboveSolid(rX, rY))
      return FALSE;

    // Check trajectory distance
    int itjd = TrajectoryDistance((int)rX, (int)rY - iHeight, fXDir, fYDir, iTx, iTy);

    // Hitting range: success
    if (itjd <= 2)
      return TRUE;
  }

  // Failure
  return FALSE;
}

const int C4LSC_Closest_MaxRange = 200, C4LSC_Closest_Step = 10;

BOOL FindClosestFree(long &rX, long &rY, int iAngle1, int iAngle2, int iExcludeAngle1, int iExcludeAngle2) {
  double pi = 3.1415926535;
  int iX, iY;
  for (int iR = C4LSC_Closest_Step; iR < C4LSC_Closest_MaxRange; iR += C4LSC_Closest_Step)
    for (int iAngle = iAngle1; iAngle < iAngle2; iAngle += C4LSC_Closest_Step)
      if (!Inside(iAngle, iExcludeAngle1, iExcludeAngle2)) {
        iX = (int)rX + (int)(sin(pi * (double)iAngle / 180.0) * (double)iR);
        iY = (int)rY - (int)(cos(pi * (double)iAngle / 180.0) * (double)iR);
        if (Inside(iX, 0, GBackWdt - 1))
          if (Inside(iY, 0, GBackHgt - 1))
            if (!GBackSemiSolid(iX, iY)) {
              rX = iX;
              rY = iY;
              return TRUE;
            }
      }
  return FALSE;
}

BOOL ConstructionCheck(C4ID id, int iX, int iY, C4Object *pByObj) {
  C4Def *ndef;
  char idostr[5];

  // Check def
  if (!(ndef = C4Id2Def(id))) {
    GetC4IdText(id, idostr);
    sprintf(OSTR, LoadResStr(IDS_OBJ_UNDEF), idostr);
    if (pByObj)
      GameMsgObject(OSTR, pByObj, FRed);
    return FALSE;
  }

  // Constructable?
  if (!ndef->Constructable) {
    sprintf(OSTR, LoadResStr(IDS_OBJ_NOCON), ndef->Name);
    if (pByObj)
      GameMsgObject(OSTR, pByObj, FRed);
    return FALSE;
  }

  // Check area
  int rtx, rty, wdt, hgt;
  wdt = ndef->Shape.Wdt;
  hgt = ndef->Shape.Hgt;
  rtx = iX - wdt / 2;
  rty = iY - hgt;
  if (Game.Landscape.AreaSolidCount(rtx, rty, wdt, hgt) > (wdt * hgt / 20)) {
    if (pByObj)
      GameMsgObject(LoadResStr(IDS_OBJ_NOROOM), pByObj, FRed);
    return FALSE;
  }
  if (Game.Landscape.AreaSolidCount(rtx, rty + hgt, wdt, 5) < (wdt * 2)) {
    if (pByObj)
      GameMsgObject(LoadResStr(IDS_OBJ_NOLEVEL), pByObj, FRed);
    return FALSE;
  }

  // Check other structures
  C4Object *other;
  if (other = Game.OverlapObject(rtx, rty, wdt, hgt, ndef->Category)) {
    sprintf(OSTR, LoadResStr(IDS_OBJ_NOOTHER), other->Def->Name);
    if (pByObj)
      GameMsgObject(OSTR, pByObj, FRed);
    return FALSE;
  }

  return TRUE;
}

void C4Landscape::ClearRect(int iTx, int iTy, int iWdt, int iHgt) {
  for (int y = iTy; y < iTy + iHgt; y++) {
    for (int x = iTx; x < iTx + iWdt; x++)
      ClearPix(x, y);
    if (Rnd3())
      Rnd3();
  }
}

BOOL C4Landscape::SaveMap(C4Group &hGroup) {
  // No map
  if (!Map)
    return FALSE;

  // Create map palette
  BYTE bypPalette[3 * 256];
  Game.TextureMap.StoreMapPalette(bypPalette, Game.Material);

  // Save map surface
  if (!SaveSurface(Config.AtTempPath(C4CFN_TempLandscape), Map, bypPalette))
    return FALSE;

  // Move temp file to group
  if (!hGroup.Move(Config.AtTempPath(C4CFN_TempLandscape), C4CFN_Landscape))
    return FALSE;

  // Success
  return TRUE;
}

BOOL C4Landscape::SetMode(int iMode) {
  // Invalid mode
  if (!Inside(iMode, C4LSC_Dynamic, C4LSC_Exact))
    return FALSE;
  // Set mode
  Mode = iMode;
  // Done
  return TRUE;
}

BOOL C4Landscape::SetColorPattern(const char *szMaterial, const char *szTexture, BOOL fIFT, BYTE &rbyCol) {
  // Sky material: sky as pattern only
  if (SEqual(szMaterial, C4TLS_MatSky)) {
    rbyCol = 0;
    Engine.DDraw.DefinePattern(Sky.Surface);
  }
  // Material-Texture
  else {
    // Get material
    int iMat = Game.Material.Get(szMaterial);
    if (!MatValid(iMat))
      return FALSE;
    // Material color + IFT
    rbyCol = Mat2PixCol(iMat);
    if (fIFT)
      rbyCol += IFT;
    // Get texture
    SURFACE sfcTexture;
    if (!(sfcTexture = Game.TextureMap.GetTexture(szTexture)))
      return FALSE;
    // Define texture pattern
    if (!Engine.DDraw.DefinePattern(sfcTexture))
      return FALSE;
  }
  // Success
  return TRUE;
}

void C4Landscape::ClearColorPattern() { Engine.DDraw.NoPattern(); }

BOOL C4Landscape::MapToLandscape() { return MapToLandscape(Map, 0, 0, MapWidth, MapHeight, Surface, Game.TextureMap); }

BOOL C4Landscape::GetMapColorIndex(const char *szMaterial, const char *szTexture, BOOL fIFT, BYTE &rbyCol) {
  // Sky
  if (SEqual(szMaterial, C4TLS_MatSky))
    rbyCol = 0;
  // Material-Texture
  else {
    sprintf(OSTR, "%s-%s", szMaterial, szTexture);
    if (!(rbyCol = Game.TextureMap.GetIndex(OSTR, FALSE)))
      return FALSE;
    if (fIFT)
      rbyCol += C4M_MaxTexIndex;
  }
  // Found
  return TRUE;
}

BOOL C4Landscape::DrawBrush(int iX, int iY, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT) {
  BYTE byCol;
  switch (Mode) {
  // Dynamic: ignore
  case C4LSC_Dynamic:
    break;
  // Static: draw to map by material-texture-index, chunk-o-zoom to landscape
  case C4LSC_Static:
    // Get map color index by material-texture
    if (!GetMapColorIndex(szMaterial, szTexture, fIFT, byCol))
      return FALSE;
    // Draw to map
    int iRadius;
    iRadius = Max(2 * iGrade / MapZoom, 1);
    if (iRadius == 1)
      Engine.DDraw.SetPixel(Map, iX / MapZoom, iY / MapZoom, byCol);
    else
      Engine.DDraw.DrawCircle(Map, iX / MapZoom, iY / MapZoom, iRadius, byCol);
    // Update landscape
    MapToLandscape(Map, iX / MapZoom - iRadius - 1, iY / MapZoom - iRadius - 1, 2 * iRadius + 2, 2 * iRadius + 2, Surface, Game.TextureMap, TRUE);
    break;
  // Exact: draw directly to landscape by color & pattern
  case C4LSC_Exact:
    // Set texture pattern & get material color
    if (!SetColorPattern(szMaterial, szTexture, fIFT, byCol))
      return FALSE;
    // Draw to landscape
    Engine.DDraw.DrawCircle(Surface, iX, iY, iGrade, byCol);
    // Clear pattern
    ClearColorPattern();
    break;
  }
  return TRUE;
}

BYTE DrawLineCol;

BOOL DrawLineLandscape(int iX, int iY, int iGrade) {
  Engine.DDraw.DrawCircle(Game.Landscape.Surface, iX, iY, iGrade, DrawLineCol);
  return TRUE;
}

BOOL DrawLineMap(int iX, int iY, int iRadius) {
  if (iRadius == 1)
    Engine.DDraw.SetPixel(Game.Landscape.Map, iX, iY, DrawLineCol);
  else
    Engine.DDraw.DrawCircle(Game.Landscape.Map, iX, iY, iRadius, DrawLineCol);
  return TRUE;
}

BOOL C4Landscape::DrawLine(int iX1, int iY1, int iX2, int iY2, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT) {
  switch (Mode) {
  // Dynamic: ignore
  case C4LSC_Dynamic:
    break;
  // Static: draw to map by material-texture-index, chunk-o-zoom to landscape
  case C4LSC_Static:
    // Get map color index by material-texture
    if (!GetMapColorIndex(szMaterial, szTexture, fIFT, DrawLineCol))
      return FALSE;
    // Draw to map
    int iRadius;
    iRadius = Max(2 * iGrade / MapZoom, 1);
    iX1 /= MapZoom;
    iY1 /= MapZoom;
    iX2 /= MapZoom;
    iY2 /= MapZoom;
    ForLine(iX1, iY1, iX2, iY2, &DrawLineMap, iRadius);
    // Update landscape
    int iUpX, iUpY, iUpWdt, iUpHgt;
    iUpX = Min(iX1, iX2) - iRadius - 1;
    iUpY = Min(iY1, iY2) - iRadius - 1;
    iUpWdt = Abs(iX2 - iX1) + 2 * iRadius + 2;
    iUpHgt = Abs(iY2 - iY1) + 2 * iRadius + 2;
    MapToLandscape(Map, iUpX, iUpY, iUpWdt, iUpHgt, Surface, Game.TextureMap, TRUE);
    break;
  // Exact: draw directly to landscape by color & pattern
  case C4LSC_Exact:
    // Set texture pattern & get material color
    if (!SetColorPattern(szMaterial, szTexture, fIFT, DrawLineCol))
      return FALSE;
    // Draw to landscape
    ForLine(iX1, iY1, iX2, iY2, &DrawLineLandscape, iGrade);
    // Clear pattern
    ClearColorPattern();
    break;
  }
  return TRUE;
}

BOOL C4Landscape::DrawBox(int iX1, int iY1, int iX2, int iY2, int iGrade, const char *szMaterial, const char *szTexture, BOOL fIFT) {
  BYTE byCol;
  switch (Mode) {
  // Dynamic: ignore
  case C4LSC_Dynamic:
    break;
  // Static: draw to map by material-texture-index, chunk-o-zoom to landscape
  case C4LSC_Static:
    // Get map color index by material-texture
    if (!GetMapColorIndex(szMaterial, szTexture, fIFT, byCol))
      return FALSE;
    // Draw to map
    iX1 /= MapZoom;
    iY1 /= MapZoom;
    iX2 /= MapZoom;
    iY2 /= MapZoom;
    Engine.DDraw.DrawBox(Map, Min(iX1, iX2), Min(iY1, iY2), Max(iX1, iX2), Max(iY1, iY2), byCol);
    // Update landscape
    MapToLandscape(Map, Min(iX1, iX2) - 1, Min(iY1, iY2) - 1, Abs(iX1 - iX2) + 3, Abs(iY1 - iY2) + 3, Surface, Game.TextureMap, TRUE);
    break;
  // Exact: draw directly to landscape by color & pattern
  case C4LSC_Exact:
    // Set texture pattern & get material color
    if (!SetColorPattern(szMaterial, szTexture, fIFT, byCol))
      return FALSE;
    // Draw to landscape
    Engine.DDraw.DrawBox(Surface, Min(iX1, iX2), Min(iY1, iY2), Max(iX1, iX2), Max(iY1, iY2), byCol);
    // Clear pattern
    ClearColorPattern();
    break;
  }
  return TRUE;
}

BYTE C4Landscape::GetMapIndex(int iX, int iY) {
  if (!Map)
    return 0;
  return Engine.DDraw.GetPixel(Map, iX, iY);
}

int C4Landscape::GetMaterialCount(int iMaterial) {
  // Invalid parameter
  if (!MatValid(iMaterial))
    return -1;
  // No valid count available
  if (!MatCountValid)
    return -1;
  // Return count
  return MatCount[iMaterial];
}
