cmake_minimum_required(VERSION 3.20)
project(
  engine # Smart Piano Engine
  VERSION 0.0.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(ALSA REQUIRED alsa)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)

  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)
  find_program(LLVM_COV llvm-cov REQUIRED)

  # Create a wrapper script for gcov behavior
  set(GCOV_WRAPPER "${CMAKE_BINARY_DIR}/gcov_wrapper.sh")
  file(WRITE "${GCOV_WRAPPER}" "#!/bin/sh\nexec \"${LLVM_COV}\" gcov \"$@\"")
  execute_process(COMMAND chmod +x "${GCOV_WRAPPER}")

  add_custom_target(coverage
    COMMAND ${LCOV} --directory . --zerocounters --gcov-tool ${GCOV_WRAPPER}
    COMMAND ${CMAKE_CTEST_COMMAND}
    COMMAND ${LCOV} --directory . --capture --output-file coverage.info --gcov-tool ${GCOV_WRAPPER} --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format
    COMMAND ${LCOV} --remove coverage.info '*/test/*' '/nix/store/*' '/usr/*' '*/.cache/*' --output-file coverage_cleaned.info --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format --ignore-errors unused
    COMMAND ${GENHTML} coverage_cleaned.info --output-directory coverage_html --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format --ignore-errors category
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report in build/coverage_html"
  )
endif()

add_compile_options(
  -Wall -Wextra -Wpedantic -Wshadow
  # -Wconversion -Werror
)

add_subdirectory(src)
enable_testing()
add_subdirectory(test)
