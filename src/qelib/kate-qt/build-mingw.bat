@echo off

rem *
rem * you only need to change these 2 sets, match the dir where you installed Qt-free and mingw
rem *
rem set MINGW=c:\mingw
rem set QTDIR=C:\qt3\mingw

echo Setting up a MinGW/Qt only environment...
echo -- QTDIR set to C:\Qt\4.0.0
echo -- PATH set to C:\Qt\4.0.0\bin
echo -- Adding C:\MinGW\bin to PATH
echo -- Adding %SystemRoot%\System32 to PATH
echo -- QMAKESPEC set to win32-g++

set QTDIR=C:\Qt\4.0.0
set PATH=C:\Qt\4.0.0\bin
set PATH=%PATH%;C:\MinGW\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

rem DO NOT EDIT BELLOW THIS LINE
rem -------------------------------------------------
set PATH=%QTDIR%\bin;%MINGW%/bin;%PATH%
set QMAKESPEC=win32-g++

echo Calling qmake...
qmake 

echo Compiling using mingw32-make
mingw32-make

echo.
echo.
echo qt-edit has been compiled. Enjoy!
echo.

pause
