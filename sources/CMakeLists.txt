cmake_minimum_required(VERSION 3.13)

project(cppmathlib VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

find_package(BISON)
find_package(FLEX)

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

BISON_TARGET(bison_fmla_parser include/core/parser/fmla/bison_fmla_parser.y ${CMAKE_CURRENT_BINARY_DIR}/bison_fmla_parser.cpp)
FLEX_TARGET(flex_fmla_lexer include/core/parser/fmla/flex_fmla_lexer.l ${CMAKE_CURRENT_BINARY_DIR}/flex_fmla_lexer.cpp)
ADD_FLEX_BISON_DEPENDENCY(flex_fmla_lexer bison_fmla_parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# yamlcpp
# -----------------------------------------#
FetchContent_Declare(yamlcpp
    GIT_REPOSITORY  https://github.com/jbeder/yaml-cpp
    GIT_TAG         9a3624205e8774953ef18f57067b3426c1c5ada6 # 0.6.3
)
FetchContent_GetProperties(yamlcpp)
if (NOT yamlcpp_POPULATED)
    FetchContent_Populate(yamlcpp)
    add_subdirectory(${yamlcpp_SOURCE_DIR} ${yamlcpp_BINARY_DIR})
endif()

# spdlog
# ----------------------------------------#
FetchContent_Declare(spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog
    GIT_TAG         4a9ccf7e38e257feecce0c579a782741254eaeef # 1.8.0
)
FetchContent_GetProperties(spdlog)
if (NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

# libraries
# --------------------------------------- #

add_library(logicantsy 
    src/core/utils.cpp 
    src/core/common.cpp 
    src/core/syntax.cpp 
    src/core/semantics/attitudes.cpp
    src/core/semantics/judgment_values_corr.cpp
    src/core/semantics/truth_tables.cpp
    src/core/semantics/genmatrix.cpp
    ${BISON_bison_fmla_parser_OUTPUTS} 
    ${FLEX_flex_fmla_lexer_OUTPUTS}
    )
target_include_directories(logicantsy PUBLIC include ${spdlog_SOURCE_DIR}/include)

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
    tests/genmatrices_tests.cpp
    tests/proof_theory_tests.cpp
    tests/truth_table_determination.cpp
    tests/combinatorics_tests.cpp
    tests/fmla_parser_tests.cpp
    tests/pnm_axiomatization.cpp
    tests/clone_generation_tests.cpp
)


add_executable(maintest ${test_sources} ${BISON_bison_fmla_parser_OUTPUTS} ${FLEX_flex_fmla_lexer_OUTPUTS})
target_link_libraries(maintest PRIVATE gtest logicantsy)
target_include_directories(maintest PRIVATE include)
gtest_discover_tests(maintest)

add_executable(ltsy src/cli/cli.cpp ${BISON_bison_fmla_parser_OUTPUTS} ${FLEX_flex_fmla_lexer_OUTPUTS})
target_link_libraries(ltsy PRIVATE logicantsy PUBLIC yaml-cpp)
target_include_directories(ltsy PRIVATE include PUBLIC ${yamlcpp_SOURCE_DIR}/include ${spdlog_SOURCE_DIR}/include)


enable_testing()
