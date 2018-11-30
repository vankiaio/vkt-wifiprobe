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

#define KEY_COUNT 	(1)

#if defined(GLOBAL_DEBUG_ON)
#define VOWSTAR_IO_DEBUG_ON
#endif
#if defined(VOWSTAR_IO_DEBUG_ON)
#define VOWSTAR_IO_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define VOWSTAR_IO_DEBUG(format, ...)
#endif

extern uint8_t led_state;

extern struct keys_param keys;
void vowstar_io_init(void);
uint8_t vowstar_io_get_relay_state(void);
void vowstar_io_set_relay_state(uint8_t state);
uint8_t vowstar_io_get_led_state(void);
void vowstar_io_set_led_state(uint8_t state);

#endif /* __VOWSTAR_IO_H__ */
