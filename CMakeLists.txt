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

add_compile_options(
  -Wall -Wextra -Wpedantic -Wshadow
  # -Wconversion -Werror
)

option(COVERAGE "Mesure couverture de code" ON)
if(COVERAGE)
  add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
  add_link_options(-fprofile-instr-generate)
endif()
set(PROFRAW_MAIN "main.profraw")
set(PROFRAW_TESTS "tests.profraw")
set(PROFDATA_FILE "coverage.profdata")

add_subdirectory(src)
enable_testing()
add_subdirectory(test)

# Exécute les exécutables pour générer données de profilage (main.profraw…)
# add_custom_target(run_main COMMAND
# LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/${PROFRAW_MAIN} ./src/main COMMENT
# "Exécution du main pour générer ${PROFRAW_MAIN}" )
add_custom_target(
  run_tests
  COMMAND LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/${PROFRAW_TESTS} ctest
  COMMENT "Exécution des tests pour générer ${PROFRAW_TESTS}")
# Fusionne les données de profilage dans coverage.profdata
add_custom_target(
  merge_coverage_data
  COMMAND
    llvm-profdata merge -sparse -output=${CMAKE_BINARY_DIR}/${PROFDATA_FILE}
    # ${CMAKE_BINARY_DIR}/${PROFRAW_MAIN}
    ${CMAKE_BINARY_DIR}/${PROFRAW_TESTS}
  DEPENDS run_tests # run_main
  COMMENT "Fusion données couverture: ${PROFDATA_FILE}")
# Génération du rapport de couverture de code HTML
add_custom_target(
  report_coverage
  COMMAND
    llvm-cov show ./src/main -instr-profile=${CMAKE_BINARY_DIR}/${PROFDATA_FILE}
    -format=html -output-dir=${CMAKE_BINARY_DIR}/coverage
  DEPENDS merge_coverage_data
  COMMENT "Rapport de couverture HTML généré: ${CMAKE_BINARY_DIR}/coverage")
# add_dependencies(run_main main)
add_dependencies(run_tests integrationTest) # WARN Besoin de tous les tests ?
add_dependencies(report_coverage merge_coverage_data)
