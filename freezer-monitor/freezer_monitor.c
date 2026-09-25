#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include "1602_lcd_ioctl.h"
#include "temperature_helper.h"
#include "reed_switch_helper.h"
#include "buzzer_helper.h"

#define LOOP_RATE_HZ 30
#define LCD_ROW_SIZE 16

const int LOOP_WAIT_TIME = 1000000/LOOP_RATE_HZ;
volatile int running = 1;
const char reed_dev[] 	= "/dev/reed_switch";
const char lcd_dev[] 	= "/dev/1602_lcd";
const char temp_dev[]   = "/dev/mcp9808";
const char buzzer_dev[] = "/dev/buzzer";
char top_row_str[LCD_ROW_SIZE] 	= "Op: --/-- --:-- ";
char bottom_row_str[LCD_ROW_SIZE] = ("--:--:--  --.-" "\xDF" "C");
int reed_fd = -1;
int lcd_fd = -1;
int temp_fd = -1;
int buzzer_fd = -1;
struct lcd_cursor cursor_pos = {0, 0};

void signal_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
		running = 0;
    }
}

int write_line_to_lcd(char* line, int size, unsigned int row)
{
	if (row > 1)
	{
		row = 1;
	}

	cursor_pos.row = row;
	cursor_pos.col = 0;
	ioctl(lcd_fd, LCD_IOCTL_SETCURSOR, &cursor_pos);
	int lcd_write_ret = write(lcd_fd, line, size);
	if (lcd_write_ret < 0)
	{
		perror("lcd write");
		syslog(LOG_ERR, "lcd write");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int reed_value = -1;
	int old_reed_value = -1;
	int new_reed = -1;

	unsigned char temp_buf[2];
	float temp_c = 0.0;
	char temp_c_str[TEMP_C_STR_SIZE] = " --.-\0";

	char buzzer_value = 0;

	int daemon_mode = 0;

	int ret = 0;
	
	struct sigaction sa;

	char elapsed_time_str[ELAPSED_TIME_STR_SIZE];
	int elapsed_sec = 0;
	struct tm *local_time;
	time_t open_start_time = 0;
	
	sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);	
	
	if (argc == 2 && strcmp(argv[1], "-d") == 0)
	{
		daemon_mode = 1;
	}
	
	if (daemon_mode)
	{
		pid_t pid;
	
	    pid = fork();
	
	    if (pid == -1)
		{
		    perror ("fork");
		    syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
            return -1;
	    }
	    if (pid > 0)
		{
			printf("Start in daemon\n");
			exit(0);
		}
		
		setsid();
		chdir("/");
	}
	
    char reed_buf;	
    ssize_t ret_byte;
	int debounce_counter = 0;
	int reed_stable = 1;
	
	reed_fd = open(reed_dev, O_RDONLY);	
	if (reed_fd == -1)
    {
		perror("open /dev/reed_switch");
		syslog(LOG_ERR, "open /dev/reed_switch");
		return -1;
	}

	lcd_fd = open(lcd_dev, O_WRONLY);
	if (reed_fd == -1)
	{
		perror("open /dev/1602_lcd");
		syslog(LOG_ERR, "open /dev/1602_lcd");
		return -1;
	}

	temp_fd = open(temp_dev, O_RDONLY);
	if (temp_fd == -1)
	{
		perror("open /dev/mcp9808");
		syslog(LOG_ERR, "open /dev/mcp9808");
		return -1;
	}

	buzzer_fd = open(buzzer_dev, O_WRONLY);
	if (buzzer_fd == -1)
	{
		perror("open /dev/buzzer");
		syslog(LOG_ERR, "open /dev/buzzer");
		return -1;
	}
	
	if (write_line_to_lcd(top_row_str, strlen(top_row_str), 0) < 0) 
	{
		return -1;
	}

	if (write_line_to_lcd(bottom_row_str, strlen(bottom_row_str), 1) < 0)
	{
		return -1;
	}

	while (running)
    {
		usleep(LOOP_WAIT_TIME);
		
		ret_byte = read(reed_fd, &reed_buf, 1);
		if (ret_byte != 1)
		{
			perror("reed switch read");
			syslog(LOG_ERR, "reed switch read");
			return -1;
		}

		ret_byte = read(temp_fd, &temp_buf, 2);
		if (ret_byte != 2)
		{
			perror("temperature read");
			syslog(LOG_ERR, "temperature read");
			return -1;
		}

		temp_c = convert_temp_buf_to_float(temp_buf);
		
		if (reed_stable)
        {
			debounce_counter = 0;
			reed_value = reed_buf;
			new_reed = 1;
			reed_stable = 0;
		}
		else
        {
			new_reed = 0;
			
			if (reed_buf == old_reed_value && reed_buf != reed_value)
			{
				debounce_counter++;
			}
			if (debounce_counter >= 5)
			{
				reed_stable = 1;
			}
			
			old_reed_value = reed_buf;
		}
        
        if (new_reed)
        {
			if (reed_value == 0)
			{
				open_start_time = time(NULL);
				local_time = localtime(&open_start_time);
				strftime(top_row_str, sizeof(top_row_str), "Op: %m/%d %H:%M ", local_time);
			}

			if (write_line_to_lcd(top_row_str, strlen(top_row_str), 0) < 0) 
			{
				return -1;
			}

			if (write_line_to_lcd(bottom_row_str, strlen(bottom_row_str), 1) < 0)
			{
				return -1;
			}
        }

		if (reed_value == 0 && open_start_time != 0)
		{
			set_elapsed_time_str(&elapsed_sec, &open_start_time, elapsed_time_str);
			memcpy(bottom_row_str, elapsed_time_str, 8);
		}

		limit_temp_c(&temp_c);

		set_temp_str(temp_c, temp_c_str);
		memcpy(bottom_row_str+9, temp_c_str, 5);

		if (write_line_to_lcd(bottom_row_str, strlen(bottom_row_str), 1) < 0)
		{
			return -1;
		}

		if (temp_c > 0.0)
		{
			ret = control_buzzer(buzzer_fd, 1);
			if (ret != 0)
			{
				return ret;
			}
		}
	
		else if (reed_value == 0 && elapsed_sec > 5)
		{
			ret = control_buzzer(buzzer_fd, 0);
			if (ret != 0)
			{
				return ret;
			}
		}
		else
		{
			ret = turn_off_buzzer(buzzer_fd);
			if (ret != 0)
			{
				return ret;
			}
		}
	}
	
	printf("Caught signal, exiting\n");
	
	if (reed_fd != -1)
	{
		close(reed_fd);
	}

	if (lcd_fd != -1)
	{
		memcpy(top_row_str, "                ", LCD_ROW_SIZE);
		memcpy(bottom_row_str, "                ", LCD_ROW_SIZE);
		write_line_to_lcd(top_row_str, strlen(bottom_row_str), 0);
		write_line_to_lcd(bottom_row_str, strlen(bottom_row_str), 1);
		cursor_pos.row = 0;
		cursor_pos.col = 0;
		ioctl(lcd_fd, LCD_IOCTL_SETCURSOR, &cursor_pos);
		close(lcd_fd);
	}

	if (temp_fd != -1)
	{
		close(temp_fd);
	}

	if (buzzer_fd != -1)
	{
		buzzer_value = 0;
		write(buzzer_fd, &buzzer_value, 1);
		close(buzzer_fd);
	}
	
	return 0;
}