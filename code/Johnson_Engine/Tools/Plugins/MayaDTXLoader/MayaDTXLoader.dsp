# Microsoft Developer Studio Project File - Name="MayaDTXLoader" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MayaDTXLoader - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MayaDTXLoader.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MayaDTXLoader.mak" CFG="MayaDTXLoader - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MayaDTXLoader - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MayaDTXLoader - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MayaDTXLoader - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\Built\Release\tools\Plugins\MayaDTXLoader"
# PROP Intermediate_Dir "..\..\..\Built\Release\tools\Plugins\MayaDTXLoader"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAYADTXLOADER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\..\Libs\STLPORT-4.0\STLPORT" /I "..\..\shared\engine" /I "..\..\..\..\libs\stdlith" /I "..\..\..\sdk\inc" /I "..\..\preprocessor" /I "..\..\shared\world" /I "..\..\dedit\misc" /D "_AFXDLL" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "PLUGIN_DLL" /D "DIRECTEDITOR_BUILD" /D "STDLITH_MFC" /D "BDEFS_MFC" /D "PRE_FLOAT" /D "PREPROCESSOR_BUILD" /D "NO_PRAGMA_LIBS" /D "LT15_COMPAT" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386 /out:"..\..\..\Built\Release\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll"
# Begin Custom Build
TargetPath=\Working\Engine\Built\Release\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll
InputPath=\Working\Engine\Built\Release\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll
SOURCE="$(InputPath)"

"..\..\..\..\development\tools\Plugins\IMFLithDTX.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy $(TargetPath) ..\..\..\..\development\tools\Plugins 
	copy $(TargetPath) ..\..\..\..\development\tools\Plugins 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "MayaDTXLoader - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Built\Debug\tools\Plugins\MayaDTXLoader"
# PROP Intermediate_Dir "..\..\..\Built\Debug\tools\Plugins\MayaDTXLoader"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAYADTXLOADER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\Libs\STLPORT-4.0\STLPORT" /I "..\..\shared\engine" /I "..\..\..\..\libs\stdlith" /I "..\..\..\sdk\inc" /I "..\..\preprocessor" /I "..\..\shared\world" /I "..\..\dedit\misc" /D "_AFXDLL" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PLUGIN_DLL" /D "DIRECTEDITOR_BUILD" /D "STDLITH_MFC" /D "BDEFS_MFC" /D "PRE_FLOAT" /D "PREPROCESSOR_BUILD" /D "NO_PRAGMA_LIBS" /D "LT15_COMPAT" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"..\..\..\Built\Debug\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll" /pdbtype:sept
# Begin Custom Build
TargetPath=\Working\Engine\Built\Debug\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll
InputPath=\Working\Engine\Built\Debug\tools\Plugins\MayaDTXLoader\IMFLithDTX.dll
SOURCE="$(InputPath)"

"..\..\..\..\development\tools\Plugins\IMFLithDTX.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo copy $(TargetPath) ..\..\..\..\development\tools\Plugins 
	copy $(TargetPath) ..\..\..\..\development\tools\Plugins 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "MayaDTXLoader - Win32 Release"
# Name "MayaDTXLoader - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\shared\engine\dtxmgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\shared\engine\genltstream.cpp
# End Source File
# Begin Source File

SOURCE=.\MayaDTXLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\shared\engine\pixelformat.cpp
# End Source File
# Begin Source File

SOURCE=..\..\shared\engine\streamsim.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\image.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "libs_debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\built\Release\libs\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "MayaDTXLoader - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "MayaDTXLoader - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "libs_release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\built\Release\libs\Lib_StdLith\Lib_StdLith.lib

!IF  "$(CFG)" == "MayaDTXLoader - Win32 Release"

!ELSEIF  "$(CFG)" == "MayaDTXLoader - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\IMF.lib
# End Source File
# End Target
# End Project
