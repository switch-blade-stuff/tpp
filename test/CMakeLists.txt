cmake_minimum_required(VERSION 3.19)
project(tpp-tests)

include_directories(${CMAKE_CURRENT_LIST_DIR}/..)

add_executable(tpp-tests
        ${CMAKE_CURRENT_LIST_DIR}/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/assert.hpp
        ${CMAKE_CURRENT_LIST_DIR}/dense_table_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/hash_tests.cpp)
target_link_libraries(tpp-tests PRIVATE tpp)

enable_testing()

macro(make_test)
    add_test(NAME ${ARGV0} COMMAND $<TARGET_FILE:tpp-tests> ${ARGV0})
endmacro()

# Dense table tests
make_test(dense_table)
make_test(ordered_dense_table)

# Hash function tests
make_test(seahash)
make_test(crc32)
make_test(fnv1a)
make_test(sdbm)
make_test(md5)