get_filename_component(CURRENT_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
list(APPEND CMAKE_MODULE_PATH ${CURRENT_CMAKE_DIR})

#include(CMakeFindDependencyMacro) # use find_package instead

# NOTE: some packages may be optional (platform-specific, etc.)
# find_package(... REQUIRED)
find_package(chromium_base REQUIRED)
# see https://doc.magnum.graphics/corrade/corrade-cmake.html#corrade-cmake-subproject
find_package(Corrade REQUIRED PluginManager)
find_package(Cling)
find_package(flexlib)
find_package(flextool)

list(REMOVE_AT CMAKE_MODULE_PATH -1)

if(NOT TARGET CONAN_PKG::external_plugin)
  message(FATAL_ERROR "Use external_plugin from conan")
endif()
set(external_plugin_LIB
  CONAN_PKG::external_plugin
)
set(external_plugin_FILE
  ${CONAN_external_plugin_ROOT}/lib/external_plugin${CMAKE_SHARED_LIBRARY_SUFFIX}
)
# conan package has '/include' dir
set(external_plugin_HEADER_DIR
  ${CONAN_external_plugin_ROOT}/include
)
if(TARGET external_plugin)
  # name of created target
  set(external_plugin_LIB
    external_plugin
  )
  # no '/include' dir on local build
  set(external_plugin_HEADER_DIR
    ${CONAN_external_plugin_ROOT}
  )
  # plugin file
  get_property(external_plugin_LIBRARY_OUTPUT_DIRECTORY
    TARGET ${external_plugin_LIB}
    PROPERTY LIBRARY_OUTPUT_DIRECTORY)
  message(STATUS "external_plugin_LIBRARY_OUTPUT_DIRECTORY == ${external_plugin_LIBRARY_OUTPUT_DIRECTORY}")
  set(external_plugin_FILE
    ${external_plugin_LIBRARY_OUTPUT_DIRECTORY}/external_plugin${CMAKE_SHARED_LIBRARY_SUFFIX}
  )
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/cmake/external_plugin-config.cmake")
  # uses Config.cmake or a -config.cmake file
  # see https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
  # BELOW MUST BE EQUAL TO find_package(... CONFIG REQUIRED)
  # NOTE: find_package(CONFIG) not supported with EMSCRIPTEN, so use include()
  include(${CMAKE_CURRENT_LIST_DIR}/cmake/external_plugin-config.cmake)
endif()

message(STATUS "external_plugin_HEADER_DIR=${external_plugin_HEADER_DIR}")
