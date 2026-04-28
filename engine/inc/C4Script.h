/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Functions mapped by C4Script */

const int C4SCR_Access_Public			= 0,
					C4SCR_Access_Protected	= 1,
					C4SCR_Access_Private		= 2;
		
#define C4SCR_Public    "public"
#define C4SCR_Protected "protected"
#define C4SCR_Private   "private"

struct C4ScriptFnDef
  {
  const char* Identifier;
  long (*Function)(C4Thread*, long, long, long, long, long, long, long, long, long, long);
	BOOL Public;
  };
extern C4ScriptFnDef C4ScriptFnMap[];

/* Engine-Calls */

#define PSF_Script							"~Script%i"
#define PSF_Initialize					"~Initialize"
#define PSF_Construction				"~Construction"
#define PSF_Destruction					"~Destruction"
#define PSF_Entrance						"~Entrance" // C4Object *pContainer
#define PSF_InitializePlayer		"~InitializePlayer" // iPlayer, iX, iY, pBase
#define PSF_OnGameOver					"~OnGameOver"
#define PSF_Time1								"~Time1"				
#define PSF_Hit									"~Hit"
#define PSF_Hit2								"~Hit2"
#define PSF_Hit3								"~Hit3"
#define PSF_Grab								"~Grab"
#define PSF_Get									"~Get"
#define PSF_Put									"~Put"
#define PSF_Collection					"~Collection" // pObject, fPut
#define PSF_Ejection						"~Ejection" // pObject
#define PSF_Entrance						"~Entrance" // pContainer
#define PSF_Departure						"~Departure" // pContainer
#define PSF_Completion					"~Completion"
#define PSF_Damage							"~Damage" // iChange, iCausedBy
#define PSF_Incineration				"~Incineration" // iCausedBy
#define PSF_Death								"~Death"
#define PSF_ActivateEntrance		"~ActivateEntrance" // pByObject
#define PSF_Activate						"~Activate" // pByObject
#define PSF_LiftTop							"~LiftTop"
#define PSF_Control							"~Control%s"	// pByObject
#define PSF_ContainedControl		"~Contained%s"	// pByObject
#define PSF_Contact							"~Contact%s"
#define PSF_ControlCommand			"~ControlCommand" // szCommand, pTarget, iTx, iTy
#define PSF_ControlCommandFinished "~ControlCommandFinished" // szCommand, pTarget, iTx, iTy
#define PSF_DeepBreath					"~DeepBreath"
#define PSF_CatchBlow						"~CatchBlow" // iLevel, pByObject
#define PSF_Stuck								"~Stuck"
#define PSF_RejectCollection		"~RejectCollect" // idObject, pObject
#define PSF_LightningStrike			"~LightningStrike"
#define PSF_GrabLost						"~GrabLost"
#define PSF_LineBreak						"~LineBreak" // iCause
#define PSF_BuildNeedsMaterial	"~BuildNeedsMaterial" // idMat1, iAmount1, idMat2, iAmount2...
#define PSF_ControlTransfer			"~ControlTransfer" // C4Object* pObj, int iTx, int iTy
#define PSF_UpdateTransferZone	"~UpdateTransferZone"
