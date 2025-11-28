@echo on

SET matrix_config_build_dir=windows-msvc
SET PATH=c:\Qt\6.10.1\msvc2022_64\bin\;c:\Program Files (x86)\Inno Setup 6\;%PATH%

RMDIR /s /q "build/%matrix_config_build_dir%"
RMDIR /s /q dist

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cmake . -B  "build/%matrix_config_build_dir%" -DCMAKE_BUILD_TYPE=Release -A x64
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --build   "build/%matrix_config_build_dir%" --parallel --verbose --config Release
if %errorlevel% neq 0 exit /b %errorlevel%
cmake --install build/%matrix_config_build_dir% --prefix dist/%matrix_config_build_dir%/usr
if %errorlevel% neq 0 exit /b %errorlevel%
windeployqt --release --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw dist/%matrix_config_build_dir%/usr/bin/qtedit4.exe
if %errorlevel% neq 0 exit /b %errorlevel%

iscc setup_script.iss
if %errorlevel% neq 0 exit /b %errorlevel%
