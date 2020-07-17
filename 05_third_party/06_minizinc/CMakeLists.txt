project (build_minizinc)

add_custom_target(build_minizinc WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/06_minizinc COMMAND ./install_minizinc.sh)
