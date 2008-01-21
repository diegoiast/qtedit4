target = qtedit4
DESTDIR =       .
TMP_DIR = build
UI_DIR =       build
MOC_DIR =       build
OBJECTS_DIR =       build


SOURCES += lib/qmdilib/src/actiongroup.cpp \
lib/qmdilib/src/actiongrouplist.cpp \
lib/qmdilib/src/qmdiclient.cpp \
lib/qmdilib/src/qmdihost.cpp \
lib/qmdilib/src/qmdiserver.cpp \
lib/qmdilib/src/qmditabwidget.cpp \
lib/qmdilib/src/qmdiworkspace.cpp \
lib/qtsourceview/src/qsvcolordef.cpp \
lib/qtsourceview/src/qsvcolordeffactory.cpp \
lib/qtsourceview/src/qsvlangdef.cpp \
lib/qtsourceview/src/qsvlangdeffactory.cpp \
lib/qtsourceview/src/qsvsyntaxhighlighter.cpp \
lib/qtsourceview/demos/demo4/colorsmodel.cpp \
lib/qtsourceview/demos/demo4/editorconfig.cpp \
lib/qtsourceview/demos/demo4/lineseditor.cpp \
lib/qtsourceview/demos/demo4/mainwindowimpl.cpp \
lib/qtsourceview/demos/demo4/newlineedit.cpp \
lib/qtsourceview/demos/demo4/privateblockdata.cpp \
lib/qtsourceview/demos/demo4/samplepanel.cpp \
lib/qtsourceview/demos/demo4/transparentwidget.cpp \
lib/qmdilib/demos/plugin-demo/configdialog.cpp \
lib/qmdilib/demos/plugin-demo/iplugin.cpp \
lib/qmdilib/demos/plugin-demo/pluginmanager.cpp \
lib/qmdilib/demos/plugin-demo/pluginmodel.cpp \
src/main.cpp \
plugins/texteditor/texteditor_plg.cpp \
 src/widgets/qmdieditor.cpp
HEADERS += lib/qmdilib/src/actiongroup.h \
lib/qmdilib/src/actiongrouplist.h \
lib/qmdilib/src/qmdiclient.h \
lib/qmdilib/src/qmdihost.h \
lib/qmdilib/src/qmdimainwindow.h \
lib/qmdilib/src/qmdiserver.h \
lib/qmdilib/src/qmditabwidget.h \
lib/qmdilib/src/qmdiworkspace.h \
lib/qtsourceview/src/debug_info.h \
lib/qtsourceview/src/qorderedmap.h \
lib/qtsourceview/src/qsvcolordeffactory.h \
lib/qtsourceview/src/qsvcolordef.h \
lib/qtsourceview/src/qsvlangdeffactory.h \
lib/qtsourceview/src/qsvlangdef.h \
lib/qtsourceview/src/qsvsyntaxhighlighter.h \
lib/qtsourceview/demos/demo4/colorsmodel.h \
lib/qtsourceview/demos/demo4/editorconfig.h \
lib/qtsourceview/demos/demo4/lineseditor.h \
lib/qtsourceview/demos/demo4/mainwindowimpl.h \
lib/qtsourceview/demos/demo4/newlineedit.h \
lib/qtsourceview/demos/demo4/privateblockdata.h \
lib/qtsourceview/demos/demo4/samplepanel.h \
lib/qtsourceview/demos/demo4/transparentwidget.h \
lib/qmdilib/demos/plugin-demo/configdialog.h \
lib/qmdilib/demos/plugin-demo/iplugin.h \
lib/qmdilib/demos/plugin-demo/pluginmanager.h \
lib/qmdilib/demos/plugin-demo/pluginmodel.h \
plugins/texteditor/texteditor_plg.h \
 src/widgets/qmdieditor.h
FORMS += lib/qtsourceview/demos/demo4/configdialog.ui \
lib/qtsourceview/demos/demo4/filemessage.ui \
lib/qtsourceview/demos/demo4/findwidget.ui \
lib/qtsourceview/demos/demo4/gotolinewidget.ui \
lib/qtsourceview/demos/demo4/mainwindow.ui \
lib/qtsourceview/demos/demo4/replacewidget.ui \
lib/qmdilib/demos/plugin-demo/plugin_list.ui
TEMPLATE = app


CONFIG += debug_and_release \
 rtti

QT += xml

INCLUDEPATH += lib/qtsourceview/src \
lib/qmdilib/src \
lib/qmdilib/demos/plugin-demo/ \
 lib/qtsourceview/demos/demo4/
RESOURCES += ../tools/qmdilib/demos/common/common.qrc

CONFIG -= stl

