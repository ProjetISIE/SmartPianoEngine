add_library(
  ${PROJECT_NAME} STATIC
  RtMidiInput.cpp
  GameEngine.cpp
  NoteGame.cpp
  ChordGame.cpp
  UdsTransport.cpp
  ChordRepository.cpp
  ChallengeFactory.cpp
  GameManager.cpp
  SocketManager.cpp
  AnswerValidator.cpp)
target_include_directories(
  ${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include ${RTMIDI_INCLUDE_DIRS}
                         ${ALSA_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${ALSA_LIBRARIES}
                                             ${RTMIDI_LIBRARIES} dl pthread m)

add_executable(${PROJECT_NAME}Main main.cpp)
target_link_libraries(${PROJECT_NAME}Main ${PROJECT_NAME})
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}Main RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
