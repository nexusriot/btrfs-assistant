cmake_minimum_required(VERSION 3.5)

project(btrfs-assistant VERSION 1.5 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

find_package(QT NAMES Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)

message(STATUS "Btrfs Assistant will be built for install into ${CMAKE_INSTALL_PREFIX}")
