cmake_minimum_required(VERSION 3.26)
set(CMAKE_CXX_STANDARD 11)
PROJECT(threadpool)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set (APP_NAME threadpool)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB SRC_LIST ${PROJECT_SOURCE_DIR}/src)

add_executable(${APP_NAME} ${SRC_LIST} main.cpp)

target_link_libraries(${APP_NAME} pthread)