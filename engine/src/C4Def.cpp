/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Object definition */

#include <C4Include.h>

//----------------------------- C4Def Compilation ----------------------------------------

#define offsC4D(x) offsetof(C4DefCore,x)
#define offsC4A(x) offsetof(C4ActionDef,x)

C4CompilerValue C4CR_DefCore[] = {

{	"DefCore", C4CV_Section, 0,0 },
{ "id",									C4CV_Id,			offsC4D(id),											1									},
{ "Version",						C4CV_Integer, offsC4D(rC4XVer1),								3									},
{ "Name",								C4CV_String,	offsC4D(Name),										C4D_MaxName				},
{ "Category",						C4CV_Integer, offsC4D(Category),								1									},
{ "MaxUserSelect",			C4CV_Integer, offsC4D(MaxUserSelect),						1									},
{ "Timer",							C4CV_Integer, offsC4D(Timer),										1									},
{ "TimerCall",					C4CV_String,	offsC4D(TimerCall),								C4D_MaxIDLen			},
{ "ContactCalls",				C4CV_Integer, offsC4D(ContactFunctionCalls),		1									},
{ "Width",							C4CV_Integer, offsC4D(Shape.Wdt),								1									},
{ "Height",							C4CV_Integer, offsC4D(Shape.Hgt),								1									},
{ "Offset",							C4CV_Integer, offsC4D(Shape.x),									2									},
{ "Value",							C4CV_Integer, offsC4D(Value),										1									},
{ "Mass",								C4CV_Integer, offsC4D(Mass),										1									},
{ "Components",					C4CV_IdList,	offsC4D(Component),								0									},
{ "SolidMask",					C4CV_Integer, offsC4D(SolidMask),								6									},
{ "TopFace",						C4CV_Integer, offsC4D(TopFace),									6									},
{ "Picture",						C4CV_Integer, offsC4D(PictureRect),							4									},
{ "Vertices",						C4CV_Integer, offsC4D(Shape.VtxNum),						1									},
{ "VertexX",						C4CV_Integer, offsC4D(Shape.VtxX),							C4D_MaxVertex			},
{ "VertexY",						C4CV_Integer, offsC4D(Shape.VtxY),							C4D_MaxVertex			},
{ "VertexCNAT",					C4CV_Integer, offsC4D(Shape.VtxCNAT),						C4D_MaxVertex			},
{ "VertexFriction",			C4CV_Integer, offsC4D(Shape.VtxFriction),				C4D_MaxVertex			},
{ "Entrance",						C4CV_Integer, offsC4D(Entrance),								4									},
{ "Collection",					C4CV_Integer, offsC4D(Collection),							4									},
{ "CollectionLimit",		C4CV_Integer, offsC4D(CollectionLimit),					1									},
{ "FireTop",						C4CV_Integer, offsC4D(Shape.FireTop),						1									},
{ "Placement",					C4CV_Integer, offsC4D(Placement),								1									}, 
{ "Exclusive",					C4CV_Integer, offsC4D(Exclusive),								1									},
{ "ContactIncinerate",	C4CV_Integer, offsC4D(ContactIncinerate),				1									},
{ "BlastIncinerate",		C4CV_Integer, offsC4D(BlastIncinerate),					1									},
{ "BurnTo",							C4CV_Id,			offsC4D(BurnTurnTo),							1									},
{ "Base",								C4CV_Integer, offsC4D(CanBeBase),								1									},
{ "Line",								C4CV_Integer, offsC4D(Line),										1									},
{ "LineConnect",				C4CV_Integer, offsC4D(LineConnect),							1									},
{ "LineIntersect",			C4CV_Integer, offsC4D(LineIntersect),						1									},
{ "Prey",								C4CV_Integer, offsC4D(Prey),										1									},
{ "Edible",							C4CV_Integer, offsC4D(Edible),									1									},
{ "CrewMember",					C4CV_Integer, offsC4D(CrewMember),							1									},
{ "NoStandardCrew",			C4CV_Integer, offsC4D(NativeCrew),							1									},
{ "Growth",							C4CV_Integer, offsC4D(Growth),									1									},
{ "Rebuy",							C4CV_Integer, offsC4D(Rebuyable),								1									},
{ "Construction",				C4CV_Integer, offsC4D(Constructable),						1									},
{ "ConstructTo",				C4CV_Id,			offsC4D(BuildTurnTo),							1									},
{ "Grab",								C4CV_Integer, offsC4D(Pushable),								1									},
{ "GrabPutGet",					C4CV_Integer, offsC4D(GrabPutGet),							1									},
{ "Collectible",				C4CV_Integer, offsC4D(Carryable),								1									},
{ "Rotate",							C4CV_Integer, offsC4D(Rotateable),							1									},
{ "Chop",								C4CV_Integer, offsC4D(Chopable),								1									},
{ "Float",							C4CV_Integer, offsC4D(Float),										1									},
{ "ContainBlast",				C4CV_Integer, offsC4D(ContainBlast),						1									},
{ "ColorByOwner",				C4CV_Integer, offsC4D(ColorByOwner),						1									},
{ "ColorByMaterial",		C4CV_String,	offsC4D(ColorByMaterial),					C4M_MaxName				},
{ "HorizontalFix",			C4CV_Integer, offsC4D(NoHorizontalMove),				1									},
{ "BorderBound",				C4CV_Integer, offsC4D(BorderBound),							1									},
{ "LiftTop",						C4CV_Integer, offsC4D(LiftTop),									1									},
{ "UprightAttach",			C4CV_Integer, offsC4D(UprightAttach),						1									},
{ "StretchGrowth",			C4CV_Integer, offsC4D(GrowthType),							1									},
{ "Basement",						C4CV_Integer, offsC4D(Basement),								1									},
{ "NoBurnDecay",				C4CV_Integer, offsC4D(NoBurnDecay),							1									},
{ "IncompleteActivity", C4CV_Integer, offsC4D(IncompleteActivity),			1									},
{ "AttractLightning",		C4CV_Integer, offsC4D(AttractLightning),				1									},
{ "Oversize",						C4CV_Integer, offsC4D(Oversize),								1									},
{ "Fragile",						C4CV_Integer, offsC4D(Fragile),									1									},
{ "Explosive",					C4CV_Integer, offsC4D(Explosive),								1									},
{ "Projectile",					C4CV_Integer, offsC4D(Projectile),							1									},
{ "NoPushEnter",				C4CV_Integer, offsC4D(NoPushEnter),							1									},
{ "DragImagePicture",		C4CV_Integer, offsC4D(DragImagePicture),				1									},
{ "VehicleControl",			C4CV_Integer, offsC4D(VehicleControl),					1									},
{ "Pathfinder",					C4CV_Integer, offsC4D(Pathfinder),							1									},

{	"Physical", C4CV_Section, 0,0 },
{ "Energy",							C4CV_Integer, offsC4D(Physical.Energy),					1									},
{ "Breath",							C4CV_Integer, offsC4D(Physical.Breath),					1									},
{ "Walk",								C4CV_Integer, offsC4D(Physical.Walk),						1									},
{ "Jump",								C4CV_Integer, offsC4D(Physical.Jump),						1									},
{ "Scale",							C4CV_Integer, offsC4D(Physical.Scale),					1									},
{ "Hangle",							C4CV_Integer, offsC4D(Physical.Hangle),					1									},
{ "Dig",								C4CV_Integer, offsC4D(Physical.Dig),						1									},
{ "Swim",								C4CV_Integer, offsC4D(Physical.Swim),						1									},
{ "Throw",							C4CV_Integer, offsC4D(Physical.Throw),					1									},
{ "Push",								C4CV_Integer, offsC4D(Physical.Push),						1									},
{ "Fight",							C4CV_Integer, offsC4D(Physical.Fight),					1									},
{ "Magic",							C4CV_Integer, offsC4D(Physical.Magic),					1									},
{ "Float",							C4CV_Integer, offsC4D(Physical.Float),					1									},
{ "CanScale",						C4CV_Integer, offsC4D(Physical.CanScale),				1									},
{ "CanHangle",					C4CV_Integer, offsC4D(Physical.CanHangle),			1									},
{ "CanDig",							C4CV_Integer, offsC4D(Physical.CanDig),					1									},
{ "CanConstruct",				C4CV_Integer, offsC4D(Physical.CanConstruct),		1									},
{ "CanChop",						C4CV_Integer, offsC4D(Physical.CanChop),				1									},
{ "CanSwimDig",					C4CV_Integer, offsC4D(Physical.CanSwimDig),			1									},
{ "CorrosionResist",		C4CV_Integer, offsC4D(Physical.CorrosionResist),1									},
{ "BreatheWater",				C4CV_Integer, offsC4D(Physical.BreatheWater),		1									},

{ NULL,C4CV_End,0,0 }   };


C4CompilerValue C4CR_ActMap[] = {

{	"Action", C4CV_Section, 0,0 },
{ "Name",								C4CV_String,	offsC4A(Name),									C4D_MaxIDLen			},
{ "Procedure",					C4CV_String,	offsC4A(ProcedureName),					C4D_MaxIDLen			},
{ "Directions",         C4CV_Integer, offsC4A(Directions),						1									},	
{ "Length",							C4CV_Integer, offsC4A(Length),								1									},
{ "Attach",							C4CV_Integer, offsC4A(Attach),								1									},
{ "Delay",							C4CV_Integer, offsC4A(Delay),									1									},
{ "Facet",							C4CV_Integer, offsC4A(Facet),									6									},
{ "FacetBase",					C4CV_Integer, offsC4A(FacetBase),							1									},
{ "FacetTopFace",				C4CV_Integer, offsC4A(FacetTopFace),					1									},
{ "FacetTargetStretch",	C4CV_Integer, offsC4A(FacetTargetStretch),		1									},
{ "NextAction",					C4CV_String,	offsC4A(NextActionName),				C4D_MaxIDLen			},
{ "NoOtherAction",			C4CV_Integer, offsC4A(NoOtherAction),					1									},
{ "StartCall",					C4CV_String,	offsC4A(StartCall),							C4D_MaxIDLen			},
{ "EndCall",						C4CV_String,	offsC4A(EndCall),								C4D_MaxIDLen			},
{ "PhaseCall",					C4CV_String,	offsC4A(PhaseCall),							C4D_MaxIDLen			},
{ "Sound",							C4CV_String,	offsC4A(Sound),									C4D_MaxIDLen			},
{ "ObjectDisabled",			C4CV_Integer, offsC4A(Disabled),							1									},
{ "DigFree",						C4CV_Integer, offsC4A(DigFree),								1									},
{ "EnergyUsage",				C4CV_Integer, offsC4A(EnergyUsage),						1									},
{ "InLiquidAction",			C4CV_String,	offsC4A(InLiquidAction),				C4D_MaxIDLen			},
{ "Reverse",						C4CV_Integer, offsC4A(Reverse),								1									},

{ NULL,C4CV_End,0,0 }   };



//-------------------------- Default Action Procedures --------------------------------------

const int C4D_MaxDFA=18;

const char *ProcedureName[C4D_MaxDFA]={ "WALK",
																				"FLIGHT",
																				"KNEEL",
																				"SCALE",
																				"HANGLE",
																				"DIG",
																				"SWIM",
																				"THROW",
																				"BRIDGE",
																				"BUILD",
																				"PUSH",
																				"CHOP",
																				"LIFT",
																				"FLOAT",
																				"ATTACH",
																				"FIGHT",
																				"CONNECT",
																				"PULL"
																			};

//-------------------------------- C4ActionDef --------------------------------------------

C4ActionDef::C4ActionDef()
	{
	Default();
	}

void C4ActionDef::Default()
	{
  ZeroMem(this,sizeof(C4ActionDef));
  Procedure=DFA_NONE;
  NextAction=ActIdle;
  Directions=1;
  Length=1;
  Delay=10000;
  FacetBase=0;
	}

//--------------------------------- C4DefCore ----------------------------------------------

C4DefCore::C4DefCore()
	{
	Default();
	}

void C4DefCore::Default()
  {
	rC4XVer1=rC4XVer2=rC4XVer3=0;
  SCopy("Undefined",Name);
	Physical.Default();
  Shape.Default();
  Entrance.Default();
  Collection.Default();
  PictureRect.Default();
  SolidMask.Default();
  TopFace.Default(); 
  Component.Default();
	BurnTurnTo=C4ID_None; 
	BuildTurnTo=C4ID_None;
	TimerCall[0]=0;
	Timer=35;
	ColorByMaterial[0]=0;
	GrowthType=0;
	Basement=0; 
	CanBeBase=0;
	CrewMember=0;
	NativeCrew=0;
	Mass=0;
	Value=0;
	Exclusive=0; 
	Category=0;          
	Growth=0; 
	Rebuyable=0; 
	ContactIncinerate=0; 
	BlastIncinerate=0; 
	Constructable=0;
	Pushable=0;
	Carryable=0;
	Rotateable=0;
	Chopable=0;
	Float=0;
	ColorByOwner=0;
	NoHorizontalMove=0;
	BorderBound=0; 
	LiftTop=0;
	CollectionLimit=0;
	GrabPutGet=0; 
	ContainBlast=0;
	UprightAttach=0;
	ContactFunctionCalls=0;
	MaxUserSelect=0;
	Line=0;
	LineConnect=0;
	LineIntersect=0;
	NoBurnDecay=0;
	IncompleteActivity=0;
	Placement=0;
	Prey=0;
	Edible=0;
	BuildAtOnce=0;
	AttractLightning=0;
	Oversize=0;
	Fragile=0;
	NoPushEnter=0;
	Explosive=0;
	Projectile=0;
	DragImagePicture=0;
	VehicleControl=0;
	Pathfinder=0;
	}

BOOL C4DefCore::Load(C4Group &hGroup)
	{
	BYTE *pSource;
	if (hGroup.LoadEntry(C4CFN_DefCore,&pSource,NULL,1))
		{
		if (!Compile((char*)pSource))
			{ delete [] pSource; return FALSE; }
		delete [] pSource;

		// Adjust category: C4D_CrewMember by CrewMember flag
		if (CrewMember) Category|=C4D_CrewMember;

		// Adjust picture rect
		if ((PictureRect.Wdt==0) || (PictureRect.Hgt==0))
			PictureRect.Set(0,0,Shape.Wdt,Shape.Hgt);

		return TRUE;
		}
	return FALSE;
	}

BOOL C4DefCore::Save(C4Group &hGroup)
	{
	char *Buffer; int BufferSize;
	if (! Decompile(&Buffer,&BufferSize) )
		return FALSE;
	if (! hGroup.Add(C4CFN_DefCore,Buffer,BufferSize,FALSE,TRUE) )
		{ delete [] Buffer; return FALSE; }
	return TRUE;
	}

BOOL C4DefCore::Compile(const char *szSource)
	{
	C4Compiler Compiler;
	Default();
	return Compiler.CompileStructure(C4CR_DefCore,
																	 szSource,
																	 this);
	}

BOOL C4DefCore::Decompile(char **ppOutput, int *ipSize)
	{
	C4Compiler Compiler;
	C4DefCore dC4D;
	*ppOutput=NULL;
	return Compiler.DecompileStructure(C4CR_DefCore,
																		 this,
																		 &dC4D,
																		 ppOutput,
																		 ipSize);
	}

//-------------------------------- C4Def -------------------------------------------------------

C4Def::C4Def()
  {
	Default();
	}

void C4Def::Default()
	{
	C4DefCore::Default();

  Picture=NULL;
  Image=NULL;
  for (int cnt=0; cnt<C4MaxColor; cnt++)
    {
    Bitmap[cnt]=NULL;
    MainFace[cnt].Set(NULL,0,0,0,0);
    }
  Desc=NULL;
  ActNum=0;
  ActMap=NULL;
	ControlFlag=ContainedControlFlag=ActivationControlFlag=0;
  Next=NULL;
  Temporary=FALSE;
	Maker[0]=0;
	Filename[0]=0;
	Creation=0;
	Script.Default();
  }

C4Def::~C4Def()
	{
	Clear();
	}

void C4Def::Clear()
  {
  if (Picture) DeleteBitmap(Picture); Picture=NULL;
  if (Image) DeleteBitmap(Image); Image=NULL;

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
  for (int cnt=0; cnt<C4MaxColor; cnt++)
    if (Bitmap[cnt]) { DestroySurface(Bitmap[cnt]); Bitmap[cnt]=NULL; }

	Script.Clear();
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  

  if (ActMap) delete [] ActMap; ActMap=NULL;
  if (Desc)   delete [] Desc; Desc=NULL;

  }

BOOL C4Def::Load(C4Group &hGroup,
                 DWORD dwLoadWhat, const char *szLanguage,
								 C4SoundSystem *pSoundSystem)
  {
  int binlen;
  char element_name[30+1];

	// Store filename, maker, creation
	SCopy(hGroup.GetFullName(),Filename);
	SCopy(hGroup.GetMaker(),Maker,C4MaxName);
	Creation = hGroup.GetCreation();

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// Verbose log filename
	if (Config.Graphics.VerboseObjectLoading>=3)
		Log(hGroup.GetFullName());
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Read DefCore
	if (!C4DefCore::Load(hGroup)) 
		{
		
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		// Read sounds even if not a valid def (for pure c4d sound folders)
		if (dwLoadWhat & C4D_Load_Sounds)
			if (pSoundSystem)
				pSoundSystem->LoadEffects(hGroup);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		return FALSE;
		}

#ifdef C4ENGINE
  // Read surface bitmap
  if (dwLoadWhat & C4D_Load_Bitmap)    
    if ( !hGroup.AccessEntry(C4CFN_DefGraphics)
      || !(Bitmap[0]=GroupReadSurface(hGroup)) )
        return FALSE;
#endif

  // Read picture section
  if (dwLoadWhat & C4D_Load_Picture)
    // Load picture section
    if ( !hGroup.AccessEntry(C4CFN_DefGraphics)
      || !hGroup.ReadDDBSection(&Picture,NULL,PictureRect.x,PictureRect.y,PictureRect.Wdt,PictureRect.Hgt) )
        return FALSE;
  
  // Read image picture section
  if (dwLoadWhat & C4D_Load_Image)
    // Load image picture section
    if ( !hGroup.AccessEntry(C4CFN_DefGraphics)
      || !hGroup.ReadDDBSection(&Image,NULL,PictureRect.x,PictureRect.y,PictureRect.Wdt,PictureRect.Hgt,32,32,TRUE) )
        return FALSE;
  
  // Read ActMap
  if (dwLoadWhat & C4D_Load_ActMap)
		if (!LoadActMap(hGroup))
      return FALSE;

  // Read script
  if (dwLoadWhat & C4D_Load_Script)
		{
		// Load script
		Script.Load("Script",hGroup,C4CFN_Script,szLanguage,id);
		// Scan control flags
		ControlFlag = Script.GetControlFlag(PSF_Control);
		ContainedControlFlag = Script.GetControlFlag(PSF_ContainedControl);
		ActivationControlFlag = Script.GetControlFlag(PSF_Activate);
		}

	// Read name
	BYTE *bypNames; const char *cpName; 
	if (hGroup.LoadEntry(C4CFN_DefNames,&bypNames,NULL,1))
		{
		SCopy(szLanguage,element_name,2); SAppend(":",element_name);
		if (cpName = SSearch((char*)bypNames,element_name))
			SCopyUntil(cpName,Name,0x0D,C4D_MaxName);
		delete [] bypNames;
		}

  // Read desc
  if (dwLoadWhat & C4D_Load_Desc)
    {
    sprintf(element_name,C4CFN_DefDesc,szLanguage);
    if (hGroup.AccessEntry(element_name,&binlen))
      {
      if ( !(Desc=new char [binlen+1])
        || !hGroup.Read(Desc,binlen) )
          return FALSE;
      Desc[binlen]=0;
      SReplaceChar(Desc,0x09,' '); // Remove tabs
      SReplaceChar(Desc,0x0D,' '); // Remove binary line feeds
      SReplaceChar(Desc,0x0A,' '); 
      }
    }

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	// Read sounds
	if (dwLoadWhat & C4D_Load_Sounds)
		if (pSoundSystem)
			pSoundSystem->LoadEffects(hGroup);

	int cnt;

  // Bitmap post-load settings
  if (Bitmap[0])
    {
    // Create owner color bitmaps
    if (ColorByOwner)
      {
      // Create additionmal bitmaps
      for (cnt=1; cnt<C4MaxColor; cnt++)
        if (!(Bitmap[cnt]=DuplicateSurface(Bitmap[0])))
          return FALSE;
			// Colorize bitmaps
      for (cnt=0; cnt<C4MaxColor; cnt++) 
        Engine.DDraw.SurfaceShiftColorRange(Bitmap[cnt],32,39,+8*cnt);
      }
    // Set MainFaces
    for (cnt=0; cnt<C4MaxColor; cnt++)
      {
      MainFace[cnt].Set(Bitmap[cnt],0,0,Shape.Wdt,Shape.Hgt);
      }
    }

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  // Temporary flag
  if (dwLoadWhat & C4D_Load_Temporary) Temporary=TRUE;

  return TRUE;
  }

BOOL C4Def::LoadActMap(C4Group &hGroup)
	{
	int actnum;

	// New format
	BYTE *pSource;
	if (hGroup.LoadEntry(C4CFN_DefActMap,&pSource,NULL,1))
		{
		if ( !(actnum = SCharCount('[',(char*)pSource))
			|| !(ActMap = new C4ActionDef [actnum])
			|| !CompileActMap((char*)pSource,ActMap,actnum) )
				{ delete [] pSource; return FALSE; }
		delete [] pSource;
		ActNum=actnum;
		CrossMapActMap();
		return TRUE;
		}	

	// No act map in group: okay
	return TRUE;
	}

void C4Def::CrossMapActMap()
	{
	int cnt,cnt2;
	for (cnt=0; cnt<ActNum; cnt++)
		{
		// Map standard procedures
		ActMap[cnt].Procedure=DFA_NONE;
		for (cnt2=0; cnt2<C4D_MaxDFA; cnt2++)
			if (SEqual(ActMap[cnt].ProcedureName,ProcedureName[cnt2]))
				ActMap[cnt].Procedure=cnt2;
    // Map next action
    if (ActMap[cnt].NextActionName[0])
      {
      if (SEqualNoCase(ActMap[cnt].NextActionName,"Hold"))
        ActMap[cnt].NextAction=ActHold;
      else
        for (cnt2=0; cnt2<ActNum; cnt2++)
          if (SEqual(ActMap[cnt].NextActionName,ActMap[cnt2].Name))
            ActMap[cnt].NextAction=cnt2;
      }
    // Check act calls
    if (SEqualNoCase(ActMap[cnt].StartCall,"None"))	ActMap[cnt].StartCall[0]=0;
    if (SEqualNoCase(ActMap[cnt].PhaseCall,"None"))	ActMap[cnt].PhaseCall[0]=0;
    if (SEqualNoCase(ActMap[cnt].EndCall,"None"))	ActMap[cnt].EndCall[0]=0;
		}
	}

BOOL C4Def::ColorizeByMaterial(C4MaterialMap &rMats, BYTE bGBM)
	{
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    printf("  ColorizeByMaterial: %s (%s)\n", Name, Filename);
  int mat;
	if (!Bitmap[0]) return FALSE; // (Bitmap[0] only)
	if (!ColorByMaterial[0]) return FALSE;
	mat=rMats.Get(ColorByMaterial);
	if (mat==MNone) return FALSE;
  printf("    Colorizing by material %d: %s\n", mat, ColorByMaterial);
  Engine.DDraw.SurfaceAllowColor(Bitmap[0],bGBM+C4M_ColsPerMat*mat,bGBM+C4M_ColsPerMat*mat+C4M_ColsPerMat-1,TRUE);

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	return TRUE;
	}

void C4Def::Draw(C4Facet &cgo, BOOL fSelected, int iColor)
	{
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	if (!Inside(iColor,0,C4MaxColor-1)) return;

	C4Facet fctPicture;
  fctPicture.Set(Bitmap[iColor],PictureRect.x,PictureRect.y,PictureRect.Wdt,PictureRect.Hgt);

	if (fSelected)
		Engine.DDraw.DrawBox(cgo.Surface,cgo.X,cgo.Y,cgo.X+cgo.Wdt-1,cgo.Y+cgo.Hgt-1,CRed);

	fctPicture.Draw(cgo);

#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	}

BOOL C4Def::DecompileActMap(C4ActionDef *pC4A, int iActNum, 
														char **ppOutput, 
														int *ipSize)
	{
	C4Compiler Compiler;
	C4ActionDef dC4A;
	*ppOutput = new char [10000];	
	*ppOutput[0]=0;
	for (int cnt=0; cnt<iActNum; cnt++)
		Compiler.DecompileStructure(C4CR_ActMap,
																&(pC4A[cnt]),
																&dC4A,
																ppOutput,
																ipSize);
	SAppend(EndOfFile,*ppOutput);
	*ipSize = SLen(*ppOutput);
	return TRUE;
	}

BOOL C4Def::CompileActMap(const char *szSource, 
					 							  C4ActionDef *pC4A, int iActNum)
	{
	C4Compiler Compiler;
	char actbuf[1000+2];
	for (int cnt=0; cnt<iActNum; cnt++)
		{
		actbuf[0]='['; SCopySegment(szSource,cnt+1,actbuf+1,'[',1000);
		pC4A[cnt].Default();
		Compiler.CompileStructure(C4CR_ActMap,
										 					actbuf,
															&(pC4A[cnt]));

		}
	return TRUE;
	}

int C4Def::ResolveIncludes(C4DefList &rDefs)
	{
	int iResult = Script.ResolveIncludes(rDefs);
	// Rebuild function data
	if (iResult)
		{
		Script.MakeFunctionTable();
		ControlFlag = Script.GetControlFlag(PSF_Control);
		ContainedControlFlag = Script.GetControlFlag(PSF_ContainedControl);
		ActivationControlFlag = Script.GetControlFlag(PSF_Activate);
		}
	return iResult;
	}

//--------------------------------- C4DefList ----------------------------------------------

C4DefList::C4DefList()
  {
	Default();
  }

C4DefList::~C4DefList()
  {
  Clear();
  }

int C4DefList::Load(C4Group &hGroup, DWORD dwLoadWhat, 
										const char *szLanguage, 
										C4SoundSystem *pSoundSystem,
										BOOL fOverload,
										BOOL fSearchMessage)
  {
	int iResult=0;
  C4Def *nDef;
  char szEntryname[_MAX_FNAME+1];
	C4Group hChild;
	BOOL fPrimaryDef=FALSE;
	BOOL fThisSearchMessage=FALSE;
  
	// This search message
	if (fSearchMessage)	
		if (SEqualNoCase(GetExtension(hGroup.GetName()),"c4d") 
		 || SEqualNoCase(GetExtension(hGroup.GetName()),"c4s")
		 || SEqualNoCase(GetExtension(hGroup.GetName()),"c4f"))
			{
			fThisSearchMessage=TRUE;
			fSearchMessage=FALSE;
			}

#ifdef C4ENGINE // Message - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		if (fThisSearchMessage) {	sprintf(OSTR,"%s...",GetFilename(hGroup.GetName())); Log(OSTR); }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	// Load primary definition
	if (nDef=new C4Def)
		if ( nDef->Load(hGroup,dwLoadWhat,szLanguage,pSoundSystem) && Add(nDef,fOverload) )
			{ iResult++; fPrimaryDef=TRUE; }
		else
			{ delete nDef; }

  // Load sub definitions
  hGroup.ResetSearch();
  printf("C4DefList::Load: searching sub-defs in %s\n", hGroup.GetName());
  while (hGroup.FindNextEntry(C4CFN_DefFiles,szEntryname))
  {
  printf("  Found sub-def entry: %s\n", szEntryname);
  	if (hChild.OpenAsChild(&hGroup,szEntryname))
  		{
  		iResult += Load(hChild,dwLoadWhat,szLanguage,pSoundSystem,fOverload,fSearchMessage);
  		hChild.Close();
  		}
  else { printf("  Failed to open sub-def as child: %s\n", szEntryname); }
  }
#ifdef C4ENGINE // Message - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		if (fThisSearchMessage) {	sprintf(OSTR,LoadResStr(IDS_PRC_DEFSLOADED),iResult); Log(OSTR); }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

  return iResult;
  }

int C4DefList::LoadFolderLocal( const char *szPath,
																DWORD dwLoadWhat, const char *szLanguage,
																C4SoundSystem *pSoundSystem,
																BOOL fOverload, char *sStoreName)
	{
	int iResult = 0;

	// Scan path for folder names
	int cnt,iBackslash,iDefs;
	char szFoldername[_MAX_PATH+1];
	for (cnt=0; (iBackslash=SCharPos('\\',szPath,cnt)) > -1; cnt++)
		{
		SCopy(szPath,szFoldername,iBackslash);
		// Load from parent folder
		if (SEqualNoCase(GetExtension(szFoldername),"c4f"))
			if (iDefs=Load(szFoldername,dwLoadWhat,szLanguage,pSoundSystem,fOverload))
				{
				iResult+=iDefs;
				// Add any folder containing defs to store list
				if (sStoreName)	{ SNewSegment(sStoreName); SAppend(szFoldername,sStoreName); }
				}
		}

	return iResult;
	}

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern BOOL C4EngineLoadProcess(const char *szMessage, int iProcess);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int C4DefList::Load(const char *szSearch,
                    DWORD dwLoadWhat, const char *szLanguage,
										C4SoundSystem *pSoundSystem,
										BOOL fOverload)
  {
  int iResult=0;

	// Empty
	if (!szSearch[0]) return iResult;

	// Segments
	char szSegment[_MAX_PATH+1];
	if (SCharCount(';',szSearch))
		{
		for (int cseg=0; SCopySegment(szSearch,cseg,szSegment,';',_MAX_PATH); cseg++)
			iResult += Load(szSegment,dwLoadWhat,szLanguage,pSoundSystem,fOverload);
		return iResult;
		}

	// Wildcard items
	if (SCharCount('*',szSearch))
		{
		struct _finddata_t fdt; intptr_t fdthnd;
		if ((fdthnd=_findfirst(szSearch,&fdt))>=0)
            {
		    do
			    {
			    iResult += Load(fdt.name,dwLoadWhat,szLanguage,pSoundSystem,fOverload);
			    }
		    while (_findnext(fdthnd,&fdt)==0);
		    _findclose(fdthnd);
            }
		return iResult;
		}

	// File specified with creation (currently not used)
	char szCreation[25+1];
	int iCreation=0;
	if (SCopyEnclosed(szSearch,'(',')',szCreation,25))
		{
		// Scan creation
		SClearFrontBack(szCreation);
		sscanf(szCreation,"%i",&iCreation);
		// Extract filename
		SCopyUntil(szSearch,szSegment,'(',_MAX_PATH);
		SClearFrontBack(szSegment);
		szSearch = szSegment;
		}

	// Load from specified file
	C4Group hGroup;
	if (!hGroup.Open(szSearch)) 
		{
		// Specified file not found (failure)
#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		sprintf(OSTR,LoadResStr(IDS_PRC_DEFNOTFOUND),szSearch);	Log(OSTR);
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		LoadFailure=TRUE;
		return iResult;
		}
	iResult += Load(hGroup,dwLoadWhat,szLanguage,pSoundSystem,fOverload,TRUE);
	hGroup.Close();

  return iResult;
  }

BOOL C4DefList::Add(C4Def *pDef, BOOL fOverload)
  {
  if (!pDef) return FALSE;

  // Check old def to overload
	C4Def *pLastDef = ID2Def(pDef->id);
	if (pLastDef && !fOverload) return FALSE;

#ifdef C4ENGINE // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	// Log overloaded def
	if (Config.Graphics.VerboseObjectLoading>=1)
		if (pLastDef)
			{	char ostr[250];	sprintf(ostr,LoadResStr(IDS_PRC_DEFOVERLOAD),pDef->Name,C4IdText(pLastDef->id)); Log(ostr); }
#endif // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  // Remove old def
	Remove(pDef->id);
  
	// Add new def
  pDef->Next=FirstDef;
  FirstDef=pDef;
	
	return TRUE;
  }

BOOL C4DefList::Remove(C4ID id)
  {
  C4Def *cdef,*prev;
  for (cdef=FirstDef,prev=NULL; cdef; prev=cdef,cdef=cdef->Next)
    if (cdef->id==id)
      {
      if (prev) prev->Next=cdef->Next;
      else FirstDef=cdef->Next;
      delete cdef;
      return TRUE;
      }
  return FALSE;
  }

void C4DefList::Remove(C4Def *def)
  {
  C4Def *cdef,*prev;
  for (cdef=FirstDef,prev=NULL; cdef; prev=cdef,cdef=cdef->Next)
    if (cdef==def)
      {
      if (prev) prev->Next=cdef->Next;
      else FirstDef=cdef->Next;
      delete cdef;
      return;
      }
  }

void C4DefList::Clear()
  { 
  C4Def *cdef,*next;
  for (cdef=FirstDef; cdef; cdef=next)
    {
    next=cdef->Next;
    delete cdef;
    }
  FirstDef=NULL;
  }

C4Def* C4DefList::ID2Def(C4ID id)
  {
  C4Def *cdef;
	if (id==C4ID_None) return NULL;
  for (cdef=FirstDef; cdef; cdef=cdef->Next)
    if (cdef->id==id)
      return cdef;
  return NULL;
  }

int C4DefList::GetIndex(C4ID id)
  {
  C4Def *cdef;
  int cindex;
  for (cdef=FirstDef,cindex=0; cdef; cdef=cdef->Next,cindex++)
    if (cdef->id==id) return cindex;
  return -1;
  }

int C4DefList::GetDefCount(DWORD dwCategory)
	{
  C4Def *cdef; int ccount=0;
  for (cdef=FirstDef; cdef; cdef=cdef->Next)
    if (cdef->Category & dwCategory)
      ccount++;
  return ccount;
	}

C4Def* C4DefList::GetDef(int iIndex, DWORD dwCategory)
  {
  C4Def *pDef; int iCurrentIndex;
  if (iIndex<0) return NULL;
  for (pDef=FirstDef,iCurrentIndex=-1; pDef; pDef=pDef->Next)
    if (pDef->Category & dwCategory)
      {
      iCurrentIndex++;
      if (iCurrentIndex==iIndex) return pDef;
      }
  return NULL;
  }

int C4DefList::RemoveTemporary()
  {
  C4Def *cdef,*prev,*next;
  int removed=0;
  for (cdef=FirstDef,prev=NULL; cdef; cdef=next)
    {
    next=cdef->Next;
    if (cdef->Temporary)
      {
      if (prev) prev->Next=next;
      else FirstDef=next;
      delete cdef;
      removed++;
      }
    else
      prev=cdef;
    }
  return removed;
  }

int C4DefList::CheckEngineVersion(int ver1, int ver2, int ver3)
	{
	int rcount=0;
  C4Def *cdef,*prev,*next;
  for (cdef=FirstDef,prev=NULL; cdef; cdef=next)
		{
		next=cdef->Next;
    if (cdef->rC4XVer1*100+cdef->rC4XVer2*10+cdef->rC4XVer3>ver1*100+ver2*10+ver3)
      {
      if (prev) prev->Next=cdef->Next;
      else FirstDef=cdef->Next;
      delete cdef;
			rcount++;
      }
		else prev=cdef;
		}
	return rcount;
	}

int C4DefList::ColorizeByMaterial(C4MaterialMap &rMats, BYTE bGBM)
	{
  C4Def *cdef;
	int rval=0;
  for (cdef=FirstDef; cdef; cdef=cdef->Next)
		if (cdef->ColorizeByMaterial(rMats,bGBM))
			rval++;
	return rval;
	}

void C4DefList::Draw(C4ID id, C4Facet &cgo, BOOL fSelected, int iColor)
	{
	C4Def *cdef = ID2Def(id);
	if (cdef) cdef->Draw(cgo,fSelected,iColor);
	}

void C4DefList::Default()
	{
  FirstDef=NULL;
	LoadFailure=FALSE;
	}

// Load scenario specified or all selected plus scenario & folder local

int C4DefList::LoadForScenario(const char *szScenario, 
															 const char *szSelection,
															 DWORD dwLoadWhat, const char *szLanguage, 
															 C4SoundSystem *pSoundSystem, BOOL fOverload)
	{
	int iDefs=0;
	char szSpecified[10 * _MAX_PATH+1];

	// User selected modules
	SCopy( szSelection, szSpecified );

	// Open scenario file & load core
	C4Group hScenario;
	C4Scenario C4S;
	if ( !hScenario.Open(szScenario)
		|| !C4S.Load(hScenario) )
			return 0;

	// Scenario definition specifications (override user selection)
	C4S.Definitions.GetModules(szSpecified);

	// Load specified
	iDefs += Load(szSpecified,dwLoadWhat,szLanguage,pSoundSystem,fOverload);

	// Load folder local
	iDefs += LoadFolderLocal(szScenario,dwLoadWhat,szLanguage,pSoundSystem,fOverload);

	// Load local
	iDefs += Load(hScenario,dwLoadWhat,szLanguage,pSoundSystem,fOverload);
	
	// Done
	return iDefs;
	}

BOOL C4DefList::Reload(C4Def *pDef, DWORD dwLoadWhat, const char *szLanguage, C4SoundSystem *pSoundSystem)
	{
	// Safety
	if (!pDef) return FALSE;
	// Clear def
	pDef->Clear(); // Assume filename is being kept
	// Reload def 
	C4Group hGroup;
	if (!hGroup.Open(pDef->Filename)) return FALSE;
	if (!pDef->Load( hGroup, dwLoadWhat, szLanguage, pSoundSystem )) return FALSE;
	hGroup.Close();
	// Resolve includes
	pDef->ResolveIncludes(*this);
	// Success
	return TRUE;
	}

void C4DefList::ResolveIncludes()
	{
  for (C4Def *pDef=FirstDef; pDef; pDef=pDef->Next)
		pDef->ResolveIncludes(*this);
	}