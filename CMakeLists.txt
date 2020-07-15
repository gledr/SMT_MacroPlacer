cmake_minimum_required(VERSION 3.12)

project(SMT_MacroPlacer)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

find_package(Doxygen)

if(DOXYGEN_FOUND)
    configure_file(${PROJECT_SOURCE_DIR}/04_configuration/Doxyfile.in ${PROJECT_SOURCE_DIR}/04_configuration/Doxyfile.in @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/04_configuration/Doxyfile.in
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)

option(PLACER_BUILD_Z3 
    "Download and Build Z3 or use System wide installed Z3 version" ON)
option(PLACER_BUILD_BOOST 
    "Download and Build Boost or use System wide installed Boost version" ON)
option(PLACER_BUILD_JSONCPP 
    "Download and Build Jsoncpp or use System wide installed Jsoncpp version" ON)  
option(PLACER_BUILD_SQLITE3
    "Download and Build Sqlite3 or use System wide installed Sqlite3 version" ON)  
    
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE Debug)

set(DEFLIB_HOME ${PROJECT_SOURCE_DIR}/02_module/def)
set(LEFLIB_HOME ${PROJECT_SOURCE_DIR}/02_module/lef)
set(MATPLOTLIBCPP_HOME ${PROJECT_SOURCE_DIR}/02_module/matplotlib-cpp)
set(ABKCOMMON_HOME ${PROJECT_SOURCE_DIR}/02_module/ABKCommon)
set(PARQUET_HOME ${PROJECT_SOURCE_DIR}/02_module/ParquetFP)

set(THIRD_PARTY_INCLUDE ${PROJECT_SOURCE_DIR}/05_third_party/include)
set(THIRD_PARTY_LIB ${PROJECT_SOURCE_DIR}/05_third_party/lib)
set(THIRD_PARTY_LIB64 ${PROJECT_SOURCE_DIR}/05_third_party/lib64)

add_subdirectory(01_src)
add_subdirectory(02_module)
add_subdirectory(04_configuration)
add_subdirectory(05_third_party)
