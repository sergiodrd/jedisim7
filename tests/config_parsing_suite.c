#include "../src/jedisim.h"
#include "unity.h"

Config *config;
char *filename = "../../sample_config.toml";

void setUp() {
  char *config_filestr = get_config_string(filename);
  config = (Config *)parse_config_file(config_filestr);
  free(config_filestr);
}
void tearDown() { free_config(config); }

void test_inputs_table() {
  TEST_ASSERT_EQUAL_INT(45000, config->num_galaxies);
  TEST_ASSERT_EQUAL_STRING("input_stamps/", config->source_images_path);
  TEST_ASSERT_EQUAL_STRING("catalog.txt", config->custom_catalog);
}

void test_physics_table() {
  TEST_ASSERT_EQUAL_FLOAT(0.06, config->HST_pix_scale);
  TEST_ASSERT_EQUAL_FLOAT(0.263, config->LSST_pix_scale);
  TEST_ASSERT_EQUAL_INT(1, config->exp_time);
  TEST_ASSERT_EQUAL_FLOAT(0.0001, config->noise_mean);
  TEST_ASSERT_EQUAL_INT(22, config->min_magnitude);
  TEST_ASSERT_EQUAL_INT(28, config->max_magnitude);
  TEST_ASSERT_EQUAL_INT(1, config->single_redshift);
  TEST_ASSERT_EQUAL_INT(1, config->fixed_redshift);
  TEST_ASSERT_EQUAL_FLOAT(0.34, config->power);
  TEST_ASSERT_EQUAL_FLOAT(0.07, config->lens_z);
}

void test_physics_lenses_table_array() {
  TEST_ASSERT_EQUAL_INT(2, config->nlenses);
  TEST_ASSERT_EQUAL_INT(6144, config->lenses[0].x_center_pixel);
  TEST_ASSERT_EQUAL_INT(6144, config->lenses[0].y_center_pixel);
  TEST_ASSERT_EQUAL_STRING("NFW", config->lenses[0].profile);
  TEST_ASSERT_EQUAL_FLOAT(200.0, config->lenses[0].profile_param_1);
  TEST_ASSERT_EQUAL_FLOAT(4.0, config->lenses[0].profile_param_2);
  TEST_ASSERT_EQUAL_INT(6144, config->lenses[1].x_center_pixel);
  TEST_ASSERT_EQUAL_INT(6144, config->lenses[1].y_center_pixel);
  TEST_ASSERT_EQUAL_STRING("SIS", config->lenses[1].profile);
  TEST_ASSERT_EQUAL_FLOAT(200.0, config->lenses[1].profile_param_1);
  TEST_ASSERT_EQUAL_FLOAT(4.0, config->lenses[1].profile_param_2);
}

void test_physics_convolve_table() {
  TEST_ASSERT_EQUAL_STRING("physics_settings/psf_scalednew_60.fits",
                           config->psf_file);
}

void test_physics_databases_table() {
  TEST_ASSERT_EQUAL_STRING("simdatabase/radius_db/", config->radius_db_path);
  TEST_ASSERT_EQUAL_STRING("simdatabase/red_db/", config->red_db_path);
}

void test_output_image_settings_table() {
  TEST_ASSERT_EQUAL_INT(config->final_size_x, 12288);
  TEST_ASSERT_EQUAL_INT(config->final_size_y, 12288);
  TEST_ASSERT_EQUAL_INT(config->x_border, 301);
  TEST_ASSERT_EQUAL_INT(config->y_border, 301);
  TEST_ASSERT_EQUAL_INT(config->x_trim, 0);
  TEST_ASSERT_EQUAL_INT(config->y_trim, 0);
}

void test_outputs_table() {
  TEST_ASSERT_EQUAL_STRING("out/catalog.txt", config->catalog_path);
  TEST_ASSERT_EQUAL_STRING("out/distorted.txt",
                           config->transformed_catalog_path);
  TEST_ASSERT_EQUAL_STRING("out/stamps/", config->transformed_stamps_path);
  TEST_ASSERT_EQUAL_STRING("out/distorted/", config->distorted_stamps_path);
  TEST_ASSERT_EQUAL_STRING("out/HST.fits", config->HST_path);
  TEST_ASSERT_EQUAL_STRING("out/HST_convolved.fits",
                           config->HST_convolved_path);
  TEST_ASSERT_EQUAL_STRING("out/HST_convolved_noise.fits",
                           config->HST_convolved_noise_path);
  TEST_ASSERT_EQUAL_STRING("out/LSST_convolved.fits",
                           config->LSST_convolved_path);
  TEST_ASSERT_EQUAL_STRING("out/LSST_convolved_noise.fits",
                           config->LSST_convolved_noise_path);
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    filename = argv[1];

  UNITY_BEGIN();
  RUN_TEST(test_inputs_table);
  RUN_TEST(test_physics_table);
  RUN_TEST(test_physics_lenses_table_array);
  RUN_TEST(test_physics_convolve_table);
  RUN_TEST(test_physics_databases_table);
  RUN_TEST(test_output_image_settings_table);
  RUN_TEST(test_outputs_table);
  return UNITY_END();
}
