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

struct keys_param keys;
LOCAL struct single_key_param *single_key[KEY_COUNT];

os_timer_t devicexx_io_led_timer;

LOCAL uint8_t relay_state = 0;
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
    switch (led_state) {

    case 1:        
        GPIO_OUTPUT_SET(PIN_LED_S, state);
        os_timer_arm(&devicexx_io_led_timer, 500, 1);
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
}

/******************************************************************************
 * FunctionName : devicexx_io_get_relay_state
 * Description  : get relay status
 * Parameters   : none
 * Returns      : uint8_t relay_state
*******************************************************************************/
uint8_t ICACHE_FLASH_ATTR
devicexx_io_get_relay_state(void)
{
    return relay_state;
}

/******************************************************************************
 * FunctionName : devicexx_io_set_relay_state
 * Description  : set relay status
 * Parameters   : uint8_t relay_state
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
devicexx_io_set_relay_state(uint8_t state)
{
//    relay_state = state;
//    PIN_FUNC_SELECT(pin_name[PIN_RELAY], pin_func[PIN_RELAY]);
//    GPIO_OUTPUT_SET(PIN_RELAY, relay_state);
//    if(state)
//    {
//        VOWSTAR_IO_DEBUG("relay on\n");
//    } else {
//        VOWSTAR_IO_DEBUG("relay off\n");
//    }

}

/******************************************************************************
 * FunctionName : devicexx_io_get_relay_state
 * Description  : get relay status
 * Parameters   : none
 * Returns      : uint8_t relay_state
*******************************************************************************/
//uint8_t ICACHE_FLASH_ATTR
//devicexx_io_get_led_state(void)
//{
//    return led_state;
//}

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
 * FunctionName : devicexx_key_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : bool key_up
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_key_short_press(bool key_up)
{
    wifi_promiscuous_enable(0);

    if (key_up)
    {
        VOWSTAR_IO_DEBUG("Key short pressed\r\n");
        delay_power_off();//关机

    }
    else
    {
        VOWSTAR_IO_DEBUG("Key short pressing.\r\n");
        devicexx_io_led_timer_tick(1);
    }
}

/******************************************************************************
 * FunctionName : devicexx_key_medium_press
 * Description  : key's medium press function, needed to be installed
 * Parameters   : bool key_up
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_key_medium_press(bool key_up)
{
    if (key_up)
    {
        VOWSTAR_IO_DEBUG("Key medium_pressed\r\n");
//        bind_flag = 0;
//        at_state = NONE;
//        wifi_set_opmode(SOFTAP_MODE);
//        vowstar_set_ssid_prefix("Vankia_WP_");
//        tcp_server();
    }
    else
    {
        VOWSTAR_IO_DEBUG("Key medium pressing.\r\n");
        devicexx_io_set_led_state(2);
    }
}

/******************************************************************************
 * FunctionName : devicexx_key_long_press
 * Description  : key's long press function, needed to be installed
 * Parameters   : bool key_up
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_key_long_press(bool key_up)
{
    if (key_up)
    {
        VOWSTAR_IO_DEBUG("Key long pressed\r\n");
        // Force enter sta-ap mode
//        devicexx_force_aplink();

        delay_power_off();//关机

    }
    else
    {
        VOWSTAR_IO_DEBUG("Key long pressing.\r\n");
        devicexx_io_set_led_state(3);
    }
}

/******************************************************************************
 * FunctionName : devicexx_key_extra_press
 * Description  : key's extra press function, needed to be installed
 * Parameters   : bool key_up
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_key_extra_press(bool key_up)
{
    if (key_up)
    {
        VOWSTAR_IO_DEBUG("Key extra pressed\r\n");
        // Force do OTA check
//        devicexx_check_update();

        delay_power_off();


    }
    else
    {
        VOWSTAR_IO_DEBUG("Key extra pressing.\r\n");
        devicexx_io_set_led_state(4);
    }
}

/******************************************************************************
 * FunctionName : devicexx_key_super_press
 * Description  : key's super press function, needed to be installed
 * Parameters   : bool key_up
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_key_super_press(bool key_up)
{
    if (key_up)
    {
        VOWSTAR_IO_DEBUG("Key super pressed\r\n");
//        // Unbind and restore system
//        VOWSTAR_IO_DEBUG("Unbind device and recovery system ...\r\n");
//        // unbind device-user relationship
//        devicexx_system_recovery();
//        VOWSTAR_IO_DEBUG("System reboot ...\r\n");
//        system_restart();
    }
    else
    {
        VOWSTAR_IO_DEBUG("Key super pressing.\r\n");
        devicexx_io_set_led_state(5);
    }
}


/******************************************************************************
 * FunctionName : devicexx_relay_init
 * Description  : init relay gpio
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
//LOCAL void ICACHE_FLASH_ATTR
//devicexx_relay_init(void)
//{
//    PIN_FUNC_SELECT(pin_name[PIN_RELAY], pin_func[PIN_RELAY]);
//    GPIO_OUTPUT_SET(PIN_RELAY, 0);
//}

/******************************************************************************
 * FunctionName : devicexx_led_init
 * Description  : init led gpio
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
devicexx_led_init(void)
{
//    PIN_FUNC_SELECT(pin_name[PIN_LED], pin_func[PIN_LED]);
    PIN_FUNC_SELECT(pin_name[PIN_LED_S], pin_func[PIN_LED_S]);
    PIN_FUNC_SELECT(pin_name[PIN_GPS_S], pin_func[PIN_GPS_S]);
    PIN_FUNC_SELECT(pin_name[PIN_POWER], pin_func[PIN_POWER]);
//    GPIO_OUTPUT_SET(PIN_LED, 1);
    GPIO_OUTPUT_SET(PIN_LED_S, 0);
    GPIO_OUTPUT_SET(PIN_GPS_S, 1);
    GPIO_OUTPUT_SET(PIN_POWER, 1);

}

/******************************************************************************
 * FunctionName : devicexx_key_init
 * Description  : init key gpio and function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
//LOCAL void ICACHE_FLASH_ATTR
//devicexx_key_init(void)
//{
//    single_key[0] = key_init_single(PIN_KEY, devicexx_key_short_press, devicexx_key_medium_press, devicexx_key_long_press, devicexx_key_extra_press, devicexx_key_super_press);
//    keys.key_num = KEY_COUNT;
//    keys.single_key = single_key;
//    key_init(&keys);
//}

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
//    devicexx_relay_init();
//    devicexx_key_init();
    devicexx_io_led_timer_tick(1);
}
