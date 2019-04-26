#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "my_sntp.h"

#include "lwip/err.h"
#include "apps/sntp/sntp.h"

static const char *TAG = "MY_SNTP";
char strftime_buf[20];

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_init();
    setenv("TZ", "CST-8", 1);
    tzset();
    vTaskDelay(3000 / portTICK_RATE_MS);
}

void stop_sntp( void)
{
	sntp_stop();
}

void get_now_time( void)
{
    time_t now;
    struct tm timeinfo;

    time(&now);   
    localtime_r(&now, &timeinfo);
    while(timeinfo.tm_year < (2019 - 1900))
    {        
        ESP_LOGI(TAG, "get time failed");
        vTaskDelay(2000 / portTICK_RATE_MS);
        time(&now);   
        localtime_r(&now, &timeinfo);
    }
    strftime(strftime_buf, sizeof(strftime_buf), "%Y%m%d%H", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
    sntp_stop();
}
