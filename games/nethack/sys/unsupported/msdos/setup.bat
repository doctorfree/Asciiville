@echo off
REM    SCCS Id: @(#)setup.bat   2002/03/17
REM    Copyright (c) NetHack PC Development Team 1990 - 2002
REM    NetHack may be freely redistributed.  See license for details.

echo.
echo   Copyright (c) NetHack PC Development Team 1990 - 2002
echo   NetHack may be freely redistributed.  See license for details.
echo.
REM setup batch file for msdos, see Install.dos for details.

if not %1.==. goto ok_parm
goto err_set

:ok_parm
echo Checking to see if directories are set up properly ...
if not exist ..\..\include\hack.h  goto err_dir
if not exist ..\..\src\hack.c      goto err_dir
if not exist ..\..\dat\wizard.des  goto err_dir
if not exist ..\..\util\makedefs.c goto err_dir
if not exist ..\..\win\tty\wintty.c goto err_dir
if not exist ..\share\lev_yacc.c   goto err_dir
echo Directories OK.

if not exist ..\..\binary\* mkdir ..\..\binary
if NOT exist ..\..\binary\license copy ..\..\dat\license ..\..\binary\license >nul

if exist ..\..\dat\data.bas goto long1ok
if exist ..\..\dat\data.base goto long1a
if exist ..\..\dat\data~1.bas goto long1b
goto err_long
:long1a
echo Changing some long-named distribution file names:
echo "Copying ..\..\dat\data.base -> ..\..\dat\data.bas"
copy ..\..\dat\data.base ..\..\dat\data.bas
if exist ..\..\dat\data.old del /Q ..\..\dat\data.old
ren ..\..\dat\data.base data.old
goto long1ok
:long1b
echo Changing some long-named distribution file names:
echo "Copying ..\..\dat\data~1.bas -> ..\..\dat\data.bas"
copy ..\..\dat\data~1.bas ..\..\dat\data.bas
if exist ..\..\dat\data.old del /Q ..\..\dat\data.old
ren ..\..\dat\data~1.bas data.old
:long1ok

if exist ..\..\include\patchlev.h goto long2ok
if exist ..\..\include\patchlevel.h goto long2a
if exist ..\..\include\patchl~1.h goto long2b
goto err_long
:long2a
echo "Copying ..\..\include\patchlevel.h -> ..\..\include\patchlev.h"
copy ..\..\include\patchlevel.h ..\..\include\patchlev.h
if exist ..\..\include\patchlev.old del /Q ..\..\include\patchlev.old
ren ..\..\include\patchlevel.h patchlev.old
goto long2ok
:long2b
echo "Copying ..\..\include\patchl~1.h -> ..\..\include\patchlev.h"
copy ..\..\include\patchl~1.h ..\..\include\patchlev.h
if exist ..\..\include\patchlev.old del /Q ..\..\include\patchlev.old
ren ..\..\include\patchl~1.h patchlev.old
:long2ok

REM Missing guidebook is not fatal to the build process
if exist ..\..\doc\guideboo.txt goto long3ok
if exist ..\..\doc\guidebook.txt goto long3a
if exist ..\..\doc\guideb~1.txt goto long3b
goto warn3long
:long3a
echo "Copying ..\..\doc\guidebook.txt -> ..\..\doc\guideboo.txt"
copy ..\..\doc\guidebook.txt ..\..\doc\guideboo.txt
if exist ..\..\doc\guideboo.old del /Q ..\..\doc\guideboo.old
ren ..\..\doc\guidebook.txt guideboo.old
goto long3ok
:long3b
echo "Copying ..\..\doc\guideb~1.txt -> ..\..\doc\guideboo.txt"
copy ..\..\doc\guideb~1.txt ..\..\doc\guideboo.txt
if exist ..\..\doc\guideboo.old del /Q ..\..\doc\guideboo.old
ren ..\..\doc\guideb~1.txt guideboo.old
goto long3ok
:warn3long
echo "Warning - There is no NetHack Guidebook (..\..\doc\guideboo.txt)"
echo "          included in your distribution.  Build will proceed anyway."
:long3ok

if "%1"=="GCC"   goto ok_gcc
if "%1"=="gcc"   goto ok_gcc
if "%1"=="nmake" goto ok_msc
if "%1"=="NMAKE" goto ok_msc
if "%1"=="BC"   goto ok_bc
if "%1"=="bc"   goto ok_bc
if "%1"=="MSC"   goto ok_msc
if "%1"=="msc"   goto ok_msc
goto err_set

:ok_gcc
echo Symbolic links, msdos style
echo "Makefile.GCC -> ..\..\src\makefile"
copy makefile.GCC ..\..\src\makefile
goto done

:ok_msc
echo Copying Makefile for Microsoft C and Microsoft NMAKE.
echo "Makefile.MSC -> ..\..\src\makefile"
copy Makefile.MSC ..\..\src\makefile
echo Copying overlay schemas to ..\..\src
copy schema*.MSC ..\..\src\schema*.DEF
:ok_cl
goto done

:ok_bc
echo Copying Makefile for Borland C and Borland's MAKE.
echo "Makefile.BC -> ..\..\src\makefile"
copy Makefile.BC ..\..\src\makefile
echo Copying overlay schemas to ..\..\src
copy schema*.BC ..\..\src
goto done

:err_long
echo.
echo ** ERROR - New file system with "long file name support" problem. **
echo A couple of NetHack distribution files that are packaged with 
echo a long filename ( exceeds 8.3) appear to be missing from your 
echo distribution.
echo The following files need to exist under the names on the
echo right in order to build NetHack:
echo.
echo  ..\..\dat\data.base        needs to be copied to ..\..\dat\data.bas
echo  ..\..\include\patchlevel.h needs to be copied to ..\..\include\patchlev.h
echo.
echo setup.bat was unable to perform the necessary changes because at least
echo one of the files doesn't exist under its short name, and the 
echo original (long) file name to copy it from was not found either.
echo.
goto end

:err_set
echo.
echo Usage:
echo "%0 <GCC | MSC | BC >"
echo.
echo    Run this batch file specifying on of the following:
echo            GCC, MSC, BC
echo.
echo    (depending on which compiler and/or make utility you are using).
echo.
echo    The GCC argument is for use with djgpp and the NDMAKE utility.
echo.
echo    The MSC argument is for use with Microsoft C and the NMAKE utility
echo    that ships with it (MSC 7.0 or greater only, including Visual C).
echo.
echo    The BC argument is for use with Borland C and Borland's MAKE utility
echo    that ships with it (Borland C++ 3.1 only).
echo.
goto end

:err_dir
echo/
echo Your directories are not set up properly, please re-read the
echo Install.dos and README documentation.
goto end

:done
echo Setup Done!
echo Please continue with next step from Install.dos.

:end
