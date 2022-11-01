#include <stdio.h>
#include "app_rtc.h"
#include "app_rtc_task.h"
#include "sdkconfig.h"
#include "esp_err.h"

#include "app_rtc_wrapper.h"

#include "console_interface.h"
#include "switch.h"
#include "led.h"
#include "encoder.h"


static void rtc_mode_monitor(void);
static void (*running_mode_function)(void) = &rtc_mode_monitor;

void app_rtc_task(void *pvParameter);

typedef void (*rtc_add_function_t)(pcf85263_datetime_t*, uint8_t);

#define ADD_FUNCTIONS_COUNT 6
rtc_add_function_t add_functions[ADD_FUNCTIONS_COUNT] = {
    rtc_time_add_hours,
    rtc_time_add_minutes,
    rtc_time_add_seconds,
    rtc_date_add_days,
    rtc_date_add_months,
    rtc_date_add_years
};



static bool rtc_mode_edit_init = true;
static bool rtc_mode_edit_save = false;

static void rtc_mode_edit(void){
    
    static pcf85263_datetime_t dt;
    static console_update_location_t location = UPDATE_LOCATION_HOURS;
    static uint32_t led_count = 0;
    static int event_count, pulse_count;
    if(rtc_mode_edit_init){
        ESP_ERROR_CHECK(set_pcf85263_mode_off());
        ESP_ERROR_CHECK(get_pcf85263_datetime(&dt));
        for (size_t i = 0; i < ADD_FUNCTIONS_COUNT; i++)
        {
            /* fix any wrong datetime */
            add_functions[i](&dt, 0);
        }
        
        location = UPDATE_LOCATION_HOURS;
        update_datetime(&dt, location);
        rtc_mode_edit_init = false;
    }

    /* wait for queue data... */
    if(ulTaskNotifyTake(pdTRUE, 0)){
        set_update_location_next(&location);
        update_datetime(&dt, location);
    }

    if (get_encoder_updated_value(&event_count, 100)) {
        printf("Encoder update: %d ", (event_count - pulse_count));
        add_functions[location](&dt, (event_count - pulse_count)); /* TODO: to fix pecision */
    } else {
        ESP_ERROR_CHECK(get_encoder_current_value(&pulse_count));
    }




    if(rtc_mode_edit_save){
        ESP_ERROR_CHECK(set_pcf85263_datetime(&dt));
        ESP_ERROR_CHECK(set_pcf85263_mode_on());
        rtc_mode_edit_save = false;
        rtc_mode_edit_init = true;
        running_mode_function = &rtc_mode_monitor;
    }

    led_set_level(led_count++);
    led_count %= 2;
    vTaskDelay(UPDATE_FREQUENCY_MS / portTICK_PERIOD_MS);
}

static void rtc_mode_monitor(void){
    /* Read the datetime */
    pcf85263_datetime_t dt;
    ESP_ERROR_CHECK(get_pcf85263_datetime(&dt));
    print_datetime(&dt);
    
    led_set_level(1);
    
    vTaskDelay(UPDATE_FREQUENCY_MS / portTICK_PERIOD_MS);
}

static void check_and_update_current_mode(void){
    if(switch_get_level() == 1){
        /* edit mode */
        running_mode_function = &rtc_mode_edit;
    } else {
        /* monitor mode */
        if(running_mode_function != &rtc_mode_monitor){
            rtc_mode_edit_save = true;
        } else {
            running_mode_function = &rtc_mode_monitor;
        }
    }
}

static TaskHandle_t rtc_task_handle = NULL;

TaskHandle_t get_rtc_task_handle(void){
    return rtc_task_handle;
}

void app_rtc_init(void){
    app_rtc_wrapper_init();

    xTaskCreate(app_rtc_task, "app_rtc_task", 2048, NULL, 1, &rtc_task_handle);
}


void app_rtc_task(void *pvParameter){

    while(1){
        check_and_update_current_mode();
        running_mode_function();
    }

}