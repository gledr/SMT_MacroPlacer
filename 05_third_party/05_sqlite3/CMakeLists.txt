project (build_sqlite3)

add_custom_target(build_sqlite3 WORKING_DIRECTORY ${SMT_MacroPlacer_SOURCE_DIR}/05_third_party/05_sqlite3 COMMAND ./install_sqlite3.sh)
