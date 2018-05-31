cmake_minimum_required(VERSION 2.8)

project(asnet CXX C)

SET(CMAKE_CXX_FLAGS "-pthread -g -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(examples)
