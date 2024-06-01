#include "pcf85263.h"


#define PCF85263_TIME_REG_START_ADDR        (0x00) /* Register addresses of the RTC - time */
#define PCF85263_TIME_REG_ADDR_LENGTH       (0x04) /* time length */
#define PCF85263_DATE_REG_START_ADDR        (PCF85263_TIME_REG_START_ADDR + PCF85263_TIME_REG_ADDR_LENGTH) /* date length */
#define PCF85263_DATE_REG_ADDR_LENGTH       (0x04) /* Register addresses of the RTC - date */
#define PCF85263_DATETIME_REG_ADDR_LENGTH   (PCF85263_TIME_REG_ADDR_LENGTH + PCF85263_DATE_REG_ADDR_LENGTH) /* datetime length */
#define PCF85263_FUNCTINS_REG_ADDR          (0x25) /* Register addresses of the RTC - controls */
#define PCF85263_CHIP_CTRL_REG_ADDR         (0x28) /* Register addresses of the RTC - Chip Functions Control */
#define PCF85263_RTCM_BITMASK               (0xEF) /* RTCM(ODE) bitmask (bit: 0x04) */
#define PCF85263_STOP_REG_ADDR              (0x2E) /* Register addresses of the RTC - Stop */

i2c_reader_t reader_invoker;
i2c_writer_t writer_invoker;


static inline uint8_t dec2bcd(uint8_t n)
{
    return (n / 10 * 16 +  n % 10);  
}

static inline uint8_t bcd2dec(uint8_t n)
{
    return ((((n & 0xF0)>>0x04) * 10) + (n & 0x0F));  
}

static void unpack_date(pcf85263_date_t* date){
    date->day = bcd2dec(date->day);
    date->month = bcd2dec(date->month);
    date->year = bcd2dec(date->year);
    
}

static pcf85263_date_t pack_date(pcf85263_date_t date){
    pcf85263_date_t packed;
    packed.day = dec2bcd(date.day);
    packed.month = dec2bcd(date.month);
    packed.year = dec2bcd(date.year);
    return packed;
}



static void unpack_time(pcf85263_time_t* time){
    time->ms = bcd2dec(time->ms);
    time->second = bcd2dec(time->second & 0x7F); /* TODO: check os value */
    time->minute = bcd2dec(time->minute & 0x7F); /* TODO: check emon value */
    time->hour = bcd2dec(time->hour); /* TODO: check 12/24hrs settings */

}

static pcf85263_time_t pack_time(pcf85263_time_t time){
    pcf85263_time_t packed;
    packed.ms = dec2bcd(time.ms);
    packed.second = dec2bcd(time.second);
    packed.minute = dec2bcd(time.minute);
    packed.hour = dec2bcd(time.hour);
    return packed;
}


static void unpack_datetime(pcf85263_datetime_t* datetime){
    unpack_date(&(datetime->date));
    unpack_time(&(datetime->time));
}

static pcf85263_datetime_t pack_datetime(pcf85263_datetime_t datetime){
    pcf85263_datetime_t packed;
    packed.date = pack_date(datetime.date);
    packed.time = pack_time(datetime.time);
    return packed;
}


static pcf85263_err_t reader_writer_check(void){
    pcf85263_err_t err = PCF85263_ERR_NONE;
    if(reader_invoker == NULL || writer_invoker == NULL){
        err = PCF85263_ERR_READER_WRITER_FUNCS;
    }

    return err;
}

static pcf85263_err_t set_register(uint8_t address, uint8_t value){
    pcf85263_err_t err = reader_writer_check();
    if(err == PCF85263_ERR_NONE){
        writer_invoker(address, &value, sizeof(uint8_t));
    }

    return err;
}

static pcf85263_err_t get_register(uint8_t address, uint8_t* value){
    pcf85263_err_t err = reader_writer_check();
    if(err == PCF85263_ERR_NONE){
        reader_invoker(address, value, sizeof(value));
    }

    return err;
}

static pcf85263_err_t set_data(uint8_t start_address, uint8_t* buffer, size_t buffer_length){
    pcf85263_err_t err = reader_writer_check();
    if(err == PCF85263_ERR_NONE){
        writer_invoker(start_address, buffer, buffer_length);
    }

    return err;
}

static pcf85263_err_t get_data(uint8_t start_address, uint8_t* buffer, size_t buffer_length){
    pcf85263_err_t err = reader_writer_check();
    if(err == PCF85263_ERR_NONE){
        reader_invoker(start_address, buffer, buffer_length);
    }

    return err;
}

pcf85263_err_t set_pcf85263_mode_on(void){
    return set_register(PCF85263_STOP_REG_ADDR, 0x00);

}
pcf85263_err_t set_pcf85263_mode_off(void){
    return set_register(PCF85263_STOP_REG_ADDR, 0x01);
}

pcf85263_err_t get_pcf85263_time(pcf85263_time_t* time){
    pcf85263_err_t err = get_data(PCF85263_TIME_REG_START_ADDR, (uint8_t*)time, PCF85263_TIME_REG_ADDR_LENGTH);
    if(err == PCF85263_ERR_NONE){
        unpack_time(time);
    }
    
    return err;
}

pcf85263_err_t set_pcf85263_time(pcf85263_time_t* time){
    pcf85263_time_t packed_time = pack_time(*time);
    return set_data(PCF85263_TIME_REG_START_ADDR, (uint8_t*)&packed_time, PCF85263_TIME_REG_ADDR_LENGTH);
}

pcf85263_err_t get_pcf85263_date(pcf85263_date_t* date){
    pcf85263_err_t err = get_data(PCF85263_DATE_REG_START_ADDR, (uint8_t*)date, PCF85263_DATE_REG_ADDR_LENGTH);
    if(err == PCF85263_ERR_NONE){
        unpack_date(date);
    }
    
    return err;
}

pcf85263_err_t set_pcf85263_date(pcf85263_date_t* date){
    pcf85263_date_t packed_date = pack_date(*date);
    return set_data(PCF85263_DATE_REG_START_ADDR, (uint8_t*)&packed_date, PCF85263_DATE_REG_ADDR_LENGTH);
}


pcf85263_err_t get_pcf85263_datetime(pcf85263_datetime_t* dt){
    pcf85263_err_t err = get_data(PCF85263_TIME_REG_START_ADDR, (uint8_t*)dt, PCF85263_DATETIME_REG_ADDR_LENGTH);
    if(err == PCF85263_ERR_NONE){
        unpack_datetime(dt);
    }

    return err;
}

pcf85263_err_t set_pcf85263_datetime(pcf85263_datetime_t* dt){
    pcf85263_datetime_t packed_datetime = pack_datetime(*dt);
    return set_data(PCF85263_TIME_REG_START_ADDR, (uint8_t*)&packed_datetime, PCF85263_DATETIME_REG_ADDR_LENGTH);
}


pcf85263_err_t pcf85263_init(i2c_reader_t reader, i2c_writer_t writer){
    pcf85263_err_t err = PCF85263_ERR_READER_WRITER_FUNCS;
    uint8_t reg_value = 0x00;
    if(reader != NULL && writer != NULL){
        reader_invoker = reader;
        writer_invoker = writer;


        err = get_register(PCF85263_CHIP_CTRL_REG_ADDR, &reg_value);
    }

    if(err == PCF85263_ERR_NONE){
        reg_value &= PCF85263_RTCM_BITMASK;
        err = set_register(PCF85263_CHIP_CTRL_REG_ADDR, reg_value);
    }

    if(err == PCF85263_ERR_NONE){
        err = set_pcf85263_mode_on();
    }

    return err;
}
