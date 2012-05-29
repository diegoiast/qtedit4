# $Id$
cmake_minimum_required(VERSION 2.6) 

# 3rd party libs
add_subdirectory( lib/qmdilib      qmdi )
add_subdirectory( lib/qtsourceview qate )

# main application
project (qtedit4) 
SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTGUI TRUE )
SET( QT_USE_QTNETWORK TRUE )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})

# build the library 
#
set(QTEDIT4_VERSION   "0.0.1")
set(QTEDIT4_SOVERSION "0")


set( QTEDIT4_SOURCES 
	lib/qmdilib/demos/plugin-demo/pluginmodel.cpp
	lib/qmdilib/demos/plugin-demo/pluginmanager.cpp
	lib/qmdilib/demos/plugin-demo/iplugin.cpp
	lib/qmdilib/demos/plugin-demo/configdialog.cpp
	lib/qtsourceview/demos/demo4/qsvtextoperationswidget.cpp
	lib/qtsourceview/demos/demo4/qsvtextedit.cpp
	lib/qtsourceview/demos/demo4/qsvsyntaxhighlighterbase.cpp
	src/widgets/filesystembrowser.cpp
	src/widgets/qmdieditor.cpp
	src/main.cpp
	plugins/systembrowser/systembrowser_plg.cpp
	plugins/help/help_plg.cpp
	plugins/texteditor/texteditor_plg.cpp
)
set( QTEDIT4_HEADERS 
	lib/qmdilib/demos/plugin-demo/pluginmodel.h
	lib/qmdilib/demos/plugin-demo/iplugin.h
	lib/qmdilib/demos/plugin-demo/pluginmanager.h
	lib/qmdilib/demos/plugin-demo/configdialog.h
	lib/qtsourceview/demos/demo4/qsvtextoperationswidget.h
	lib/qtsourceview/demos/demo4/qsvtextedit.h
	lib/qtsourceview/demos/demo4/qsvsyntaxhighlighterbase.h
	src/widgets/filesystembrowser.h
	src/widgets/qmdieditor.h
	plugins/systembrowser/systembrowser_plg.h
	plugins/help/help_plg.h
	plugins/texteditor/texteditor_plg.h
)
set( QTEDIT4_UIS
	lib/qtsourceview/demos/demo4/searchform.ui
	lib/qtsourceview/demos/demo4/replaceform.ui
	lib/qtsourceview/demos/demo4/bannermessage.ui
	lib/qmdilib/demos/plugin-demo/plugin_list.ui
	lib/qmdilib/demos/plugin-demo/plugins/editor/editor_cfg.ui
	src/widgets/filesystembrowser.ui
)
SET( QTEDIT4_RESOURCES
	lib/qmdilib/demos/common/common.qrc
)

add_definitions(-DCORE_EXPORT=)
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/src/
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/src/
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/src/qate
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/demos/demo4
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/demos/plugin-demo
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/demos/plugin-demo/plugins/editor
	.
)


# QT4_WRAP_UI ( QTEDIT4_UI  ${QTEDIT4_UIS} )
QT4_ADD_RESOURCES( QTEDIT4_RCC ${QTEDIT4_RESOURCES})
QT4_WRAP_UI( QTEDIT4_UI  ${QTEDIT4_UIS} )
QT4_WRAP_CPP( QTEDIT4_MOC ${QTEDIT4_HEADERS})
add_executable(qtedit4 ${QTEDIT4_SOURCES} ${QTEDIT4_MOC} ${QTEDIT4_UI} ${QTEDIT4_RCC})
add_dependencies(qtedit4 QMDILIB_STATIC QATE_STATIC)
target_link_libraries(qtedit4 ${QT_LIBRARIES} QMDILIB_STATIC QATE_STATIC)
