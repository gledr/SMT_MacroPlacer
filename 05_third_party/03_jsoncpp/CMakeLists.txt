project (build_jsoncpp)

add_custom_target(build_jsoncpp WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/03_jsoncpp COMMAND ./install_jsoncpp.sh)
