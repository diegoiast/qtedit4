QT         += xml
CONFIG     += warn_on 
CONFIG	   -= debug
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
	helpdisplay.h \
	qelib/qecodeeditor.h \
	qelib/qecodehighlighter.h \
	qelib/qecpphighlighter/kateitemdata.h \
	qelib/qecpphighlighter/kateitemdatamanager.h \
	qelib/qtsourceview/qegtklangdef.h \
	qelib/qtsourceview/qorderedmap.h \
	qelib/qtsourceview/qegtkhighlighter.h \
	qmdilib/actiongroup.cpp \
	qmdilib/actiongrouplist.cpp \
	qmdilib/qmdiclient.h \
	qmdilib/qmdihost.h \
	qmdilib/qmdiserver.h \
	qmdilib/qmditabwidget.h

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	textdisplay.cpp \
	optionsdialog.cpp \
	helpdisplay.cpp \
	qelib/qecodeeditor.cpp \
	qelib/qecodehighlighter.cpp \
	qelib/qecpphighlighter/qemymap.cpp \
	qelib/qecpphighlighter/qecpphighlighter.cpp \
	qelib/qecpphighlighter/kateitemdata.cpp \
	qelib/qecpphighlighter/kateitemdatamanager.cpp \
	qelib/qtsourceview/qegtklangdef.cpp \
	qelib/qtsourceview/qorderedmap.cpp \
	qelib/qtsourceview/qegtkhighlighter.cpp \
	qmdilib/actiongroup.cpp \
	qmdilib/actiongrouplist.cpp \
	qmdilib/qmdiclient.cpp \
	qmdilib/qmdihost.cpp \ 
	qmdilib/qmdiserver.cpp \
	qmdilib/qmditabwidget.cpp 
