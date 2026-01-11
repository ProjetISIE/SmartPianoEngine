add_library(
  ${PROJECT_NAME}Lib STATIC
  UdsTransport.cpp RtMidiInput.cpp GameEngine.cpp NoteGame.cpp)
target_include_directories(
  ${PROJECT_NAME}Lib PUBLIC ${CMAKE_SOURCE_DIR}/include ${RTMIDI_INCLUDE_DIRS}
                            ${ALSA_INCLUDE_DIRS})
target_link_libraries(
  ${PROJECT_NAME}Lib
  PUBLIC Threads::Threads # System library
         ${ALSA_LIBRARIES} # System audio library
         ${RTMIDI_LIBRARIES} # MIDI library
         dl
         pthread
         m)

add_executable(${PROJECT_NAME} main_new.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}Lib)
