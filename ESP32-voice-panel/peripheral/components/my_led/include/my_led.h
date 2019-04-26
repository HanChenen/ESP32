#ifndef _MY_LED_H_
#define _MY_LED_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

void control_io_init(void);
void led_init(void);
void led1_overturn(void);
void led2_overturn(void);
void led3_overturn(void);
void led4_overturn(void);
void led_all_overturn(void);

#endif
