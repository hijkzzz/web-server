cmake_minimum_required(VERSION 3.6)
project(speedX)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

set(SOURCE_FILES
        main.cpp
        net/EventLoop.cpp)

add_executable(speedX ${SOURCE_FILES})