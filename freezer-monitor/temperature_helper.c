#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include "temperature_helper.h"

struct timeval last_temp_check_time, current_temp_check_time;
int elapsed_temp_time_check_ms = 0;
float previous_temp_check_c = 0.0;
bool currently_bad_temp = false;
bool last_check_ret = false;

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

bool check_bad_temp(float current_temp_c)
{
    if (!currently_bad_temp && current_temp_c > 0.0)
    {
        currently_bad_temp = true;
        gettimeofday(&last_temp_check_time, NULL);
        previous_temp_check_c = current_temp_c;
    }
    else if (current_temp_c <= 0.0)
    {
        currently_bad_temp = false;
        last_check_ret = false;
        return last_check_ret;
    }
    
    if (currently_bad_temp)
    {
        gettimeofday(&current_temp_check_time, NULL);
        elapsed_temp_time_check_ms = (current_temp_check_time.tv_sec - last_temp_check_time.tv_sec) * 1000 + (current_temp_check_time.tv_usec - last_temp_check_time.tv_usec) / 1000;
        printf("elapsed_temp_time_check_ms: %d\n", elapsed_temp_time_check_ms);
        if (elapsed_temp_time_check_ms >= TEMP_CHECK_PERIOD_MS)
        {
            printf("new check, previous_temp_check_c: %d\n", previous_temp_check_c);
            printf("new check, current_temp_c: %d\n", current_temp_c);
            if (previous_temp_check_c <= current_temp_c)
            {
                last_check_ret = true;
            }
            else
            {
                last_check_ret = false;
            }
            gettimeofday(&last_temp_check_time, NULL);
            previous_temp_check_c = current_temp_c;
        }
    }

    return last_check_ret;
}