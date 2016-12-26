cmake_minimum_required(VERSION 3.4)
project(speedX)

enable_testing()

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O3 -Wall")
set(CMAKE_CXX_STANDARD 11)
add_definitions(-DNDEBUG)

include_directories(.)

add_subdirectory(net)
add_subdirectory(http)

aux_source_directory(. SOURCE_FILES)
add_executable(speedX ${SOURCE_FILES})
target_link_libraries(speedX http)
