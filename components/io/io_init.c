#include "io_init.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "encoder.h"
#include "push_button.h"

#define LED_PIN_SEL  (1ULL<<CONFIG_GPIO_OUTPUT_LED)
#define SWITCH_PIN_SEL  (1ULL<<CONFIG_GPIO_INPUT_SWITCH)


static void gpio_init(uint64_t bit_mask, gpio_mode_t mode, gpio_pulldown_t pulldown, gpio_pullup_t pullup, gpio_int_type_t intr){
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    io_conf.intr_type = intr;
    io_conf.mode = mode;
    io_conf.pin_bit_mask = bit_mask;
    io_conf.pull_down_en = pulldown;
    io_conf.pull_up_en = pullup;

    //configure GPIO with the given settings
    gpio_config(&io_conf);
}


void io_init(void)
{
    /* Push Button Setup */
    push_button_init();
    
    /* LED Setup */
    gpio_init(LED_PIN_SEL, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);

    /* Switch Setup */
    gpio_init(SWITCH_PIN_SEL, GPIO_MODE_INPUT, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE, GPIO_INTR_DISABLE);

    /* Encoder Setup */
    encoder_init();
    
}
