#include <stddef.h>
#include <stdio.h>
#include "temperature_helper.h"

float convert_temp_buf_to_float(unsigned char * const temp_buf)
{
    float temp_c;

    if ((temp_buf[0] & 0x10) == 0x10)
    {
        temp_buf[0] &= 0x0f;
        temp_c = -1.0 * (((int)(temp_buf[0] << 8) | temp_buf[1]) / 10.0);
    }
    else
    {
        temp_c = ((int)(temp_buf[0] << 8) | temp_buf[1]) / 10.0;
    }

    return temp_c;
}

void limit_temp_c(float * const temp_c)
{
    if (temp_c == NULL)
    {
        return;
    }

    if (*temp_c > 99.9)
    {
        *temp_c = 99.9;
    }
    else if (*temp_c < -99.9)
    {
        *temp_c = -99.9;
    }
}

void set_temp_str(const float temp_c, char temp_c_str[TEMP_C_STR_SIZE])
{
    if (temp_c >= 10.0)
    {
        snprintf(temp_c_str, TEMP_C_STR_SIZE, " %02.1f", temp_c);
    }
    else if (temp_c < 10.0 && temp_c >= 0)
    {
        snprintf(temp_c_str, TEMP_C_STR_SIZE, "  %01.1f", temp_c);
    }
    else if(temp_c < 0 && temp_c > -10.0)
    {
        snprintf(temp_c_str, TEMP_C_STR_SIZE, " %02.1f", temp_c);
    }
    else // temp_c <= -10.0
    {
        snprintf(temp_c_str, TEMP_C_STR_SIZE, "%02.1f", temp_c);
    }
}