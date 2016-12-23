add_executable(test1 test1.cpp)
target_link_libraries(test1 net)
add_test(NAME test1 COMMAND test1)
set_tests_properties (test1
        PROPERTIES PASS_REGULAR_EXPRESSION "Timeout!")

add_executable(test2 test2.cpp)
target_link_libraries(test2 net)
add_test(NAME test2 COMMAND test2)

add_executable(test3 test3.cpp)
target_link_libraries(test3 net)
add_test(NAME test3 COMMAND test3)

add_executable(test4 test4.cpp)
target_link_libraries(test4 net)
add_test(NAME test4 COMMAND test4)

add_executable(test5 test5.cpp)
target_link_libraries(test5 net)
add_test(NAME test5 COMMAND test5)

add_executable(test6 test6.cpp)
target_link_libraries(test6 net)
add_test(NAME test6 COMMAND test6)

add_executable(test7 test7.cpp)
target_link_libraries(test7 net)
add_test(NAME test7 COMMAND test7)
