cmake_minimum_required(VERSION 3.19)

macro(test_target_cxx)
    project(tpp-tests-cxx${ARGV0})

    add_executable(${PROJECT_NAME}
            ${CMAKE_CURRENT_LIST_DIR}/main.cpp
            ${CMAKE_CURRENT_LIST_DIR}/assert.hpp
            ${CMAKE_CURRENT_LIST_DIR}/dense_table_tests.cpp
            ${CMAKE_CURRENT_LIST_DIR}/swiss_table_tests.cpp)

    target_link_libraries(${PROJECT_NAME} PRIVATE tpp)

    # On MSVC, use c++latest instead of c++20 for experimental module support
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND TPP_USE_MODULES AND ${ARGV0} EQUAL 20)
        set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
        target_compile_options(${PROJECT_NAME} PRIVATE /experimental:module)
    else ()
        target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_${ARGV0})
    endif ()

    # Enable max error reporting
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif ()

    # Enable ASan in debug builds
    if (CMAKE_BUILD_TYPE STREQUAL "Debug"#[[ OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"]])
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
            target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address /Zi)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer -fstack-protector-strong)
            target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined -fsanitize=address)
        endif ()
    endif ()

    # Dense table tests
    add_test(NAME dense_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_set)
    add_test(NAME dense_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_map)
    add_test(NAME ordered_dense_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_dense_set)
    add_test(NAME ordered_dense_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_dense_map)
    add_test(NAME dense_multiset-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_multiset)
    add_test(NAME dense_multimap-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_multimap)

    # Sparse table tests
    add_test(NAME sparse_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> sparse_set)
    add_test(NAME sparse_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> sparse_map)
    add_test(NAME ordered_sparse_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_sparse_set)
    add_test(NAME ordered_sparse_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_sparse_map)

    # Stable table tests
    add_test(NAME stable_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> stable_set)
    add_test(NAME stable_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> stable_map)
    add_test(NAME ordered_stable_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_stable_set)
    add_test(NAME ordered_stable_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_stable_map)
endmacro()

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)
enable_testing()

test_target_cxx(17) # Test with C++17
test_target_cxx(20) # Test with C++20
