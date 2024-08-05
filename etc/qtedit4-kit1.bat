@echo off

rem This is a kit definition for qtedit4. All your tasks
rem will run trought this file.

rem available enritonment variables:
rem %source_directory% - where the source being executed is saved
rem %build_directory%  - where code should be compile into
rem %run_directory%    - where this taks should be run, as defined in 
rem                      the task's definition in the JSON file
rem %task%             - the actual code to be run

rem The following meta variables are for qtedit4. Note the prefix:
rem @@ name = Visual Studio 2022 (64)
rem @@ author = auto generated - by qtedit4

rem from this point on - echo is on. Every command will be displayed
rem in the build output.
@echo on

echo "Running from kit %0%"
echo "Source is in        : %source_directort%"
echo "Binaries will be in : %build_directory%"
echo "Working directory is: %run_directory%"

@rem detected cmake
echo "Adding cmake to the path"
set PATH=c:\Program Files\CMake\bin\;%PATH%

@rem detected Visual studio 2022
call C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat

@rem execute task    
cd %run_directory%
%task%
