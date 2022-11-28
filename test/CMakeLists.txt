cmake_minimum_required(VERSION 3.19)
project(tpp-tests)

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_executable(tpp-tests
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/assert.hpp
        ${CMAKE_CURRENT_LIST_DIR}/hash_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/dense_table_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/swiss_table_tests.cpp)
target_link_libraries(tpp-tests PRIVATE tpp)
target_compile_features(tpp-tests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(tpp-tests PRIVATE /W4 /WX)
else()
    target_compile_options(tpp-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()

macro(make_test)
    add_test(NAME ${ARGV0} COMMAND $<TARGET_FILE:tpp-tests> ${ARGV0})
endmacro()

# Dense table tests
make_test(dense_map)
make_test(ordered_dense_map)
make_test(dense_set)
make_test(ordered_dense_set)
make_test(dense_multiset)

# Hash function tests
make_test(seahash)
make_test(crc32)
make_test(fnv1a)
make_test(sdbm)
make_test(md5)