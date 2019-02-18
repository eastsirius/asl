
cmake_minimum_required(VERSION 2.6)
project(asl)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

option(ASL_WITHOUT_TESTS "Build without tests" NO)
option(ASL_WITHOUT_EXAMPLES "Build without examples" NO)

message("Build type: " ${CMAKE_BUILD_TYPE})
message("ASL_WITHOUT_TESTS: " ${ASL_WITHOUT_TESTS}) 
message("ASL_WITHOUT_EXAMPLES: " ${ASL_WITHOUT_EXAMPLES})

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(ASL_ROOT_PATH ${CMAKE_SOURCE_DIR})

add_subdirectory(src/asl)

if(NOT ASL_WITHOUT_TESTS)
  add_subdirectory(test/utest)
endif(NOT ASL_WITHOUT_TESTS)

