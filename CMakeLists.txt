cmake_minimum_required(VERSION 3.19)
project(tpp)

# Library options
option(TPP_USE_MODULES "Toggles support for C++20 modules" OFF)
if (${TPP_USE_MODULES})
    add_compile_definitions(TPP_USE_MODULES)
endif ()

option(TPP_NO_SIMD "Toggles availability of SIMD optimizations for swiss tables" OFF)
if (${TPP_NO_SIMD})
    add_compile_definitions(TPP_NO_SIMD)
endif ()

option(TPP_NO_HASH "Toggles availability of hash function utilities" OFF)
if (${TPP_NO_HASH})
    add_compile_definitions(TPP_NO_HASH)
endif ()

set(TPP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/tpp)
add_library(${PROJECT_NAME} INTERFACE
        # Common definitions & utilities
        ${TPP_SOURCE_DIR}/detail/arch.hpp
        ${TPP_SOURCE_DIR}/detail/define.hpp
        ${TPP_SOURCE_DIR}/detail/utility.hpp
        ${TPP_SOURCE_DIR}/detail/multikey.hpp
        ${TPP_SOURCE_DIR}/detail/table_common.hpp

        # Hash utilities
        ${TPP_SOURCE_DIR}/hash.hpp

        # Dense table containers
        ${TPP_SOURCE_DIR}/detail/dense_table.hpp
        ${TPP_SOURCE_DIR}/dense_set.hpp
        ${TPP_SOURCE_DIR}/dense_map.hpp
        ${TPP_SOURCE_DIR}/dense_multiset.hpp
        ${TPP_SOURCE_DIR}/dense_multimap.hpp

        # Swiss table containers
        ${TPP_SOURCE_DIR}/detail/stable_traits.hpp
        ${TPP_SOURCE_DIR}/detail/swiss_table.hpp
        ${TPP_SOURCE_DIR}/sparse_set.hpp
        ${TPP_SOURCE_DIR}/sparse_map.hpp
        ${TPP_SOURCE_DIR}/stable_set.hpp
        ${TPP_SOURCE_DIR}/stable_map.hpp)

# Unit tests
option(TPP_TESTS "Enable unit tests" OFF)
if (${TPP_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()