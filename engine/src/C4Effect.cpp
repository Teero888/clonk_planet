/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Some helper functions for various landscape effects */

#include <C4Include.h>

void Splash(int tx, int ty, int amt, C4Object *pByObj)
  {
  int cnt;	
  // Splash bubbles and liquid
  for (cnt=0; cnt<amt; cnt++)
    {
    BubbleOut(tx+Random(16)-8,ty+Random(16)-6);    
    if (GBackLiquid(tx,ty))
    Game.PXS.Create(Game.Landscape.ExtractMaterial(tx,ty),
              tx,ty-10,
              (float)(Random(201)-100)/100.0,
              (float)-Random(250)/100.0); 
    }
  // Splash sound
  if (amt>=20) SoundEffect("Splash2",0,100,pByObj);
  else if (amt>1) SoundEffect("Splash1",0,100,pByObj);
  }

void BubbleOut(int tx, int ty)
  {
	// User-defined smoke level
	int SmokeLevel = Config.Graphics.SmokeLevel; 
	// Network active: enforce fixed smoke level
	if (Game.Network.Active) SmokeLevel=150;
	// Enough bubbles out there already
	if (Game.Objects.ObjectCount(C4Id("FXU1")) >= SmokeLevel) return;
	// Create bubble
	Game.CreateObject(C4Id("FXU1"),NO_OWNER,tx,ty);
  }

void Smoke(int tx, int ty, int level)
  {
	// User-defined smoke level
	int SmokeLevel = Config.Graphics.SmokeLevel; 
	// Network active: enforce fixed smoke level
	if (Game.Network.Active) SmokeLevel=150;
	// Enough smoke out there already
	if (Game.Objects.ObjectCount(C4Id("FXS1")) >= SmokeLevel) return;
	// Create smoke
	level=BoundBy(level,3,32);
	C4Object *pObj;
	if (pObj = Game.CreateObjectConstruction(C4Id("FXS1"),NO_OWNER,tx,ty,FullCon*level/32))
		pObj->Call(PSF_Activate);
  }

void Explosion(int tx, int ty, int level, C4Object *inobj, int iCausedBy, C4Object *pByObj) 
  {
  int grade=BoundBy((level/10)-1,1,3);
  // Sound
  SCopy("Blast*",OSTR); SReplaceChar(OSTR,'*','0'+grade);
  SoundEffect(OSTR,0,100,pByObj);
	// Check blast containment
  C4Object *container=inobj;
  while (container && !container->Def->ContainBlast) container=container->Contained;
  // Uncontained blast effects
  if (!container)
    {
		// Incinerate landscape
    if (!Game.Landscape.Incinerate(tx,ty))
			if (!Game.Landscape.Incinerate(tx,ty-10))
				if (!Game.Landscape.Incinerate(tx-5,ty-5))
					Game.Landscape.Incinerate(tx+5,ty-5);
		// Blast free landscape
		Game.Landscape.BlastFree(tx,ty,level,grade);
		// Create blast object
		C4Object *pBlast;
		if (pBlast = Game.CreateObjectConstruction(C4Id("FXB1"),iCausedBy,tx,ty+level,FullCon*level/20))
			pBlast->Call(PSF_Activate);
    }
  // Blast objects
  Game.BlastObjects(tx,ty,level,inobj,iCausedBy);
  if (container!=inobj) Game.BlastObjects(tx,ty,level,container,iCausedBy);
  }

