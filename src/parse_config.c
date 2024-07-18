#include "jedisim.h"
#include "string.h"
#include "toml.h"

// the return char* must be freed after use
char *get_config_string(char *path) {
  // open file
  FILE *config_fp = fopen(path, "r");
  if (!config_fp)
    error("could not open config file - ", path);

  // determine file size
  fseek(config_fp, 0, SEEK_END);
  long size = ftell(config_fp);
  fseek(config_fp, 0, SEEK_SET);

  // allocate memory for the contents of the file (+ 1 to null terminate)
  char *buffer = (char *)malloc(size + 1);
  if (!buffer)
    error("could not allocate memory for config file - ", path);

  // read file contents into the buffer
  fread(buffer, 1, size, config_fp);

  // null-terminate the string
  buffer[size] = '\0';

  fclose(config_fp);
  return buffer;
}

// macros to make extracting toml fields more convenient
#define PARSE_TOML_FIELD(table, key, type, target, type_letter)                \
  if (!toml_key_exists(table, key))                                            \
    error("couldn't parse config file - ", #key " not found in " #table ".");  \
  extracted = toml_##type##_in(table, key);                                    \
  if (!extracted.ok)                                                           \
    error("couldn't parse config file - ",                                     \
          #key " must be of type " #type ".");                                 \
  target = extracted.u.type_letter

#define PARSE_TOML_STRING(table, key, target)                                  \
  if (!toml_key_exists(table, key))                                            \
    error("couldn't parse config file - ", #key " not found in " #table ".");  \
  extracted = toml_string_in(table, key);                                      \
  if (!extracted.ok)                                                           \
    error("couldn't parse config file - ", #key " must be of type string.");   \
  target = strdup(extracted.u.s);                                              \
  free(extracted.u.s)

const Config *const parse_config_file(char *config_filestr) {
  Config *config = (Config *)calloc(1, sizeof(Config));
  char errbuf[200];

  // get main toml table
  toml_table_t *conf = toml_parse(config_filestr, errbuf, sizeof(errbuf));
  if (!conf)
    error("couldn't parse config file - ", errbuf);

  // datum to extract fields (used in macro)
  toml_datum_t extracted;

  // extract [inputs] table
  toml_table_t *inputs = toml_table_in(conf, "inputs");
  if (!inputs)
    error("couldn't parse config file - ", "inputs table not found.");

  PARSE_TOML_FIELD(inputs, "num_galaxies", int, config->num_galaxies, i);
  PARSE_TOML_STRING(inputs, "source_images_path", config->source_images_path);
  extracted = toml_string_in(inputs, "custom_catalog");
  if (extracted.ok)
    config->custom_catalog = strdup(extracted.u.s);
  free(extracted.u.s);

  // extract [physics] table
  toml_table_t *physics = toml_table_in(conf, "physics");
  if (!physics)
    error("couldn't parse config file - ", "physics table not found.");

  PARSE_TOML_FIELD(physics, "HST_pix_scale", double, config->HST_pix_scale, d);
  PARSE_TOML_FIELD(physics, "LSST_pix_scale", double, config->LSST_pix_scale,
                   d);
  PARSE_TOML_FIELD(physics, "exp_time", double, config->exp_time, d);
  PARSE_TOML_FIELD(physics, "noise_mean", double, config->noise_mean, d);
  PARSE_TOML_FIELD(physics, "min_magnitude", int, config->min_magnitude, i);
  PARSE_TOML_FIELD(physics, "max_magnitude", int, config->max_magnitude, i);
  PARSE_TOML_FIELD(physics, "single_redshift", bool, config->single_redshift,
                   b);
  PARSE_TOML_FIELD(physics, "fixed_redshift", double, config->fixed_redshift,
                   d);
  PARSE_TOML_FIELD(physics, "power", double, config->power, d);
  PARSE_TOML_FIELD(physics, "lens_z", double, config->lens_z, d);

  // extract [[physics.lenses]] table array
  toml_array_t *lenses = toml_array_in(physics, "lenses");
  if (!lenses)
    error("couldn't parse config file - ", "lenses table array not found.");
  config->nlenses = toml_array_nelem(lenses);
  config->lenses = (Lens *)calloc(config->nlenses, sizeof(Lens));

  for (int i = 0; i < config->nlenses; i++) {
    toml_table_t *lens = toml_table_at(lenses, i);
    PARSE_TOML_FIELD(lens, "x_center_pixel", int,
                     config->lenses[i].x_center_pixel, i);
    PARSE_TOML_FIELD(lens, "y_center_pixel", int,
                     config->lenses[i].y_center_pixel, i);
    PARSE_TOML_STRING(lens, "profile", config->lenses[i].profile);
    PARSE_TOML_FIELD(lens, "profile_param_1", double,
                     config->lenses[i].profile_param_1, d);
    PARSE_TOML_FIELD(lens, "profile_param_2", double,
                     config->lenses[i].profile_param_2, d);
  }

  // extract [physics.convolve] table
  toml_table_t *convolve = toml_table_in(physics, "convolve");
  if (!convolve)
    error("couldn't parse config file - ", "physics.convolve table not found.");

  PARSE_TOML_STRING(convolve, "psf_file", config->psf_file);

  // extract [physics.databases] table
  toml_table_t *databases = toml_table_in(physics, "databases");
  if (!databases)
    error("couldn't parse config file - ",
          "physics.databases table not found.");

  PARSE_TOML_STRING(databases, "radius_db_path", config->radius_db_path);
  PARSE_TOML_STRING(databases, "red_db_path", config->red_db_path);

  // extract [output_image_settings] table
  toml_table_t *o_i_s = toml_table_in(conf, "output_image_settings");
  if (!o_i_s)
    error("couldn't parse config file - ",
          "output_image_settings table not found.");

  PARSE_TOML_FIELD(o_i_s, "final_size_x", int, config->final_size_x, i);
  PARSE_TOML_FIELD(o_i_s, "final_size_y", int, config->final_size_y, i);
  PARSE_TOML_FIELD(o_i_s, "x_border", int, config->x_border, i);
  PARSE_TOML_FIELD(o_i_s, "y_border", int, config->y_border, i);
  PARSE_TOML_FIELD(o_i_s, "x_trim", int, config->x_trim, i);
  PARSE_TOML_FIELD(o_i_s, "y_trim", int, config->y_trim, i);

  // extract [outputs] table
  toml_table_t *outputs = toml_table_in(conf, "outputs");
  if (!outputs)
    error("couldn't parse config file - ", "outputs table not found.");

  // we're not using the macro here, because the fields are optional
  // since we used calloc instead of malloc, the values are initialized
  // to NULL, which will signify the value not being used.
  extracted = toml_string_in(outputs, "catalog_path");
  if (extracted.ok)
    config->catalog_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "transformed_catalog_path");
  if (extracted.ok)
    config->transformed_catalog_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "transformed_stamps_path");
  if (extracted.ok)
    config->transformed_stamps_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "distorted_stamps_path");
  if (extracted.ok)
    config->distorted_stamps_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "HST_path");
  if (extracted.ok)
    config->HST_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "HST_convolved_path");
  if (extracted.ok)
    config->HST_convolved_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "HST_convolved_noise_path");
  if (extracted.ok)
    config->HST_convolved_noise_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "LSST_convolved_path");
  if (extracted.ok)
    config->LSST_convolved_path = strdup(extracted.u.s);
  free(extracted.u.s);

  extracted = toml_string_in(outputs, "LSST_convolved_noise_path");
  if (extracted.ok)
    config->LSST_convolved_noise_path = strdup(extracted.u.s);
  free(extracted.u.s);

  toml_free(conf);

  return config;
}

// free a config object
void free_config(Config *config) {
  for (int i = 0; i < config->nlenses; i++)
    free((void *)config->lenses[i].profile);
  free((void *)config->lenses);

  free((void *)config->source_images_path);
  free((void *)config->custom_catalog);
  free((void *)config->psf_file);
  free((void *)config->radius_db_path);
  free((void *)config->red_db_path);
  free((void *)config->catalog_path);
  free((void *)config->transformed_catalog_path);
  free((void *)config->transformed_stamps_path);
  free((void *)config->distorted_stamps_path);
  free((void *)config->HST_path);
  free((void *)config->HST_convolved_path);
  free((void *)config->HST_convolved_noise_path);
  free((void *)config->LSST_convolved_path);
  free((void *)config->LSST_convolved_noise_path);
}
