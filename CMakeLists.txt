cmake_minimum_required(VERSION 3.6)
project(speedX)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

aux_source_directory(. SOURCE_FILES)
aux_source_directory(net SOURCE_FILES)
aux_source_directory(base SOURCE_FILES)

add_executable(speedX ${SOURCE_FILES})

target_link_libraries(speedX boost_log)
