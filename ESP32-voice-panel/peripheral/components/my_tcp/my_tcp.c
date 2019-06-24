#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "my_tcp.h"
#include "my_sntp.h"
#include "mbedtls/md5.h"
#include "user_touch_pad.h"

static const char *TAG = "TCP_CLIENT"; 

char *host_num = "MHA0000M";
extern char strftime_buf[20];
//wifi
EventGroupHandle_t tcp_event_group;                     
const int WIFI_CONNECTED_BIT = BIT0;
const int LOG_IN_BIT = BIT1;
const int ACTIVATE_BIT = BIT2;
const int RECEIVE_COMMAND = BIT3;
const int SEND_HEART = BIT4;

extern QueueHandle_t message_queue;

//client 
//STA模式配置信息,即要连上的路由器的账号密码
#define GATEWAY_SSID            "MY_SSID"               //账号
#define GATEWAY_PAS             "MY_PASSWORD"           //密码
#define TCP_SERVER_ADRESS       "192.168.0.171"         //作为client，要连接的TCP服务器地址
#define TCP_PORT                9825                    //通信端口

//socket
static struct sockaddr_in server_addr;                  //server地址
static int connect_socket = 0;                          //连接socket

/*
 * wifi 事件
 */
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:        //STA模式-开始连接
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED: //STA模式-断线
        esp_wifi_connect();
        xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:    //STA模式-连接成功
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:       //STA模式-获取IP
        ESP_LOGI(TAG, "got ip:%s\n",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}
/*
 * WIFI作为STA的初始化
 */
void wifi_init_sta()
{
    tcpip_adapter_init();
    tcp_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = GATEWAY_SSID,           //STA账号
            .password = GATEWAY_PAS         //STA密码
        },      
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s \n",GATEWAY_SSID, GATEWAY_PAS);
}
/*
 * 函数功能：建立tcp client 
 */
esp_err_t create_tcp_client()
{
    ESP_LOGI(TAG, "will connect gateway ssid : %s port:%d",TCP_SERVER_ADRESS, TCP_PORT);
    //新建socket
    connect_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_socket < 0)
    {
        ESP_LOGI(TAG, "create socket error");
        close(connect_socket);
        return ESP_FAIL;
    }
    //配置连接服务器信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_ADRESS);
    ESP_LOGI(TAG, "connectting server...");
    //连接服务器
    if (connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        ESP_LOGE(TAG, "connect failed!");
        close(connect_socket);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "connect success!");
    return ESP_OK;
}
/*
 * 函数功能：以下连续三个函数实现将hex转化为字符串
 */
unsigned char IntToHexChar_1(unsigned char c)
{
    return c < 10 ? (c + '0') : (c - 10 + 'a');
}

unsigned char IntToHexChar_2(unsigned char c)
{
    return c < 10 ? (c + '0') : (c - 10 + 'A');
}
/*
 * 函数功能：hex转字符串
 * 输入：
 *      des：目标地址
 *      src：源地址
 *      inlength：输入长度
 *      flag：为真时，转为小写，为假时，转为大写
 * 输出：无
 */
void hex_to_char(char *des, unsigned char *src, int inlength, bool flag)
{
    if(flag)
    {
        for(uint8_t i = 0; i < inlength; i ++)
        {
            des[2 * i] = IntToHexChar_1((src[i] & 0xf0 ) >> 4);
            des[2 * i + 1] = IntToHexChar_1(src[i] & 0x0f);
        }
        
    }
    else
    {
        for(uint8_t i = 0; i < inlength; i ++)
        {
            des[2 * i] = IntToHexChar_2((src[i] & 0xf0 ) >> 4);
            des[2 * i + 1] = IntToHexChar_2(src[i] & 0x0f);
        }
    }
    des[2 * inlength] = '\0';
}
/*
 * 函数功能：生成MD5密钥
 */
void MD5_secret_key(char md5_secret_key[32], char *HOST_num)
{
    unsigned char secret_key[16];
    char secret_key_prefix[20] = "QWER";    
    char HOST_num_6B[20];

    get_now_time();
    strcpy(HOST_num_6B, strchr(HOST_num, 'H') + 1);  //提取主机序号的后六位
    ESP_LOGI(TAG, "HOST_num_6B: %s", HOST_num_6B); 
    strcat(strftime_buf, HOST_num_6B);
    strcat(secret_key_prefix, strftime_buf);

    ESP_LOGI(TAG, "secret_key_raw: %s", secret_key_prefix);
    mbedtls_md5_ret((unsigned char *)secret_key_prefix, sizeof(secret_key_prefix), secret_key); 
    hex_to_char(md5_secret_key, secret_key, 16, true);
    ESP_LOGI(TAG, "secret_key_md5: %s", md5_secret_key);
}
/*
 * 函数功能：用12个字符的mac地址，生成8个字符的密钥mac
 */
void secret_mac(char Secret_mac[8])
{
    uint8_t  Mac[6];
    char Mac_char[12];
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, Mac));
    hex_to_char(Mac_char, Mac, 6, false);
    strncpy(Secret_mac, Mac_char, 8);
}
/*
 * 任务：链接登录指令
 */
char *COM_send_connect()
{
    static char COM_connect[60];
    char Mac_char[8], md5_secret_key[32];
    
    strcpy(COM_connect, "NT|");
    strcat(COM_connect, host_num);
    strcat(COM_connect, "|");
    secret_mac(Mac_char);
    strcat(COM_connect, Mac_char);
    strcat(COM_connect, "|");   
    MD5_secret_key(md5_secret_key, host_num);
    strcat(COM_connect, md5_secret_key);

    return COM_connect;  
}
/*
 * 任务：发送登录指令
 */
void log_in(void *pvParameters)
{   
    while(1)
    {
        char *com_connect = COM_send_connect();
        xEventGroupWaitBits(tcp_event_group, LOG_IN_BIT, false, true, portMAX_DELAY);
        int temp = send(connect_socket, com_connect, strlen(com_connect), 0);
        if(temp == -1)
        {
            close(connect_socket);
            int socket_ret = create_tcp_client();
            if (socket_ret == ESP_FAIL)
            {
                ESP_LOGI(TAG, "create tcp socket error,stop...");
            }
        }
        vTaskDelay(100 / portTICK_RATE_MS);      
    }   
}
/*
 * 任务：接收激活指令
 */
void activate_equipment(void *pvParameters)
{
    int len = 0;
    char databuff[10];
    while(1)
    {
        xEventGroupWaitBits(tcp_event_group, ACTIVATE_BIT, false, true, portMAX_DELAY);        
        memset(databuff, 0x00, sizeof(databuff));
        len = recv(connect_socket, databuff, sizeof(databuff), 0);
        if(len > 0)
        {
            ESP_LOGI(TAG, "recv_data: %s", databuff);
            char temp = strncmp(databuff, "ok", 2);
            if(temp == 0)
            {
                ESP_LOGI(TAG, "com_connect success");
                xEventGroupClearBits(tcp_event_group, LOG_IN_BIT);
                xEventGroupClearBits(tcp_event_group, ACTIVATE_BIT);
                xEventGroupSetBits(tcp_event_group, RECEIVE_COMMAND);
                xEventGroupSetBits(tcp_event_group, SEND_HEART);
            }
            else
            {
                ESP_LOGI(TAG, "com_connect failed");
            }               
        }
        vTaskDelay(10);   
    }
}
/*
 * 任务：接收控制指令
 */
void receive_command(void *pvParameters)
{

    int  len = 0;
    char databuff[10];
    char switch_value = NO_TP;

    while(1)
    {
        xEventGroupWaitBits(tcp_event_group, RECEIVE_COMMAND, false, true, portMAX_DELAY);
        memset(databuff, 0x00, sizeof(databuff));
        len = recv(connect_socket, databuff, sizeof(databuff), 0);
        if(len > 0)
        {
            ESP_LOGI(TAG, "recv_data: %s", databuff);
            if(strncmp(databuff, "18", 2) == 0)
            {
                ESP_LOGI(TAG, "swich1 on");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_LEFT;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else if(strncmp(databuff, "19", 2) == 0)
            {
                ESP_LOGI(TAG, "swich1 off");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_LEFT;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else if(strncmp(databuff, "20", 2) == 0)
            {
                ESP_LOGI(TAG, "swich2 on");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_RIGHT;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else if(strncmp(databuff, "21", 2) == 0)
            {
                ESP_LOGI(TAG, "swich2 off");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_RIGHT;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else if(strncmp(databuff, "22", 2) == 0)
            {
                ESP_LOGI(TAG, "swich3 on");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_DOWN;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else if(strncmp(databuff, "23", 2) == 0)
            {
                ESP_LOGI(TAG, "swich3 off");
                send(connect_socket, "ok", 2, 0);
                switch_value = TP_DOWN;
                xQueueSend(message_queue, &switch_value, 10);
                continue;
            }
            else
            {
                ESP_LOGI(TAG, "data error");
                send(connect_socket, "no", 2, 0);
            }                          
        }
        vTaskDelay(10);    
    }
}
/*
 * 任务：发送心跳包
 */
void send_heartbeat(void *pvParameters)
{
    const char *heartbeat = "test";
    while(1)
    {
        xEventGroupWaitBits(tcp_event_group, SEND_HEART, false, true, portMAX_DELAY);
        int temp = send(connect_socket, heartbeat, strlen(heartbeat), 0);
        if(temp == -1)
        {
            close(connect_socket);
            int socket_ret = create_tcp_client();
            if (socket_ret == ESP_FAIL)
            {
                ESP_LOGI(TAG, "create tcp socket error,stop...");
            }
            xEventGroupSetBits(tcp_event_group, LOG_IN_BIT);
            xEventGroupSetBits(tcp_event_group, ACTIVATE_BIT);
            xEventGroupClearBits(tcp_event_group, RECEIVE_COMMAND);
            xEventGroupClearBits(tcp_event_group, SEND_HEART);
        }
        vTaskDelay(4000 / portTICK_RATE_MS);
    }
}
/*
 * 任务：建立TCP连接，创建任务函数
 */
void my_tcp_connect(void *pvParameters)
{
    //等待WIFI连接信号量，死等
    xEventGroupWaitBits(tcp_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    vTaskDelay(3000 / portTICK_RATE_MS);  
    int socket_ret = create_tcp_client();
    if (socket_ret == ESP_FAIL)
    {
        ESP_LOGI(TAG, "create tcp socket error,stop...");
    }
    initialize_sntp();
    xEventGroupSetBits(tcp_event_group, LOG_IN_BIT);
    xEventGroupSetBits(tcp_event_group, ACTIVATE_BIT);
    xTaskCreate(&log_in, "log_in", 4 * 1024, NULL, 6, NULL);
    xTaskCreate(&activate_equipment, "activate_equipment", 4 * 1024, NULL, 7, NULL);
    xTaskCreate(&receive_command, "receive_command", 4 * 1024, NULL, 8, NULL);
    xTaskCreate(&send_heartbeat, "send_heartbeat", 4 * 1024, NULL, 9, NULL);
    vTaskDelete(NULL);
}
