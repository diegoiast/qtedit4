#
# main APP
#
TARGET = qtedit4
TEMPLATE = app
CONFIG += release c++14
QT += xml widgets network concurrent

# qmdi
QMDILIB_SRC_DIR  = lib/qmdilib/src/
include ($$QMDILIB_SRC_DIR/qmdilib.pri)

# QtSourceView + Qate
QATE_SRC_DIR = lib/qtsourceview/src/
QTSOURCEVIEW_SRC_DIR = $$QATE_SRC_DIR
include ($$QATE_SRC_DIR/qsvte.pri)
include ($$QATE_SRC_DIR/qate.pri)

# plugins - not part of qmdilib yet
QMDI_PLUGIN_SOURCES = \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/pluginmodel.cpp \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/pluginmanager.cpp \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/iplugin.cpp \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/configdialog.cpp

QMDI_PLUGIN_HEADERS = \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/pluginmodel.h \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/pluginmanager.h \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/iplugin.h \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/configdialog.h

QMDI_PLUGIN_INCLUDES = \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo \
        $$QMDILIB_SRC_DIR/../demos/plugin-demo/plugins/editor

# TODO - why do I need to add QMDILIB_SRC_DIR ?
INCLUDEPATH += $$QMDILIB_SRC_DIR $$QMDI_PLUGIN_INCLUDES

#DEFINES += CORE_EXPORT= Q_CONCURRENT_EXPORT=
#DEFINES		+=	CORE_EXPORT=Q_DECL_EXPORT QTCONCURRENT_EXPORT=Q_DECL_EXPORT  Q_CONCURRENT_EXPORT=Q_DECL_EXPORT

INCLUDEPATH += $$QMDI_INCLUDES src/widgets .

SOURCES +=  $$QMDI_PLUGIN_SOURCES \
	src/widgets/filesystembrowser.cpp \
	src/widgets/qmdieditor.cpp \
	src/main.cpp \
	src/plugins/systembrowser/systembrowser_plg.cpp \
	src/plugins/help/help_plg.cpp \
	src/plugins/texteditor/texteditor_plg.cpp \
        src/plugins/ProjectManager/ProjectManagerPlg.cpp \
	src/plugins/ProjectManager/GenericItems.cpp

HEADERS +=  $$QMDI_PLUGIN_HEADERS \
	src/widgets/filesystembrowser.h \
	src/widgets/qmdieditor.h \
	src/plugins/systembrowser/systembrowser_plg.h \
	src/plugins/help/help_plg.h \
	src/plugins/texteditor/texteditor_plg.h \
        src/plugins/ProjectManager/ProjectManagerPlg.h \
	src/plugins/ProjectManager/GenericItems.h

FORMS += \
        lib/qmdilib/demos/plugin-demo/plugin_list.ui \
        lib/qmdilib/demos/plugin-demo/plugins/editor/editor_cfg.ui \
	src/widgets/filesystembrowser.ui \
	src/plugins/ProjectManager/ProjectManagerGUI.ui

RESOURCES += lib/qmdilib/demos/common/common.qrc
