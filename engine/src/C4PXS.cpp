/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Pixel Sprite system for tiny bits of moving material */

#include <C4Include.h>

void C4PXS::Execute()
  {
	int ctx,cty,inmat;

	// Safety
	if (!MatValid(Mat)) 
    { Deactivate(); return; }

  // Out of bounds
  if ((x<0) || (x>=GBackWdt) || (y<-10) || (y>=GBackHgt))
    { Deactivate(); return; }
  
  // InMat
	if (MatValid(inmat=GBackMat((int)x,(int)y)))
		{
		// InMatConvert
		if (Game.Material.Map[Mat].InMatConvert==inmat)
			if (!MatValid(Mat=Game.Material.Map[Mat].InMatConvertTo))
				{ Deactivate(); return; }
		// Incindiary vs. Extinguisher
		if ((Game.Material.Map[Mat].Incindiary && Game.Material.Map[inmat].Extinguisher)
		 || (Game.Material.Map[Mat].Extinguisher && Game.Material.Map[inmat].Incindiary))
			{
			ClearBackPix((int)x,(int)y);
			Deactivate();
			if (!Rnd3()) Smoke((int)x,(int)y,3);
			if (!Rnd3()) SoundEffect("Pshshsh");
			return;
			}
		}

  // Gravity 
  ydir+=fixtof(GravAccel);

  // Speed bounds
  double maxspeed=(double)Game.Material.Map[Mat].MaxAirSpeed/10.0;
  if (xdir<-maxspeed) xdir=-maxspeed; if (xdir>+maxspeed) xdir=+maxspeed;
  if (ydir<-maxspeed) ydir=-maxspeed; if (ydir>+maxspeed) ydir=+maxspeed;
  
  // Movement target      
  double wdrift=   (double) Game.Weather.GetWind((int)x,(int)y)
								 / 20.0*(double) Game.Material.Map[Mat].WindDrift/100.0;

  double ctcox = x + xdir + wdrift;
  double ctcoy = y + ydir; 
  if (!Rnd3()) if (Game.Material.Map[Mat].WindDrift>=50) 
		// Wind updrift
    ctcoy -= wdrift; 
  
  // Movement & contact
  if (GBackDensity((int)ctcox,(int)ctcoy)<Game.Material.Map[Mat].Density)
		// Free movement
    { x=ctcox; y=ctcoy; return; } 

  // Contact: Stop
  xdir=ydir=0.0;
   
  // Incindiary mats smoke on contact
	if (Game.Material.Map[Mat].Incindiary)
		if (!Random(25)) Smoke( (int) x, (int) y, 4+Rnd3() );

  // Move by mat path/slide
  ctx = (int) x; cty = (int) y;
  if (Game.Landscape.FindMatSlide(ctx,cty,+1,Game.Material.Map[Mat].Density,Game.Material.Map[Mat].MaxSlide))
    { x=ctx; y=cty; return; }
      
	// Dead contact material reaction check: corrosion
	if (Game.Material.Map[Mat].Corrosive)
		{
		ctx = (int) x; cty = (int) y;
		int tmat=GBackMat(ctx,cty+1);
		if (MatValid(tmat))
			if (Random(100) < Game.Material.Map[Mat].Corrosive)
				if (Random(100) < Game.Material.Map[tmat].Corrode)
					{
					ClearBackPix(ctx,cty+1);
					if (!Random(5)) Smoke(ctx,cty,3+Random(3));
					if (!Random(20)) SoundEffect("Corrode");
					Deactivate(); return;    
					}
		}

	// Dead contact material reaction check: incineration
	if (Game.Material.Map[Mat].Incindiary)
		if (Game.Landscape.Incinerate((int)x,(int)y+1))
			{	Deactivate(); return; }

  // Else: dead
  Game.Landscape.InsertMaterial(Mat,(int)x,(int)y);
  Deactivate(); return;    

  }

void C4PXS::Deactivate()
  {
  Mat=MNone;
  }

C4PXSSystem::C4PXSSystem()
	{
	Default();
	}

C4PXSSystem::~C4PXSSystem()
	{
	Clear();
	}

void C4PXSSystem::Default()
	{
	Count=0;
	for (int cnt=0; cnt<PXSMaxChunk; cnt++)
		Chunk[cnt]=NULL;
	}

void C4PXSSystem::Clear()
	{
	for (int cnt=0; cnt<PXSMaxChunk; cnt++)
		{
		if (Chunk[cnt]) delete [] Chunk[cnt];
		Chunk[cnt]=NULL;
		}
	}

C4PXS* C4PXSSystem::New()
  {
  int cnt,cnt2;
  C4PXS *pxp;
  // Check chunks for available space
  for (cnt=0; cnt<PXSMaxChunk; cnt++)
    {
    // Create new chunk if necessary
    if (!Chunk[cnt]) 
      {
      if (!(Chunk[cnt]=new C4PXS[PXSChunkSize])) return NULL;
      for (cnt2=0,pxp=Chunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++) pxp->Mat=MNone;;
      }
    // Check this chunk for space
    for (cnt2=0,pxp=Chunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++)
      if (pxp->Mat==MNone)
        return pxp;
    }
  return NULL;
  }

BOOL C4PXSSystem::Create(int mat, double ix, double iy, double ixdir, double iydir)
  {
  C4PXS *pxp;
  if (!MatValid(mat)) return FALSE;
  if (!(pxp=New())) return FALSE;
  pxp->Mat=mat;
  pxp->x=ix; pxp->y=iy;
  pxp->xdir=ixdir; pxp->ydir=iydir;
  return TRUE;
  }

void C4PXSSystem::Execute()
  {
  int cchunk,cnt2;
  C4PXS *pxp;
  BOOL fChunkEmpty;

	// Execute all chunks
	Count=0;
  for (cchunk=0; cchunk<PXSMaxChunk; cchunk++)
    if (Chunk[cchunk])
      {
			// Execute chunk pxs, check for empty
      fChunkEmpty=TRUE;
      for (cnt2=0,pxp=Chunk[cchunk]; cnt2<PXSChunkSize; cnt2++,pxp++)
        if (pxp->Mat!=MNone)
          { 
          pxp->Execute(); 
					Count++;
          fChunkEmpty=FALSE; 
          }
      // Delete chunk
      if (fChunkEmpty) 
        { delete [] Chunk[cchunk]; Chunk[cchunk]=NULL; }
      }
  }

void C4PXSSystem::Draw(C4FacetEx &cgo)
  {
  int cnt,cnt2;
  C4PXS *pxp;
	BYTE *psurface;
	int ipitch;

	// Lock target surface
	if (!(psurface=LockSurface(cgo.Surface,ipitch))) return;

	// Draw PXS
  for (cnt=0; cnt<PXSMaxChunk; cnt++)
    if (Chunk[cnt])
      for (cnt2=0,pxp=Chunk[cnt]; cnt2<PXSChunkSize; cnt2++,pxp++)
        if (pxp->Mat!=MNone)
					if (Inside((int)pxp->x-cgo.TargetX,0,cgo.Wdt-1))
						if (Inside((int)pxp->y-cgo.TargetY,0,cgo.Hgt-1))
							psurface[ipitch*((int)pxp->y-cgo.TargetY+cgo.Y)+((int)pxp->x-cgo.TargetX+cgo.X)]
								= (BYTE) (Mat2PixCol(pxp->Mat) + SafeRandom(3));

	// Unlock target surface
	UnLockSurface(cgo.Surface);

  }

void C4PXSSystem::Cast(int mat, int num, int tx, int ty, int level)
  {
  int cnt;
  for (cnt=0; cnt<num; cnt++)
    Create(mat,
           tx,ty,
           (float)(Random(level+1)-level/2)/10.0,
           (float)(Random(level+1)-level)/10.0);
  }



BOOL C4PXSSystem::Save(C4Group &hGroup)
	{
	int cnt;

	// Check used chunk count
	int iChunks=0; 
	for (cnt=0; cnt<PXSMaxChunk; cnt++) 
		if (Chunk[cnt]) iChunks++;
	if (!iChunks)
		{
		hGroup.Delete(C4CFN_PXS);
		return TRUE;
		}

	// Save chunks to temp file
	CStdFile hTempFile;
	if (!hTempFile.Create(Config.AtTempPath(C4CFN_TempPXS))) 
		return FALSE;
	for (cnt=0; cnt<PXSMaxChunk; cnt++) 
		if (Chunk[cnt]) 
			if (!hTempFile.Write(Chunk[cnt],PXSChunkSize * sizeof(C4PXS)))
				return FALSE;
	if (!hTempFile.Close())
		return FALSE;

	// Move temp file to group
	if (!hGroup.Move( Config.AtTempPath(C4CFN_TempPXS),
										C4CFN_PXS )) 
		return FALSE;

	return TRUE;
	}

BOOL C4PXSSystem::Load(C4Group &hGroup)
	{
	int iBinSize,iChunkNum;
	int iChunkSize = PXSChunkSize * sizeof(C4PXS);
	if (!hGroup.AccessEntry(C4CFN_PXS,&iBinSize)) return FALSE;
	if ( ((iBinSize % iChunkSize)!=0) || ((iBinSize / iChunkSize)>PXSMaxChunk) ) return FALSE;
	iChunkNum = iBinSize / iChunkSize;
	for (int cnt=0; cnt<iChunkNum; cnt++)
		{
		if (!(Chunk[cnt]=new C4PXS[PXSChunkSize])) return FALSE;
		if (!hGroup.Read(Chunk[cnt],iChunkSize)) return FALSE;
		}
	return TRUE;
	}

void C4PXSSystem::Synchronize()
	{
	Count=0;
	}
