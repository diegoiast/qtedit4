@echo offf

@rem clean files
del /q qrc_qhocr.cpp
del /q /s *.exe
del /q /s Makefile.*

@rem clean directories
rd /q /s .tmp
rd /q /s debug
rd /q /s release
rd /q /s bin
