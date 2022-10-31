#include <stdio.h>
#include "app_rtc.h"
#include "app_rtc_task.h"
#include "sdkconfig.h"
#include "esp_err.h"

#include "driver/i2c.h"

#include "pcf85263.h"
#include "console_interface.h"
#include "switch.h"
#include "led.h"
#include "encoder.h"


#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /* GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /* GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /* I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000                     /* I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000                       /* I2C master read/write timeout in mSec */
#define I2C_MASTER_TIMEOUT_TICKS    (I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS)


static void rtc_mode_monitor(void);
static void (*running_mode_function)(void) = &rtc_mode_monitor;

void app_rtc_task(void *pvParameter);

/**
 * @brief Read a sequence of bytes from the registers
 */
static void rtc_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_master_write_to_device(I2C_MASTER_NUM, get_pcf85263_device_address(), &reg_addr, 1, I2C_MASTER_TIMEOUT_TICKS);
    i2c_master_read_from_device(I2C_MASTER_NUM, get_pcf85263_device_address(), data, len, I2C_MASTER_TIMEOUT_TICKS);
}

/**
 * @brief Write a sequence of bytes to the registers
 */
static void rtc_register_write(uint8_t reg_addr, const uint8_t *data, size_t len)
{
    i2c_master_write_to_device(I2C_MASTER_NUM, get_pcf85263_device_address(), data, len, I2C_MASTER_TIMEOUT_TICKS);

}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

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
        printf("Watch point event, count: %d ", event_count);
    } else {
        ESP_ERROR_CHECK(get_encoder_current_value(&pulse_count));
        printf("Pulse count: %d\n", pulse_count);
    }

    if(rtc_mode_edit_save){
        ESP_ERROR_CHECK(set_pcf85263_datetime(&dt));
        ESP_ERROR_CHECK(set_pcf85263_mode_on());
        rtc_mode_edit_save = false;
        rtc_mode_edit_init = true;
        running_mode_function = &rtc_mode_monitor;
    }


    printf("Blinking...\n");
    // led_toggle();
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


void set_rtc_running_mode(rtc_running_mode_t mode){
/* TODO: remove
    if(mode == RUNNING_MODE_MONITOR){
        running_mode_function = &rtc_mode_monitor;
    } else if(mode == RUNNING_MODE_EDIT){
        running_mode_function = &rtc_mode_edit;
    }
*/
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
    ESP_ERROR_CHECK(i2c_master_init());
    pcf85263_init(rtc_register_read, rtc_register_write);


    xTaskCreate(app_rtc_task, "app_rtc_task", 2048, NULL, 1, &rtc_task_handle);
}


void app_rtc_task(void *pvParameter){

    while(1){
        check_and_update_current_mode();
        running_mode_function();
    }

}