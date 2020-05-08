# Microsoft Developer Studio Project File - Name="Object" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Object - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak" CFG="Object - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Object - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Final" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SA/Johnson_Code/Johnson_Client/ObjectDLL/SA", IUVBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Release\ObjectDLL\SA"
# PROP Intermediate_Dir "..\..\Built\Release\ObjectDLL\SA"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I ".\\" /I "..\objectshared" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\..\libs\regmgr" /I "..\..\libs" /I "..\..\Shared\SA" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_NOLFBUILD" /D "USE_INTEL_COMPILER" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /D "MEMTRACK_SERVER" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ObjectShared.lib ButeMgr.lib MFCStub.lib Cryptmgr.lib Lib_StdLith.lib Lib_lith.lib RegMgr.lib ServerDir.lib LTMem.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386 /out:"..\..\Built\Release\ObjectDLL\SA\Object.lto" /libpath:"..\..\..\Libs\Built\Release" /libpath:"..\..\Built\Release\Libs" /libpath:"..\..\..\Engine\built\Release\libs"
# Begin Custom Build
TargetPath=\SA\Johnson_Code\Johnson_Client\Built\Release\ObjectDLL\SA\Object.lto
InputPath=\SA\Johnson_Code\Johnson_Client\Built\Release\ObjectDLL\SA\Object.lto
SOURCE="$(InputPath)"

"$(SA_REZ_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(SA_REZ_DIR) 
	copy $(TargetPath) $(SA_REZ_DIR) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Johnson_Built\Johnson_Client\Debug\ObjectDLL\SA"
# PROP Intermediate_Dir "..\..\..\Johnson_Built\Johnson_Client\Debug\ObjectDLL\SA"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Shared\SA" /I ".\\" /I "..\objectshared" /I "..\..\..\Johnson_Libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Johnson_Engine\sdk\inc" /I "..\..\..\Johnson_Libs\stdlith" /I "..\..\..\Johnson_Libs\butemgr" /I "..\..\..\Johnson_Libs\cryptmgr" /I "..\..\..\Johnson_Libs\lith" /I "..\..\..\Johnson_Libs\mfcstub" /I "..\..\..\Johnson_Libs\regmgr" /I "..\..\libs" /I "..\..\libs\serverdir" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /D "MEMTRACK_SERVER" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ObjectShared.lib ButeMgr.lib MFCStub.lib Cryptmgr.lib Lib_StdLith.lib Lib_lith.lib RegMgr.lib ServerDir.lib LTMem.lib /nologo /subsystem:windows /dll /profile /debug /machine:I386 /out:"..\..\..\Lib\Debug\Object.lto" /implib:"..\..\..\Lib\Debug\Object.lib" /libpath:"..\..\..\Lib\Debug"
# Begin Custom Build
TargetPath=\SA\Johnson_Code\Lib\Debug\Object.lto
InputPath=\SA\Johnson_Code\Lib\Debug\Object.lto
SOURCE="$(InputPath)"

"$(SA_MAIN_DIR)\game\Object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(SA_MAIN_DIR)\game 
	copy $(TargetPath) $(SA_MAIN_DIR)\game 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Final"
# PROP BASE Intermediate_Dir "Final"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Built\Final\ObjectDLL\SA"
# PROP Intermediate_Dir "..\..\Built\Final\ObjectDLL\SA"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I ".\\" /I "..\objectshared" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\..\libs\regmgr" /I "..\..\libs" /I "..\..\Shared\SA" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_NOLFBUILD" /D "USE_INTEL_COMPILER" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /D "_FINAL" /Yu"stdafx.h" /FD /Zm1000 /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I ".\\" /I "..\objectshared" /I "..\..\..\libs\STLPORT-4.0\STLPORT" /I "..\..\shared" /I "..\..\..\Engine\sdk\inc" /I "..\..\..\libs\stdlith" /I "..\..\..\libs\butemgr" /I "..\..\..\libs\cryptmgr" /I "..\..\..\libs\lith" /I "..\..\..\libs\mfcstub" /I "..\..\..\libs\regmgr" /I "..\..\libs" /I "..\..\Shared\SA" /I "..\..\libs\serverdir" /D "NDEBUG" /D "_NOLFBUILD" /D "USE_INTEL_COMPILER" /D "WIN32" /D "_WINDOWS" /D "NOPS2" /D "_SERVERBUILD" /D "_FINAL" /D "MEMTRACK_SERVER" /D "USE_FIRE_GUARD" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"..\..\Built\Final\ObjectDLL\SA\Object.lto"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib ObjectShared.lib ButeMgr.lib MFCStub.lib Cryptmgr.lib Lib_StdLith.lib Lib_lith.lib RegMgr.lib ServerDir.lib LTMem.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libc" /out:"..\..\Built\Final\ObjectDLL\SA\Object.lto" /libpath:"..\..\..\Libs\Built\Final" /libpath:"..\..\Built\Final\Libs" /libpath:"..\..\..\Engine\built\Final\libs" /Release
# SUBTRACT LINK32 /profile /pdb:none
# Begin Custom Build
TargetPath=\SA\Johnson_Code\Johnson_Client\Built\Final\ObjectDLL\SA\Object.lto
InputPath=\SA\Johnson_Code\Johnson_Client\Built\Final\ObjectDLL\SA\Object.lto
SOURCE="$(InputPath)"

"$(SA_REZ_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Rem IncrediBuild_OutputFile $(TargetPath) 
	echo copy $(TargetPath) $(SA_REZ_DIR) 
	copy $(TargetPath) $(SA_REZ_DIR) 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "Object - Win32 Release"
# Name "Object - Win32 Debug"
# Name "Object - Win32 Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c"
# Begin Source File

SOURCE=.\GlobalsInit.cpp
# End Source File
# Begin Source File

SOURCE=.\SAGameServerShell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\SA\SAMissionButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\SA\SAVersionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=.\SAGameServerShell.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\SA\SAMissionButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\SA\SAVersionMgr.h
# End Source File
# Begin Source File

SOURCE=.\Stdafx.h
# End Source File
# End Group
# End Target
# End Project
