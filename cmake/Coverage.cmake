# Module pour gérer la couverture de code avec LCOV et LLVM
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
  # Configuration des flags de compilation
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
  # Recherche des outils nécessaires
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)
  find_program(LLVM_COV llvm-cov REQUIRED)
  # Création du script wrapper pour faire passer llvm-cov pour gcov
  set(GCOV_WRAPPER "${CMAKE_BINARY_DIR}/gcov_wrapper.sh")
  file(WRITE "${GCOV_WRAPPER}" "#!/bin/sh\nexec \"${LLVM_COV}\" gcov \"$@\"")
  execute_process(COMMAND chmod +x "${GCOV_WRAPPER}")
  # Définition de la cible 'coverage'
  add_custom_target(coverage
    COMMAND ${LCOV} --directory . --zerocounters --gcov-tool ${GCOV_WRAPPER}
    COMMAND ${CMAKE_CTEST_COMMAND}
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_coverage.sh "${LCOV}" "${GENHTML}" "${GCOV_WRAPPER}" "${CMAKE_SOURCE_DIR}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report in build/coverage_html"
  )
endif()
