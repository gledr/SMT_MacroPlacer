project (build_boost)

add_custom_target(build_boost WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/04_boost COMMAND ./install_boost.sh)
