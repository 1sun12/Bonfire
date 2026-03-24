cmake_minimum_required(VERSION 3.16)
project(Bonfire VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

qt_add_executable(Bonfire
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/shard.h
    src/database.h
    src/database.cpp
    src/sm2.h
    src/syntaxhighlighter.h
    src/syntaxhighlighter.cpp
    src/views/dashboardwidget.h
    src/views/dashboardwidget.cpp
    src/views/librarywidget.h
    src/views/librarywidget.cpp
    src/views/shardeditorwidget.h
    src/views/shardeditorwidget.cpp
    src/views/reviewwidget.h
    src/views/reviewwidget.cpp
    src/dialogs/quickcapturedialog.h
    src/dialogs/quickcapturedialog.cpp
)

target_link_libraries(Bonfire PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)
target_include_directories(Bonfire PRIVATE src)
