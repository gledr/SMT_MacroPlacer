cmake_minimum_required(VERSION 3.1)

project(InitialPlacer)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE DEBUG)

set(DEFLIB_HOME${PROJECT_SOURCE_DIR}/02_module/def)
set(LEFLIB_HOME${PROJECT_SOURCE_DIR}/02_module/lef)

set(THIRD_PARTY_INCLUDE ${PROJECT_SOURCE_DIR}/05_third_party/include)
set(THIRD_PARTY_LIB ${PROJECT_SOURCE_DIR}/05_third_party/lib)
set(THIRD_PARTY_LIB64 ${PROJECT_SOURCE_DIR}/05_third_party/lib64)
set(ABKCOMMON_HOME ${PROJECT_SOURCE_DIR}/02_module/ABKCommon)
set(PARQUET_HOME ${PROJECT_SOURCE_DIR}/02_module/ParquetFP)

add_subdirectory(01_src)
add_subdirectory(02_module)
add_subdirectory(04_configuration)
add_subdirectory(05_third_party)
