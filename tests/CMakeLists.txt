add_executable(tests tests.c)
target_link_libraries(tests PRIVATE unity)
add_test(NAME run_all COMMAND tests)
