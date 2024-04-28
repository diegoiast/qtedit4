cmake_minimum_required(VERSION 3.20)

project (qmdilib) 

include(FetchContent)
include(cmake/conan.cmake)
include(cmake/CPM.cmake)
include(cmake/CreateResources.cmake)
include(cmake/mold-linker.cmake)

cpmaddpackage(
    NAME     Format.cmake
    VERSION     1.8.1
    GITHUB_REPOSITORY
    TheLartians/Format.cmake
    OPTIONS # set to yes skip cmake formatting
    "FORMAT_SKIP_CMAKE NO"
    # path to exclude (optional, supports regular expressions)
    "CMAKE_FORMAT_EXCLUDE cmake/*"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QMDILIB_VERSION "0.1.0")
set(QMDILIB_SOVERSION "0")



option(QMDILIB_BUILD_EXAMPLES "Build examples for qmdilib" ON)

find_package(Qt6 COMPONENTS Widgets PrintSupport)

add_library(qmdilib 
    src/qmdiactiongroup.h
    src/qmdiactiongroup.cpp
    src/qmdiactiongrouplist.h
    src/qmdiactiongrouplist.cpp
    src/qmdiclient.h
    src/qmdiclient.cpp
    src/qmdihost.h
    src/qmdihost.cpp
    src/qmdiserver.h
    src/qmdiserver.cpp
    src/qmditabwidget.h
    src/qmditabwidget.cpp
)
target_link_libraries(qmdilib Qt::Widgets)
target_include_directories(qmdilib PUBLIC src)
set_property(TARGET qmdilib PROPERTY AUTOMOC ON)

set_target_properties(qmdilib PROPERTIES
    VERSION     ${QMDILIB_VERSION}
    SOVERSION   ${QMDILIB_SOVERSION}
)

if (QMDILIB_BUILD_EXAMPLES)

add_executable(demo1 demos/demo1/main.cpp demos/demo1/mainwindow.cpp)
set_property(TARGET demo1 PROPERTY AUTOMOC ON)
target_link_libraries(demo1 qmdilib)

add_executable(demo2 
    demos/demo2/main2.cpp
    demos/demo2/mainwindow2.cpp
    demos/common/helpbrowse.cpp
    demos/common/qexeditor.cpp
    demos/common/common.qrc
)
set_property(TARGET demo2 PROPERTY AUTOMOC ON)
target_link_libraries(demo2 qmdilib)
target_include_directories(demo2 PRIVATE demos/common)

add_executable(pluginmanager
    demos/common/qexeditor.cpp
    demos/common/qexeditor.h
    demos/common/helpbrowse.cpp
    demos/common/helpbrowse.h

    demos/plugin-demo/configdialog.cpp
    demos/plugin-demo/configdialog.h
    demos/plugin-demo/iplugin.cpp
    demos/plugin-demo/iplugin.h
    demos/plugin-demo/main6.cpp
    demos/plugin-demo/pluginmanager.cpp
    demos/plugin-demo/plugin_list.ui
    demos/plugin-demo/richtext.qrc
    demos/plugin-demo/pluginmanager.cpp
    demos/plugin-demo/pluginmodel.cpp
    demos/plugin-demo/pluginmodel.h
    demos/plugin-demo/plugins/editor/editor_plg.cpp
    demos/plugin-demo/plugins/editor/editor_plg.h
    demos/plugin-demo/plugins/editor/editor_cfg.ui
    demos/plugin-demo/plugins/editor/qexeditor2.cpp
    demos/plugin-demo/plugins/editor/qexeditor2.h
    demos/plugin-demo/plugins/help/help_plg.cpp
    demos/plugin-demo/plugins/help/help_plg.h
    demos/plugin-demo/plugins/help/help_cfg.ui
    demos/plugin-demo/plugins/richtext/richtextwidget.cpp
    demos/plugin-demo/plugins/richtext/richtextwidget.h
    demos/plugin-demo/plugins/richtext/richtextclient.cpp
    demos/plugin-demo/plugins/richtext/richtext_plg.cpp
    demos/plugin-demo/plugins/richtext/richtext_plg.h
    demos/plugin-demo/plugins/richtext/richtextclient.h
)
set_property(TARGET pluginmanager PROPERTY AUTOMOC ON)
set_property(TARGET pluginmanager PROPERTY AUTOUIC ON)
set_property(TARGET pluginmanager PROPERTY AUTORCC ON)
target_include_directories(pluginmanager PRIVATE demos/common demos/plugin-demo/)
target_link_libraries(pluginmanager qmdilib)

endif()
