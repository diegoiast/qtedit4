#
# qmdi
#
QMDI_SOURCES = \
	../tools/qmdilib/src/qmdiactiongroup.cpp \
	../tools/qmdilib/src/qmdiactiongrouplist.cpp \
	../tools/qmdilib/src/qmdiclient.cpp \
	../tools/qmdilib/src/qmdihost.cpp \
	../tools/qmdilib/src/qmdiserver.cpp \
	../tools/qmdilib/src/qmditabwidget.cpp \
	../tools/qmdilib/demos/plugin-demo/pluginmodel.cpp \
	../tools/qmdilib/demos/plugin-demo/pluginmanager.cpp \
	../tools/qmdilib/demos/plugin-demo/iplugin.cpp \
	../tools/qmdilib/demos/plugin-demo/configdialog.cpp

QMDI_HEADERS = \
	../tools/qmdilib/src/actiongroup.h \
	../tools/qmdilib/src/actiongrouplist.h \
	../tools/qmdilib/src/qmdiclient.h \
	../tools/qmdilib/src/qmdihost.h \
	../tools/qmdilib/src/qmdimainwindow.h \
	../tools/qmdilib/src/qmdiserver.h \
	../tools/qmdilib/src/qmditabwidget.h \
	../tools/qmdilib/demos/plugin-demo/pluginmodel.h \
	../tools/qmdilib/demos/plugin-demo/pluginmanager.h \
	../tools/qmdilib/demos/plugin-demo/configdialog.h

QMDI_INCLUDES = \
	../tools/qmdilib/src/ \
	../tools/qmdilib/demos/plugin-demo \
	../tools/qmdilib/demos/plugin-demo/plugins/editor \

#
# qate
#
QATE_SOURCES = \
	../tools/qtsourceview/src/qate/context.cpp \
	../tools/qtsourceview/src/qate/defaultcolors.cpp \
	../tools/qtsourceview/src/qate/definitiondownloader.cpp \
	../tools/qtsourceview/src/qate/dynamicrule.cpp \
	../tools/qtsourceview/src/qate/highlightdefinition.cpp \
	../tools/qtsourceview/src/qate/highlightdefinitionhandler.cpp \
	../tools/qtsourceview/src/qate/highlightdefinitionmanager.cpp \
	../tools/qtsourceview/src/qate/highlightdefinitionmetadata.cpp \
	../tools/qtsourceview/src/qate/highlighter.cpp \
	../tools/qtsourceview/src/qate/includerulesinstruction.cpp \
	../tools/qtsourceview/src/qate/itemdata.cpp \
	../tools/qtsourceview/src/qate/keywordlist.cpp \
	../tools/qtsourceview/src/qate/mimedatabase.cpp \
	../tools/qtsourceview/src/qate/progressdata.cpp \
	../tools/qtsourceview/src/qate/rule.cpp \
	../tools/qtsourceview/src/qate/specificrules.cpp

QATE_HEADERS = \
	../tools/qtsourceview/src/qate/definitiondownloader.h \
	../tools/qtsourceview/src/qate/highlightdefinitionmanager.h \
	../tools/qtsourceview/src/qate/highlighter.h


QATE_INCLUDES = \
	../tools/qtsourceview/src/ \
	../tools/qtsourceview/src/qate \
	../tools/qtsourceview/demos/demo4/ \
	../tools/qtsourceview/demos/demo-qate4


#
# main APP
# 
TARGET = qtedit4
TEMPLATE = app
CONFIG += release
QT += xml widgets network concurrent
DEFINES += CORE_EXPORT= Q_CONCURRENT_EXPORT=

INCLUDEPATH += $$QMDI_INCLUDES $$QATE_INCLUDES src/widgets .


SOURCES += $$QMDI_SOURCES $$QATE_SOURCES \
	../tools/qtsourceview/demos/demo4/qsvtextoperationswidget.cpp \
	../tools/qtsourceview/demos/demo4/qsvtextedit.cpp \
	../tools/qtsourceview/demos/demo4/qsvsyntaxhighlighterbase.cpp \
	../tools/qtsourceview/demos/demo4/qsvdefaulthighlighter.cpp \
	../tools/qtsourceview/demos/demo-qate4/qatehighlighter.cpp \
	src/widgets/filesystembrowser.cpp \
	src/widgets/qmdieditor.cpp \
	src/main.cpp \
	src/plugins/systembrowser/systembrowser_plg.cpp \
	src/plugins/help/help_plg.cpp \
	src/plugins/texteditor/texteditor_plg.cpp \
        src/plugins/ProjectManager/ProjectManagerPlg.cpp \
	src/plugins/ProjectManager/GenericItems.cpp

HEADERS += $$QMDI_HEADERS $$QATE_HEADERS \
	../tools/qtsourceview/demos/demo4/qsvtextoperationswidget.h \
	../tools/qtsourceview/demos/demo4/qsvtextedit.h \
	src/widgets/filesystembrowser.h \
	src/widgets/qmdieditor.h \
	src/plugins/systembrowser/systembrowser_plg.h \
	src/plugins/help/help_plg.h \
	src/plugins/texteditor/texteditor_plg.h \
        src/plugins/ProjectManager/ProjectManagerPlg.h
	src/plugins/ProjectManager/GenericItems.h
FORMS += \
	../tools/qtsourceview/demos/demo4/searchform.ui \
	../tools/qtsourceview/demos/demo4/replaceform.ui \
	../tools/qtsourceview/demos/demo4/bannermessage.ui \
	../tools/qmdilib/demos/plugin-demo/plugin_list.ui \
	../tools/qmdilib/demos/plugin-demo/plugins/editor/editor_cfg.ui \
	src/widgets/filesystembrowser.ui \
	src/plugins/ProjectManager/ProjectManagerGUI.ui

RESOURCES += ../tools/qmdilib/demos/common/common.qrc
