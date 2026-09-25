#ifndef TEMPERATURE_HELPER_H_
#define TEMPERATURE_HELPER_H_

#include <stdbool.h>

#define TEMP_C_STR_SIZE 6
#define TEMP_CHECK_PERIOD_MS 5000

float convert_temp_buf_to_float(unsigned char * const temp_buf);

void limit_temp_c(float * const temp_c);

void set_temp_str(const float temp_c, char temp_c_str[TEMP_C_STR_SIZE]);

bool check_bad_temp(float current_temp_c);

#endif // TEMPERATURE_HELPER_H_