@echo off

rem *
rem * A small utility for compiling Qt applications on windows. 
rem * Sets paths, and calls the make utility.
rem * Version 0.02, Diego Iastrubni <elcuco@kde.org> 2005
rem *
rem * Public domain
rem 
rem * v 0.02 - use system environment if found, otherwise use "MY_*_DIR" variables


rem *
rem * You only need to change the 2 sets, match the dir where you installed Qt-free and mingw
rem * If you installed from the installers found on the site, thos variables will not be used. 
rem * The  values here are just examples, please ovveride them
set MY_QT_DIR=c:\qt
set MY_MINGW_DIR=c:\mingw

set MY_APPNAME=qhocr

rem *
rem * if QTDIR is not set, lets use user defaults. Oterwise, we can use the ones from the system
rem *
if "%QTDIR%" == "" goto SET_ENV

echo.
echo Using system default libraries
echo.
goto COMPILE

:SET_ENV
echo.
echo System libraries not found. Setting sane defaults...
echo.
set MINGWDIR=%MY_MINGW_DIR%
set QTDIR=%MY_QT_DIR%
set LD_LIBRARY_PATH=%QTDIR%\lib

set PATH=%MINGWDIR%\bin;%QTDIR%\bin;%PATH%
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++


:COMPILE
echo Setting up a MinGW/Qt only environment...
echo -- QTDIR set to %QTDIR%
echo -- MINGWDIR set to %MINGWDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- PATH set to %PATH%
echo -- LD_LIBRARY_PATH set to %LD_LIBRARY_PATH%
echo -----------------------------------------------

rem DO NOT EDIT BELLOW THIS LINE
rem -------------------------------------------------

echo.
echo.
echo Calling qmake...
qmake 

del bin\%MY_APPNAME%.exe
echo Compiling using mingw32-make
mingw32-make all 2> error.txt

if EXIST bin\%MY_APPNAME%.exe GOTO COMPILATION_OK

:COMPILATION_FAILED
echo.
echo.
echo Compilation failed. 
echo.
pause
goto END_BATCH

:COMPILATION_OK
copy bin\%MY_APPNAME%.exe .
echo.
echo.
echo Application %MY_APPNAME% has been compiled. Enjoy!
echo.

%MY_APPNAME%

:END_BATCH
