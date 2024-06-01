#include "switch.h"
#include "sdkconfig.h"
#include "driver/gpio.h"


int switch_get_level(void){
    return gpio_get_level(CONFIG_GPIO_INPUT_SWITCH);
}