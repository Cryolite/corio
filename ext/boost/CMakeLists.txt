include(ExternalProject)

set(BOOST_LIBRARIES
  "exception"
  "stacktrace")

set(BOOST_DOWNLOAD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/download"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost")

set(BOOST_BUILD_COMMAND
  "${CMAKE_CURRENT_SOURCE_DIR}/build"
  "--source-dir=${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost")
list(APPEND
  BOOST_BUILD_COMMAND
  "--prefix=${CMAKE_CURRENT_BINARY_DIR}/boost-prefix"
  "--"
  "--build-type=minimal"
  "--build-dir=<BINARY_DIR>"
  "--layout=system")
foreach(library IN LISTS BOOST_LIBRARIES)
  list(APPEND
    BOOST_BUILD_COMMAND
    "--with-${library}")
endforeach()
list(APPEND
  BOOST_BUILD_COMMAND
  "-sLIBBACKTRACE_PATH=${CMAKE_CURRENT_BINARY_DIR}/../libbacktrace/libbacktrace-prefix"
  "-sICU_PATH=${CMAKE_CURRENT_BINARY_DIR}/../icu4c/icu4c-prefix")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND
    BOOST_BUILD_COMMAND
    "toolset=clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  list(APPEND
    BOOST_BUILD_COMMAND
    "toolset=gcc")
else()
  message(FATAL_ERROR
    "An unsupported compiler: CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
endif()
if(CMAKE_CXX_STANDARD STREQUAL "11")
  list(APPEND
    BOOST_BUILD_COMMAND
    "cxxflags=-std=c++11")
elseif(CMAKE_CXX_STANDARD STREQUAL "14")
  list(APPEND
    BOOST_BUILD_COMMAND
    "cxxflags=-std=c++14")
elseif(CMAKE_CXX_STANDARD STREQUAL "17")
  list(APPEND
    BOOST_BUILD_COMMAND
    "cxxflags=-std=c++17")
elseif(CMAKE_CXX_STANDARD STREQUAL "20")
  list(APPEND
    BOOST_BUILD_COMMAND
    "cxxflags=-std=c++2a")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
    list(APPEND
      BOOST_BUILD_COMMAND
      "cxxflags=-stdlib=libstdc++"
      "linkflags=-stdlib=libstdc++")
  elseif(CORIO_CXX_STDLIB STREQUAL "libc++")
    list(APPEND
      BOOST_BUILD_COMMAND
      "cxxflags=-stdlib=libc++"
      "linkflags=-stdlib=libc++")
  else()
    message(FATAL_ERROR
      "An unsupported C++ standard library: CORIO_CXX_STDLIB=${CORIO_CXX_STDLIB}.")
  endif()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND
    BOOST_BUILD_COMMAND
    "variant=debug"
    "cflags=-fsanitize=address"
    "cxxflags=-fsanitize=address"
    "linkflags=-fsanitize=address"
    "cflags=-fsanitize=pointer-compare"
    "cxxflags=-fsanitize=pointer-compare"
    "linkflags=-fsanitize=pointer-compare"
    "cflags=-fsanitize=pointer-subtract"
    "cxxflags=-fsanitize=pointer-subtract"
    "linkflags=-fsanitize=pointer-subtract"
    "cflags=-fsanitize=undefined"
    "cxxflags=-fsanitize=undefined"
    "linkflags=-fsanitize=undefined")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CORIO_CXX_STDLIB STREQUAL "libstdc++")
      list(APPEND
        BOOST_BUILD_COMMAND
        "define=_GLIBCXX_DEBUG"
        "define=_GLIBCXX_DEBUG_PEDANTIC")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND
      BOOST_BUILD_COMMAND
      "define=_GLIBCXX_DEBUG"
      "define=_GLIBCXX_DEBUG_PEDANTIC")
  endif()
else()
  list(APPEND
    BOOST_BUILD_COMMAND
    "variant=release"
    "cflags=-flto"
    "cxxflags=-flto"
    "linkflags=-flto")
endif()
if(BUILD_SHARED_LIBS)
  list(APPEND
    BOOST_BUILD_COMMAND
    "link=shared")
else()
  list(APPEND
    BOOST_BUILD_COMMAND
    "link=static")
endif()
list(APPEND
  BOOST_BUILD_COMMAND
  "threading=multi")

ExternalProject_Add(boost_build
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/boost-prefix"
  DOWNLOAD_COMMAND ${BOOST_DOWNLOAD_COMMAND}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ${BOOST_BUILD_COMMAND}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  DEPENDS libbacktrace_build icu4c_build)

add_library(boost_headers
  INTERFACE)
target_include_directories(boost_headers
  INTERFACE
  "${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/include")
add_dependencies(boost_headers
  boost_build)

foreach(spec "atomic;ATOMIC" "container;CONTAINER" "context;CONTEXT"
    "date_time;DATE_TIME" "exception" "iostreams;IOSTREAMS" "math_c99f"
    "math_c99l" "math_c99" "program_options;PROGRAM_OPTIONS" "regex;REGEX"
    "graph;GRAPH;regex" "serialization;SERIALIZATION" "signals;SIGNALS"
    "system;SYSTEM" "chrono;CHRONO;system" "contract;CONTRACT;system"
    "filesystem;FILESYSTEM;system" "fiber;FIBERS;context;system;filesystem"
    "random;RANDOM;system" "test_exec_monitor;TEST" "thread;THREAD;system"
    "coroutine;COROUTINES;context;system;chrono;thread"
    "locale;LOCALE;system;chrono;thread"
    "log;LOG;atomic;date_time;regex;system;chrono;filesystem;thread"
    "log_setup;LOG;atomic;date_time;regex;system;chrono;filesystem;thread;log"
    "timer;TIMER;system;chrono" "prg_exec_monitor;TEST;system;timer"
    "type_erasure;TYPE_ERASURE;system;chrono;thread"
    "unit_test_framework;TEST;system;timer"
    "wave;WAVE;date_time;system;chrono;filesystem;thread"
    "wserialization;SERIALIZATION;serialization")
  list(GET spec 0 lib)
  add_library("boost_${lib}"
      INTERFACE)
  list(LENGTH spec len)
  if(len GREATER_EQUAL 2)
    list(GET spec 1 LIB)
    if(BUILD_SHARED_LIBS)
      set_property(
        TARGET "boost_${lib}"
        APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "BOOST_${LIB}_DYN_LINK")
    endif()
  endif()
  target_link_libraries("boost_${lib}"
    INTERFACE
    boost_headers)
  if(len GREATER_EQUAL 3)
    list(REMOVE_AT spec 0 1)
    foreach(dep IN LISTS spec)
      target_link_libraries("boost_${lib}"
        INTERFACE
        "boost_${dep}")
    endforeach()
  endif()
  target_link_libraries("boost_${lib}"
    INTERFACE
    "-L${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/lib"
    "-lboost_${lib}")
  add_dependencies("boost_${lib}"
    boost_build)
endforeach()

add_library(boost_stacktrace_addr2line
  INTERFACE)
target_link_libraries(boost_stacktrace_addr2line
  INTERFACE
  boost_headers)
set_property(
  TARGET boost_stacktrace_addr2line
  APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_STACKTRACE_USE_ADDR2LINE)
target_link_libraries(boost_stacktrace_addr2line
  INTERFACE
  "-ldl"
  "-rdynamic")
add_dependencies(boost_stacktrace_addr2line
  boost_build)

add_library(boost_stacktrace_backtrace
  INTERFACE)
target_link_libraries(boost_stacktrace_backtrace
  INTERFACE
  boost_headers)
set_property(
  TARGET boost_stacktrace_backtrace
  APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_STACKTRACE_USE_BACKTRACE)
target_link_libraries(boost_stacktrace_backtrace
  INTERFACE
  libbacktrace
  "-ldl"
  "-rdynamic")
add_dependencies(boost_stacktrace_backtrace
  boost_build)

add_library(boost_stacktrace_basic
  INTERFACE)
target_link_libraries(boost_stacktrace_basic
  INTERFACE
  boost_headers)
target_link_libraries(boost_stacktrace_basic
  INTERFACE
  "-ldl"
  "-rdynamic")
add_dependencies(boost_stacktrace_basic
  boost_build)

add_library(boost_stacktrace_noop
  INTERFACE)
target_link_libraries(boost_stacktrace_noop
  INTERFACE
  boost_headers)
set_property(
  TARGET boost_stacktrace_noop
  APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS BOOST_STACKTRACE_USE_NOOP)
add_dependencies(boost_stacktrace_noop
  boost_build)
