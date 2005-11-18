@echo offf

@rem clean files
del /q qrc_qhocr.cpp
del /q /s *.exe
del /q /s Makefile
del /q /s bin/*
del /q /s tmp\*
del /q    error.txt

@rem clean directories
rd /q /s .tmp
rd /q /s debug
rd /q /s release

