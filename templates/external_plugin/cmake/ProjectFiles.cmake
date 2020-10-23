include_guard( DIRECTORY )

set(include_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}")
set(src_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

list(APPEND ${PROJECT_NAME}_SOURCES
  ${src_DIR}/plugin_main.cc
  ${include_DIR}/EventHandler.hpp
  ${src_DIR}/EventHandler.cc
  ${include_DIR}/Tooling.hpp
  ${src_DIR}/Tooling.cc
)
