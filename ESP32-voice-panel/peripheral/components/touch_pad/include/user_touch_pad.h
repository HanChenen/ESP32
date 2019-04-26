#ifndef _USER_TP_H_
#define _USER_TP_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/touch_pad.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

#define  TP_LEFT        0
#define  TP_RIGHT       1
#define  TP_UP          2
#define  TP_DOWN        3
#define  NO_TP          4

void open_touch_int(void);
uint8_t tp_example_read(void);

#endif
