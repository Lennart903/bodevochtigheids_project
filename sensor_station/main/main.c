#include "esp_now_setup.h"
#include "adc_setup.h"
#include "ledc_setup.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

#define REPLY_WAITING_TIME 500



void app_main(void) 
{
    adc_oneshot_unit_handle_t adc_handle;
    int adc_sensor_data = 0;

    esp_now_setup();

    adc_init(&adc_handle);


    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &adc_sensor_data));

    uint16_t data_16bit = (uint16_t)adc_sensor_data;

    hub_reply_received = false;
    send_data(&data_16bit, sizeof(data_16bit));

    uint16_t waiting_time = REPLY_WAITING_TIME;

    while (!hub_reply_received && waiting_time > 0) 
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        waiting_time -= 10;
    }

    if (hub_reply_received && led_state == 1) 
    {
        led_init();
        blink_led(); 
    }


    enter_deep_sleep();
}

