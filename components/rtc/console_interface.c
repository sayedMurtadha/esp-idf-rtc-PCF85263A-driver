#include <stdio.h>
#include "console_interface.h"

#include <stdbool.h>


void print_datetime(pcf85263_datetime_t* dt){
    printf("Time: %02d:%02d:%02d,  Date: %02d/%02d/20%02d\n",
            dt->time.hour,
            dt->time.minute,
            dt->time.second,
            dt->date.day,
            dt->date.month,
            dt->date.year
    );
}

static void update_single_part(uint8_t data, bool edit){
    if(edit){
        printf("<");
    }
    printf("%02d", data);
    if(edit){
        printf(">");
    }
}

void set_update_location_next(console_update_location_t* current){
    ++(*current);
    *current %= UPDATE_LOCATION_ELEMENTS_COUNT;
}

void update_datetime(pcf85263_datetime_t* dt, console_update_location_t location){
    console_update_location_t current = UPDATE_LOCATION_HOURS;

    // "EDIT MODE,  Time: 08:<51>:25, Date: 25/10/2022"
    printf("EDIT MODE,  Time: ");
    update_single_part(dt->time.hour, (current++ == location));
    printf(":");
    update_single_part(dt->time.minute, (current++ == location));
    printf(":");
    update_single_part(dt->time.second, (current++ == location));
    
    printf(", Date: ");
    update_single_part(dt->date.day, (current++ == location));
    printf("/");
    update_single_part(dt->date.month, (current++ == location));
    printf("/20");
    update_single_part(dt->date.year, (current++ == location));
    printf("\n");
}
