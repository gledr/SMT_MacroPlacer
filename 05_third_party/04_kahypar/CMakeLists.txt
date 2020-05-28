project (build_kahypar)

add_custom_target(build_kahypar WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/04_kahypar COMMAND ./install_kahypar.sh)
