#include "user_touch_pad.h"

static const char* TAG = "Touch pad";

#define TOUCH_THRESH_NO_USE   (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

static bool s_pad_activated[TOUCH_PAD_MAX];
static uint32_t s_pad_init_val[TOUCH_PAD_MAX];


static void tp_example_set_thresholds(void)
{
    uint16_t touch_value;
    for (int i = 4; i < TOUCH_PAD_MAX; i ++) 
    {
        if ((i == 5) || (i == 6)) {continue;}       
        //read filtered value
        touch_pad_read_filtered(i, &touch_value);
        s_pad_init_val[i] = touch_value;
        ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", i, touch_value);
        //set interrupt threshold.
        ESP_ERROR_CHECK(touch_pad_set_thresh(i, touch_value * 99 / 100));
    }
}
/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    //clear interrupt
    touch_pad_clear_status();
    for (int i = 4; i < TOUCH_PAD_MAX; i ++) 
    {
        if ((i == 5) || (i == 6)) {continue;} 
        if ((pad_intr >> i) & 0x01) 
        {
            s_pad_activated[i] = true;
        }
    }
}

/*
 * Before reading touch pad, we need to initialize the RTC IO.
 */
static void tp_example_touch_pad_init(void)
{
    for (int i = 4;i < TOUCH_PAD_MAX; i ++) 
    {
        if ((i == 5) || (i == 6)) {continue;} 
        //init RTC IO and mode for touch pad.
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

void open_touch_int(void)
{
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing touch pad");
    touch_pad_init();
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Set reference voltage for charging/discharging
    // For most usage scenarios, we recommend using the following combination:
    // the high reference valtage will be 2.7V - 1V = 1.7V, The low reference voltage will be 0.5V.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    // Init touch pad IO
    tp_example_touch_pad_init();
    // Initialize and start a software filter to detect slight change of capacitance.
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // Set thresh hold
    tp_example_set_thresholds();
    // Register touch interrupt ISR
    touch_pad_isr_register(tp_example_rtc_intr, NULL);   
}

uint8_t tp_example_read(void)
{
    touch_pad_intr_enable();
    if (s_pad_activated[4] == true)
    {
        ESP_LOGI(TAG, "T4 activated!");
        vTaskDelay(200 / portTICK_PERIOD_MS);
        s_pad_activated[4] = false;
        return TP_UP;
    }    
    if (s_pad_activated[7] == true)
    {
        ESP_LOGI(TAG, "T7 activated!");
        vTaskDelay(200 / portTICK_PERIOD_MS);
        s_pad_activated[7] = false;
        return TP_LEFT;
    }
    if (s_pad_activated[8] == true)
    {
        ESP_LOGI(TAG, "T8 activated!");
        vTaskDelay(200 / portTICK_PERIOD_MS);
        s_pad_activated[8] = false;
        return TP_DOWN;
    }
    if (s_pad_activated[9] == true)
    {
        ESP_LOGI(TAG, "T9 activated!");
        vTaskDelay(200 / portTICK_PERIOD_MS);
        s_pad_activated[9] = false;
        return TP_RIGHT;
    }
    return NO_TP;
}





