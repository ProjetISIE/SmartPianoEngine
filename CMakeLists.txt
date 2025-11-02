cmake_minimum_required(VERSION 3.16)
project(
  SmartPiano
  VERSION 0.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt5
  COMPONENTS Core Gui Network Multimedia MultimediaWidgets Test
  REQUIRED)
find_package(ALSA REQUIRED)
find_package(Threads REQUIRED)
find_package(rtmidi REQUIRED)
find_package(doctest REQUIRED)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/GameManager.cpp
  src/SocketManager.cpp
  src/GenererNoteAleatoire.cpp
  src/ValidationNote.cpp
  src/LectureNoteJouee.cpp
  src/BaseAccords.cpp
  include/SocketManager.h)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Qt5::Core # Qt5 library
          Qt5::Gui # Qt5 library
          Qt5::Network # Qt5 library
          Qt5::Multimedia # Qt5 library
          Qt5::MultimediaWidgets # Qt5 library
          Threads::Threads # System library
          ALSA::ALSA # System audio library
          rtmidi # MIDI library
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CTest) # TESTS

add_executable(BaseAccordsTest src/BaseAccords.cpp test/BaseAccordsTest.cpp)
target_include_directories(BaseAccordsTest PRIVATE include)
target_link_libraries(
  BaseAccordsTest PRIVATE doctest::doctest # C++ testing library
                          Qt5::Test # Qt5 testing library
)

add_test(NAME BaseAccordsTest COMMAND BaseAccordsTest)
