@REM  SCCS Id: @(#)nhsetup.bat	$Date: 2003/08/22 13:23:33 $
@REM  Copyright (c) Alex Kompel, 2002
@REM  NetHack may be freely redistributed.  See license for details. 
@REM  Win32 nhsetup batch file, see Install.ce for details
@REM
@echo off
REM
REM  Make sure directories necessary for build exist
REM
if NOT exist ..\..\wince\*.* mkdir ..\..\wince
REM
REM  Get these files from the win\win32 port
REM
copy ..\..\win\win32\mnsel.uu    ..\..\wince\mnsel.uu
copy ..\..\win\win32\mnselcnt.uu ..\..\wince\mnselcnt.uu
copy ..\..\win\win32\mnunsel.uu  ..\..\wince\mnunsel.uu
copy ..\..\win\win32\petmark.uu  ..\..\wince\petmark.uu
copy ..\..\sys\winnt\nhico.uu    ..\..\wince\nhico.uu
copy ..\..\sys\wince\menubar.uu    ..\..\wince\menubar.uu
copy ..\..\sys\wince\keypad.uu    ..\..\wince\keypad.uu
REM
REM  Get these files from sys\wince
REM
copy bootstrp.mak ..\..\wince\bootstrp.mak
copy wince.vcw    ..\..\wince.vcw
copy hpc.vcp      ..\..\wince\wince_hpc.vcp
copy palmpc.vcp   ..\..\wince\wince_palm_pc.vcp
copy pocketpc.vcp ..\..\wince\wince_pocket_pc.vcp
copy smartphn.vcp ..\..\wince\wince_smartphone.vcp
echo.
echo Proceed with the following steps:
echo.
echo        cd ..\..\wince
echo        nmake /f bootstrp.mak
echo.
echo Then start Embedded Visual C and open 
echo the workspace wince.vcw (at the top of the NetHack tree)
echo to build.  See Install.ce for details.
echo.

