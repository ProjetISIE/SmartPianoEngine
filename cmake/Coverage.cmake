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
  # C'est nécessaire car lcov attend un exécutable 'gcov' simple
  set(GCOV_WRAPPER "${CMAKE_BINARY_DIR}/gcov_wrapper.sh")
  file(WRITE "${GCOV_WRAPPER}" "#!/bin/sh\nexec \"${LLVM_COV}\" gcov \"$@\"")
  execute_process(COMMAND chmod +x "${GCOV_WRAPPER}")

  # Définition de la cible 'coverage'
  add_custom_target(coverage
    # 1. Réinitialiser les compteurs
    COMMAND ${LCOV} --directory . --zerocounters --gcov-tool ${GCOV_WRAPPER}
    
    # 2. Exécuter les tests
    COMMAND ${CMAKE_CTEST_COMMAND}
    
    # 3. Capturer les données (avec suppression d'erreurs pour compatibilité Clang/Lcov)
    COMMAND ${LCOV} --directory . --capture --output-file coverage.info 
            --gcov-tool ${GCOV_WRAPPER} 
            --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format
    
    # 4. Nettoyer les données (retirer tests, libs système, etc.)
    COMMAND ${LCOV} --remove coverage.info 
            '*/test/*' '/nix/store/*' '/usr/*' '*/.cache/*'
            --output-file coverage_cleaned.info 
            --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format --ignore-errors unused
    
    # 5. Générer le rapport HTML
    COMMAND ${GENHTML} coverage_cleaned.info 
            --output-directory coverage_html 
            --ignore-errors inconsistent --ignore-errors unsupported --ignore-errors format --ignore-errors category
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report in build/coverage_html"
  )
endif()
