#ifndef APP_RTC_H
#define APP_RTC_H

#define UPDATE_FREQUENCY_MS 1000

typedef enum{
    RUNNING_MODE_MONITOR = 0,
    RUNNING_MODE_EDIT
}rtc_running_mode_t;

void app_rtc_init(void);

#endif // !APP_RTC_H

