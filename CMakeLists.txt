# $Id$
cmake_minimum_required(VERSION 2.6) 

# http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_change_the_default_build_mode_and_see_it_reflected_in_the_GUI.3F
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# 3rd party libs
add_subdirectory( ../tools/qmdilib      qmdi )
add_subdirectory( ../tools/qtsourceview qate )

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
	../tools/qmdilib/demos/plugin-demo/pluginmodel.cpp
	../tools/qmdilib/demos/plugin-demo/pluginmanager.cpp
	../tools/qmdilib/demos/plugin-demo/iplugin.cpp
	../tools/qmdilib/demos/plugin-demo/configdialog.cpp
	../tools/qtsourceview/demos/demo4/qsvtextoperationswidget.cpp
	../tools/qtsourceview/demos/demo4/qsvtextedit.cpp
	../tools/qtsourceview/demos/demo4/qsvsyntaxhighlighterbase.cpp
	../tools/qtsourceview/demos/demo4/qsvdefaulthighlighter.cpp
	../tools/qtsourceview/demos/demo-qate4/qatehighlighter.cpp
	src/widgets/filesystembrowser.cpp
	src/widgets/qmdieditor.cpp
	src/main.cpp
	src/plugins/systembrowser/systembrowser_plg.cpp
	src/plugins/help/help_plg.cpp
	src/plugins/texteditor/texteditor_plg.cpp
        src/plugins/ProjectManager/ProjectManagerPlg.cpp
	src/plugins/ProjectManager/GenericItems.cpp
)
        #../tools/qmdilib/demos/plugin-demo/iplugin.h
#	../tools/qtsourceview/demos/demo4/qsvsyntaxhighlighterbase.h
set( QTEDIT4_HEADERS 
	../tools/qmdilib/demos/plugin-demo/pluginmodel.h
	../tools/qmdilib/demos/plugin-demo/pluginmanager.h
	../tools/qmdilib/demos/plugin-demo/configdialog.h
	../tools/qtsourceview/demos/demo4/qsvtextoperationswidget.h
	../tools/qtsourceview/demos/demo4/qsvtextedit.h
	src/widgets/filesystembrowser.h
	src/widgets/qmdieditor.h
	src/plugins/systembrowser/systembrowser_plg.h
	src/plugins/help/help_plg.h
	src/plugins/texteditor/texteditor_plg.h
        src/plugins/ProjectManager/ProjectManagerPlg.h
	src/plugins/ProjectManager/GenericItems.h
)
set( QTEDIT4_UIS
	../tools/qtsourceview/demos/demo4/searchform.ui
	../tools/qtsourceview/demos/demo4/replaceform.ui
	../tools/qtsourceview/demos/demo4/bannermessage.ui
	../tools/qmdilib/demos/plugin-demo/plugin_list.ui
	../tools/qmdilib/demos/plugin-demo/plugins/editor/editor_cfg.ui
	src/widgets/filesystembrowser.ui
	src/plugins/ProjectManager/ProjectManagerGUI.ui
)
SET( QTEDIT4_RESOURCES
	../tools/qmdilib/demos/common/common.qrc
)

add_definitions(-DCORE_EXPORT=)
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src/widgets
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/src/
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/src/
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/demos/demo4
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qtsourceview/demos/demo-qate4
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/demos/plugin-demo
	${CMAKE_CURRENT_SOURCE_DIR}/lib/qmdilib/demos/plugin-demo/plugins/editor
	../tools/qmdilib/src/
	../tools/qmdilib/demos/plugin-demo/
	../tools/qtsourceview/src/
	../tools/qtsourceview/demos/demo4/
	../tools/qtsourceview/demos/demo-qate4
	.
)


# QT4_WRAP_UI ( QTEDIT4_UI  ${QTEDIT4_UIS} )
QT4_ADD_RESOURCES( QTEDIT4_RCC ${QTEDIT4_RESOURCES})
QT4_WRAP_UI( QTEDIT4_UI  ${QTEDIT4_UIS} )
QT4_WRAP_CPP( QTEDIT4_MOC ${QTEDIT4_HEADERS})
add_executable(qtedit4 ${QTEDIT4_SOURCES} ${QTEDIT4_MOC} ${QTEDIT4_UI} ${QTEDIT4_RCC})
add_dependencies(qtedit4 QMDILIB_STATIC QATE_STATIC)
target_link_libraries(qtedit4 ${QT_LIBRARIES} QMDILIB_STATIC QATE_STATIC)

######################

QT4_WRAP_CPP(generic-item-complete_MOC tests/generic-item-complete-window.h)
add_executable(generic-item-complete
	tests/generic-item-complete-window.cpp
	tests/generic-item-complete-test.cpp
	src/plugins/ProjectManager/GenericItems.cpp
	${generic-item-complete_MOC}
)
target_link_libraries(generic-item-complete ${QT_LIBRARIES})
