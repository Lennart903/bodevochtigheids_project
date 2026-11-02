#ifndef ESP_NOW_SETUP_H
#define ESP_NOW_SETUP_H

#include <stdint.h>
#include <stdbool.h>  
#include <stddef.h>   

extern volatile bool hub_reply_received;
extern uint8_t led_state;

void send_data(const void *data, size_t size);
void esp_now_setup(void);
void enter_deep_sleep(void);

#endif
