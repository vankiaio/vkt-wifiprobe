/*
 * tcpclient.h
 *
 *  Created on: 2018Äê10ÔÂ18ÈÕ
 *      Author: taotf
 */

#ifndef APP_MAIN_TCPCLIENT_H_
#define APP_MAIN_TCPCLIENT_H_

extern os_timer_t checkTimer_wifistate;
//extern struct espconn user_tcp_conn;
extern struct espconn  tcpserver; ;
extern uint8_t upgrade_tcp;
extern uint8_t http_head[];
extern uint8_t http_data[];
#endif /* APP_MAIN_TCPCLIENT_H_ */
