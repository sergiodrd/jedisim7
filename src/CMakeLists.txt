add_executable(jedisim jedisim.h main.c parse_config.c)
target_link_libraries(jedisim PUBLIC tomlc99)

add_library(jedisimlib STATIC jedisim.h parse_config.c)
target_link_libraries(jedisimlib PUBLIC tomlc99)
