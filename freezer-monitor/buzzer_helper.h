#ifndef BUZZER_HELPER_H_
#define BUZZER_HELPER_H_

#include <stdbool.h>
#include <time.h>

#define TEMP_ABOVE_0_OFF_TIME 500
#define TEMP_ABOVE_0_ON_TIME 1000
#define DOOR_OPEN_OFF_TIME_MS 700
#define DOOR_OPEN_ON_TIME_MS 300

int control_buzzer(const int buzzer_fd, const int buzzer_type);

int turn_off_buzzer();

#endif // BUZZER_HELPER_H_