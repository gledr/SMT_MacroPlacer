project (build_z3)

add_custom_target(build_z3 WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/01_z3 COMMAND ./install_z3.sh)
