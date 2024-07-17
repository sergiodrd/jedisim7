#include "toml.h"
#include <stdio.h>
#include <stdlib.h>

static void error(const char *msg1, const char *msg2) {
  fprintf(stderr, "ERROR: %s%s\n", msg1, msg2 ? msg2 : "");
  exit(1);
}

typedef struct {
  long x_center_pixel;
  long y_center_pixel;
  char *profile;
  double profile_param_1;
  double profile_param_2;
} Lens;

typedef struct {
  // inputs
  long num_galaxies;        // number of galaxies to simulate
  char *source_images_path; // input galaxy postage stamps
  char *custom_catalog;     // add this to use a custom catalog file

  // physics
  double HST_pix_scale;  // in arcseconds per pixel
  double LSST_pix_scale; // in arcseconds per pixel
  double exp_time;       // exposure time in seconds
  double noise_mean;     // mean for poisson noise
  long min_magnitude;    // minimum magnitude galaxy to simulate (inclusive)
  long max_magnitude;    // maximum magnitude galaxy to simulate (inclusive)
  int single_redshift;   // use a single source galaxy redshift? 0 = no, 1=yes
  long fixed_redshift;   // the single source galaxy redshift to use
  double power;          // power for the power law galaxy distribution
  double lens_z;         // the redshift of the lenses

  // lenses
  Lens *lenses;

  // convolve
  char *psf_file; // the PSF to use

  // cosmic reality databases
  char *radius_db_path; // radii database
  char *red_db_path;    // redshifts database

  // output image settings
  long final_size_x; // number of pixels in the x direction
  long final_size_y; // number of pixels in the y direction
  long x_border;     // must be large enough so that no image can overflow
  long y_border;     // must be large enough so that no image can overflow
  long x_trim;       // larger than x_border to ensure no edge effects
  long y_trim;       // larger than y_border to ensure no edge effects

  // outputs (can be null to omit the output)
  char *catalog_path;
  char *transformed_catalog_path; // must end with "/"
  char *transformed_stamps_path;  // must end with "/"
  char *HST_path; // HST image after transforming, distorting, and pasting
  char *HST_convolved_path;        // HST image after convolution
  char *HST_convolved_noise_path;  // HST image after noise
  char *LSST_convolved_path;       // LSST image after convolution
  char *LSST_convolved_noise_path; // LSST image after noise
} Config;

char *help[] = {"TODO: make help message", 0};

const Config *const parse_config_file(FILE *fp);

int main(int argc, char *argv[]) {
  // make sure the config file is provided
  if (argc != 2) {
    for (int i = 0; help[i] != 0; i++)
      fprintf(stderr, "%s\n", help[i]);
    exit(1);
  }

  // open the config file
  FILE *config_fp = fopen(argv[1], "r");
  if (!config_fp)
    error("could not open config file - ", argv[1]);

  // parse the config file (closes config_fp)
  const Config *const config = parse_config_file(config_fp);

  // free the config
  free((void *)config);

  return 0;
}

// useful macro for parsing
#define PARSE_TOML_FIELD(table, field, type, toml_function, target)            \
  raw_buf = toml_raw_in(table, field);                                         \
  if (!raw_buf)                                                                \
    error("couldn't parse config file - ",                                     \
          field " not found in the " #table " table.");                        \
  if (toml_##toml_function(raw_buf, target))                                   \
    error("couldn't parse config file - ", field " must be a " type);

const Config *const parse_config_file(FILE *fp) {

  Config *config = (Config *)calloc(1, sizeof(Config));
  char errbuf[200];

  // get main toml table
  toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);
  if (!conf)
    error("couldn't parse config file - ", errbuf);

  // buffer to read in fields
  const char *raw_buf;

  // extract [inputs] table
  toml_table_t *inputs = toml_table_in(conf, "inputs");
  if (!inputs)
    error("couldn't parse config file - ", "inputs table not found.");

  PARSE_TOML_FIELD(inputs, "num_galaxies", "integer", rtoi,
                   &config->num_galaxies);
  PARSE_TOML_FIELD(inputs, "source_images_path", "string", rtos,
                   &config->source_images_path)

  // extract [physics] table
  toml_table_t *physics = toml_table_in(conf, "physics");
  if (!inputs)
    error("couldn't parse config file - ", "physics table not found.");

  PARSE_TOML_FIELD(physics, "HST_pix_scale", "double", rtod,
                   &config->HST_pix_scale)
  PARSE_TOML_FIELD(physics, "LSST_pix_scale", "double", rtod,
                   &config->LSST_pix_scale)
  PARSE_TOML_FIELD(physics, "exp_time", "double", rtod, &config->exp_time)
  PARSE_TOML_FIELD(physics, "noise_mean", "double", rtod, &config->noise_mean)
  PARSE_TOML_FIELD(physics, "min_magnitude", "integer", rtoi,
                   &config->min_magnitude)
  PARSE_TOML_FIELD(physics, "max_magnitude", "integer", rtoi,
                   &config->max_magnitude)
  PARSE_TOML_FIELD(physics, "single_redshift", "integer", rtob,
                   &config->single_redshift)
  PARSE_TOML_FIELD(physics, "fixed_redshift", "integer", rtoi,
                   &config->fixed_redshift)
  PARSE_TOML_FIELD(physics, "power", "double", rtod, &config->power)
  PARSE_TOML_FIELD(physics, "lens_z", "double", rtod, &config->lens_z)

  // extract [[physics.lenses]] table array
  toml_table_t *lenses = toml_table_in(conf, "output_image_settings");
  if (!inputs)
    error("couldn't parse config file - ",
          "output_image_settings table not found.");

  // extract [output_image_settings] table
  toml_table_t *o_i_s = toml_table_in(conf, "output_image_settings");
  if (!inputs)
    error("couldn't parse config file - ",
          "output_image_settings table not found.");

  PARSE_TOML_FIELD(o_i_s, "final_size_x", "integer", rtoi,
                   &config->final_size_x);
  PARSE_TOML_FIELD(o_i_s, "final_size_y", "integer", rtoi,
                   &config->final_size_y);
  PARSE_TOML_FIELD(o_i_s, "x_border", "integer", rtoi, &config->x_border);
  PARSE_TOML_FIELD(o_i_s, "y_border", "integer", rtoi, &config->y_border);
  PARSE_TOML_FIELD(o_i_s, "x_trim", "integer", rtoi, &config->x_trim);
  PARSE_TOML_FIELD(o_i_s, "y_trim", "integer", rtoi, &config->y_trim);

  return config;
}
