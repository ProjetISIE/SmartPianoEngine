add_library(
  ${PROJECT_NAME}Lib STATIC
  GameManager.cpp
  SocketManager.cpp
  GenererNoteAleatoire.cpp
  ValidationNote.cpp
  LectureNoteJouee.cpp
  BaseAccords.cpp
  ${CMAKE_SOURCE_DIR}/include/SocketManager.hpp)
target_include_directories(${PROJECT_NAME}Lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
  ${PROJECT_NAME}Lib
  PUBLIC Qt5::Core # Qt5 library
         Qt5::Gui # Qt5 library
         Qt5::Network # Qt5 library
         Qt5::Multimedia # Qt5 library
         Qt5::MultimediaWidgets # Qt5 library
         Threads::Threads # System library
         ALSA::ALSA # System audio library
         rtmidi # MIDI library
         dl pthread m
)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}Lib)
