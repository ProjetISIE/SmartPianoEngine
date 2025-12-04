add_executable(BaseAccords BaseAccordsTest.cpp)
target_include_directories(BaseAccords PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BaseAccords PRIVATE ${PROJECT_NAME}Lib doctest::doctest
                                          Qt5::Test)
add_test(NAME BaseAccords COMMAND BaseAccords)

add_executable(GameManager GameManagerTest.cpp)
target_include_directories(GameManager PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(GameManager PRIVATE ${PROJECT_NAME}Lib doctest::doctest
                                          Qt5::Test)
add_test(NAME GameManager COMMAND GameManager)

add_executable(GenererNoteAleatoire GenererNoteAleatoireTest.cpp)
target_include_directories(GenererNoteAleatoire
                           PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(GenererNoteAleatoire PRIVATE ${PROJECT_NAME}Lib
                                                   doctest::doctest Qt5::Test)
add_test(NAME GenererNoteAleatoire COMMAND GenererNoteAleatoire)

add_executable(LectureNoteJouee LectureNoteJoueeTest.cpp)
target_include_directories(LectureNoteJouee PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(LectureNoteJouee PRIVATE ${PROJECT_NAME}Lib doctest::doctest
                                               Qt5::Test)
add_test(NAME LectureNoteJouee COMMAND LectureNoteJouee)

add_executable(SocketManager SocketManagerTest.cpp)
target_include_directories(SocketManager PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(SocketManager PRIVATE ${PROJECT_NAME}Lib doctest::doctest
                                            Qt5::Test)
# FIXME Seems like log files can’t be written or found in nix build env
# add_test(NAME SocketManager COMMAND SocketManager)

add_executable(ValidationNote ValidationNoteTest.cpp)
target_include_directories(ValidationNote PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ValidationNote PRIVATE ${PROJECT_NAME}Lib doctest::doctest
                                             Qt5::Test)
add_test(NAME ValidationNote COMMAND ValidationNote)
