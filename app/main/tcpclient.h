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
//extern struct espconn user_tcp_conn;
extern struct espconn  tcpserver;
extern uint8_t http_answer[];
extern uint8_t upgrade_tcp;
extern uint8_t http_head[];
extern uint8_t http_data[];

void tcp_server(void);
void tcp_client_init(uint8_t ssid[32], uint8_t pwd[32]);

#endif /* DXX_WP_OLED_APP_MAIN_TCPCLIENT_H_ */
