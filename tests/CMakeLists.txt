add_executable(config_parsing_suite config_parsing_suite.c)
target_link_libraries(config_parsing_suite PRIVATE unity jedisimlib)
add_test(NAME config_parsing_suite COMMAND config_parsing_suite)
