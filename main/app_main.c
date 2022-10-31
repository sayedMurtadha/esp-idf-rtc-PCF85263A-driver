/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "io_init.h"
#include "app_rtc.h"

void app_main(void)
{
    io_init();
    app_rtc_init();
    
}
