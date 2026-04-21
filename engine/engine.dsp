# Microsoft Developer Studio Project File - Name="engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=engine - Win32 Debug Shareware
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak" CFG="engine - Win32 Debug Shareware"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "engine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Release Shareware" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Debug Shareware" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\planet"
# PROP Intermediate_Dir "..\intermediate\EngineRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\inc" /I "..\standard\inc" /D "C4ENGINE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\clonk.pdb" /machine:I386 /out:"..\planet\clonk.c4x"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\planet"
# PROP Intermediate_Dir "..\intermediate\EngineDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\standard\inc" /D "C4ENGINE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\EngineDebug/clonk.pdb" /debug /machine:I386 /out:"..\planet\clonk.c4x" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_Release_Shareware"
# PROP BASE Intermediate_Dir "engine___Win32_Release_Shareware"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\planet"
# PROP Intermediate_Dir "..\intermediate\EngineReleaseShareware"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I ".\inc" /I "..\standard\inc" /I ".\shareware" /D "C4ENGINE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\inc" /I "..\standard\inc" /I ".\sec" /D "C4ENGINE" /D "C4SHAREWARE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\clonk.pdb" /machine:I386 /out:"..\planet\clonk.c4x"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\clonk.pdb" /machine:I386 /out:"..\planet\clonk.c4x"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "engine___Win32_Debug_Shareware"
# PROP BASE Intermediate_Dir "engine___Win32_Debug_Shareware"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\planet"
# PROP Intermediate_Dir "..\intermediate\EngineDebugShareware"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\standard\inc" /I ".\shareware" /D "C4ENGINE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\inc" /I "..\standard\inc" /I ".\sec" /D "C4ENGINE" /D "C4SHAREWARE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\EngineDebug/clonk.pdb" /debug /machine:I386 /out:"..\planet\clonk.c4x" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib vfw32.lib wsock32.lib ddraw.lib dsound.lib dxguid.lib /nologo /subsystem:windows /pdb:"..\intermediate\EngineDebug/clonk.pdb" /debug /machine:I386 /out:"..\planet\clonk.c4x" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "engine - Win32 Release"
# Name "engine - Win32 Debug"
# Name "engine - Win32 Release Shareware"
# Name "engine - Win32 Debug Shareware"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\C4Action.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Application.cpp
# End Source File
# Begin Source File

SOURCE=.\sec\C4Code.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\C4Command.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ComponentHost.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Config.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sec\C4ConfigShareware.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\C4Console.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Control.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Def.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4EditCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Effect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Engine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Facet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4FacetEx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4FileClasses.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4FogOfWar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4FullScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Game.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4GameMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4GamePadCon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4GraphicsResource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4GraphicsSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Group.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Id.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4IDList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4InfoCore.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4InputHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Landscape.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Log.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MassMover.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MasterServerClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Material.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MaterialList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MessageBoard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MouseControl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Movement.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4MusicSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4NameList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Network.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4NetworkClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Object.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ObjectCom.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ObjectInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ObjectInfoList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ObjectList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Packet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4PacketQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4PathFinder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Player.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4PlayerList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4PropertyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4PXS.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Random.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4RankSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Region.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Scenario.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Script.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ScriptHost.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Shape.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Sky.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4SoundSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Stream.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Surface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4SurfaceFile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Texture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4ToolsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4TransferZone.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Video.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Viewport.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4Weather.cpp
# End Source File
# Begin Source File

SOURCE=.\src\C4WinMain.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Brush.bmp
# End Source File
# Begin Source File

SOURCE=.\res\brush1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Brush2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c4b.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4d.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4f.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4g.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4i.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4m.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4p.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4s.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4v.ico
# End Source File
# Begin Source File

SOURCE=.\res\c4x.ico
# End Source File
# Begin Source File

SOURCE=.\res\cp.ico
# End Source File
# Begin Source File

SOURCE=.\res\Cursor.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Cursor2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dynamic1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\engine.rc
# End Source File
# Begin Source File

SOURCE=.\res\Fill.bmp
# End Source File
# Begin Source File

SOURCE=.\res\fill1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Halt.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Halt2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\IFT.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ift1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Line.bmp
# End Source File
# Begin Source File

SOURCE=.\res\line1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mouse.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mouse1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\NoIFT.bmp
# End Source File
# Begin Source File

SOURCE=.\res\picker1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Play.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Play2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rect1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rectangle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\static1.bmp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Group "Standard"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\Programme\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=..\standard\inc\Bitmap256.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\CStdFile.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\DSoundX.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\Fixed.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\Midi.h
# End Source File
# Begin Source File

SOURCE=..\standard\unibase\skstream.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\Standard.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdConfig.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdDDraw.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdFacet.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdFile.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdFont.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdHTTP.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdJoystick.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdRandom.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdRegistry.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdResStr.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdSurface.h
# End Source File
# Begin Source File

SOURCE=..\standard\inc\StdVideo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\inc\C4Application.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Command.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Compiler.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ComponentHost.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Components.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Config.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Console.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Constants.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Control.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Def.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4EditCursor.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Effects.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Engine.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Facet.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4FacetEx.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4FileClasses.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4FogOfWar.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4FullScreen.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Game.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4GameMessage.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4GamePadCon.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4GraphicsResource.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4GraphicsSystem.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Group.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Id.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4IDList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Include.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4InfoCore.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4InputHandler.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Landscape.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Log.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Map.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MassMover.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MasterServerClient.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Material.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MaterialList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Menu.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MessageBoard.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MouseControl.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4MusicSystem.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4NameList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Network.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4NetworkClient.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Object.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ObjectCom.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ObjectInfo.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ObjectInfoList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ObjectList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Packet.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4PacketQueue.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4PathFinder.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Physics.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Player.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4PlayerList.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4PropertyDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Prototypes.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4PXS.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Random.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4RankSystem.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Region.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Scenario.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Script.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ScriptHost.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Shape.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Sky.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4SoundSystem.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Stream.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Surface.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4SurfaceFile.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Texture.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Thread.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4ToolsDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4TransferZone.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4UserMessages.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Video.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Viewport.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Weather.h
# End Source File
# Begin Source File

SOURCE=.\inc\C4Wrappers.h
# End Source File
# End Group
# Begin Group "Library Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\standard\lib\Standard.lib

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\standard\lib\StandardD.lib

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release Shareware"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "engine - Win32 Debug Shareware"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
