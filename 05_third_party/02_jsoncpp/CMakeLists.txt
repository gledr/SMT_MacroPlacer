project (build_jsoncpp)

add_custom_target(build_jsoncpp WORKING_DIRECTORY ${InitialPlacer_SOURCE_DIR}/05_third_party/02_jsoncpp COMMAND ./install_jsoncpp.sh)
