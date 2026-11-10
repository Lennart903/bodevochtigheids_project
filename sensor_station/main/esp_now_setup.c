#include "esp_now_setup.h"
#include <stdio.h>
#include "esp_now.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "nvs_flash.h"




#define ESP_NOW_CHANNEL 11

#define READ_INTERVAL_SEC 60

static const uint8_t HUB_MAC_ADDR[] = {0xB8, 0x1F, 0x3F, 0x0F, 0x81, 0xF1};
volatile bool hub_reply_received = false;
uint8_t led_state = 0;


static void data_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    if(data_len == 1)
    {
        led_state = data[0];

        hub_reply_received = true;
    }
}

static void data_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS) {
        printf("--- RADIO CB: SUCCESS (The Hub's antenna caught it!) ---\n");
    } else {
        printf("--- RADIO CB: FAIL (The Hub was asleep, wrong channel, or wrong MAC) ---\n");
    }
}

static void wifi_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); 
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK( esp_wifi_set_channel(ESP_NOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_channel(ESP_NOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));

    //ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
}




void esp_now_setup(void)
{
    wifi_init();


    ESP_ERROR_CHECK(esp_now_init());

    ESP_ERROR_CHECK(esp_now_register_send_cb(data_send_cb));     //register the callback function when a packet is send
    ESP_ERROR_CHECK(esp_now_register_recv_cb(data_recv_cb));     //register the callback function when a packet is received


    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    
    peer.channel = ESP_NOW_CHANNEL;
    memcpy(peer.peer_addr, HUB_MAC_ADDR, 6);
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

}

void send_data(const void *data, size_t size)
{
    esp_err_t err = esp_now_send(HUB_MAC_ADDR, (const uint8_t *)data, size);
    if (err != ESP_OK) {
        printf("--- ESP-NOW SEND ERROR: %s ---\n", esp_err_to_name(err));
    } else {
        printf("--- Packet injected into radio... waiting for Hub to ACK ---\n");
    }
}

void enter_deep_sleep(void)
{
    
    int64_t active_time_us = esp_timer_get_time();

    
    int64_t read_interval_us = (int64_t)READ_INTERVAL_SEC * 1000000ULL;

    
    int64_t adjusted_sleep_us = read_interval_us - active_time_us;

    // 4. Guard against negative sleep times if execution takes unusually long
    if (adjusted_sleep_us <= 0) {
        
        adjusted_sleep_us = 1000; // Fallback minimal sleep
    }

    

    
    esp_now_deinit();
    esp_wifi_stop();

    // 6. Set wake timer with adjusted duration
    esp_sleep_enable_timer_wakeup((uint64_t)adjusted_sleep_us);
    esp_deep_sleep_start();
}