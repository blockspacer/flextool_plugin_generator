cmake_minimum_required(VERSION 3.8)

# project() must be before checks, see https://stackoverflow.com/a/26437667/10904212
set(PROJECT_NAME "external_plugin")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/parse_version_file.cmake)

project(${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  LANGUAGES CXX C)

set(LIB_NAME ${PROJECT_NAME})

# --------------------------- configuration ------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_cmake_utils.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/fetch_conan_auto_install.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan_setup.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/")

find_package(flexlib REQUIRED)
if(${flexlib_HEADER_DIR} STREQUAL "")
  message(FATAL_ERROR "unable to find flexlib_HEADER_DIR")
endif()

find_package(flex_reflect_plugin REQUIRED)
if(${flex_reflect_plugin_FILE} STREQUAL "")
  message(FATAL_ERROR "unable to find flex_reflect_plugin_FILE")
endif()

find_package(basis_plugin_helper REQUIRED)

set_common_plugin_options(${PROJECT_NAME}) # from basis_plugin_helper (conan package)

set_common_plugin_modules()

# Keep symbols for JIT resolution
set(LLVM_NO_DEAD_STRIP 1)

restrict_supported_platforms(LINUX)

## ---------------------------- sources -------------------------------- ##

generate_version_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
  ${CMAKE_BINARY_DIR}/version.hpp)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProjectFiles.cmake)

## ---------------------------- targets -------------------------------- ##

add_library(${LIB_NAME}-test-includes INTERFACE)

if(${PROJECT_NAME}_BUILD_SHARED_LIBS)
  set(CORE_LIB_TYPE SHARED)
else()
  set(CORE_LIB_TYPE STATIC)
  message(FATAL_ERROR "ONLY SHARED PLUGINS SUPPORTED FOR NOW")
endif(${PROJECT_NAME}_BUILD_SHARED_LIBS)

set(CONFIG_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/conf/${LIB_NAME}.conf")

if(NOT ${PROJECT_NAME}_SOURCES)
  message(FATAL_ERROR "Either ${PROJECT_NAME}_SOURCES is empty or not set")
endif()
add_plugin_library("${CONFIG_FILE_PATH}" "${LIB_NAME}" "${CORE_LIB_TYPE}"
  "${${PROJECT_NAME}_SOURCES}"
)

# startup target for MS Visual Studio IDE
set_vs_startup_project(${LIB_NAME}) # from Utils.cmake

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${LIB_NAME}
  PUBLIC "$<$<BOOL:${MSVC}>:/permissive->")

add_relative_include_dir(${LIB_NAME} ${LIB_NAME}-test-includes PUBLIC PUBLIC "include")

if(NOT COMMON_PLUGIN_LIBS)
  message(FATAL_ERROR "Either COMMON_PLUGIN_LIBS is empty or not set")
endif()
list(APPEND USED_3DPARTY_LIBS
  ${COMMON_PLUGIN_LIBS}
  ${flexlib_LIB}
  CONAN_PKG::openssl
)

set(USED_SYSTEM_LIBS
    Threads::Threads # pthread, https://cmake.org/cmake/help/v3.13/module/FindThreads.html
    ${CMAKE_DL_LIBS} # https://cmake.org/cmake/help/v3.13/variable/CMAKE_DL_LIBS.html
    stdc++fs # C++17 std::filesystem
    CACHE INTERNAL "USED_SYSTEM_LIBS")

target_link_libraries(${LIB_NAME} PUBLIC
  ${USED_3DPARTY_LIBS}
  ${USED_SYSTEM_LIBS}
)

target_compile_options(${LIB_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall>)

target_compile_options(${LIB_NAME} PRIVATE
  -fno-rtti)

target_compile_definitions(${LIB_NAME} PRIVATE
  ${${PROJECT_NAME}_PRIVATE_DEFINES}
  # https://stackoverflow.com/a/30877725
  BOOST_SYSTEM_NO_DEPRECATED BOOST_ERROR_CODE_HEADER_ONLY
  # see https://github.com/mosra/corrade/blob/af9d4216f07307a2dff471664eed1e50e180568b/modules/UseCorrade.cmake#L568
  CORRADE_DYNAMIC_PLUGIN=1
)

target_compile_definitions(${LIB_NAME} PUBLIC
  ${${PROJECT_NAME}_PUBLIC_DEFINES}
  ${CLING_DEFINITIONS}
  DISABLE_DOCTEST=1
)

## ---------------------------- exports -------------------------------- ##

# install and export steps
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Exports.cmake)

## ---------------------------- conf -------------------------------- ##

copy_to_bin_dirs("${CONFIG_FILE_PATH}"
  "${LIB_NAME}.conf")

## ---------------------------- tests -------------------------------- ##

if(ENABLE_TESTS)
  set_plugin_tools(${LIB_NAME})
  set(TEST_TEMPLATE_FILE_PATH
    # usage: --extra-arg=-DTEST_PATH="${TEST_PATH}"
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/code_generation/example.cxtpl
  )
  add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
else()
  target_compile_definitions(${LIB_NAME} PRIVATE
    # dummy to supress warning in editor
    TEST_TEMPLATE_FILE_PATH=""
  )
endif()

## ---------------------------- docs -------------------------------- ##

if(BUILD_DOXY_DOC)
  add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/docs )
endif()
