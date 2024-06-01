#include "push_button.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "app_rtc_task.h"

#define GPIO_INPUT_PIN_SEL  (1ULL<<CONFIG_GPIO_INPUT_PUSH_BUTTON)
#define ESP_INTR_FLAG_DEFAULT 0

#define DEBOUNCE_PRESET_MS 250

static QueueHandle_t gpio_evt_queue = NULL;
static uint32_t gpio_to_send = CONFIG_GPIO_INPUT_PUSH_BUTTON;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = *((uint32_t*)arg);
    if(gpio_num == CONFIG_GPIO_INPUT_PUSH_BUTTON){
        xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    }
}

static void button_pushed(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

            /* Debouncing Wait */
            vTaskDelay(DEBOUNCE_PRESET_MS / portTICK_PERIOD_MS);
            printf("Button is pushed\n");
            xQueueReset(gpio_evt_queue);
            xTaskNotifyGive(get_rtc_task_handle());
        }
    }
}

void push_button_init(void){
    /* Switch Setup */
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(button_pushed, "button_pushed", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(CONFIG_GPIO_INPUT_PUSH_BUTTON, gpio_isr_handler, (void*) &gpio_to_send);
}

