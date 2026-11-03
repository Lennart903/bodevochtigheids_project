#include "connection_setup.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"


#define WIFI_PASSWORD     "4T5SH8jr"
#define WIFI_SSID         "Orange-13304"
#define SENSOR_MAC_ADDR (uint8_t[]){0xB8, 0x1F, 0x3F, 0x0F, 0x98, 0x4C}
#define SUPABASE_URL "https://tniockgjahhdubpfjlaa.supabase.co/rest/v1/sensor_readings"
#define SUPABASE_KEY "sb_publishable__wl-rKNf318hez_4VhidgQ_RzDIQ4IF"


static const char *TAG = "HUB_NET";

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT  BIT0

static QueueHandle_t sensor_queue;

uint8_t led_state = 0;

typedef struct {
    uint8_t mac_addr[6];
    uint16_t adc_raw;
} sensor_packet_t;



static float convert_to_humidity(uint16_t raw_adc) 
{
    const float AIR_VALUE = 3500.0f;   // Calibrated ADC in dry air (0%)
    const float WATER_VALUE = 1200.0f; // Calibrated ADC in water (100%)
    
    float humidity = ((AIR_VALUE - (float)raw_adc) / (AIR_VALUE - WATER_VALUE)) * 100.0f;
    if (humidity > 100.0f) humidity = 100.0f;
    if (humidity < 0.0f)   humidity = 0.0f;
    
    return humidity;
}



static void http_upload_task(void *pvParameters)
{
    sensor_packet_t packet;

    while (1)
    {
        if(xQueueReceive(sensor_queue, &packet, portMAX_DELAY))
        {
            float humidity_pct = convert_to_humidity(packet.adc_raw);
            ESP_LOGI(TAG, "Received Raw ADC: %d -> Soil Moisture: %.1f%%", packet.adc_raw, humidity_pct);

            char post_data[64];
            snprintf(post_data, sizeof(post_data), "{\"humidity\": %.1f}", humidity_pct);

            esp_http_client_config_t http_cfg = {
                .url = SUPABASE_URL,
                .method = HTTP_METHOD_POST,
                .crt_bundle_attach = esp_crt_bundle_attach
            };

            esp_http_client_handle_t client = esp_http_client_init(&http_cfg);

            // Supabase API Headers
            esp_http_client_set_header(client, "Content-Type", "application/json");
            esp_http_client_set_header(client, "apikey", SUPABASE_KEY);
            esp_http_client_set_header(client, "Authorization", "Bearer " SUPABASE_KEY);
            esp_http_client_set_post_field(client, post_data, strlen(post_data));

            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK) {
                ESP_LOGI("HUB", "HTTPS POST Success! Status: %d", esp_http_client_get_status_code(client));
            } else {
                ESP_LOGE("HUB", "HTTPS POST Failed: %s", esp_err_to_name(err));
            }
            esp_http_client_cleanup(client);
        }
    }
}



static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("Wi-Fi started! Attempting to connect...\n");
        esp_wifi_connect(); 
    }
    // ADD THIS BLOCK: If it fails or drops, try again
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        printf("Wi-Fi disconnected! Reason code: %d\n", disconnected->reason);
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        printf("We got an IP address! We are online.\n");
    }
}



static void data_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len)
{
    if(data_len >= sizeof(uint16_t))
    {
        ESP_ERROR_CHECK(esp_now_send(esp_now_info->src_addr, &led_state, 1));

        sensor_packet_t packet;
        memcpy(packet.mac_addr, esp_now_info->src_addr, 6);
        memcpy(&packet.adc_raw, data, sizeof(uint16_t));
        xQueueSend(sensor_queue, &packet, 0);
    }
}

static void data_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{

}



void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL)); 


    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    wifi_config_t wifi_data = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            // Minimum security mode required (WPA2 is standard)
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .pmf_cfg = {
                .capable = true,
                .required = false
            }
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_data));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

void esp_now_setup(void)
{
    sensor_queue = xQueueCreate(10, sizeof(sensor_packet_t));
    xTaskCreate(http_upload_task, "http_upload", 4096, NULL, 5, NULL);

    ESP_ERROR_CHECK(esp_now_init());

    ESP_ERROR_CHECK(esp_now_register_send_cb(data_send_cb));     //register the callback function when a packet is send
    ESP_ERROR_CHECK(esp_now_register_recv_cb(data_recv_cb));     //register the callback function when a packet is received

    uint8_t wifi_channel;
    wifi_second_chan_t second_channel;
    esp_wifi_get_channel(&wifi_channel, &second_channel);
    
    printf("--- DEBUG: Tell your Solar Node to use Channel %d ---\n", wifi_channel);
}



