/******************************************************************************
 * Copyright 2015 Vowstar (Shenzhen) Co.,Ltd.
 *
 * FileName: vowstar_io.h
 *
 * Description: KEY button application.
 *
 * Modification history:
 *     2015/05/08, v1.0 create this file.
*******************************************************************************/

#ifndef __VOWSTAR_IO_H__
#define __VOWSTAR_IO_H__


extern uint8_t led_state;
extern os_timer_t devicexx_io_led_timer;


void devicexx_io_led_timer_tick();
#endif /* __VOWSTAR_IO_H__ */
