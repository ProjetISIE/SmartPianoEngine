cmake_minimum_required(VERSION 3.20)
project(
  SmartPiano
  VERSION 0.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt5
  COMPONENTS Core Gui Network Multimedia MultimediaWidgets Test
  REQUIRED)
find_package(ALSA REQUIRED) # Consider removing
find_package(Threads REQUIRED)
find_package(rtmidi REQUIRED)
find_package(doctest REQUIRED)
find_package(PkgConfig REQUIRED) # Consider removing
pkg_check_modules(JACK REQUIRED jack)

add_subdirectory(src)
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CTest) # TESTS
add_subdirectory(test)
