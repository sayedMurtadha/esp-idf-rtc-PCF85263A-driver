/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#include "driver/gpio.h"


#include "io_init.h"
#include "encoder.h"




void app_main(void)
{
    io_init();

    // Report counter value
    int pulse_count = 0;
    int event_count = 0;
    for (int i = 0;; i = !i) {

        if (get_encoder_updated_value(&event_count, 1000)) {
            printf("Watch point event, count: %d ", event_count);
        } else {
            ESP_ERROR_CHECK(get_encoder_current_value(&pulse_count));
            printf("Pulse count: %d ", pulse_count);
        }


        printf("Switch val: %d\n", gpio_get_level(CONFIG_GPIO_INPUT_SWITCH));

        printf("Blinking...\n");
        gpio_set_level(CONFIG_GPIO_OUTPUT_LED, i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    
    
}
