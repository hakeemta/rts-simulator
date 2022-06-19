cmake_minimum_required(VERSION 3.7)
project(RTSSimulator VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# add_definitions(-std=c++17)
set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAG, "${CXX_FLAGS}")

file(GLOB LIB_FILES src/* src/*/*)
file(GLOB INC_FILES include/*)

add_library(RTSSimulatorLib SHARED ${LIB_FILES})
target_include_directories(RTSSimulatorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB PROJECT_SRCS src/main.cpp)
add_executable(RTSSimulator ${PROJECT_SRCS})
target_link_libraries(RTSSimulator PRIVATE RTSSimulatorLib)
