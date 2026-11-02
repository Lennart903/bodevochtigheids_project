#include "adc_setup.h"
#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"


void adc_init(adc_oneshot_unit_handle_t * adc_handle)
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


