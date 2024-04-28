cmake_minimum_required(VERSION 3.20)

project (qtsourceview)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(QTSOURCEVIEW_BUILD_EXAMPLES "Build examples for qmdilib" ON)

find_package(Qt6 COMPONENTS Widgets PrintSupport Core5Compat Xml)

add_library(qsvsh
    src/qsvsh/CMakeLists.txt
    src/qsvsh/debug_info.h
    src/qsvsh/qorderedmap.h
    src/qsvsh/qsvcolordef.cpp
    src/qsvsh/qsvcolordeffactory.cpp
    src/qsvsh/qsvcolordeffactory.h
    src/qsvsh/qsvcolordef.h
    src/qsvsh/qsvlangdef.cpp
    src/qsvsh/qsvlangdeffactory.cpp
    src/qsvsh/qsvlangdeffactory.h
    src/qsvsh/qsvlangdef.h
    src/qsvsh/qsvsyntaxhighlighter.cpp
    src/qsvsh/qsvsyntaxhighlighter.h
    src/qsvsh/qtsourceview.qrc
)
target_link_libraries(qsvsh Qt::Widgets Qt::Xml)
target_include_directories(qsvsh PRIVATE src)
target_include_directories(qsvsh PUBLIC src)

add_library(qsvte
    src/qsvte/bannermessage.ui 
    src/qsvte/qsvdefaulthighlighter.cpp 
    src/qsvte/qsvdefaulthighlighter.h 
    src/qsvte/qsvsyntaxhighlighterbase.cpp 
    src/qsvte/qsvsyntaxhighlighterbase.h src/qsvte/qsvtextedit.cpp 
    src/qsvte/qsvtextedit.h 
    src/qsvte/qsvtextoperationswidget.cpp 
    src/qsvte/qsvtextoperationswidget.h 
    src/qsvte/replaceform.ui 
    src/qsvte/searchform.ui
)
target_link_libraries(qsvte PUBLIC Qt::Widgets)
target_include_directories(qsvte PRIVATE src)
target_include_directories(qsvte PUBLIC src)

if (QTSOURCEVIEW_BUILD_EXAMPLES)

add_executable(demo1
    demos/demo1/main1.cpp
)
target_link_libraries(demo1 qsvsh)

add_executable(demo2
    demos/demo2/main2.cpp
    demos/demo2/mainwindow2.cpp
)
target_link_libraries(demo2 qsvsh)

add_executable(demo3
    demos/demo3/main3.cpp
    demos/demo3/mainwindow3.cpp
)
target_link_libraries(demo3 qsvsh)


add_executable(demo4
    demos/demo4/main4.cpp
    demos/demo4/mainwindow4.cpp
)
target_link_libraries(demo4 qsvte)

add_executable(demo5
    demos/demo5/main5.cpp
)
target_link_libraries(demo5 qsvte qsvsh)

endif()
