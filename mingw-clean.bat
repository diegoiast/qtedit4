@echo off

@rem clean files
del /q qrc_qtedit.cpp
del /q /s *.exe
del /q /s Makefile
del /q /s Makefile.qtedit4
del /q /s Makefile.*.Release
del /q /s Makefile.*.Debug
del /q /s object_script.*.Release
del /q /s object_script.*.Debug
del /q /s bin\*
del /q /s tmp\*
del /q    error.txt

@rem clean directories
rd /q /s .tmp
rd /q /s debug
rd /q /s release

