/* Copyright (C) 1998-2000  Matthes Bender  RedWolf Design */

/* Main header to include all others */

/* Displayed at startup - put your own information here */

#define C4EngineInfo "RedWolf Design Clonk Planet Engine"

/* Application window caption */

#define C4EngineCaption "Clonk Planet Engine"

/* Use 4.9x.x version for offspin engines - modify version resource accordingly */

const int C4XVer1 = 4, C4XVer2 = 6, C4XVer3 = 5, C4XVer4 = 0;

#include <Windows.h>
#include <MMSystem.h>
#include <ShellAPI.h>
#include <Math.h>
#include <Stdio.h>
#include <Io.h>
#include <StdLib.h>
#include <Time.h>
#include <DDraw.h>
#include <VfW.h>
#include <Commdlg.h>

#include <Standard.h>
#include <StdFile.h>
#include <CStdFile.h>
#include <StdRegistry.h>
#include <StdResStr.h>
#include <StdConfig.h>
#include <StdRandom.h>
#include <StdSurface.h>
#include <StdFacet.h>
#include <StdFont.h>
#include <StdDDraw.h>
#include <StdVideo.h>
#include <StdJoystick.h>
#include <Bitmap256.h>

#include <C4Prototypes.h>
#include <C4Application.h>
#include <C4Group.h>
#include <C4Surface.h>
#include <C4Compiler.h>
#include <C4Constants.h>
#include <C4Facet.h>
#include <C4FacetEx.h>
#include <C4ID.h>
#include <C4IDList.h>
#include <C4NameList.h>
#include <C4RankSystem.h>
#include <C4Scenario.h>
#include <C4InfoCore.h>
#include <C4Shape.h>
#include <C4ComponentHost.h>
#include <C4ScriptHost.h>
#include <C4Script.h>
#include <C4Def.h>
#include <C4Material.h>
#include <C4Texture.h>
#include <C4Map.h>
#include <C4Components.h>
#include <C4ObjectInfo.h>
#include <C4ObjectInfoList.h>
#include <C4FileClasses.h>
#include <C4Packet.h>
#include <C4PacketQueue.h>
#include <C4Stream.h>
#include <C4UserMessages.h>
#include <C4SurfaceFile.h>


#ifdef C4SHAREWARE
#include <C4ConfigShareware.h>
#else
#include <C4Config.h>
#endif


#ifdef C4ENGINE

#include "..\res\Resource.h"

#include <C4Log.h>
#include <C4Random.h>
#include <C4MusicSystem.h>
#include <C4SoundSystem.h>
#include <C4Region.h>
#include <C4Menu.h>
#include <C4ObjectList.h>
#include <C4Object.h>
#include <C4GameMessage.h>
#include <C4Weather.h>
#include <C4MassMover.h>
#include <C4Sky.h>
#include <C4Landscape.h>
#include <C4MaterialList.h>
#include <C4GamePadCon.h>  
#include <C4FogOfWar.h>
#include <C4Player.h>
#include <C4PlayerList.h>
#include <C4Viewport.h>
#include <C4MouseControl.h>
#include <C4EditCursor.h>
#include <C4MessageBoard.h>
#include <C4Video.h>
#include <C4GraphicsSystem.h>
#include <C4GraphicsResource.h>
#include <C4PropertyDlg.h>
#include <C4ToolsDlg.h>
#include <C4Console.h>
#include <C4FullScreen.h>
#include <C4PXS.h>
#include <C4NetworkClient.h>
#include <C4MasterServerClient.h>
#include <C4InputHandler.h>
#include <C4Control.h>
#include <C4Network.h>
#include <C4TransferZone.h>
#include <C4PathFinder.h>
#include <C4Game.h>
#include <C4Effects.h>
#include <C4Command.h>
#include <C4ObjectCom.h>
#include <C4Thread.h>
#include <C4Physics.h>
#include <C4Engine.h>

extern C4Application	Application;
extern C4Engine				Engine; 
extern C4Console			Console;
extern C4FullScreen		FullScreen;
extern C4Game					Game;

#ifdef C4SHAREWARE
extern C4ConfigShareware Config;
#else
extern C4Config	Config;
#endif

#include <C4Wrappers.h>

#endif
