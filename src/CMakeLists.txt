add_library(
  ${PROJECT_NAME} STATIC
  RtMidiInput.cpp
  GameEngine.cpp
  NoteGame.cpp
  ChordGame.cpp
  ChordRepository.cpp
  ChallengeFactory.cpp
  AnswerValidator.cpp)
target_include_directories(
  ${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include ${RTMIDI_INCLUDE_DIRS}
                         ${ALSA_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads ${ALSA_LIBRARIES}
                                             ${RTMIDI_LIBRARIES} dl pthread m)

add_library(${PROJECT_NAME}comm STATIC UdsTransport.cpp)
target_include_directories(${PROJECT_NAME}comm
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}comm PUBLIC Threads::Threads dl pthread m)

add_executable(main main.cpp)
target_link_libraries(main ${PROJECT_NAME} ${PROJECT_NAME}comm)
include(GNUInstallDirs)
install(TARGETS main RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
