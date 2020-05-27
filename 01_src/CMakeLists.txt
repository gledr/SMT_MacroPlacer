find_package(Boost REQUIRED COMPONENTS program_options filesystem)

############################################################
# include directories 
############################################################
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${DEFLIB_HOME}/def
    ${DEFLIB_HOME}/defzlib
    ${DEFLIB_HOME}/cdef
    ${DEFLIB_HOME}/cdefzlib

    ${LEFLIB_HOME}/lef
    ${LEFLIB_HOME}/lefzlib
    ${LEFLIB_HOME}/clef
    ${LEFLIB_HOME}/clefzlib

    ${THIRD_PARTY_INCLUDE}
    ${THIRD_PARTY_LIB}
    ${PARQUET_HOME}/src
    ${ABKCOMMON_HOME}
    ${KAHYPAR_HOME}/include

    compontents
    macrocircuit
    smt
    tree
    evaluate
    utils
    clustering
    .
)

############################################################
# Set Picky Compiler only for own code 
# to suppress "millions" of warnings from modules
############################################################
set_source_files_properties(
  .
  PROPERTIES
  COMPILE_FLAGS "-Wall -Wextra"
)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(smt_placer
    main.cpp
    placer.cpp
    object.cpp
    macrocircuit/defParser.cpp
    macrocircuit/lefParser.cpp
    macrocircuit/lefdefIO.cpp
    macrocircuit/macrocircuit.cpp
    macrocircuit/supplement.cpp
    macrocircuit/bookshelf.cpp
    macrocircuit/layout.cpp
    macrocircuit/supplementlayout.cpp
    macrocircuit/partitioning.cpp
    macrocircuit/parquet.cpp
    compontents/component.cpp
    compontents/macro.cpp
    compontents/pin.cpp
    compontents/terminal.cpp
    compontents/partition.cpp
    compontents/cell.cpp
    compontents/supplementmacro.cpp
    compontents/supplementpin.cpp
    compontents/supplementterminal.cpp
    tree/edge.cpp
    tree/node.cpp
    tree/tree.cpp
    evaluate/evaluate.cpp
    utils/timer.cpp
    utils/logger.cpp
    utils/base_logger.cpp
    utils/utils.cpp
    utils/base_database.cpp
    utils/database.cpp
    utils/exception.cpp
    clustering/centroid.cpp
    clustering/cluster.cpp
    clustering/kmeans.cpp
)

add_executable(bookshelf_converter
    utils/bookshelf_converter.cpp
    object.cpp
    macrocircuit/bookshelf.cpp
    macrocircuit/supplement.cpp
    macrocircuit/supplementlayout.cpp
    tree/tree.cpp
    tree/node.cpp
    tree/edge.cpp
    utils/exception.cpp
    utils/logger.cpp
    utils/base_logger.cpp
    utils/utils.cpp
    compontents/terminal.cpp
    compontents/macro.cpp
    compontents/component.cpp
    compontents/pin.cpp
    compontents/cell.cpp
    compontents/supplementpin.cpp
    compontents/supplementmacro.cpp
)

add_dependencies(smt_placer
    def
    lef
    build_z3
    build_jsoncpp
    build_sqlite3
    placer_config
    ABKCommon
    ParquetFP
)

target_link_libraries(smt_placer
    ${Boost_LIBRARIES}
    def
    defzlib
    lef
    lefzlib
    ${THIRD_PARTY_LIB}/libz3.so
    ${THIRD_PARTY_LIB}/libjsoncpp.a
    ${THIRD_PARTY_LIB}/libsqlite3.a
    ${PARQUET_HOME}/lib/libParquetFP.a
    ${ABKCOMMON_HOME}/lib/libABKCommon.a
    ${KAHYPAR_HOME}/build/lib/libkahypar.so
    pthread
    dl
)

target_link_libraries(bookshelf_converter
    ${Boost_LIBRARIES}
    pthread
    ${THIRD_PARTY_LIB}/libz3.so
    ${THIRD_PARTY_LIB}/libjsoncpp.a
)
