cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_COVERAGE "Build coverage" OFF)

set(
  HUNTER_CACHE_SERVERS
  "https://github.com/bmstu-iu8-cpp-sem-3/hunter-cache"
  CACHE STRING "Default cache server"
)

include("tools/gate/cmake/HunterGate.cmake")

huntergate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
  SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
)

project(server)
string(APPEND CMAKE_CXX_FLAGS " -pedantic -Werror -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS " -Wno-unused-command-line-argument")
string(APPEND CMAKE_CXX_FLAGS " -Wshadow -Wnon-virtual-dtor")

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

hunter_add_package(Boost COMPONENTS log thread system date_time)
find_package(Boost CONFIG REQUIRED log thread system date_time)

add_library(server STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/server.cpp
)
add_executable(tests
  #${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sources/main.cpp
)

target_include_directories(server
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(tests
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(tests GTest::main Boost::log Boost::thread Boost::boost Boost::system Boost::date_time server)
target_link_libraries(server Boost::log Boost::thread Boost::boost Boost::system Boost::date_time)

enable_testing()
add_test(NAME unit_tests COMMAND tests)

if(BUILD_COVERAGE)
  set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  find_package(codecov)
  add_coverage(server)
  add_coverage(tests)
  list(APPEND LCOV_REMOVE_PATTERNS "'${PROJECT_SOURCE_DIR}/tests/*'")
  coverage_evaluate()
endif()
