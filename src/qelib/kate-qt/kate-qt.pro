#
# kate-qt engine
#

QT += xml
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
TARGET = kate-qt-test

MOC_DIR     = .tmp
UI_DIR      = .tmp
OBJECTS_DIR = .tmp


# Input
HEADERS += katelanguage.h \
	katehighlight.h \
	katewordlist.h \
	katecontext.h \
	katehighlightrule.h \
	kateitemdata.h \
	kategeneral.h \
	katefolding.h \
	katecomment.h \
	katekeywords.h \
	kateitemdatamanager.h
	#katecolorfile.h


SOURCES += katelanguage.cpp \
	katehighlight.cpp \
	katewordlist.cpp \
	katecontext.cpp \
	katehighlightrule.cpp \
	kateitemdata.cpp \
	kategeneral.cpp \
	katefolding.cpp \
	katecomment.cpp \
	katekeywords.cpp \
	kateitemdatamanager.cpp \
	main.cpp

#	katecolorfile.cpp \
