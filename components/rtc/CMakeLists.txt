idf_component_register(SRCS "rtc.c" "rtc_driver/pcf85263.c"
    PRIV_REQUIRES driver freertos encoder INCLUDE_DIRS include PRIV_INCLUDE_DIRS rtc_driver/include)