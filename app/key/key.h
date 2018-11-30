/******************************************************************************
 * Copyright Vowstar Co., Ltd.
 *
 * FileName: key.h
 *
 * Description: key driver, now can use different gpio and install different function
 *
 * Modification history:
 *     2015/5/1, v1.0 create this file.
*******************************************************************************/

#ifndef __KEY_H__
#define __KEY_H__

#include "gpio.h"

typedef enum LedState LedState;
typedef void (* key_function)(bool key_up);

#define KEY_STATUS_NONE             0x00

#define KEY_STATUS_DOWN_SHORT       0x01
#define KEY_STATUS_DOWN_MEDIUM      0x02
#define KEY_STATUS_DOWN_LONG        0x03
#define KEY_STATUS_DOWN_EXTRA       0x04
#define KEY_STATUS_DOWN_SUPER       0x05

#define KEY_STATUS_UP_SHORT         0x11
#define KEY_STATUS_UP_MEDIUM        0x12
#define KEY_STATUS_UP_LONG          0x13
#define KEY_STATUS_UP_EXTRA         0x14
#define KEY_STATUS_UP_SUPER         0x15

struct single_key_param {
    uint8 gpio_id;
    uint8 gpio_func;
    uint32 gpio_name;
    os_timer_t key_50ms;
    uint8 press_status;
    key_function short_press;
    key_function medium_press;
    key_function long_press;
    key_function extra_press;
    key_function super_press;
};

struct keys_param {
    uint8 key_num;
    struct single_key_param **single_key;
};

struct single_key_param *key_init_single(uint8 gpio_id, key_function short_press, key_function medium_press, key_function long_press, key_function extra_press, key_function super_press);
void key_init(struct keys_param *key);

#if defined(GLOBAL_DEBUG)
#define KEY_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define KEY_DEBUG(format, ...)
#endif


#endif
