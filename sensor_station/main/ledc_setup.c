#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BLINK_GPIO 8

void led_init(void)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 1); // Start OFF (Active-LOW = 3.3V)
}

void blink_led(void)
{
    gpio_set_level(BLINK_GPIO, 0); // Turn ON (0V)
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(BLINK_GPIO, 1); // Turn OFF (3.3V)
}