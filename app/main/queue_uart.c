/******************************************************************************
 * Copyright 2015 Vankia Co.,Ltd.
 *
 * FileName: queue.c
 *
 * Description: Queue of data
 *
 * Modification history:
 *     2015/8/4, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "c_types.h"
#include "user_config.h"
#include "user_interface.h"
#include "queue_uart.h"

#ifndef BYTE_SIZE
#define BYTE_SIZE (8)
#endif

#define QUEUE_UART_SWAP(x) ((uint16_t )((((uint16_t )x) << BYTE_SIZE) | (((uint16_t )x) >> BYTE_SIZE)))

LOCAL os_timer_t queue_uart_timer_receive;

LOCAL uint8_t queue_uart_data[QUEUE_UART_BUFFER_LENGTH + 1] = {0};
LOCAL uint16_t queue_uart_data_index = 0;

LOCAL queue_uart_receive_callback_t queue_uart_receive_callback_handle = NULL;
LOCAL queue_uart_send_callback_t queue_uart_send_callback_handle = NULL;

void ICACHE_FLASH_ATTR queue_uart_receive_callback_register(queue_uart_receive_callback_t callback)
{
	queue_uart_receive_callback_handle = callback;
}

void ICACHE_FLASH_ATTR queue_uart_send_callback_register(queue_uart_send_callback_t callback)
{
	queue_uart_send_callback_handle = callback;
}

LOCAL void ICACHE_FLASH_ATTR queue_uart_timer_receive_tick(void)
{
	os_timer_disarm(&queue_uart_timer_receive);
	if (queue_uart_receive_callback_handle)
	{
		if ((queue_uart_data_index > 0) && (queue_uart_data_index <= QUEUE_UART_BUFFER_LENGTH))
		{
			// Terminal the buffer
			// queue_uart_data[queue_uart_data_index + 1] = '\0';
			// Callback
			queue_uart_receive_callback_handle(queue_uart_data, queue_uart_data_index);
			QUEUE_UART_DEBUG("Queue UART Receive OK\r\n");
		}
	}
	queue_uart_data_index = 0;
}

void ICACHE_FLASH_ATTR queue_uart_input(uint8_t data)
{
	queue_uart_data[queue_uart_data_index] = data;
	queue_uart_data_index++;
	os_timer_disarm(&queue_uart_timer_receive);
	os_timer_setfn(&queue_uart_timer_receive, (os_timer_func_t *)queue_uart_timer_receive_tick, NULL);
	os_timer_arm(&queue_uart_timer_receive, QUEUE_UART_INTERVAL_MS, 0);
	if (queue_uart_data_index >= QUEUE_UART_BUFFER_LENGTH)
	{
		queue_uart_timer_receive_tick();
	}
}

void ICACHE_FLASH_ATTR queue_uart_send(const unsigned char *pdata, unsigned short length)
{
	uart0_tx_buffer((uint8_t *)pdata, length);
	if (queue_uart_send_callback_handle)
	{
		queue_uart_send_callback_handle(pdata, length);
	}
	QUEUE_UART_DEBUG("Queue UART Send OK\r\n");
}
