#include "app_rtc_wrapper.h"

#include "driver/i2c.h"


#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /* GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /* GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /* I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000                     /* I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /* I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000                       /* I2C master read/write timeout in mSec */
#define I2C_MASTER_TIMEOUT_TICKS    (I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS)


#define ADD_MOD(num1,num2,mod) (num1 = (num1+num2)%mod)
#define ADD_MOD_START_ONE(num1,num2,mod) \
do { \
    ADD_MOD(num1,num2,mod); \
    if(num1 == 0){ \
        num1 = (num2 < 0) ? mod - 1 : 1; \
    } \
} while(0)


void rtc_time_add_seconds(pcf85263_datetime_t* datetime, uint8_t seconds){
    ADD_MOD(datetime->time.second, seconds, 60);
}

void rtc_time_add_minutes(pcf85263_datetime_t* datetime, uint8_t minutes){
    ADD_MOD(datetime->time.minute, minutes, 60);
}

void rtc_time_add_hours(pcf85263_datetime_t* datetime, uint8_t hours){
    ADD_MOD(datetime->time.hour, hours, 24);
}


void rtc_date_add_days(pcf85263_datetime_t* datetime, uint8_t days){
    ADD_MOD_START_ONE(datetime->date.day, days, 32);

}

void rtc_date_add_months(pcf85263_datetime_t* datetime, uint8_t months){
    ADD_MOD_START_ONE(datetime->date.month, months, 13);

}

void rtc_date_add_years(pcf85263_datetime_t* datetime, uint8_t years){
    ADD_MOD_START_ONE(datetime->date.year, years, 32);

}


/**
 * @brief Read a sequence of bytes from the registers
 */
void rtc_register_read(uint8_t reg_addr, uint8_t *data, size_t len){
    i2c_master_write_to_device(I2C_MASTER_NUM, get_pcf85263_device_address(), &reg_addr, 1, I2C_MASTER_TIMEOUT_TICKS);
    i2c_master_read_from_device(I2C_MASTER_NUM, get_pcf85263_device_address(), data, len, I2C_MASTER_TIMEOUT_TICKS);
}

/**
 * @brief Write a sequence of bytes to the registers
 */
void rtc_register_write(uint8_t reg_addr, const uint8_t *data, size_t len){
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

void app_rtc_wrapper_init(void){
    ESP_ERROR_CHECK(i2c_master_init());
    pcf85263_init(rtc_register_read, rtc_register_write);
}