aux_source_directory(. LIB_SOURCE_FILES)
add_library(http ${LIB_SOURCE_FILES})
target_link_libraries(http net)
