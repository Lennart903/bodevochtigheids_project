#ifndef ESP_NOW_SETUP_H
#define ESP_NOW_SETUP_H

uint32_t esp_now_setup(void);
static void data_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len);
static void data_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status);
tatic void wifi_init();

#endif
