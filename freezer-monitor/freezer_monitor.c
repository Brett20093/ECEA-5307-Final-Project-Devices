#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>

volatile int terminate = 0;
const char reed_dev[] = "/dev/reed_switch";
int reed_fd = -1;

int reed_value = 0;
int old_reed_value = 0;
int new_reed = 0;

void signal_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
		terminate = 1;
    }
}

int main (int argc, char **argv)
{
	int daemon_mode = 0;
	
	struct sigaction sa;
	
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

	while (!terminate)
    {
		usleep(10000);
		
		ret_byte = read(reed_fd, &reed_buf, 1);
		if (ret_byte != 1)
		{
			perror("read");
			syslog(LOG_ERR, "read");
			return -1;
		}
		
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
			
			if (reed_buf == old_reed_value)
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
            printf("Reed switch value: %d\n", reed_value);
        }
	}
	
	printf("Caught signal, exiting\n");
	
	if (reed_fd != -1)
	{
		close(reed_fd);
	}
	
	return 0;
}