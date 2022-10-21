cmake_minimum_required(VERSION 3.20)
project (qtedit4)


# 3rd party libs
set(QMDILIB_BUILD_EXAMPLES Off)
set(QTSOURCEVIEW_BUILD_EXAMPLES Off)
add_subdirectory( lib/qmdilib      )
add_subdirectory( lib/qtsourceview )

find_package(Qt6 COMPONENTS Widgets PrintSupport Network)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(qtedit4
        lib/qmdilib/demos/plugin-demo/iplugin.cpp
        lib/qmdilib/demos/plugin-demo/pluginmodel.cpp
        lib/qmdilib/demos/plugin-demo/pluginmodel.h
        lib/qmdilib/demos/plugin-demo/pluginmanager.cpp
        lib/qmdilib/demos/plugin-demo/pluginmanager.h
        lib/qmdilib/demos/plugin-demo/configdialog.cpp
        lib/qmdilib/demos/plugin-demo/configdialog.h
        lib/qmdilib/demos/common/common.qrc

	src/widgets/qmdieditor.cpp
        src/widgets/qmdieditor.h
        src/plugins/texteditor/texteditor_plg.cpp
        src/plugins/texteditor/texteditor_plg.h
	src/plugins/help/help_plg.cpp
        src/plugins/help/help_plg.h
        src/plugins/ProjectManager/ProjectManagerPlg.cpp
        src/plugins/ProjectManager/ProjectManagerPlg.h
	src/plugins/ProjectManager/GenericItems.cpp
        src/plugins/ProjectManager/GenericItems.h
        src/main.cpp

        lib/qmdilib/demos/plugin-demo/plugin_list.ui
        lib/qmdilib/demos/plugin-demo/plugins/editor/editor_cfg.ui
	src/plugins/ProjectManager/ProjectManagerGUI.ui

)
#src/widgets/filesystembrowser.cpp
#src/widgets/filesystembrowser.h
#src/widgets/filesystembrowser.ui
#src/plugins/systembrowser/systembrowser_plg.cpp
#src/plugins/systembrowser/systembrowser_plg.h

target_include_directories( qtedit4 PUBLIC
        src/widgets
        lib/qmdilib/demos/plugin-demo
        lib/qmdilib/demos/plugin-demo/plugins/editor
        lib/qtsourceview/demos/demo4/
        lib/qtsourceview/demos/demo-qate4
)

target_link_libraries(qtedit4 qmdilib qsvte qsvsh )

######################

add_executable(generic-item-complete
        tests/generic-item-complete-window.cpp
        tests/generic-item-complete-test.cpp
        src/plugins/ProjectManager/GenericItems.cpp
)
target_link_libraries(generic-item-complete Qt::Widgets)
target_include_directories(generic-item-complete PUBLIC
        tests/
        .
)
