#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "buzzer_helper.h"

bool buzzer_running = false;
int current_buzzer_state = 0;
struct timeval buzzer_start_run_time, buzzer_current_run_time;
int elapsed_buzzer_time_ms = 0;

int control_buzzer(const int buzzer_fd, const int buzzer_type)
{
    int on_time = 0;
    int off_time = 0;
    if (buzzer_type == 0)
    {
        on_time = DOOR_OPEN_ON_TIME_MS;
        off_time = DOOR_OPEN_OFF_TIME_MS;
    }
    else
    {
        on_time = TEMP_ABOVE_0_ON_TIME;
        off_time = TEMP_ABOVE_0_OFF_TIME;
    }

    if (buzzer_fd <= 0)
    {
        return -1;
    }

    if (!buzzer_running)
    {
        buzzer_running = true;
        gettimeofday(&buzzer_start_run_time, NULL);
    }

    gettimeofday(&buzzer_current_run_time, NULL);

    elapsed_buzzer_time_ms = (buzzer_current_run_time.tv_sec - buzzer_start_run_time.tv_sec) * 1000 + (buzzer_current_run_time.tv_usec - buzzer_start_run_time.tv_usec) / 1000;
        
    if (elapsed_buzzer_time_ms <= on_time && current_buzzer_state != 1)
    {
        current_buzzer_state = 1;
        if (write(buzzer_fd, &current_buzzer_state, 1) == -1)
        {
            perror("buzzer write");
            syslog(LOG_ERR, "buzzer write");
            return -1;
        }
    }
    else if (elapsed_buzzer_time_ms > on_time && elapsed_buzzer_time_ms <= on_time + off_time && current_buzzer_state != 0)
    {
        current_buzzer_state = 0;
        if (write(buzzer_fd, &current_buzzer_state, 1) == -1)
        {
            perror("buzzer write");
            syslog(LOG_ERR, "buzzer write");
            return -1;
        }
    }
    
    if (elapsed_buzzer_time_ms > on_time + off_time)
    {
        gettimeofday(&buzzer_start_run_time, NULL);
    }

    return 0;
}

int turn_off_buzzer(int buzzer_fd)
{
    if (buzzer_fd <= 0)
    {
        return -1;
    }

    buzzer_running = false;

    int buzzer_value = 0;
    if (write(buzzer_fd, &buzzer_value, 1) == -1)
    {
        perror("buzzer write");
        syslog(LOG_ERR, "buzzer write");
        return -1;
    }

    return 0;
}