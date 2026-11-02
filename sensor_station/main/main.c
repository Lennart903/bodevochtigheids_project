#include "esp_now_setup.h"
#include "adc_setup.h"



void app_main(void) 
{
    adc_oneshot_unit_handle_t adc_handle;

    esp_now_setup();

    adc_init(&adc_handle);

}