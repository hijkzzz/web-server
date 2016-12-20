aux_source_directory(. LIB_SOURCE_FILES)
add_library(net ${LIB_SOURCE_FILES})
target_link_libraries(net boost_log pthread)

add_subdirectory(tests)
