cmake_minimum_required(VERSION 3.20)
project (qtedit4)

include(cmake/CPM.cmake)
set(QMDILIB_BUILD_EXAMPLES off)
set(DISABLE_QUTEPART_TESTS on)
CPMAddPackage("gh:elcuco/qmdilib#main")

set(QTSOURCEVIEW_BUILD_EXAMPLES Off)
add_subdirectory( lib/qtsourceview )

find_package(Qt6 COMPONENTS Widgets PrintSupport Network)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(qtedit4_sources
    ${qmdilib_SOURCE_DIR}/demos/common/common.qrc
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/configdialog.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/configdialog.h
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/iplugin.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmodel.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmodel.h
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugin_list.ui
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmanager.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmanager.h   
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginwindow.ui
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystembrowser.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystembrowser.h

    src/widgets/qmdieditor.cpp
    src/widgets/qmdieditor.h
    src/plugins/texteditor/texteditor_plg.cpp
    src/plugins/texteditor/texteditor_plg.h
    src/plugins/help/help_plg.cpp
    src/plugins/help/help_plg.h
    src/plugins/ProjectManager/ProjectManagerPlg.cpp
    src/plugins/ProjectManager/ProjectManagerPlg.h
    src/plugins/ProjectManager/ProjectManagerGUI.ui
    src/plugins/ProjectManager/ProjectSearch.cpp
    src/plugins/ProjectManager/ProjectSearch.h
    src/plugins/ProjectManager/ProjectSearchGUI.ui
    src/plugins/ProjectManager/GenericItems.cpp
    src/plugins/ProjectManager/GenericItems.h
    src/main.cpp

)

if (WIN32)
    add_executable(qtedit4 WIN32 ${qtedit4_sources})
else()
    add_executable(qtedit4 ${qtedit4_sources})
endif()

# lib/qmdilib/demos/plugin-demo/plugins/editor
target_include_directories(qtedit4 PUBLIC
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo
    src/widgets
    lib/qtsourceview/demos/demo4/
    lib/qtsourceview/demos/demo-qate4
)
target_link_libraries(qtedit4 qmdilib qsvte qsvsh)

if(MSVC)
   # if we don't do this - file will be under Debug/Relase subdir
   set_target_properties(qtedit4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}$<0:>)
   target_compile_options(qtedit4 PRIVATE /W4)
else()
  set_target_properties(qtedit4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  target_compile_options(qtedit4 PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET qtedit4 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/lib/qtsourceview/data
    ${CMAKE_BINARY_DIR}/share
    COMMENT "Copying directory to build directory ${EXECUTABLE_OUTPUT_PATH}/data"
)

install(TARGETS qtedit4
        RUNTIME DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME qtedit4.svg)
install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.desktop
        DESTINATION share/applications
        RENAME qtedit4.desktop)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/lib/qtsourceview/data/
        DESTINATION share/
        DIRECTORY_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        FILES_MATCHING PATTERN "*/*"
        PATTERN "*"
)
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
set_target_properties(generic-item-complete PROPERTIES EXCLUDE_FROM_ALL TRUE)