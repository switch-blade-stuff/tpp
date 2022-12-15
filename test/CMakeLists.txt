cmake_minimum_required(VERSION 3.19)

macro(make_test_target)
    project(tpp-tests-cxx${ARGV0})

    add_executable(${PROJECT_NAME}
            ${CMAKE_CURRENT_LIST_DIR}/main.cpp
            ${CMAKE_CURRENT_LIST_DIR}/assert.hpp
            ${CMAKE_CURRENT_LIST_DIR}/hash_tests.cpp
            ${CMAKE_CURRENT_LIST_DIR}/dense_table_tests.cpp
            ${CMAKE_CURRENT_LIST_DIR}/swiss_table_tests.cpp)

    target_link_libraries(${PROJECT_NAME} PRIVATE tpp)

    if (MSVC AND TPP_USE_MODULES AND ${ARGV0} EQUAL 20)
        target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
        target_compile_options(${PROJECT_NAME} PRIVATE /experimental:module)
    else ()
        target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_${ARGV0})
    endif ()

    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
    else ()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -fstack-protector-all)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            #target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined -fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer)
            #target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=undefined -fsanitize=memory -fsanitize-memory-track-origins)
        endif ()
    endif ()

    # Dense table tests
    add_test(NAME dense_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_map)
    add_test(NAME ordered_dense_map-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_dense_map)
    add_test(NAME dense_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_set)
    add_test(NAME ordered_dense_set-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> ordered_dense_set)
    add_test(NAME dense_multiset-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> dense_multiset)

    # Hash function tests
    add_test(NAME seahash-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> seahash)
    add_test(NAME crc32-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> crc32)
    add_test(NAME fnv1a-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> fnv1a)
    add_test(NAME sdbm-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> sdbm)
    add_test(NAME md5-cxx${ARGV0} COMMAND $<TARGET_FILE:${PROJECT_NAME}> md5)
endmacro()

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)
enable_testing()

make_test_target(17)
make_test_target(20)