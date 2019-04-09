/******************************************************************************
 * Copyright 2015 Vowstar (Shenzhen) Co.,Ltd.
 *
 * FileName: devicexx_io.c
 *
 * Description: Key button application.
 *
 * Modification history:
 *     2015/05/08, v1.0 create this file.
*******************************************************************************/

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "gpio.h"
#include "pin_map.h"
#include "key.h"
#include "devicexx.h"
#include "devicexx_io.h"
#include "sniffer.h"
#include "devicexx_app.h"
#include "platform.h"


os_timer_t devicexx_io_led_timer;

uint8_t led_state = 0;
static uint8_t state = 0;

/******************************************************************************
 * FunctionName : devicexx_io_led_timer_tick
 * Description  : tick led timer
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
devicexx_io_led_timer_tick()
{

    os_timer_disarm(&devicexx_io_led_timer);
    os_timer_setfn(&devicexx_io_led_timer, (os_timer_func_t *)devicexx_io_led_timer_tick, NULL);

    state = ~ state;
#ifdef OLED_VERSION
#else
    switch (led_state) {
    case 1:
        GPIO_OUTPUT_SET(PIN_LED_S, state);
        os_timer_arm(&devicexx_io_led_timer, 800, 1);
//        os_printf("1,500 \n");
        break;
    case 2:
        GPIO_OUTPUT_SET(PIN_LED_S, state);
        os_timer_arm(&devicexx_io_led_timer, 250, 1);
//        os_printf("2,500 \n");
        break;
    case 3:
        GPIO_OUTPUT_SET(PIN_LED_S, state);
        os_timer_arm(&devicexx_io_led_timer, 60, 1);
//        os_printf("3,250 \n");
        break;
    case 4:
        GPIO_OUTPUT_SET(PIN_LED_S, state);
        os_timer_arm(&devicexx_io_led_timer, 500, 1);
//        os_printf("4,500 \n");
        break;
    case 5:
        GPIO_OUTPUT_SET(PIN_LED_S, 0);
//        os_timer_disarm(&devicexx_io_led_timer);
        break;
    default:
        os_timer_arm(&devicexx_io_led_timer, 500, 1);
        break;
    }
#endif
}





/******************************************************************************
 * FunctionName : devicexx_io_set_led_state
 * Description  : set led status
 * Parameters   : uint8_t relay_state
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
devicexx_io_set_led_state(uint8_t state)
{
    led_state = state;
}


/******************************************************************************
 * FunctionName : devicexx_led_init
 * Description  : init led gpio
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_led_init(void)
{

#ifdef OLED_VERSION
    PIN_FUNC_SELECT(pin_name[OLED_POWER], pin_func[OLED_POWER]);
    PIN_FUNC_SELECT(pin_name[PIN_POWER], pin_func[PIN_POWER]);

    GPIO_OUTPUT_SET(OLED_POWER, 1);
    os_printf("oled_power on\n");
    GPIO_OUTPUT_SET(PIN_POWER, 1);

#else
//    PIN_FUNC_SELECT(pin_name[PIN_LED], pin_func[PIN_LED]);
    PIN_FUNC_SELECT(pin_name[PIN_LED_S], pin_func[PIN_LED_S]);
    PIN_FUNC_SELECT(pin_name[PIN_GPS_S], pin_func[PIN_GPS_S]);
    PIN_FUNC_SELECT(pin_name[PIN_POWER], pin_func[PIN_POWER]);
//    GPIO_OUTPUT_SET(PIN_LED, 1);
    GPIO_OUTPUT_SET(PIN_LED_S, 0);
    GPIO_OUTPUT_SET(PIN_GPS_S, 1);
    GPIO_OUTPUT_SET(PIN_POWER, 1);
#endif
}


/******************************************************************************
 * FunctionName : devicexx_io_init
 * Description  : init key application function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
devicexx_io_init(void)
{
    devicexx_led_init();
#ifdef OLED_VERSION
#else
    devicexx_io_led_timer_tick();
#endif
}
