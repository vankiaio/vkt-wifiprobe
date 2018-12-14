/*
 * tcpclient.h
 *
 *  Created on: 2018��10��18��
 *      Author: taotf
 */

#ifndef APP_MAIN_TCPCLIENT_H_
#define APP_MAIN_TCPCLIENT_H_

extern os_timer_t checkTimer_wifistate;
//extern struct espconn user_tcp_conn;
extern struct espconn  tcpserver;
extern uint8_t http_answer[];
extern uint8_t upgrade_tcp;
extern uint8_t http_head[];
extern uint8_t http_data[];

void tcp_server(void);
void tcp_client_init(uint8_t ssid[32], uint8_t pwd[32]);

#endif /* APP_MAIN_TCPCLIENT_H_ */
