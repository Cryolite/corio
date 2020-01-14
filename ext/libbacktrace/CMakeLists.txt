include(ExternalProject)

set(LIBBACKTRACE_DOWNLOAD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/download"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/src/libbacktrace")

set(LIBBACKTRACE_CONFIGURE_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/configure"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/src/libbacktrace"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/build"
  "--"
  "--prefix=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix")
if(BUILD_SHARED_LIBS)
  list(APPEND
    LIBBACKTRACE_CONFIGURE_COMMAND
    "--enable-shared"
    "--disable-static")
endif()
list(APPEND
  LIBBACKTRACE_CONFIGURE_COMMAND
  "CC=${CMAKE_C_COMPILER}")

set(LIBBACKTRACE_BUILD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/build")

set(LIBBACKTRACE_INSTALL_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/install"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/build")

# `make check` fails on clang.
set(LIBBACKTRACE_TEST_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/test"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/build")

ExternalProject_Add(libbacktrace_build
  PREFIX "libbacktrace-prefix"
  DOWNLOAD_COMMAND ${LIBBACKTRACE_DOWNLOAD_COMMAND}
  CONFIGURE_COMMAND ${LIBBACKTRACE_CONFIGURE_COMMAND}
  BUILD_COMMAND ${LIBBACKTRACE_BUILD_COMMAND}
  INSTALL_COMMAND ${LIBBACKTRACE_INSTALL_COMMAND}
  TEST_COMMAND "")

add_library(libbacktrace
  INTERFACE)
target_include_directories(libbacktrace
  INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/include")
target_link_libraries(libbacktrace
  INTERFACE
  "-L${CMAKE_CURRENT_BINARY_DIR}/libbacktrace-prefix/lib"
  -lbacktrace)
add_dependencies(libbacktrace
  libbacktrace_build)
