cmake_minimum_required(VERSION 3.7)
project(RTSSimulatorProj VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# add_definitions(-std=c++17)
set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAG, "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -lncurses")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

file(GLOB LIB_FILES src/* src/*/*)
add_library(RTSSimulatorLib SHARED ${LIB_FILES})
target_include_directories(RTSSimulatorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms)

target_link_libraries(RTSSimulatorLib ${CURSES_LIBRARIES})

add_executable(RTSSimulator src/main.cpp)
target_link_libraries(RTSSimulator PRIVATE RTSSimulatorLib)