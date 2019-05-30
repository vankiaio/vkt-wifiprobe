/*
 * tcpclient.h
 *
 *  Created on: 2018Äê10ÔÂ18ÈÕ
 *      Author: taotf
 */

#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_
#include "c_types.h"
#include "os_type.h"

extern os_timer_t checkTimer_wifistate;
extern struct station_config stationConf;
//extern struct espconn user_tcp_conn;
extern struct espconn  tcpserver;
extern uint8_t http_answer[];
extern uint8_t upgrade_tcp;
extern uint8_t http_head[];
extern uint8_t http_data[];
extern uint8_t wifi_state;
void tcp_server(void);
void tcp_client_init(uint8_t ssid[32], uint8_t pwd[32]);
void Check_WifiState(void);
void post_callback(void * ctx, char * response_body, size_t response_body_size, int http_status, char * response_headers);
void save_to_flash(void);

#endif /* DXX_WP_OLED_APP_MAIN_TCPCLIENT_H_ */







//#include "espconn.h"
//#include "mem.h"
//char buffer[1024];
//#define GET "GET /%s HTTP/1.1\r\nContent-Type: text/html;charset=utf-8\r\nAccept: */*\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n"
//#define POST "POST /%s HTTP/1.1\r\nAccept: */*\r\nContent-Length: %d\r\nContent-Type: application/json\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n%s"
//struct espconn user_tcp_conn;
//void my_station_init(struct ip_addr *remote_ip,struct ip_addr *local_ip,int remote_port);
//
//
//#endif /* APP_INCLUDE_CLIENT_H_ */
//
