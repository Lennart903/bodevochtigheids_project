#include "connection_setup.h"


void app_main(void)
{
    wifi_init();
    esp_now_setup();
}
