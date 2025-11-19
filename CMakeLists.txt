cmake_minimum_required(VERSION 3.20)
project (qtedit4)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/mold-linker.cmake)

set(QMDILIB_BUILD_EXAMPLES off)
set(QMDILIB_TESTS off)
set(DISABLE_QUTEPART_TESTS on)
set(QHEXVIEW_ENABLE_DIALOGS on)
set(COMMAND_PALLETE_ENABLE_TESTS off)

option(QTEDIT4T4_WORK_OFFLINE off)

if (QTEDIT4_WORK_OFFLINE)
    # For this to work, you need to make these libraries available yourself.
    # On Linux/macOS, you can run the script `get-code.sh`. This might work on Windows using
    # git-bash, but is untested.
    add_subdirectory("lib/qmdilib")
    add_subdirectory("lib/qutepart-cpp")
    add_subdirectory("lib/command-palette-widget")
    add_subdirectory("lib/image-viewer")
    add_subdirectory("lib/QJsonModel")
    add_subdirectory("lib/QSimpleUpdater")
    add_subdirectory("lib/QHexView")
else()
    include(cmake/CPM.cmake)
    include(cmake/icons-breeze.cmake)

    CPMAddPackage("gh:diegoiast/qmdilib#d044a71f5d3de265c34bfececff2502c284663f9")
    CPMAddPackage("gh:diegoiast/qutepart-cpp#e90c41b8cd511217cc7408be208d517f1ca8b38a")
    CPMAddPackage("gh:diegoiast/command-palette-widget#3c4e338216936097e89e8a19e1c325018d7e750c")
    CPMAddPackage("gh:palacaze/image-viewer#125ea784f93327b471cd9b6b156285e40e70280a")
    CPMAddPackage("gh:dridk/QJsonModel#f5fa5988c0ee52fff39a42973c964d5480951f86")
    CPMAddPackage("gh:alex-spataru/QSimpleUpdater#8e7017f7fbdc2b4b1a26ed1eef9ebcba6a50639c")
    CPMAddPackage("gh:Dax89/QHexView#5fe6d9851ecfae0acf26d1659ee3cb579d03202f")
    download_breeze_icons(6.4.0)
endif()


find_package(Qt6 COMPONENTS Widgets PrintSupport Network Svg Xml)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)
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

    src/widgets/HistoryLineEdit.cpp
    src/widgets/HistoryLineEdit.h
    src/widgets/gotolineform.ui
    src/widgets/bannermessage.ui
    src/widgets/replaceform.ui
    src/widgets/searchform.ui
    src/widgets/textoperationswidget.cpp
    src/widgets/textoperationswidget.h
    src/widgets/textpreview.cpp
    src/widgets/textpreview.h
    src/widgets/xmltreemodel.cpp
    src/widgets/xmltreemodel.h
    src/widgets/LoadingWidget.cpp
    src/widgets/LoadingWidget.hpp
    src/widgets/qmdiSplitTab.cpp
    src/widgets/qmdiSplitTab.h
    src/widgets/qmdieditor.cpp
    src/widgets/qmdieditor.h
    src/widgets/SplitTabWidget.cpp
    src/widgets/SplitTabWidget.h
    src/widgets/qmdiSplitTab.cpp
    src/widgets/qmdiSplitTab.h
    src/widgets/FilesList.cpp
    src/widgets/FilesList.hpp

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
    src/plugins/ProjectManager/BuildRunOutput.ui
    src/plugins/ProjectManager/CompilerOutputDecoders.cpp
    src/plugins/ProjectManager/CompilerOutputDecoders.h
    src/plugins/ProjectManager/kitdefinitions.h
    src/plugins/ProjectManager/kitdefinitions.cpp
    src/plugins/ProjectManager/kitdefinitionmodel.h
    src/plugins/ProjectManager/kitdefinitionmodel.cpp
    src/plugins/ProjectManager/kitdetector.h
    src/plugins/ProjectManager/kitdetector.cpp
    src/plugins/ProjectManager/ProjectBuildConfig.cpp
    src/plugins/ProjectManager/ProjectBuildConfig.h
    src/plugins/ProjectManager/ProjectIssuesWidget.h
    src/plugins/ProjectManager/ProjectIssuesWidget.cpp
    src/plugins/ProjectManager/ProjectIssuesWidget.ui
    src/plugins/ProjectManager/ProjectManagerPlg.cpp
    src/plugins/ProjectManager/ProjectManagerPlg.h
    src/plugins/ProjectManager/ProjectManagerGUI.ui
    src/plugins/ProjectManager/ProjectSearch.cpp
    src/plugins/ProjectManager/ProjectSearch.h
    src/plugins/ProjectManager/ProjectSearchGUI.ui
    src/plugins/CTags/CTagsPlugin.cpp
    src/plugins/CTags/CTagsPlugin.hpp
    src/plugins/CTags/CTagsLoader.cpp
    src/plugins/CTags/CTagsLoader.hpp
    src/plugins/SplitTabsPlugin/SplitTabsPlugin.cpp
    src/plugins/SplitTabsPlugin/SplitTabsPlugin.hpp
    src/AnsiToHTML.cpp
    src/AnsiToHTML.hpp
    src/main.cpp
    qtedit4.qrc
)

if (WIN32)
    add_executable(qtedit4 WIN32 ${qtedit4_sources} qtedit4.rc)
else()
    add_executable(qtedit4 ${qtedit4_sources})
endif()

if (!MINGW)
    set_property(TARGET qtedit4 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_include_directories(qtedit4 PUBLIC
    ${qmdilib_SOURCE_DIR}/demos/plugin-demo
    ${qutepart_SOURCE_DIR}/include
    src
)
target_link_libraries(qtedit4 PUBLIC Qt6::Core Qt6::Widgets Qt6::Concurrent Qt6::Network Qt6::Svg Qt6::Xml
    qmdilib
    qutepart
    CommandPaletteWidget
    Pal::ImageViewer
    QHexView
    QSimpleUpdater
    QJsonModelStatic
)

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
