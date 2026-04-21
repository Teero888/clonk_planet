# Microsoft Developer Studio Project File - Name="Standard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Standard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Standard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Standard.mak" CFG="Standard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Standard - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Standard - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Standard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir "..\intermediate\StandardRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "./inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Standard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\lib"
# PROP Intermediate_Dir "..\intermediate\StandardDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\StandardD.lib"

!ENDIF 

# Begin Target

# Name "Standard - Win32 Release"
# Name "Standard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ZLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=.\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infblock.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infcodes.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\infutil.c
# End Source File
# Begin Source File

SOURCE=.\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=.\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.c
# End Source File
# End Group
# Begin Group "Unibase"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\unibase\skstream.cpp

!IF  "$(CFG)" == "Standard - Win32 Release"

# ADD CPP /I "..\standard\inc" /I "..\standard\zlib" /I "..\standard\unibase"

!ELSEIF  "$(CFG)" == "Standard - Win32 Debug"

# ADD CPP /I "\dev\clonk\standard\inc"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Bitmap256.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CStdFile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DInputX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DSoundX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Midi.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PathFinder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Standard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdDDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdFacet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdFile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdFont.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdHTTP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdJoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdRegistry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdResStr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdVideo.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\..\Programme\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=.\inc\Bitmap256.h
# End Source File
# Begin Source File

SOURCE=.\inc\CStdFile.h
# End Source File
# Begin Source File

SOURCE=.\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\inc\DInputX.h
# End Source File
# Begin Source File

SOURCE=.\inc\DSoundX.h
# End Source File
# Begin Source File

SOURCE=.\inc\Fixed.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infblock.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\zlib\infutil.h
# End Source File
# Begin Source File

SOURCE=.\inc\Midi.h
# End Source File
# Begin Source File

SOURCE=.\inc\PathFinder.h
# End Source File
# Begin Source File

SOURCE=.\unibase\skstream.h
# End Source File
# Begin Source File

SOURCE=.\inc\Standard.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdBitmap.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdConfig.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdDDraw.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdFacet.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdFile.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdFont.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdHTTP.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdJoystick.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdRandom.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdRegistry.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdResStr.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdSurface.h
# End Source File
# Begin Source File

SOURCE=.\inc\StdVideo.h
# End Source File
# Begin Source File

SOURCE=.\zlib\ZLib.h
# End Source File
# Begin Source File

SOURCE=.\zlib\zutil.h
# End Source File
# End Group
# End Target
# End Project
