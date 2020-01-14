include(ExternalProject)

set(GOOGLETEST_CMAKE_OPTIONS
  "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
  "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
if(DEFINED CMAKE_CXX_STANDARD)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")
endif()
if(DEFINED CMAKE_BUILD_TYPE)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()
if(DEFINED BUILD_SHARED_LIBS)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
    list(APPEND
      GOOGLETEST_CXX_FLAGS
      "-stdlib=libstdc++")
    list(APPEND
      GOOGLETEST_EXE_LINKER_FLAGS
      "-stdlib=libstdc++")
    list(APPEND
      GOOGLETEST_SHARED_LINKER_FLAGS
      "-stdlib=libstdc++")
  elseif(CORIO_CXX_STDLIB STREQUAL "libc++")
    list(APPEND
      GOOGLETEST_CXX_FLAGS
      "-stdlib=libc++")
    list(APPEND
      GOOGLETEST_EXE_LINKER_FLAGS
      "-stdlib=libc++")
    list(APPEND
      GOOGLETEST_SHARED_LINKER_FLAGS
      "-stdlib=libc++")
  else()
    message(FATAL_ERROR
      "An unsupported C++ standard library: CORIO_CXX_STDLIB=${CORIO_CXX_STDLIB}.")
  endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
      list(APPEND
        GOOGLETEST_CXX_FLAGS
        "-D_GLIBCXX_DEBUG"
        "-D_GLIBCXX_DEBUG_PEDANTIC")
    endif()
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND
      GOOGLETEST_CXX_FLAGS
      "-D_GLIBCXX_DEBUG"
      "-D_GLIBCXX_DEBUG_PEDANTIC")
  endif()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  foreach(var
      "GOOGLETEST_C_FLAGS" "GOOGLETEST_CXX_FLAGS" "GOOGLETEST_EXE_LINKER_FLAGS" "GOOGLETEST_SHARED_LINKER_FLAGS")
    foreach(opt
        "-fsanitize=address" "-fsanitize=pointer-compare" "-fsanitize=pointer-subtract" "-fsanitize=undefined")
      list(APPEND
        "${var}"
        "${opt}")
    endforeach()
  endforeach()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    foreach(var
        "GOOGLETEST_C_FLAGS" "GOOGLETEST_CXX_FLAGS" "GOOGLETEST_EXE_LINKER_FLAGS" "GOOGLETEST_SHARED_LINKER_FLAGS")
      list(APPEND
        "${var}"
        "-pthread")
    endforeach()
  else()
    message(FATAL_ERROR
      "An unsupported C++ compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}.")
  endif()
else()
  message(FATAL_ERROR
    "An unsupported system: CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}.")
endif()
if(DEFINED GOOGLETEST_C_FLAGS)
  list(JOIN GOOGLETEST_C_FLAGS " " GOOGLETEST_C_FLAGS)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_C_FLAGS=${GOOGLETEST_C_FLAGS}")
endif()
if(DEFINED GOOGLETEST_CXX_FLAGS)
  list(JOIN GOOGLETEST_CXX_FLAGS " " GOOGLETEST_CXX_FLAGS)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_CXX_FLAGS=${GOOGLETEST_CXX_FLAGS}")
endif()
if(DEFINED GOOGLETEST_EXE_LINKER_FLAGS)
  list(JOIN GOOGLETEST_EXE_LINKER_FLAGS " " GOOGLETEST_EXE_LINKER_FLAGS)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_EXE_LINKER_FLAGS=${GOOGLETEST_EXE_LINKER_FLAGS}")
endif()
if(DEFINED GOOGLETEST_SHARED_LINKER_FLAGS)
  list(JOIN GOOGLETEST_SHARED_LINKER_FLAGS " " GOOGLETEST_SHARED_LINKER_FLAGS)
  list(APPEND
    GOOGLETEST_CMAKE_OPTIONS
    "-DCMAKE_SHARED_LINKER_FLAGS=${GOOGLETEST_SHARED_LINKER_FLAGS}")
endif()

set(CMAKE_CONFIGURE_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/configure"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/src/googletest"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/build"
  "--")
list(APPEND
  CMAKE_CONFIGURE_COMMAND
  "${GOOGLETEST_CMAKE_OPTIONS}")

set(CMAKE_BUILD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "--build-dir=${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/build")

ExternalProject_Add(googletest_build
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           master
  SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/src/googletest"
  PREFIX            "googletest-prefix"
  CONFIGURE_COMMAND "${CMAKE_CONFIGURE_COMMAND}"
  BUILD_COMMAND     "${CMAKE_BUILD_COMMAND}"
  INSTALL_COMMAND   ""
  TEST_COMMAND      "")

add_library(gtest
  INTERFACE)
target_include_directories(gtest
  INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/src/googletest/googletest/include"
  "${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/src/googletest/googlemock/include")
target_link_libraries(gtest
  INTERFACE
  "-L${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/build/lib")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_libraries(gtest
    INTERFACE
    "-lgtestd")
else()
  target_link_libraries(gtest
    INTERFACE
    "-lgtest")
endif()
add_dependencies(gtest
  googletest_build)

add_library(gtest_main
  INTERFACE)
target_link_libraries(gtest_main
  INTERFACE
  gtest
  "-L${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/build/lib")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_libraries(gtest_main
    INTERFACE
    "-lgtest_maind")
else()
  target_link_libraries(gtest
    INTERFACE
    "-lgtest_main")
endif()
add_dependencies(gtest_main
  googletest_build)
