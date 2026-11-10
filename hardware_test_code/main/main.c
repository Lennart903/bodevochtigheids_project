#include <stdio.h>
#include "driver/ledc.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#define LEDC_MODE         LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL      LEDC_CHANNEL_0
#define LEDC_DUTY_RES     LEDC_TIMER_12_BIT            // 10-bit resolution (0 to 1023)
#define LEDC_MAX_DUTY     ((1 << LEDC_DUTY_RES) - 1)  // 1023
#define WIFI_PASSWORD     "4T5SH8jr"
#define WIFI_SSID         "Orange-13304"



void adc_setup(adc_oneshot_unit_handle_t * adc_handle);
void ledc_init(void);
void led_turn_on(void);
void led_turn_off(void);
void wifi_setup(void);


void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("Wi-Fi started! Attempting to connect...\n");
        esp_wifi_connect(); 
        led_turn_on();
    }
    // ADD THIS BLOCK: If it fails or drops, try again
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        printf("Wi-Fi disconnected! Reason code: %d\n", disconnected->reason);
        esp_wifi_connect();
        led_turn_on(); // Turn LED back on to show it's trying to connect
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        printf("We got an IP address! We are online.\n");
        led_turn_off();
    }
}



void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle;

    
    
    
    adc_setup(&adc1_handle);
    ledc_init();

    
    wifi_setup();
    



    led_turn_off();
    
    for(;;)
    {
        int raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &raw));

        // Blink LED to signal a reading
        
        
        

        printf("de waarde van adc is: %d\n", raw);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void wifi_setup(void)
{
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

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL); 


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
}


void adc_setup(adc_oneshot_unit_handle_t * adc_handle)
{
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .clk_src  = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
        .unit_id  = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(*adc_handle, ADC_CHANNEL_3, &chan_cfg));
}

void ledc_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 400,
        .clk_cfg          = LEDC_AUTO_CLK,            // Auto-select stable clock
        .duty_resolution  = LEDC_DUTY_RES,            // Matched to 10-bit
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t channel_cfg = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .gpio_num       = 8,                          // Onboard LED
        .duty           = LEDC_MAX_DUTY,              // Start OFF (3.3V for active-LOW)
        .hpoint         = 0,
    };
    ledc_channel_config(&channel_cfg);
}

// Active-LOW helper functions:
void led_turn_on(void) 
{
    // 0% duty = Pin LOW (0V) -> LED turns ON
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);      
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void led_turn_off(void) 
{
    // 100% duty = Pin HIGH (3.3V) -> LED turns OFF
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_MAX_DUTY); 
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}