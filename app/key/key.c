/******************************************************************************
 * Copyright Vowstar Co., Ltd.
 *
 * FileName: key.c
 *
 * Description: key driver, now can use different gpio and install different function
 *
 * Modification history:
 *     2015/5/1, v1.0 create this file.
*******************************************************************************/
 
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"
#include "pin_map.h"
#include "user_config.h"
#include "key.h"

#define KEY_PRESS_COUNT_SHORT   (2000 / 50)
#define KEY_PRESS_COUNT_MEDIUM  (7000 / 50)
#define KEY_PRESS_COUNT_LONG    (12000 / 50)
#define KEY_PRESS_COUNT_EXTRA   (17000 / 50)
LOCAL uint32 key_press_count = 0;
LOCAL void key_intr_handler(struct keys_param *keys);

/******************************************************************************
 * FunctionName : key_init_single
 * Description  : init single key's gpio and register function
 * Parameters   : uint8 gpio_id - which gpio to use
 *                uint32 gpio_name - gpio mux name
 *                uint32 gpio_func - gpio function
 *                key_function short_press - short press function, needed to install
 *                key_function medium_press - medium press function, needed to install
 *                key_function long_press - long press function, needed to install
 *                key_function extra_press - extra press function, needed to install
 * Returns      : single_key_param - single key parameter, needed by key init
*******************************************************************************/
struct single_key_param *ICACHE_FLASH_ATTR
key_init_single(uint8 gpio_id, key_function short_press, key_function medium_press, key_function long_press, key_function extra_press, key_function super_press)
{
    struct single_key_param *single_key = (struct single_key_param *)os_zalloc(sizeof(struct single_key_param));

    single_key->gpio_id = gpio_id;
    single_key->gpio_name = (uint32)pin_name[gpio_id];
    single_key->gpio_func = (uint8)pin_func[gpio_id];

    single_key->short_press = short_press;
    single_key->medium_press = medium_press;
    single_key->long_press = long_press;
    single_key->extra_press = extra_press;
    single_key->super_press = super_press;

    return single_key;
}

/******************************************************************************
 * FunctionName : key_init
 * Description  : init keys
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
key_init(struct keys_param *keys)
{
    uint8 i;

    ETS_GPIO_INTR_ATTACH(key_intr_handler, keys);

    ETS_GPIO_INTR_DISABLE();

    for (i = 0; i < keys->key_num; i++) {
        keys->single_key[i]->press_status = KEY_STATUS_NONE;

        PIN_FUNC_SELECT(keys->single_key[i]->gpio_name, keys->single_key[i]->gpio_func);

        gpio_output_set(0, 0, 0, GPIO_ID_PIN(keys->single_key[i]->gpio_id));

        gpio_register_set(GPIO_PIN_ADDR(keys->single_key[i]->gpio_id), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                          | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                          | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

        // clear gpio status
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(keys->single_key[i]->gpio_id));

        // enable internal pullup
        CLEAR_PERI_REG_MASK(keys->single_key[i]->gpio_name, PERIPHS_IO_MUX_PULLUP2);
        SET_PERI_REG_MASK(keys->single_key[i]->gpio_name, PERIPHS_IO_MUX_PULLUP);

        // enable interrupt
        gpio_pin_intr_state_set(GPIO_ID_PIN(keys->single_key[i]->gpio_id), GPIO_PIN_INTR_NEGEDGE);
    }

    ETS_GPIO_INTR_ENABLE();
}

/******************************************************************************
 * FunctionName : key_50ms_cb
 * Description  : 50ms timer callback to check it's a real key push
 * Parameters   : single_key_param *single_key - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
key_50ms_cb(struct single_key_param *single_key)
{
    // high, then key is up
    if (1 == GPIO_INPUT_GET(GPIO_ID_PIN(single_key->gpio_id))) {
        os_timer_disarm(&single_key->key_50ms);

        if (key_press_count < KEY_PRESS_COUNT_SHORT)
        {
            single_key->press_status = KEY_STATUS_UP_SHORT;
            if (single_key->short_press) {
                single_key->short_press(true);
            }

        }
        else if (key_press_count < KEY_PRESS_COUNT_MEDIUM)
        {
            single_key->press_status = KEY_STATUS_UP_MEDIUM;
            if (single_key->medium_press) {
                single_key->medium_press(true);
            }
        }
        else if (key_press_count < KEY_PRESS_COUNT_LONG)
        {
            single_key->press_status = KEY_STATUS_UP_LONG;
            if (single_key->long_press) {
                single_key->long_press(true);
            }
        }
        else if (key_press_count < KEY_PRESS_COUNT_EXTRA)
        {
            single_key->press_status = KEY_STATUS_UP_EXTRA;
            if (single_key->extra_press) {
                single_key->extra_press(true);
            }
        }
        else
        {
            single_key->press_status = KEY_STATUS_UP_SUPER;
            if (single_key->super_press) {
                single_key->super_press(true);
            }
        }

        key_press_count = 0;
        gpio_pin_intr_state_set(GPIO_ID_PIN(single_key->gpio_id), GPIO_PIN_INTR_NEGEDGE);
    } else {
        os_timer_disarm(&single_key->key_50ms);
        os_timer_setfn(&single_key->key_50ms, (os_timer_func_t *)key_50ms_cb, single_key);
        os_timer_arm(&single_key->key_50ms, 50, 0);
        key_press_count++;
        if (key_press_count % 10 == 0)
        {
            if (key_press_count < KEY_PRESS_COUNT_SHORT)
            {
                single_key->press_status = KEY_STATUS_DOWN_SHORT;
                if (single_key->short_press) {
                    single_key->short_press(false);
                }
            }
            else if (key_press_count < KEY_PRESS_COUNT_MEDIUM)
            {
                single_key->press_status = KEY_STATUS_DOWN_MEDIUM;
                if (single_key->medium_press) {
                    single_key->medium_press(false);
                }
            }
            else if (key_press_count < KEY_PRESS_COUNT_LONG)
            {
                single_key->press_status = KEY_STATUS_DOWN_LONG;
                if (single_key->long_press) {
                    single_key->long_press(false);
                }
            }
            else if (key_press_count < KEY_PRESS_COUNT_EXTRA)
            {
                single_key->press_status = KEY_STATUS_DOWN_EXTRA;
                if (single_key->extra_press) {
                    single_key->extra_press(false);
                }
            }
            else
            {
                KEY_DEBUG("Key super pressing.\r\n");
                single_key->press_status = KEY_STATUS_DOWN_SUPER;
                if (single_key->super_press) {
                    single_key->super_press(false);
                }
            }
        }
        if (key_press_count > 0xFFF0)
        {
            key_press_count = 0xFFF0;
        }
    }
}

/******************************************************************************
 * FunctionName : key_intr_handler
 * Description  : key interrupt handler
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_RAM_ATTR
key_intr_handler(struct keys_param *keys)
{
    uint8 i;
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    for (i = 0; i < keys->key_num; i++) {
        if (gpio_status & BIT(keys->single_key[i]->gpio_id)) {
            // disable interrupt
            gpio_pin_intr_state_set(GPIO_ID_PIN(keys->single_key[i]->gpio_id), GPIO_PIN_INTR_DISABLE);

            // clear interrupt status
            GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(keys->single_key[i]->gpio_id));

            if (0 == GPIO_INPUT_GET(GPIO_ID_PIN(keys->single_key[i]->gpio_id))) {

                // 50ms, check if this is a real key up
                os_timer_disarm(&keys->single_key[i]->key_50ms);
                os_timer_setfn(&keys->single_key[i]->key_50ms, (os_timer_func_t *)key_50ms_cb, keys->single_key[i]);
                os_timer_arm(&keys->single_key[i]->key_50ms, 50, 0);
            }
            gpio_pin_intr_state_set(GPIO_ID_PIN(keys->single_key[i]->gpio_id), GPIO_PIN_INTR_NEGEDGE);
        }
    }
}

