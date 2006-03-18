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

FORMS+= ui/configdialog.ui ui/inlinefind.ui ui/inlinereplace.ui ui/inlinegotoline.ui 

HEADERS += \
	qmdilib/actiongroup.cpp \
	qmdilib/actiongrouplist.cpp \
	qmdilib/qmdiclient.h \
	qmdilib/qmdihost.h \
	qmdilib/qmdiserver.h \
	qmdilib/qmditabwidget.h \
	qelib/qecodeeditor.h \
	qelib/qecpphighlighter/kateitemdata.h \
	qelib/qecpphighlighter/kateitemdatamanager.h \
	qelib/qtsourceview/qegtklangdef.h \
	qelib/qtsourceview/qorderedmap.h \
	qelib/qtsourceview/qegtkhighlighter.h \
	qelib/qtsourceview/qelangdeffactory.h \
	editorsettings.h \ 
	textdisplay.h \
	texteditorex.h \
	linenumberwidget.h \
	configdialog.h \
	mainwindow.h \
	helpdisplay.h \
	
SOURCES += \
	qmdilib/actiongroup.cpp \
	qmdilib/actiongrouplist.cpp \
	qmdilib/qmdiclient.cpp \
	qmdilib/qmdihost.cpp \ 
	qmdilib/qmdiserver.cpp \
	qmdilib/qmditabwidget.cpp \
	qelib/qecodeeditor.cpp \
	qelib/qecpphighlighter/qemymap.cpp \
	qelib/qecpphighlighter/kateitemdata.cpp \
	qelib/qecpphighlighter/kateitemdatamanager.cpp \
	qelib/qtsourceview/qegtklangdef.cpp \
	qelib/qtsourceview/qegtkhighlighter.cpp \
	qelib/qtsourceview/qelangdeffactory.cpp \
	editorsettings.cpp \ 
	textdisplay.cpp \
	texteditorex.cpp \
	helpdisplay.cpp \
	linenumberwidget.cpp \
	configdialog.cpp \
	mainwindow.cpp \
	main.cpp \
