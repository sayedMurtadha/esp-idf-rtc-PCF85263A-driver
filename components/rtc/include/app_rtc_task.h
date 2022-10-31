#ifndef APP_RTC_TASK_H
#define APP_RTC_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t get_rtc_task_handle(void);

#endif // !APP_RTC_TASK_H