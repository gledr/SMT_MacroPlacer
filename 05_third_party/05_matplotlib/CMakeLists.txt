project (build_matplotlib)

add_custom_target(build_matplotlib WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/05_matplotlib COMMAND ./install_matplotlib.sh)
