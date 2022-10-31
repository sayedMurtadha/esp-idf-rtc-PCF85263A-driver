#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#include <stdio.h>
#endif // DEBUG
#include "encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"


#define QUEUE_LENGTH 10

#ifdef DEBUG
#define DEBUG_MSG(...) printf(__VA_ARGS__)
#endif // DEBUG


QueueHandle_t encoder_queue;
pcnt_unit_handle_t encoder_pcnt_unit = NULL;

static bool encoder_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    // send event data to queue, from this interrupt callback
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

void encoder_init(void){
    pcnt_unit_config_t unit_config = {
        .high_limit = ENCODER_PCNT_HIGH_LIMIT,
        .low_limit = ENCODER_PCNT_LOW_LIMIT,
    };
    
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &encoder_pcnt_unit));

    DEBUG_MSG("set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(encoder_pcnt_unit, &filter_config));

    DEBUG_MSG("install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = CONFIG_ROT_ENC_A_GPIO,
        .level_gpio_num = CONFIG_ROT_ENC_B_GPIO,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(encoder_pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = CONFIG_ROT_ENC_B_GPIO,
        .level_gpio_num = CONFIG_ROT_ENC_A_GPIO,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(encoder_pcnt_unit, &chan_b_config, &pcnt_chan_b));

    DEBUG_MSG("set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    DEBUG_MSG("add watch points and register callbacks");
    int watch_points[] = {ENCODER_PCNT_LOW_LIMIT, -50, 0, 50, ENCODER_PCNT_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(encoder_pcnt_unit, watch_points[i]));
    }
    pcnt_event_callbacks_t cbs = {
        .on_reach = encoder_on_reach,
    };
    encoder_queue = xQueueCreate(QUEUE_LENGTH, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(encoder_pcnt_unit, &cbs, encoder_queue));

    DEBUG_MSG("enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(encoder_pcnt_unit));
    DEBUG_MSG("clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(encoder_pcnt_unit));
    DEBUG_MSG("start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(encoder_pcnt_unit));

}

bool get_encoder_updated_value(int* buffer, uint32_t timeout_ms){
    return xQueueReceive(encoder_queue, &buffer, pdMS_TO_TICKS(timeout_ms)) != 0;

}

esp_err_t get_encoder_current_value(int* buffer){
    return pcnt_unit_get_count(encoder_pcnt_unit, buffer);
}