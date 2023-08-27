cmake_minimum_required(VERSION 3.23)
project(tpp VERSION 1.0.0 LANGUAGES CXX)

# Include CMake modules
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Set up INTERFACE library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Library options
option(TPP_NO_SIMD "Toggles availability of SIMD optimizations for swiss tables" OFF)
if (${TPP_NO_SIMD})
    target_compile_definitions(${PROJECT_NAME} INTERFACE TPP_NO_SIMD)
endif ()

# Add sources
target_sources(${PROJECT_NAME} INTERFACE FILE_SET HEADERS BASE_DIRS tpp FILES
        # Common definitions & utilities
        tpp/detail/arch.hpp
        tpp/detail/define.hpp
        tpp/detail/utility.hpp
        tpp/detail/multikey.hpp
        tpp/detail/table_common.hpp

        # Dense table containers
        tpp/detail/dense_table.hpp
        tpp/dense_multiset.hpp
        tpp/dense_multimap.hpp
        tpp/dense_set.hpp
        tpp/dense_map.hpp

        # Swiss table containers
        tpp/detail/stable_traits.hpp
        tpp/detail/swiss_table.hpp
        tpp/sparse_set.hpp
        tpp/sparse_map.hpp
        tpp/stable_set.hpp
        tpp/stable_map.hpp)

# Configure CMake package
set(TPP_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(TPP_CMAKE_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
set(TPP_CMAKE_TARGETS_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake")
set(TPP_CMAKE_VERSION_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in ${TPP_CMAKE_CONFIG_PATH} INSTALL_DESTINATION ${TPP_INSTALL_CMAKE_DIR})
write_basic_package_version_file(${TPP_CMAKE_VERSION_PATH} VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)

# Configure CMake install
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ${PROJECT_NAME}-targets FILE ${PROJECT_NAME}-targets.cmake NAMESPACE tpp:: DESTINATION ${TPP_INSTALL_CMAKE_DIR})
install(FILES ${TPP_CMAKE_CONFIG_PATH} ${TPP_CMAKE_VERSION_PATH} DESTINATION ${TPP_INSTALL_CMAKE_DIR})

# Configure CMake export
export(EXPORT ${PROJECT_NAME}-targets FILE ${TPP_CMAKE_TARGETS_PATH} NAMESPACE tpp::)

# Unit tests
option(TPP_TESTS "Enable unit tests" OFF)
if (${TPP_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt)
endif ()
