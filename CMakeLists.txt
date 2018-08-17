cmake_minimum_required(VERSION 3.7.1)
project(term)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

#include_directories(../common/)

add_executable(term termMain.cpp commonDefs.hpp inputInterpreter.cpp inputInterpreter.hpp commands.hpp help.hpp)
target_link_libraries(term pthread)
