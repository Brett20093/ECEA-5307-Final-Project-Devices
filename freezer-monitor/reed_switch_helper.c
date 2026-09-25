#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "reed_switch_helper.h"

void set_elapsed_time_str(int * const elapsed_sec, const time_t * const open_start_time, char elapsed_time_str[ELAPSED_TIME_STR_SIZE])
{
    if (elapsed_sec == NULL || open_start_time == NULL)
    {
        return;
    }

    *elapsed_sec = (int)difftime(time(NULL), *open_start_time);
    int hr = *elapsed_sec / 3600;
    int min = (*elapsed_sec % 3600) / 60;
    int sec = *elapsed_sec % 60;

    if (hr > 99)
    {
        hr = 99;
    }

    snprintf(elapsed_time_str, ELAPSED_TIME_STR_SIZE, "%02d:%02d:%02d", hr, min, sec);
}