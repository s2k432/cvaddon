# Microsoft Developer Studio Project File - Name="cvaddon_colour_tracking" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=cvaddon_colour_tracking - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cvaddon_colour_tracking.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cvaddon_colour_tracking.mak" CFG="cvaddon_colour_tracking - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cvaddon_colour_tracking - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cvaddon_colour_tracking - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "cvaddon_colour_tracking - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Ob2 /I "../cvaddon_fast_sym" /I "../cvaddon_colour" /I "../cvaddon_util" /I "../util" /I "F:/_WORK/_Software/_CV/_CODE_and_LIB/cvblobslib_OpenCV_v5" /I "../cvaddon_image_io" /I "../cvaddon_image" /I "../cvaddon_motion" /I "../cvaddon_file_io" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /O3 /QxB /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib cv.lib highgui.lib cvblobslib.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "cvaddon_colour_tracking - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../cvaddon_fast_sym" /I "../cvaddon_colour" /I "../cvaddon_util" /I "../util" /I "F:/_WORK/_Software/_CV/_CODE_and_LIB/cvblobslib_OpenCV_v5" /I "../cvaddon_image_io" /I "../cvaddon_image" /I "../cvaddon_motion" /I "../cvaddon_file_io" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib cv.lib highgui.lib cvblobslib.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "cvaddon_colour_tracking - Win32 Release"
# Name "cvaddon_colour_tracking - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\cvaddon_colour\cvaddon_fast_bgr2hsv.cpp
# End Source File
# Begin Source File

SOURCE=..\cvaddon_fast_sym\cvaddon_fast_sym_detect.cpp
# End Source File
# Begin Source File

SOURCE=..\cvaddon_colour\cvaddon_hsv_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\main_display_results.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\main_ground_truth.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\main_pca_time.cpp
# End Source File
# Begin Source File

SOURCE=.\main_sym.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\cvaddon_motion\cvaddon_block_motion.h
# End Source File
# Begin Source File

SOURCE=..\cvaddon_colour\cvaddon_fast_bgr2hsv.h
# End Source File
# Begin Source File

SOURCE=..\cvaddon_fast_sym\cvaddon_fast_sym_detect.h
# End Source File
# Begin Source File

SOURCE=..\cvaddon_colour\cvaddon_hsv_filter.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
