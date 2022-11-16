cmake_minimum_required(VERSION 3.19)
project(tpp)

# Library options
option(TPP_USE_MODULES "Toggles support for C++20 modules" ON)

if (${TPP_USE_MODULES})
    add_compile_definitions(TPP_USE_MODULES)
endif ()

option(TPP_NO_HASH "Toggles availability of hash function utilities" OFF)
option(TPP_OPTIONAL_HASH "Enable hash specializations for the <optional> header" OFF)
option(TPP_VARIANT_HASH "Enable hash specializations for the <variant> header" OFF)
option(TPP_STRING_HASH "Enable hash specializations for the <string> header" OFF)
option(TPP_STRING_VIEW_HASH "Enable hash specializations for the <string> header" OFF)
option(TPP_BITSET_HASH "Enable hash specializations for the <bitset> header" OFF)
option(TPP_MEMORY_HASH "Enable hash specializations for the <memory> header" OFF)
option(TPP_TYPEINDEX_HASH "Enable hash specializations for the <typeindex> header" OFF)
option(TPP_FILESYSTEM_HASH "Enable hash specializations for the <filesystem> header" OFF)
option(TPP_THREAD_HASH "Enable hash specializations for the <thread> header" OFF)
option(TPP_SYSTEM_ERROR_HASH "Enable hash specializations for the <system_error> header" OFF)
option(TPP_COROUTINE_HASH "Enable hash specializations for the <coroutine> header (requires C++20)" OFF)
option(TPP_STACKTRACE_HASH "Enable hash specializations for the <stacktrace> header (requires C++23)" OFF)
option(TPP_STL_HASH_ALL "Enables all STL header specific options described above" OFF)

if (${TPP_NO_HASH})
    add_compile_definitions(TPP_NO_HASH)
else ()
    if (${TPP_STL_HASH_ALL})
        set(TPP_OPTIONAL_HASH ON CACHE BOOL "Enable hash specializations for the <optional> header")
        set(TPP_VARIANT_HASH ON CACHE BOOL "Enable hash specializations for the <variant> header")
        set(TPP_STRING_HASH ON CACHE BOOL "Enable hash specializations for the <string> header")
        set(TPP_STRING_VIEW_HASH ON CACHE BOOL "Enable hash specializations for the <string_view> header")
        set(TPP_BITSET_HASH ON CACHE BOOL "Enable hash specializations for the <bitset> header")
        set(TPP_MEMORY_HASH ON CACHE BOOL "Enable hash specializations for the <memory> header")
        set(TPP_TYPEINDEX_HASH ON CACHE BOOL "Enable hash specializations for the <typeindex> header")
        set(TPP_FILESYSTEM_HASH ON CACHE BOOL "Enable hash specializations for the <filesystem> header")
        set(TPP_THREAD_HASH ON CACHE BOOL "Enable hash specializations for the <thread> header")
        set(TPP_SYSTEM_ERROR_HASH ON CACHE BOOL "Enable hash specializations for the <system_error> header")
        set(TPP_COROUTINE_HASH ON CACHE BOOL "Enable hash specializations for the <coroutine> header (requires C++20)")
        set(TPP_STACKTRACE_HASH ON CACHE BOOL "Enable hash specializations for the <stacktrace> header (requires C++23)")
        add_compile_definitions(TPP_STL_HASH_ALL)
    endif ()

    if (${TPP_OPTIONAL_HASH})
        add_compile_definitions(TPP_OPTIONAL_HASH)
    endif ()
    if (${TPP_VARIANT_HASH})
        add_compile_definitions(TPP_VARIANT_HASH)
    endif ()
    if (${TPP_STRING_HASH})
        add_compile_definitions(TPP_STRING_HASH)
    endif ()
    if (${TPP_STRING_VIEW_HASH})
        add_compile_definitions(TPP_STRING_VIEW_HASH)
    endif ()
    if (${TPP_BITSET_HASH})
        add_compile_definitions(TPP_BITSET_HASH)
    endif ()
    if (${TPP_MEMORY_HASH})
        add_compile_definitions(TPP_MEMORY_HASH)
    endif ()
    if (${TPP_TYPEINDEX_HASH})
        add_compile_definitions(TPP_TYPEINDEX_HASH)
    endif ()
    if (${TPP_FILESYSTEM_HASH})
        add_compile_definitions(TPP_FILESYSTEM_HASH)
    endif ()
    if (${TPP_THREAD_HASH})
        add_compile_definitions(TPP_THREAD_HASH)
    endif ()
    if (${TPP_SYSTEM_ERROR_HASH})
        add_compile_definitions(TPP_SYSTEM_ERROR_HASH)
    endif ()
    if (${TPP_SYSTEM_ERROR_HASH})
        add_compile_definitions(TPP_SYSTEM_ERROR_HASH)
    endif ()
    if (${TPP_COROUTINE_HASH})
        add_compile_definitions(TPP_COROUTINE_HASH)
    endif ()
    if (${TPP_STACKTRACE_HASH})
        add_compile_definitions(TPP_STACKTRACE_HASH)
    endif ()
endif ()

set(TPP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/tpp)
add_library(tpp INTERFACE
        # Common utilities
        ${TPP_SOURCE_DIR}/detail/common.hpp
        ${TPP_SOURCE_DIR}/detail/table_util.hpp

        # Hash utilities
        ${TPP_SOURCE_DIR}/hash.hpp
        ${TPP_SOURCE_DIR}/stl_hash.hpp

        # DenseHash table containers
        ${TPP_SOURCE_DIR}/detail/dense_table.hpp
        ${TPP_SOURCE_DIR}/dense_set.hpp
        ${TPP_SOURCE_DIR}/dense_map.hpp
        ${TPP_SOURCE_DIR}/dense_bimap.hpp

        # SwissHash table containers
        #${TPP_SOURCE_DIR}/detail/swiss_table.hpp
        #${TPP_SOURCE_DIR}/swiss_set.hpp
        #${TPP_SOURCE_DIR}/swiss_map.hpp
        #${TPP_SOURCE_DIR}/swiss_bimap.hpp
        )

# Unit tests
option(TPP_TESTS "Enable unit tests" OFF)

if (${TPP_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()