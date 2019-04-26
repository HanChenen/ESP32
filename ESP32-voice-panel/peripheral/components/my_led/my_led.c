#include "my_led.h"

static const char* TAG = "LED";

#define LED1            GPIO_NUM_2
#define LED2            GPIO_NUM_14
#define LED3            GPIO_NUM_19
#define LED4            GPIO_NUM_22

#define CONTROL_IO1     GPIO_NUM_4
#define CONTROL_IO2     GPIO_NUM_12
#define CONTROL_IO3     GPIO_NUM_15     

bool led_all_flag = 1;
bool led1_flag = 1;
bool led2_flag = 1;
bool led3_flag = 1;
bool led4_flag = 1;

void control_io_init(void)
{
    gpio_reset_pin(CONTROL_IO1);
    gpio_reset_pin(CONTROL_IO2);
    gpio_reset_pin(CONTROL_IO3);

    gpio_set_direction(CONTROL_IO1, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONTROL_IO2, GPIO_MODE_OUTPUT);
    gpio_set_direction(CONTROL_IO3, GPIO_MODE_OUTPUT);

    gpio_set_level(CONTROL_IO1, !led1_flag);
    gpio_set_level(CONTROL_IO2, !led2_flag);
    gpio_set_level(CONTROL_IO3, !led3_flag);

}

void led_init(void)
{
    gpio_reset_pin(LED2); //14脚比较特殊，不复位用不了

    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);

    gpio_set_level(LED1, led1_flag);
    gpio_set_level(LED2, led2_flag);
    gpio_set_level(LED3, led3_flag);
    gpio_set_level(LED4, led4_flag);
}
void led1_overturn(void)
{
    gpio_set_level(CONTROL_IO1, led1_flag);
    led1_flag = !led1_flag;
    gpio_set_level(LED1, led1_flag);
    
}
void led2_overturn(void)
{
    gpio_set_level(CONTROL_IO2, led2_flag);
    led2_flag = !led2_flag;
    gpio_set_level(LED2, led2_flag);

}
void led3_overturn(void)
{
    gpio_set_level(CONTROL_IO3, led3_flag);
    led3_flag = !led3_flag;
    gpio_set_level(LED3, led3_flag);
    
}
void led4_overturn(void)
{
    led4_flag = !led4_flag;
    gpio_set_level(LED4, led4_flag);
}
void led_all_overturn(void)
{
    led_all_flag = !led_all_flag;
    gpio_set_level(LED1, led_all_flag);
    gpio_set_level(LED2, led_all_flag);
    gpio_set_level(LED3, led_all_flag);
    gpio_set_level(LED4, led_all_flag);   
}