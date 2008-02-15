target = qtedit4
DESTDIR = .
TMP_DIR = build
UI_DIR = build
MOC_DIR = build
OBJECTS_DIR = build
SOURCES += ../tools/qmdilib/src/actiongroup.cpp \
 ../tools/qmdilib/src/actiongrouplist.cpp \
 ../tools/qmdilib/src/qmdiclient.cpp \
 ../tools/qmdilib/src/qmdihost.cpp \
 ../tools/qmdilib/src/qmdiserver.cpp \
 ../tools/qmdilib/src/qmditabwidget.cpp \
 ../tools/qmdilib/src/qmdiworkspace.cpp \
 ../tools/qtsourceview/src/qsvcolordef.cpp \
 ../tools/qtsourceview/src/qsvcolordeffactory.cpp \
 ../tools/qtsourceview/src/qsvlangdef.cpp \
 ../tools/qtsourceview/src/qsvlangdeffactory.cpp \
 ../tools/qtsourceview/src/qsvsyntaxhighlighter.cpp \
 ../tools/qtsourceview/demos/demo4/colorsmodel.cpp \
 ../tools/qtsourceview/demos/demo4/editorconfig.cpp \
 ../tools/qtsourceview/demos/demo4/qsveditor.cpp \
 ../tools/qtsourceview/demos/demo4/mainwindowimpl.cpp \
 ../tools/qtsourceview/demos/demo4/qsvlineedit.cpp \
 ../tools/qtsourceview/demos/demo4/qsvprivateblockdata.cpp \
 ../tools/qtsourceview/demos/demo4/qsveditorpanel.cpp \
 ../tools/qtsourceview/demos/demo4/transparentwidget.cpp \
 ../tools/qmdilib/demos/plugin-demo/configdialog.cpp \
 ../tools/qmdilib/demos/plugin-demo/iplugin.cpp \
 ../tools/qmdilib/demos/plugin-demo/pluginmanager.cpp \
 ../tools/qmdilib/demos/plugin-demo/pluginmodel.cpp \
 src/main.cpp \
 plugins/texteditor/texteditor_plg.cpp \
 src/widgets/qmdieditor.cpp \
 plugins/systembrowser/systembrowser_plg.cpp \
 src/widgets/filesystembrowser.cpp \
 plugins/help/help_plg.cpp
HEADERS += ../tools/qmdilib/src/actiongroup.h \
 ../tools/qmdilib/src/actiongrouplist.h \
 ../tools/qmdilib/src/qmdiclient.h \
 ../tools/qmdilib/src/qmdihost.h \
 ../tools/qmdilib/src/qmdimainwindow.h \
 ../tools/qmdilib/src/qmdiserver.h \
 ../tools/qmdilib/src/qmditabwidget.h \
 ../tools/qmdilib/src/qmdiworkspace.h \
 ../tools/qtsourceview/src/debug_info.h \
 ../tools/qtsourceview/src/qorderedmap.h \
 ../tools/qtsourceview/src/qsvcolordeffactory.h \
 ../tools/qtsourceview/src/qsvcolordef.h \
 ../tools/qtsourceview/src/qsvlangdeffactory.h \
 ../tools/qtsourceview/src/qsvlangdef.h \
 ../tools/qtsourceview/src/qsvsyntaxhighlighter.h \
 ../tools/qtsourceview/demos/demo4/colorsmodel.h \
 ../tools/qtsourceview/demos/demo4/editorconfig.h \
 ../tools/qtsourceview/demos/demo4/mainwindowimpl.h \
 ../tools/qtsourceview/demos/demo4/qsveditor.h \
 ../tools/qtsourceview/demos/demo4/qsvlineedit.h \
 ../tools/qtsourceview/demos/demo4/qsvprivateblockdata.h \
 ../tools/qtsourceview/demos/demo4/qsveditorpanel.h \
 ../tools/qtsourceview/demos/demo4/transparentwidget.h \
 ../tools/qmdilib/demos/plugin-demo/configdialog.h \
 ../tools/qmdilib/demos/plugin-demo/iplugin.h \
 ../tools/qmdilib/demos/plugin-demo/pluginmanager.h \
 ../tools/qmdilib/demos/plugin-demo/pluginmodel.h \
 plugins/texteditor/texteditor_plg.h \
 src/widgets/qmdieditor.h \
 plugins/systembrowser/systembrowser_plg.h \
 src/widgets/filesystembrowser.h \
 plugins/help/help_plg.h
FORMS += ../tools/qtsourceview/demos/demo4/configdialog.ui \
 ../tools/qtsourceview/demos/demo4/filemessage.ui \
 ../tools/qtsourceview/demos/demo4/findwidget.ui \
 ../tools/qtsourceview/demos/demo4/gotolinewidget.ui \
 ../tools/qtsourceview/demos/demo4/mainwindow.ui \
 ../tools/qtsourceview/demos/demo4/replacewidget.ui \
 ../tools/qmdilib/demos/plugin-demo/plugin_list.ui \
 src/widgets/filesystembrowser.ui
TEMPLATE = app
CONFIG += rtti stl release
QT += xml
INCLUDEPATH += ../tools/qtsourceview/src \
 ../tools/qmdilib/src \
 ../tools/qmdilib/demos/plugin-demo/ \
 ../tools/qtsourceview/demos/demo4/ \
 src/widgets \
 .
RESOURCES += ../tools/qmdilib/demos/common/common.qrc
