#ifndef APP_LED_H
#define APP_LED_H

#include "esp_err.h"

esp_err_t led_set_level(uint32_t value);
esp_err_t led_toggle();

#endif // !APP_LED_H