#ifndef __QUEUE_UART_H__
#define __QUEUE_UART_H__

#if defined(GLOBAL_DEBUG_ON)
#define QUEUE_UART_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define QUEUE_UART_DEBUG(format, ...)
#endif

#define QUEUE_UART_INTERVAL_MS		(80)
#define QUEUE_UART_BUFFER_LENGTH 	(1024)

#include "uart.h"

typedef void (*queue_uart_receive_callback_t)(const unsigned char *pdata, unsigned short length);
typedef void (*queue_uart_send_callback_t)(const unsigned char *pdata, unsigned short length);

void queue_uart_receive_callback_register(queue_uart_receive_callback_t callback);
void queue_uart_send_callback_register(queue_uart_send_callback_t callback);

void queue_uart_input(uint8_t data);
void queue_uart_send(const unsigned char *pdata, unsigned short length);

#endif /* __QUEUE_UART_H__ */
