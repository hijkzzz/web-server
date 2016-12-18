add_executable(test1 test1.cpp)
target_link_libraries(test1
                        net)
add_test(NAME test1 COMMAND test1)
set_tests_properties (test1
        PROPERTIES PASS_REGULAR_EXPRESSION "Timeout!")

add_executable(test2 test2.cpp)
target_link_libraries(test2
                        net)
add_test(NAME test2 COMMAND test2)
