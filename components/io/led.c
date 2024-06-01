#include "led.h"
#include "sdkconfig.h"
#include "driver/gpio.h"


esp_err_t led_set_level(uint32_t value){
    return gpio_set_level(CONFIG_GPIO_OUTPUT_LED, value);
}

esp_err_t led_toggle(){
    return gpio_set_level(CONFIG_GPIO_OUTPUT_LED, !gpio_get_level(CONFIG_GPIO_OUTPUT_LED));
}