#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "my_led.h"
#include "my_tcp.h"
#include "user_touch_pad.h"

static const char* TAG = "APP MAIN";
QueueHandle_t message_queue;

void touch_task(void *pvParameters)
{
    char TP_value = NO_TP;
    char switch_value = NO_TP;
    while(1)
    {
        if(xQueueReceive(message_queue, &switch_value, 10))
        {
            ESP_LOGI(TAG, "switch_value: %d", switch_value);
            TP_value = switch_value;
            goto switch_value;
        }
        TP_value = tp_example_read();
switch_value:
        switch (TP_value)
        {
            case TP_LEFT:
                led3_overturn();
                break;
            case TP_RIGHT:
                led2_overturn();
                break;
            case TP_UP:
                led4_overturn();
                break;
            case TP_DOWN:
                led1_overturn();
                break;
            default:
                break;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }   
}

void app_main()
{
    char count = 0;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    led_init();
    while(1)
    {
        count++;
        led_all_overturn();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if(count > 50)
        {
            count = 0;
            break;
        }

    }
    wifi_init_sta();    
    open_touch_int();
    control_io_init();
    message_queue = xQueueCreate(1, sizeof(char));
    xTaskCreate(&touch_task, "touch_task", 4 * 1024, NULL, 4, NULL);
    xTaskCreate(&my_tcp_connect, "my_tcp_connect", 4 * 1024, NULL, 5, NULL);   
}











