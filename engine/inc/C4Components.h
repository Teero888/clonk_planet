/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Known component file names */

//========================= Component File Names ============================================

#define C4CFN_Engine					"Clonk.c4x"

#define C4CFN_Reference				"Network.c4f"

#define C4CFN_Material				"Material.c4g"
#define C4CFN_Sound						"Sound.c4g"
#define C4CFN_Graphics				"Graphics.c4g"
#define C4CFN_Music						"Music.c4g"

#define C4CFN_Objects					"Objects.c4d"

#define C4CFN_Mouse						"Mouse.c4f"
#define C4CFN_Keyboard				"Keyboard.c4f"
#define C4CFN_Easy						"Easy.c4f"

#define C4CFN_ScenarioCore		"Scenario.txt"
#define C4CFN_PlayerInfoCore	"Player.txt"
#define C4CFN_DefCore					"DefCore.txt"
#define C4CFN_ObjectInfoCore	"ObjectInfo.txt"

#define C4CFN_Landscape				"Landscape.bmp"
#define C4CFN_Sky							"Sky.bmp"
#define C4CFN_Script					"Script.c|Script%s.c|Script*.c|C4Script%s.c|C4Script*.c"
#define C4CFN_Info						"Info.txt"
#define C4CFN_Author					"Author.txt"
#define C4CFN_Game						"Game.txt"
#define C4CFN_PXS							"PXS.c4b"
#define C4CFN_MassMover				"MassMover.c4b"
#define C4CFN_TexMap					"TexMap.txt"
#define C4CFN_MatMap					"MatMap.txt"
#define C4CFN_Title						"Title.txt"
#define C4CFN_ScenarioTitle		"Title.bmp"
#define C4CFN_ScenarioIcon		"Icon.bmp"
#define C4CFN_ScenarioObjects	"Objects.txt"
#define C4CFN_ScenarioDesc		"Desc%s.rtf"
#define C4CFN_DefGraphics			"Graphics.bmp"
#define C4CFN_DefNames				"Names.txt"
#define C4CFN_DefActMap				"ActMap.txt"
#define C4CFN_DefDesc					"Desc%s.txt"
#define C4CFN_Portrait				"Portrait.bmp"
#define C4CFN_ControlQueue		"Control.c4q"

#define C4CFN_Log							"Clonk4.log"
#define C4CFN_Intro						"Clonk4.avi"
#define C4CFN_Names						"Names.txt"

#define C4CFN_Splash					"Splash.c4v"

#define C4CFN_TempMusic				"~Music.tmp"
#define C4CFN_TempMusic2			"~Music2.tmp"
#define C4CFN_TempSky					"~Sky.tmp"
#define C4CFN_TempLandscape		"~Landscape.tmp"
#define C4CFN_TempPXS					"~PXS.tmp"
#define C4CFN_TempTitle				"~Title.tmp"

#define C4CFN_DefFiles				"*.c4d"
#define C4CFN_PlayerFiles			"*.c4p"
#define C4CFN_MaterialFiles		"*.c4m"
#define C4CFN_ObjectInfoFiles	"*.c4i"
#define C4CFN_MusicFiles			"*.mid"
#define C4CFN_SoundFiles			"*.wav"
#define C4CFN_BitmapFiles			"*.bmp"
#define C4CFN_ScenarioFiles   "*.c4s"
#define C4CFN_FolderFiles			"*.c4f"
#define C4CFN_QueueFiles			"*.c4q"
#define C4CFN_AnimationFiles	"*.c4v"

//================================= File Load Sequences ================================================

#define C4FLS_Scenario	"Loader*.bmp|Scenario.txt|Title.txt|Info.txt|Script*.c|Names.txt|Game.txt|Control.c4q|Music.c4g|*.mid|*.wav|Desc*.rtf|Title.bmp|*.c4d|Material.c4g|MatMap.txt|Landscape.bmp|Sky.bmp|PXS.c4b|MassMover.c4b|Objects.txt|Icon.bmp|Author.txt"
#define C4FLS_Def				"DefCore.txt|Graphics.bmp|ActMap.txt|C4Script.c|Script*.c|Names.txt|Title.txt|Desc*.txt|Title.bmp|Icon.bmp|Author.txt|*.wav|*.c4d"
#define C4FLS_Player		"Player.txt|Portrait.bmp|*.c4i"
#define C4FLS_Object		"ObjectInfo.txt|Portrait.bmp"
#define C4FLS_Folder		"Loader*.bmp|*.txt|*.rtf|Title.bmp|Icon.bmp|Author.txt|*.c4s"
#define C4FLS_Material	"TexMap.txt|*.bmp|*.c4m"
#define C4FLS_Graphics	"Portrait*.bmp|Loader*.bmp|Resource.bmp|Control.bmp|Fire.bmp|Background.bmp|Flag.bmp|Crew.bmp|Player.bmp|Rank.bmp|Entry.bmp|Captain.bmp|Cursor.bmp|SelectMark.bmp|MenuSymbol.bmp|Menu.bmp|FogOfWar.bmp|*.pal"
#define C4FLS_Objects		"Names.txt|Desc*.txt|*.c4d"
#define C4FLS_Mouse			"*.txt|*.rtf|Title.bmp|Icon.bmp|Tutorial01.c4s|Tutorial02.c4s|Tutorial03.c4s|Objects.c4d"
#define C4FLS_Keyboard	"*.txt|*.rtf|Title.bmp|Icon.bmp|Tutorial01.c4s|Tutorial02.c4s|Tutorial03.c4s|Tutorial04.c4s|Tutorial05.c4s|Tutorial06.c4s|Tutorial07.c4s|Tutorial08.c4s|Tutorial09.c4s|Tutorial10.c4s"
#define C4FLS_Easy			"*.txt|*.rtf|Title.bmp|Icon.bmp|Goldmine.c4s|Monsterkill.c4s|Economy.c4s|Melee.c4s|Lake.c4s|Castle.c4s"

