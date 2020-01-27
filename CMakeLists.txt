cmake_minimum_required(VERSION 3.16.2)
project(corio)

enable_testing()

message(STATUS
  "CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message(STATUS
  "CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 20)
message(STATUS
  "CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")

if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
  add_compile_options(-Werror -Wall)
else()
  message(FATAL_ERROR
    "An unsupported C++ compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fcoroutines-ts)
    add_link_options(-fcoroutines-ts)
  else()
    message(FATAL_ERROR
      "An unsupported C++ compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}.")
  endif()
else()
  message(FATAL_ERROR
    "An unsupported system: CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}.")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
message(STATUS
  "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
    add_compile_options(-stdlib=libstdc++)
    add_link_options(-stdlib=libstdc++)
  elseif(CORIO_CXX_STDLIB STREQUAL "libc++")
    add_compile_options(-stdlib=libc++)
    add_link_options(-stdlib=libc++)
  else()
    message(FATAL_ERROR
      "An unsupported C++ standard library: CXX_TEMPLATE_CXX_STDLIB=${CXX_TEMPLATE_CXX_STDLIB}.")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CXX_TEMPLATE_CXX_STDLIB STREQUAL "libstdc++")
      add_definitions(
        -D_GLIBCXX_DEBUG
        -D_GLIBCXX_DEBUG_PEDANTIC)
    endif()
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(
      -D_GLIBCXX_DEBUG
      -D_GLIBCXX_DEBUG_PEDANTIC)
  endif()
endif()

if(NOT DEFINED CORIO_ENABLE_ASSERT)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CORIO_ENABLE_ASSERT ON)
  else()
    set(CORIO_ENABLE_ASSERT OFF)
  endif()
endif()
message(STATUS
  "CORIO_ENABLE_ASSERT=${CORIO_ENABLE_ASSERT}")
if (CORIO_ENABLE_ASSERT)
  add_definitions(-DCORIO_ENABLE_ASSERT)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=pointer-compare)
    add_link_options(-fsanitize=pointer-compare)
    add_compile_options(-fsanitize=pointer-subtract)
    add_link_options(-fsanitize=pointer-subtract)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    add_compile_options(-pthread)
    add_link_options(-pthread)
  else()
    message(FATAL_ERROR
      "An unsupported C++ compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}.")
  endif()
else()
  message(FATAL_ERROR
    "An unsupported system: CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}.")
endif()


add_subdirectory(ext/libbacktrace)
add_subdirectory(ext/icu4c)
add_subdirectory(ext/boost)
add_subdirectory(ext/googletest)

add_executable(corio_test
  src/util/exception.cpp
  test/util/assert.cpp
  test/util/scope_guard.cpp
  test/util/index_pack.cpp
  test/util/throw.cpp
  test/thread_unsafe/mutex.cpp
  test/thread_unsafe/promise.cpp
  test/thread_unsafe/future.cpp
  test/thread_unsafe/coroutine.cpp)
target_link_libraries(corio_test
  boost_stacktrace_backtrace
  gtest_main)

add_test(
  NAME corio_test
  COMMAND env ASAN_OPTIONS=detect_leaks=1 "${CMAKE_CURRENT_BINARY_DIR}/corio_test" --gtest_catch_exceptions=0)

add_executable(sleep_sort
  src/util/exception.cpp
  sample/sleep_sort.cpp)
target_link_libraries(sleep_sort
  boost_stacktrace_backtrace)
