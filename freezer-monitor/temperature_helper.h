#ifndef TEMPERATURE_HELPER_H_
#define TEMPERATURE_HELPER_H_

#define TEMP_C_STR_SIZE 6

float convert_temp_buf_to_float(unsigned char * const temp_buf);

void limit_temp_c(float * const temp_c);

void set_temp_str(const float temp_c, char temp_c_str[TEMP_C_STR_SIZE]);

#endif // TEMPERATURE_HELPER_H_