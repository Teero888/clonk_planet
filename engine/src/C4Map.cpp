/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Create map from dynamic landscape data in scenario */

#include <C4Include.h>

C4MapCreator::C4MapCreator()
  {
  Reset();
  }

void C4MapCreator::Reset()
  {
  MapIFT=128;
  MapBuf=NULL;
  MapWdt=MapHgt=MapBufWdt=0;
  Exclusive=-1; 
  }

void C4MapCreator::SetPix(int x, int y, BYTE col)
  {
  // Safety
  if (!Inside(x,0,MapWdt-1) || !Inside(y,0,MapHgt-1)) return;
  // Exclusive 
  if (Exclusive>-1) if (GetPix(x,y)!=Exclusive) return;
  // Set pix
  MapBuf[MapBufWdt*y+x]=col;
  }

void C4MapCreator::SetSpot(int x, int y, int rad, BYTE col)
  {
  int ycnt,xcnt,lwdt,dpy;
  for (ycnt=-rad; ycnt<=rad; ycnt++)
    {
    lwdt= (int) sqrt(rad*rad-ycnt*ycnt); dpy=y+ycnt;
    for (xcnt=-lwdt; xcnt<lwdt+(lwdt==0); xcnt++)
      SetPix(x+xcnt,dpy,col);
    }
  }

void C4MapCreator::DrawLayer(int x, int y, int size, BYTE col)
  {
  int cnt,cnt2;
  for (cnt=0; cnt<size; cnt++)
    {
    x+=Random(9)-4; y+=Random(3)-1;
    for (cnt2=Random(3); cnt2<5; cnt2++)
      { SetPix(x+cnt2,y,col); SetPix(x+cnt2+1,y+1,col); }
    }
  }

BYTE C4MapCreator::GetPix(int x, int y)
  {
  // Safety
  if (!Inside(x,0,MapWdt-1) || !Inside(y,0,MapHgt-1)) return 0;
  // Get pix
  return MapBuf[MapBufWdt*y+x];
  }

void C4MapCreator::Create(BYTE *bypBuffer, int iBufWdt,
												  C4SLandscape &rLScape, C4TextureMap &rTexMap,
													BOOL fLayers, int iPlayerNum)
  {
  double pi = 3.1415926535;
  double fullperiod= 20.0 * pi;
  BYTE ccol;
  int cx,cy;
	char szEarth[C4M_MaxDefName+1];
	char szLiquid[C4M_MaxDefName+1];
	char szLayer[C4M_MaxDefName+1];
  
  // Safeties
  if (!bypBuffer) return;
  iPlayerNum=BoundBy(iPlayerNum,1,C4S_MaxPlayer);
  
  // Set creator variables
  MapBuf=bypBuffer; 
  MapBufWdt=iBufWdt;
	rLScape.GetMapSize(MapWdt,MapHgt,iPlayerNum);
  
  // Reset map (0 is sky)
  ZeroMem(MapBuf,MapBufWdt*MapHgt);
 
  // Surface
	SCopy(rLScape.Material,szEarth,C4M_MaxDefName); 
	if (!SCharCount('-',szEarth)) SAppend("-Smooth",szEarth);
  ccol=rTexMap.GetIndex(szEarth)+MapIFT;
  float amplitude= (float) rLScape.Amplitude.Evaluate();
  float phase=     (float) rLScape.Phase.Evaluate();
  float period=    (float) rLScape.Period.Evaluate();
  if (rLScape.MapPlayerExtend) period *= iPlayerNum;
  float natural=   (float) rLScape.Random.Evaluate();
  int level0=    Min(MapWdt,MapHgt)/2;
  int maxrange=  level0*3/4;
  double cy_curve,cy_natural; // -1.0 - +1.0 !
  
  double rnd_cy,rnd_tend; // -1.0 - +1.0 !
  rnd_cy= (double) (Random(2000+1)-1000)/1000.0;
  rnd_tend= (double) (Random(200+1)-100)/20000.0;
  
  for (cx=0; cx<MapWdt; cx++)
    {

    rnd_cy+=rnd_tend;   
    rnd_tend+= (double) (Random(100+1)-50)/10000;
    if (rnd_tend>+0.05) rnd_tend=+0.05;
    if (rnd_tend<-0.05) rnd_tend=-0.05;
    if (rnd_cy<-0.5) rnd_tend+=0.01;
    if (rnd_cy>+0.5) rnd_tend-=0.01;

    cy_natural=rnd_cy*natural/100.0;
    cy_curve=sin(fullperiod*period/100.0*(float)cx/(float)MapWdt
                 +2.0*pi*phase/100.0) * amplitude/100.0;            
    
    cy=level0+BoundBy((int)((float)maxrange*(cy_curve+cy_natural)),
                      -maxrange,+maxrange);
                  

    SetPix(cx,cy,ccol);
    }
  
  // Raise bottom to surface
  ccol=rTexMap.GetIndex(szEarth)+MapIFT;
  for (cx=0; cx<MapWdt; cx++)
    for (cy=MapHgt-1; (cy>=0) && !GetPix(cx,cy); cy--)
      SetPix(cx,cy,ccol);

  // Raise liquid level
  Exclusive=0;
	SCopy(rLScape.Liquid,szLiquid,C4M_MaxDefName); 
	if (!SCharCount('-',szLiquid)) SAppend("-Smooth",szLiquid);
  ccol=rTexMap.GetIndex(szLiquid);
  int wtr_level=rLScape.LiquidLevel.Evaluate();
  for (cx=0; cx<MapWdt; cx++)
    for (cy=MapHgt*(100-wtr_level)/100; cy<MapHgt; cy++)
      SetPix(cx,cy,ccol);
  Exclusive=-1;

	// Layers
  if (fLayers)
    {

		// Base material
		Exclusive=rTexMap.GetIndex(szEarth)+MapIFT;

		int cnt,clayer,layer_num,sptx,spty;

		// Process layer name list
		for (clayer=0; clayer<C4MaxNameList; clayer++)
			if (rLScape.Layers.Name[clayer][0])
				{
				// Get layer mat def
				SCopy(rLScape.Layers.Name[clayer],szLayer,C4M_MaxDefName);
				if (!SCharCount('-',szLayer)) SAppend("-Rough",szLayer);
				// Draw layers
				ccol=rTexMap.GetIndex(szLayer)+MapIFT;
				layer_num=rLScape.Layers.Count[clayer]; 
				layer_num=layer_num*MapWdt*MapHgt/15000;
				for (cnt=0; cnt<layer_num; cnt++)
					{
					// Place layer
					sptx=Random(MapWdt); 
					for (spty=0; (spty<MapHgt) && (GetPix(sptx,spty)!=Exclusive); spty++);
					spty+=5+Random((MapHgt-spty)-10);

					DrawLayer(sptx,spty,Random(15),ccol);

					}
				}

		Exclusive=-1;

    }

  }

BOOL C4MapCreator::Load(
				BYTE **pbypBuffer, 
				int &rBufWdt, int &rMapWdt, int &rMapHgt,
				C4Group &hGroup, const char *szEntryName,
				C4TextureMap &rTexMap)
	{
	BOOL fOwnBuf=FALSE;

	CBitmap256Info Bmp;

	// Access entry in group, read bitmap info
	if (!hGroup.AccessEntry(szEntryName)) return FALSE;
	if (!hGroup.Read(&Bmp,sizeof(Bmp))) return FALSE;
	if (!Bmp.Valid()) return FALSE;
	if (!hGroup.Advance(Bmp.FileBitsOffset())) return FALSE;
	
	// If buffer is present, check for sufficient size
	if (*pbypBuffer)
		{
		if ((Bmp.Info.biWidth>rMapWdt)
		 || (Bmp.Info.biHeight>rMapHgt) ) return FALSE;
		}
	// Else, allocate buffer, set sizes
	else
		{
		rMapWdt = Bmp.Info.biWidth;
		rMapHgt = Bmp.Info.biHeight;
		rBufWdt = rMapWdt; DWordAlign(rBufWdt);
		if (!(*pbypBuffer = new BYTE [rBufWdt*rMapHgt]))
			return FALSE;
		fOwnBuf=TRUE;
		}

	// Read bits to buffer
	for (int cline=Bmp.Info.biHeight-1; cline>=0; cline--)
		if (!hGroup.Read(*pbypBuffer+rBufWdt*cline,rBufWdt))
			{ if (fOwnBuf) delete [] *pbypBuffer; return FALSE; }

	// Validate texture indices
	MapBuf=*pbypBuffer;
	MapBufWdt=rBufWdt;
	MapWdt=rMapWdt; MapHgt=rMapHgt;
	ValidateTextureIndices(rTexMap);

	return TRUE;
	}

void C4MapCreator::ValidateTextureIndices(C4TextureMap &rTextureMap)
	{
	int iX,iY;
	for (iY=0; iY<MapHgt; iY++)
		for (iX=0; iX<MapWdt; iX++)
			if (!rTextureMap.GetMaterialTexture(GetPix(iX,iY)))
				SetPix(iX,iY,0);
	}