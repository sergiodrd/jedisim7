#include "jedisim.h"
#include <stdio.h>
#include <stdlib.h>

char *help[] = {"TODO: make help message", 0};

int main(int argc, char *argv[]) {
  // make sure the config file is provided
  if (argc != 2) {
    for (int i = 0; help[i] != 0; i++)
      fprintf(stderr, "%s\n", help[i]);
    exit(1);
  }

  // open the config file
  char *config_str = get_config_string(argv[1]);

  // parse the config file
  const Config *const config = parse_config_file(config_str);
  free(config_str);

  // free the config
  free((void *)config);

  return 0;
}
