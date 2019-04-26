#ifndef __TCP_BSP_H__
#define __TCP_BSP_H__

#ifdef __cplusplus
extern "C" {
#endif

//using esp as station
void wifi_init_sta();
void my_tcp_connect(void *pvParameters);
void send_heartbeat(void *pvParameters);
void receive_data(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /*#ifndef __TCP_BSP_H__*/
