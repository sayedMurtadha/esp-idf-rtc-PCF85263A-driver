idf_component_register(SRCS "switch.c" "led.c" "io_init.c" "push_button.c"
    PRIV_REQUIRES rtc driver freertos encoder INCLUDE_DIRS include)