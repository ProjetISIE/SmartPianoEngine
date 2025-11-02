cmake_minimum_required(VERSION 3.16)
project(
  PianoTrainer
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt5 5.15
  COMPONENTS Core Gui Network Multimedia MultimediaWidgets
  REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)
pkg_check_modules(PulseAudio REQUIRED libpulse)
find_package(Threads REQUIRED)

set(SRC
    src/main.cpp
    src/GameManager.cpp
    src/SocketManager.cpp
    src/GenererNoteAleatoire.cpp
    src/ValidationNote.cpp
    src/LectureNoteJouee.cpp
    src/GestionSon.cpp
    src/BaseAccords.cpp
    rtmidi/RtMidi.cpp)
set(HEADER
    include/GameManager.h
    include/SocketManager.h
    include/GenererNoteAleatoire.h
    include/ValidationNote.h
    include/LectureNoteJouee.h
    include/GestionSon.h
    include/BaseAccords.h
    rtmidi/RtMidi.h
    include/Logger.h)

add_executable(${PROJECT_NAME} ${SRC} ${HEADER})
target_include_directories(${PROJECT_NAME} PRIVATE include rtmidi)
target_compile_definitions(${PROJECT_NAME} PRIVATE __LINUX_ALSA__)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Qt5::Core # Qt5 libraries
          Qt5::Gui # Qt5 libraries
          Qt5::Network # Qt5 libraries
          Qt5::Multimedia # Qt5 libraries
          Qt5::MultimediaWidgets # Qt5 libraries
          ${ALSA_LIBRARIES} # System libraries
          ${PulseAudio_LIBRARIES} # System libraries
          Threads::Threads # System libraries
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
