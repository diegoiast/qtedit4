QT      += xml core gui widgets

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/qsvdefaulthighlighter.h \
        $$QTSOURCEVIEW_SRC_DIR/qsvte/qsvsyntaxhighlighterbase.h \
        $$QTSOURCEVIEW_SRC_DIR/qsvte/qsvtextedit.h \
        $$QTSOURCEVIEW_SRC_DIR/qsvte/qsvtextoperationswidget.h

FORMS += \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/bannermessage.ui \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/replaceform.ui \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/searchform.ui

SOURCES += \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/qsvdefaulthighlighter.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/qsvsyntaxhighlighterbase.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/qsvtextedit.cpp \
	$$QTSOURCEVIEW_SRC_DIR/qsvte/qsvtextoperationswidget.cpp
	
INCLUDEPATH += $$QTSOURCEVIEW_SRC_DIR/
