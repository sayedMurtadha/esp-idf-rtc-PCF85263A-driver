#ifndef APP_CONSOLE_INTERFACE_H
#define APP_CONSOLE_INTERFACE_H

#include "pcf85263.h"

typedef enum {
    UPDATE_LOCATION_HOURS = 0,
    UPDATE_LOCATION_MINUTES,
    UPDATE_LOCATION_SECONDS,
    UPDATE_LOCATION_DAYS,
    UPDATE_LOCATION_MONTHS,
    UPDATE_LOCATION_YEARS,
    UPDATE_LOCATION_ELEMENTS_COUNT

} console_update_location_t;

void print_datetime(pcf85263_datetime_t* dt);
void update_datetime(pcf85263_datetime_t* dt, console_update_location_t location);
void set_update_location_next(console_update_location_t* current);

#endif // !APP_CONSOLE_INTERFACE_H