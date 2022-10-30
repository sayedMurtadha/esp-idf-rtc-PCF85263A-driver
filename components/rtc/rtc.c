#include <stdio.h>
#include "rtc.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

#include "driver/i2c.h"

#include "pcf85263.h"


#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /* GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /* GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /* I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000                     /* I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000                       /* I2C master read/write timeout in mSec */
#define I2C_MASTER_TIMEOUT_TICKS    (I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS)

void rtc_task(void *pvParameter);

/**
 * @brief Read a sequence of bytes from the registers
 */
static esp_err_t rtc_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_master_write_to_device(I2C_MASTER_NUM, get_pcf85263_device_address(), &reg_addr, 1, I2C_MASTER_TIMEOUT_TICKS);
    return i2c_master_read_from_device(I2C_MASTER_NUM, get_pcf85263_device_address(), data, len, I2C_MASTER_TIMEOUT_TICKS);
}

/**
 * @brief Write a sequence of bytes to the registers
 */
static esp_err_t rtc_register_write(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_to_device(I2C_MASTER_NUM, get_pcf85263_device_address(), data, len, I2C_MASTER_TIMEOUT_TICKS);

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


static void rtc_mode_edit(void){
    printf("rtc_mode_edit: NOT IMPLEMENTED");
    ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
}

static void rtc_mode_monitor(void){
    /* Read the datetime */
    static pcf85263_datetime_t dt;
    ESP_ERROR_CHECK(get_pcf85263_datetime((uint8_t*)&dt));
    
    printf("Time: %d:%d:%d,  Date: %d/%d/20%d\n",
            dt.time.hour,
            dt.time.minute,
            dt.time.second,
            dt.date.day,
            dt.date.month,
            dt.date.year
    );
}

static void (*running_mode_function)(void) = &rtc_mode_monitor;


void set_rtc_running_mode(rtc_running_mode_t mode){
    if(mode == RUNNING_MODE_MONITOR){
        running_mode_function = &rtc_mode_monitor;
    } else if(mode == RUNNING_MODE_EDIT){
        running_mode_function = &rtc_mode_edit;
    }

}


void rtc_init(void){
    ESP_ERROR_CHECK(i2c_master_init());
    pcf85263_init(&rtc_register_read, &rtc_register_write);


    xTaskCreate(&rtc_task, "rtc_task", 2048, NULL, 1, NULL);
    
}


void rtc_task(void *pvParameter){

    while(1){
        running_mode_function();
        vTaskDelay(UPDATE_FREQUENCY_MS / portTICK_PERIOD_MS);
    }

}