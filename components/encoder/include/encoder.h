#ifndef APP_ENCODER_H
#define APP_ENCODER_H

#include <stdbool.h>
#include "esp_err.h"

#define ENCODER_PCNT_HIGH_LIMIT 100
#define ENCODER_PCNT_LOW_LIMIT  -100

void encoder_init(void);

bool get_encoder_updated_value(int* buffer, uint32_t timeout_ms);
esp_err_t get_encoder_current_value(int* buffer);

#endif // !APP_ENCODER_H