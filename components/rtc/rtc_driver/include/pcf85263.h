#ifndef PCF85263_H
#define PCF85263_H

#include <stdint.h>
#include <stddef.h>

#define PCF85263_ADDRESS    (0x51)
#define I2C_MAX_FREQ_HZ     400000

/* TODO: add return error code */
typedef void (*i2c_reader_t)(uint8_t read_address, uint8_t * read_buffer, size_t read_size);
typedef void (*i2c_writer_t)(uint8_t write_address, const uint8_t * write_buffer, size_t write_size);

typedef enum 
{
    PCF85263_ERR_NONE = 0,
    PCF85263_ERR_NOT_INITIALIZED,
    PCF85263_ERR_READER_WRITER_FUNCS,
    /* ... More error codes in future ... */

    PCF85263_ERR_OTHERS = 255
    
}pcf85263_err_t;

typedef struct 
{
    uint8_t ms;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    
}pcf85263_time_t;

typedef struct 
{
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;

}pcf85263_date_t;


typedef struct 
{
    pcf85263_time_t time;
    pcf85263_date_t date;

}pcf85263_datetime_t;

pcf85263_err_t pcf85263_init(i2c_reader_t reader, i2c_writer_t writer);

pcf85263_err_t set_pcf85263_mode_on(void);
pcf85263_err_t set_pcf85263_mode_off(void);

pcf85263_err_t get_pcf85263_time(pcf85263_time_t* time);
pcf85263_err_t set_pcf85263_time(pcf85263_time_t* time);

pcf85263_err_t get_pcf85263_date(pcf85263_date_t* date);
pcf85263_err_t set_pcf85263_date(pcf85263_date_t* date);

pcf85263_err_t get_pcf85263_datetime(pcf85263_datetime_t* dt);
pcf85263_err_t set_pcf85263_datetime(pcf85263_datetime_t* dt);

static inline uint8_t get_pcf85263_device_address(void){
    return PCF85263_ADDRESS;
}

#endif // !PCF85263_H