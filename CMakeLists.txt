cmake_minimum_required(VERSION 3.20)
project (qtedit4)

set(CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/mold-linker.cmake)

set(QMDILIB_BUILD_EXAMPLES off)
set(QMDILIB_TESTS off)
set(DISABLE_QUTEPART_TESTS on)
set(QHEXVIEW_ENABLE_DIALOGS on)
set(QTEDIT4_WORK_OFFLINE off)

if (QTEDIT4_WORK_OFFLINE)
    # for this to work, you need to make these libraries available yourself.
    add_subdirectory("lib/qmdilib")
    add_subdirectory("lib/qutepart-cpp")
    add_subdirectory("lib/command-palette-widget")
    add_subdirectory("lib/qhexview")
    add_subdirectory("lib/qsimpleupdater")
    add_subdirectory("lib/image-viewer")
else()
    include(cmake/CPM.cmake)
    include(cmake/icons-breeze.cmake)
    
    CPMAddPackage("gh:diegoiast/qmdilib#main")
    CPMAddPackage("gh:diegoiast/qutepart-cpp#main")
    CPMAddPackage("gh:diegoiast/command-palette-widget#main")
    CPMAddPackage("gh:palacaze/image-viewer#master")
    # latest commit from the 5.0 branch
    CPMAddPackage("gh:Dax89/QHexView#40f40f5d6a546646904c094ebefc8216067ab586")
    # latest commit in master 
    CPMAddPackage("gh:alex-spataru/QSimpleUpdater#7f4d8c65686d37b3040335b711eecd4bdc3001fe")

    # Using private fork until upstream fixes the cmake file
    # CPMAddPackage("gh:/dridk/QJsonModel#ed3652666d4954c2262eb5ceee8fda3e613f8c0c")
    CPMAddPackage("gh:/diegoiast/QJsonModel#b4252600ce35cc695ec60807098f9cc36aaa0b1d")
    
    download_breeze_icons(6.4.0)
endif()


find_package(Qt6 COMPONENTS Widgets PrintSupport Network Svg Xml)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(qtedit4_sources
    ${qmdilib_SOURCE_DIR}/demos/common/common.qrc
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/iplugin.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmodel.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmodel.h
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugin_list.ui
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmanager.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginmanager.h   
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/pluginwindow.ui
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystembrowser.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystembrowser.h
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystemwidget.cpp
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo/plugins/filesystem/filesystemwidget.h
    ${qutepart_SOURCE_DIR}/qutepart-syntax-files.qrc
    ${qutepart_SOURCE_DIR}/qutepart-theme-data.qrc

    src/widgets/qmdieditor.cpp
    src/widgets/qmdieditor.h
    src/widgets/textoperationswidget.cpp  
    src/widgets/textoperationswidget.h  
    src/widgets/replaceform.ui
    src/widgets/searchform.ui  
    src/widgets/bannermessage.ui
    src/widgets/gotolineform.ui
    src/widgets/textpreview.cpp
    src/widgets/textpreview.h
    src/widgets/xmltreemodel.cpp
    src/widgets/xmltreemodel.h
    src/plugins/texteditor/texteditor_plg.cpp
    src/plugins/texteditor/texteditor_plg.h
    src/plugins/texteditor/thememanager.cpp
    src/plugins/texteditor/thememanager.h
    src/plugins/help/help_plg.cpp
    src/plugins/help/help_plg.h
    src/plugins/imageviewer/imageviewer_plg.cpp
    src/plugins/imageviewer/imageviewer_plg.h
    src/plugins/hexviewer/hexviewer_plg.cpp
    src/plugins/hexviewer/hexviewer_plg.h
    src/plugins/ProjectManager/ProjectBuildConfig.cpp
    src/plugins/ProjectManager/ProjectBuildConfig.h
    src/plugins/ProjectManager/ProjectManagerPlg.cpp
    src/plugins/ProjectManager/ProjectManagerPlg.h
    src/plugins/ProjectManager/ProjectManagerGUI.ui
    src/plugins/ProjectManager/ProjectSearch.cpp
    src/plugins/ProjectManager/ProjectSearch.h
    src/plugins/ProjectManager/ProjectSearchGUI.ui
    src/plugins/ProjectManager/output.ui
    src/plugins/ProjectManager/GenericItems.cpp
    src/plugins/ProjectManager/GenericItems.h
    src/plugins/ProjectManager/kitdefinitions.h
    src/plugins/ProjectManager/kitdefinitions.cpp
    src/plugins/ProjectManager/kitdefinitionmodel.h
    src/plugins/ProjectManager/kitdefinitionmodel.cpp
    src/plugins/ProjectManager/kitdetector.h
    src/plugins/ProjectManager/kitdetector.cpp
    src/main.cpp
    qtedit4.qrc
)

if (WIN32)
    add_executable(qtedit4 WIN32 ${qtedit4_sources} qtedit4.rc)
else()
    add_executable(qtedit4 ${qtedit4_sources})
endif()


target_include_directories(qtedit4 PUBLIC
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo
    ${qutepart_SOURCE_DIR}/include 
    src
)
target_link_libraries(qtedit4 PUBLIC Qt6::Core Qt6::Widgets Qt6::PrintSupport Qt6::Svg Qt6::Xml qmdilib qutepart CommandPaletteWidget Pal::ImageViewer QHexView QSimpleUpdater QJsonModelStatic)

if(MSVC)
   # if we don't do this - file will be under Debug/Relase subdir
   set_target_properties(qtedit4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}$<0:>)
   target_compile_options(qtedit4 PRIVATE /W4)
else()
  set_target_properties(qtedit4 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  target_compile_options(qtedit4 PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS qtedit4
        RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_BINARY_DIR}/share/icons/
    DESTINATION share/icons
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    FILES_MATCHING PATTERN "*/*"
    PATTERN "*")

if (WIN32)
    install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.svg
            DESTINATION /)
    install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.ico
            DESTINATION /)
else()
    install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.svg
            DESTINATION share/icons/hicolor/scalable/apps)
    install(FILES ${CMAKE_SOURCE_DIR}/qtedit4.desktop
            DESTINATION share/applications
            RENAME qtedit4.desktop)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/share/
            DESTINATION share/
            DIRECTORY_PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
            FILES_MATCHING PATTERN "*/*"
            PATTERN "*"
            PATTERN "*")
endif()
