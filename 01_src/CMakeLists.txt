find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(Python2 COMPONENTS Development NumPy)
find_package(PkgConfig REQUIRED)

pkg_check_modules(JSONCPP jsoncpp)

add_definitions(-D_CPPUNWIND)

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
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(placer_utils
    utils/timer.cpp
    utils/logger.cpp
    utils/base_logger.cpp
    utils/utils.cpp
    utils/base_database.cpp
    utils/database.cpp
    utils/exception.cpp
)

target_link_libraries (placer_utils 
    #${THIRD_PARTY_LIB}/libsqlite3.a
    sqlite3
)

target_include_directories(placer_utils PRIVATE 
    ${JSONCPP_LIBRARIES}
)

#add_dependencies(placer_utils
    #build_z3
    #build_sqlite3
#)

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
    macrocircuit/plotter.cpp
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
    #build_z3
    #build_jsoncpp
    build_kahypar
    build_matplotlib
    placer_config
    ABKCommon
    ParquetFP
    placer_utils
)

target_link_libraries(smt_placer PUBLIC
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    def
    defzlib
    lef
    lefzlib
    ${THIRD_PARTY_LIB}/libz3.so
    #${THIRD_PARTY_LIB}/libjsoncpp.a
    ${JSONCPP_LIBRARIES}
    ${THIRD_PARTY_LIB}/libkahypar.so
    ${PARQUET_HOME}/lib/libParquetFP.a
    ${ABKCOMMON_HOME}/lib/libABKCommon.a
    pthread
    dl
    placer_utils
)

target_include_directories(smt_placer PRIVATE 
    ${Python2_INCLUDE_DIRS} 
    ${Python2_NumPy_INCLUDE_DIRS}
    ${JSONCPP_LIBRARIES}
)

target_link_libraries(bookshelf_converter
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    pthread
    ${THIRD_PARTY_LIB}/libz3.so
    #${THIRD_PARTY_LIB}/libjsoncpp.a
    jsoncpp
    dl
    placer_utils
)
