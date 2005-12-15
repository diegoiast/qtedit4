QT         += xml
CONFIG     += warn_on debug
TARGET      = qtedit4
DESTDIR     = ../bin


MOC_DIR     = ../tmp 
UI_DIR      = ../tmp 
OBJECTS_DIR = ../tmp 

INCLUDEPATH += qelib qelib/qecpphighlighter
#LIB        += kate-qt
RESOURCES   += qtedit4.qrc

FORMS+= ui/optionsdialog.ui ui/inlinefind.ui ui/inlinereplace.ui ui/inlinegotoline.ui 

HEADERS += \
	mainwindow.h \
	textdisplay.h \
	optionsdialog.h \
	qelib/qecodeeditor.h \
	qelib/qecodehighlighter.h \
	qelib/qecpphighlighter/kateitemdata.h \
	qelib/qecpphighlighter/kateitemdatamanager.h \
	qexdilib/qexitabwidget.h \
	qexdilib/qextabwidget.h
	

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	textdisplay.cpp \
	optionsdialog.cpp \
	qelib/qecodeeditor.cpp \
	qelib/qecodehighlighter.cpp \
	qelib/qecpphighlighter/qemymap.cpp \
	qelib/qecpphighlighter/qecpphighlighter.cpp \
	qelib/qecpphighlighter/kateitemdata.cpp \
	qelib/qecpphighlighter/kateitemdatamanager.cpp \
	qexdilib/qexitabwidget.cpp \
	qexdilib/qextabwidget.cpp
