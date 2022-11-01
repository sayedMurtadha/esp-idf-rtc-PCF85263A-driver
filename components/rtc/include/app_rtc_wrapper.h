#ifndef APP_RTC_WRAPPER_H
#define APP_RTC_WRAPPER_H

#include "pcf85263.h"

void rtc_time_add_seconds(pcf85263_datetime_t* datetime, uint8_t seconds);
void rtc_time_add_minutes(pcf85263_datetime_t* datetime, uint8_t minutes);
void rtc_time_add_hours(pcf85263_datetime_t* datetime, uint8_t hours);

void rtc_date_add_days(pcf85263_datetime_t* datetime, uint8_t days);
void rtc_date_add_months(pcf85263_datetime_t* datetime, uint8_t months);
void rtc_date_add_years(pcf85263_datetime_t* datetime, uint8_t years);

void rtc_register_read(uint8_t reg_addr, uint8_t *data, size_t len);
void rtc_register_write(uint8_t reg_addr, const uint8_t *data, size_t len);

void app_rtc_wrapper_init(void);


#endif // !APP_RTC_WRAPPER_H