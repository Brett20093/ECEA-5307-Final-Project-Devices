#ifndef REED_SWITCH_HELPER_H_
#define REED_SWITCH_HELPER_H_

#define ELAPSED_TIME_STR_SIZE 9

void set_elapsed_time_str(int * const elapsed_sec, const time_t * const open_start_time, char elapsed_time_str[ELAPSED_TIME_STR_SIZE]);

#endif // REED_SWITCH_HELPER_H_