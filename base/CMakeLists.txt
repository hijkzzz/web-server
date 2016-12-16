aux_source_directory(. LIB_SOURCE_FILES)
add_library(base ${LIB_SOURCE_FILES})
target_link_libraries(base
                        boost_log)
