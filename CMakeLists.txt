cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
# add_definitions(-std=c++17)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAG, "${CXX_FLAGS}")

project(RTSSimulator)

file(GLOB project_SRCS src/*.cpp src/*/*.cpp)

add_executable(RTSSimulator ${project_SRCS})
