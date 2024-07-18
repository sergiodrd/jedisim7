#ifndef JEDISIM
#define JEDISIM

#include <stdio.h>
#include <stdlib.h>

// for handling fatal errors
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
  int min_magnitude;     // minimum magnitude galaxy to simulate (inclusive)
  int max_magnitude;     // maximum magnitude galaxy to simulate (inclusive)
  int single_redshift;   // use a single source galaxy redshift? 0 = no, 1=yes
  double fixed_redshift; // the single source galaxy redshift to use
  double power;          // power for the power law galaxy distribution
  double lens_z;         // the redshift of the lenses

  // lenses
  int nlenses; // amount of lenses
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
  char *distorted_stamps_path;    // must end with "/"
  char *HST_path; // HST image after transforming, distorting, and pasting
  char *HST_convolved_path;        // HST image after convolution
  char *HST_convolved_noise_path;  // HST image after noise
  char *LSST_convolved_path;       // LSST image after convolution
  char *LSST_convolved_noise_path; // LSST image after noise
} Config;

// config helpers
char *get_config_string(char *path);
const Config *const parse_config_file(char *config_filestr);
void free_config(Config *config);

#endif
