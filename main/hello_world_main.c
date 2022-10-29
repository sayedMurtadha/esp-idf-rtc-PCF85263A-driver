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

#include "driver/i2c.h"

#include "io_init.h"
#include "encoder.h"



#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define RTC_WRITE_ADDR                 (0xA2>>1)        /*!< Slave address of the RTC - write */
#define RTC_READ_ADDR                  0xA3        /*!< Slave address of the RTC - read */

#define RTC_TIME_REG_ADDR              0x00        /*!< Register addresses of the RTC - time */
#define RTC_FUNCTINS_REG_ADDR          0x25        /*!< Register addresses of the RTC - controls */
#define RTC_RTCM_REG_ADDR              0x26        /*!< Register addresses of the RTC - RTCM(ODE) */
#define RTC_STOP_REG_ADDR              0x2E        /*!< Register addresses of the RTC - Stop */

typedef struct 
{
    uint8_t ms;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;

}datetime_t;



/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t rtc_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_master_write_to_device(I2C_MASTER_NUM, RTC_WRITE_ADDR, &reg_addr, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    return i2c_master_read_from_device(I2C_MASTER_NUM, RTC_WRITE_ADDR, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t rtc_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, RTC_WRITE_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
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

void app_main(void)
{
    io_init();

    i2c_master_init();
    printf("I2C initialized successfully\n");

    /* test datetime */
    rtc_register_write_byte(RTC_STOP_REG_ADDR, 0x01);
    rtc_register_write_byte(0, 0x34);
    rtc_register_write_byte(1, 0x12);
    rtc_register_write_byte(2, 0x21);
    rtc_register_write_byte(3, 0x20);
    rtc_register_write_byte(4, 0x05);
    rtc_register_write_byte(5, 0x03);
    rtc_register_write_byte(6, 0x12);
    rtc_register_write_byte(7, 0x30);

    /* SET RTCM to 0 */
    rtc_register_write_byte(RTC_RTCM_REG_ADDR, 0x00);
    rtc_register_write_byte(RTC_STOP_REG_ADDR, 0x00);


    for (uint8_t i = 0x25, data; i < 0x30; i++)
    {
        rtc_register_read(i, &data, 1);
        printf("register %d: %d\n", i, data);
    }
    

    printf("Hello world!\n");

    datetime_t dt;
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

        /* Read the datetime */
        ESP_ERROR_CHECK(rtc_register_read(RTC_TIME_REG_ADDR, (uint8_t*)&dt, sizeof(datetime_t)));
        
        printf("Time: %d%d:%d%d:%d%d,  Date: %d%d/%d%d/20%d%d",
                ((dt.hour & 0xF0)>>0x04), /* to be fixed by setting 24hrs first */
                (dt.hour & 0x0F),
                ((dt.minute & 0x70)>>0x04), /* to check emon vale */
                (dt.minute & 0x0F),
                ((dt.second & 0x70)>>0x04), /* to check os vale */
                (dt.second & 0x0F),
                ((dt.day & 0x30)>>0x04),
                (dt.day & 0x0F),
                ((dt.month & 0x10)>>0x04),
                (dt.month & 0x0F),
                ((dt.year & 0xF0)>>0x04),
                (dt.year & 0x0F)
        );

        printf("Blinking...\n");
        gpio_set_level(CONFIG_GPIO_OUTPUT_LED, i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    
    
}
