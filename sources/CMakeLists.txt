cmake_minimum_required(VERSION 3.13)

project(cppmathlib VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)


# define build types and configurations
# --------------------------------------- #
if (CMAKE_CONFIGURATION_TYPES)
    if (NOT "Test" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Test)
    endif()
else()
    set(allowableBuildTypes Debug Test Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowableBuildTypes}")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
        message(FATAL_ERROR "Invalid build type ${CMAKE_BUILD_TYPE}") 
    endif()
endif()

set(CMAKE_TEST_POSTFIX _tests)

# use c++17
# --------------------------------------- #
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libraries
# --------------------------------------- #
add_library(logicantsy 
    src/core/utils.cpp 
    src/core/common.cpp 
    src/core/semantics/truth_tables.cpp
    )
target_include_directories(logicantsy PUBLIC include)

# executables
# --------------------------------------- #

# test definitions
# use googletest framework
# --------------------------------------- #
FetchContent_Declare(googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         2fe3bd994b3189899d93f1d5a881e725e046fdc2 # 1.8.1
)
FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

include(GoogleTest)

set(test_sources ${test_sources}
    tests/main.cpp
    tests/syntax_tests.cpp
    tests/truth_table_tests.cpp
    tests/core_utils_tests.cpp
    tests/nmatrices_tests.cpp
    tests/proof_theory_tests.cpp
)

add_executable(maintest ${test_sources})
target_link_libraries(maintest PRIVATE gtest logicantsy)
target_include_directories(maintest PRIVATE include)

gtest_discover_tests(maintest)

enable_testing()
