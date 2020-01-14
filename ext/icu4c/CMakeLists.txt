include(ExternalProject)

set(ICU4C_DOWNLOAD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/download"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/src/icu4c")

set(ICU4C_CONFIGURE_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/configure"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/src/icu4c"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/build")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--build-type=debug")
else()
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--build-type=release")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND
      ICU4C_CONFIGURE_COMMAND
      "--platform"
      "Linux")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND
      ICU4C_CONFIGURE_COMMAND
      "--platform"
      "Linux/gcc")
  else()
    message(FATAL_ERROR
      "An unsupported C++ compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}.")
  endif()
else()
  message(FATAL_ERROR
    "An unsupported system: CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}.")
endif()
if(CMAKE_C_COMPILER)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--cc"
    "${CMAKE_C_COMPILER}")
endif()
if(CMAKE_CXX_COMPILER)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--cxx"
    "${CMAKE_CXX_COMPILER}")
endif()
if(DEFINED CMAKE_CXX_STANDARD)
  if(CMAKE_CXX_STANDARD STREQUAL "20")
    # `-std=c++20` does not compile as of ICU4C 65.1
    list(APPEND
      ICU4C_CONFIGURE_CXXFLAGS
      "-std=c++17")
  else()
    list(APPEND
      ICU4C_CONFIGURE_CXXFLAGS
      "-std=c++${CMAKE_CXX_STANDARD}")
  endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
    list(APPEND
      ICU4C_CONFIGURE_CXXFLAGS
      "-stdlib=libstdc++")
    list(APPEND
      ICU4C_CONFIGURE_LDFLAGS
      "-stdlib=libstdc++")
  elseif(CORIO_CXX_STDLIB STREQUAL "libc++")
    list(APPEND
      ICU4C_CONFIGURE_CXXFLAGS
      "-stdlib=libc++")
    list(APPEND
      ICU4C_CONFIGURE_LDFLAGS
      "-stdlib=libc++")
  else()
    message(FATAL_ERROR
      "An unsupported C++ standard library: CORIO_CXX_STDLIB=${CORIO_CXX_STDLIB}.")
  endif()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
      list(APPEND
        ICU4C_CONFIGURE_CPPFLAGS
        "-D_GLIBCXX_DEBUG"
        "-D_GLIBCXX_DEBUG_PEDANTIC")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND
      ICU4C_CONFIGURE_CPPFLAGS
      "-D_GLIBCXX_DEBUG"
      "-D_GLIBCXX_DEBUG_PEDANTIC")
  endif()
  list(APPEND
    ICU4C_CONFIGURE_CFLAGS
    "-fsanitize=address"
    "-fsanitize=pointer-compare"
    "-fsanitize=pointer-subtract"
    "-fsanitize=undefined")
  list(APPEND
    ICU4C_CONFIGURE_CXXFLAGS
    "-fsanitize=address"
    "-fsanitize=pointer-compare"
    "-fsanitize=pointer-subtract"
    "-fsanitize=undefined")
  list(APPEND
    ICU4C_CONFIGURE_LDFLAGS
    "-fsanitize=address"
    "-fsanitize=pointer-compare"
    "-fsanitize=pointer-subtract"
    "-fsanitize=undefined")
else()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang does not compile with `-flto` as of 65.1.
  else()
    list(APPEND
      ICU4C_CONFIGURE_CFLAGS
      "-flto")
    list(APPEND
      ICU4C_CONFIGURE_CXXFLAGS
      "-flto")
    list(APPEND
      ICU4C_CONFIGURE_LDFLAGS
      "-flto")
  endif()
endif()
list(JOIN ICU4C_CONFIGURE_CPPFLAGS " " ICU4C_CONFIGURE_CPPFLAGS)
if(ICU4C_CONFIGURE_CPPFLAGS)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--cppflags"
    "${ICU4C_CONFIGURE_CPPFLAGS}")
endif()
list(JOIN ICU4C_CONFIGURE_CFLAGS " " ICU4C_CONFIGURE_CFLAGS)
if(ICU4C_CONFIGURE_CFLAGS)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--cflags"
    "${ICU4C_CONFIGURE_CFLAGS}")
endif()
list(JOIN ICU4C_CONFIGURE_CXXFLAGS " " ICU4C_CONFIGURE_CXXFLAGS)
if(ICU4C_CONFIGURE_CXXFLAGS)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--cxxflags"
    "${ICU4C_CONFIGURE_CXXFLAGS}")
endif()
list(JOIN ICU4C_CONFIGURE_LDFLAGS " " ICU4C_CONFIGURE_LDFLAGS)
if(ICU4C_CONFIGURE_LDFLAGS)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--ldflags"
    "${ICU4C_CONFIGURE_LDFLAGS}")
endif()
list(APPEND
  ICU4C_CONFIGURE_COMMAND
  "--"
  "--prefix=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix")
if(NOT BUILD_SHARED_LIBS)
  list(APPEND
    ICU4C_CONFIGURE_COMMAND
    "--disable-shared"
    "--enable-static")
endif()

set(ICU4C_BUILD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/build")

set(ICU4C_INSTALL_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/install"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/build")

# `make check` fails with Clang.
set(ICU4C_TEST_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/test"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/build")

ExternalProject_Add(icu4c_build
  PREFIX "icu4c-prefix"
  DOWNLOAD_COMMAND ${ICU4C_DOWNLOAD_COMMAND}
  CONFIGURE_COMMAND ${ICU4C_CONFIGURE_COMMAND}
  BUILD_COMMAND ${ICU4C_BUILD_COMMAND}
  INSTALL_COMMAND ${ICU4C_INSTALL_COMMAND}
  TEST_COMMAND "")

add_library(icu4c_headers
  INTERFACE)
target_include_directories(icu4c_headers
  INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/include")
add_dependencies(icu4c_headers
  icu4c_build)

foreach(spec "data" "uc;data" "i18n;data;uc" "io;data;uc;i18n"
    "tu;data;uc;i18n" "test;data;uc;i18n;tu")
  list(GET spec 0 lib)
  add_library("icu4c_${lib}"
    INTERFACE)
  target_link_libraries("icu4c_${lib}"
    INTERFACE
    icu4c_headers)
  list(LENGTH spec len)
  if(len GREATER_EQUAL 2)
    list(REMOVE_AT spec 0)
    foreach(dep IN LISTS spec)
      target_link_libraries("icu4c_${lib}"
        INTERFACE
        "icu4c_${dep}")
    endforeach()
  endif()
  target_link_libraries("icu4c_${lib}"
    INTERFACE
    "-L${CMAKE_CURRENT_BINARY_DIR}/icu4c-prefix/lib"
    "-licu4c_${lib}")
  add_dependencies("icu4c_${lib}"
    icu4c_build)
endforeach()
