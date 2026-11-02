#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_now_setup.h"
#include "esp_now.h"

#define ESP_NOW_CHANNEL 1
#define HUB_MAC_ADDR (uint8_t[]){0xB8, 0x1F, 0x3F, 0x0F, 0x81, 0xF0}



static void data_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{

}

static void data_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{

}

static void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); 
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK( esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
}




uint32_t esp_now_setup(void)
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