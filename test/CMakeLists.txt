add_executable(BaseAccordsTest BaseAccordsTest.cpp)
target_include_directories(BaseAccordsTest PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BaseAccordsTest PRIVATE SmartPianoLib doctest::doctest
                                              Qt5::Test)
add_test(NAME BaseAccordsTest COMMAND BaseAccordsTest)
