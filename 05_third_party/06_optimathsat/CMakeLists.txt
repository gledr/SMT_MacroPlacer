project (build_optimathsat)

add_custom_target(build_optimathsat WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/06_optimathsat COMMAND ./install_optimathsat.sh)
