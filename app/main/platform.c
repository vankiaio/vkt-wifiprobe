/******************************************************************************
 * Copyright 2015 Vowstar (Shenzhen) Co.,Ltd.
 *
 * FileName: platform.c
 *
 * Description: Platform application.
 *
 * Modification history:
 *     2015/05/08, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "user_config.h"
#include "user_interface.h"
#include "platform.h"
#include "uart.h"
#include "mem.h"
#include "gpio16.h"
#include "espconn.h"
#include "espnow_app.h"
#include "devicexx.h"
#include "devicexx_io.h"
#include "devicexx_app.h"
#include "platform.h"

#include "sniffer.h"

os_timer_t timer_1S;
os_timer_t timer_3S;
os_timer_t timer_4S;

//
//LOCAL void ICACHE_FLASH_ATTR
//devicexx_state_changed_callback(devicexx_state_t state)
//{
//	if (devicexx_state() != state)
//		PLATFORM_DEBUG("Platform: devicexx_state error \r\n");
//
//	switch (state) {
//	case WIFI_IDLE:
//		PLATFORM_DEBUG("Platform: WIFI_IDLE\r\n");
//		break;
//	case WIFI_SMARTLINK_START:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_START\r\n");
//		devicexx_io_set_led_state(3);
//		break;
//
//	case WIFI_SMARTLINK_LINKING:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_LINKING\r\n");
//		break;
//	case WIFI_SMARTLINK_FINDING:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_FINDING\r\n");
//		break;
//	case WIFI_SMARTLINK_TIMEOUT:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_TIMEOUT\r\n");
//		// devicexx_force_aplink();
//		// devicexx_io_set_led_state(4);
//		break;
//	case WIFI_SMARTLINK_GETTING:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_GETTING\r\n");
//		break;
//	case WIFI_SMARTLINK_OK:
//		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_OK\r\n");
//		break;
//	case WIFI_AP_OK:
//		PLATFORM_DEBUG("Platform: WIFI_AP_OK\r\n");
//		break;
//	case WIFI_AP_ERROR:
//		PLATFORM_DEBUG("Platform: WIFI_AP_ERROR\r\n");
//		break;
//	case WIFI_AP_STATION_OK:
//		PLATFORM_DEBUG("Platform: WIFI_AP_STATION_OK\r\n");
//		break;
//	case WIFI_AP_STATION_ERROR:
//		PLATFORM_DEBUG("Platform: WIFI_AP_STATION_ERROR\r\n");
//		break;
//	case WIFI_STATION_OK:
//		PLATFORM_DEBUG("Platform: WIFI_STATION_OK\r\n");
//		break;
//	case WIFI_STATION_ERROR:
//		PLATFORM_DEBUG("Platform: WIFI_STATION_ERROR\r\n");
//		break;
//	case DEVICEXX_CONNECTING:
//		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTING\r\n");
//		break;
//	case DEVICEXX_CONNECTING_ERROR:
//		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTING_ERROR\r\n");
//		break;
//	case DEVICEXX_CONNECTED:
//		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTED \r\n");
//		devicexx_io_set_led_state(devicexx_io_get_relay_state());
//		break;
//	case DEVICEXX_DISCONNECTED:
//		PLATFORM_DEBUG("Platform: DEVICEXX_DISCONNECTED\r\n");
//		break;
//	default:
//		break;
//	}
//}



void ICACHE_FLASH_ATTR
delay_power_on()
{
    os_timer_disarm(&timer_1S);
    os_timer_setfn(&timer_1S, (os_timer_func_t *)power_on, NULL);
    os_timer_arm(&timer_1S, 1000, 0);

    GPIO_OUTPUT_SET(PIN_POWER, 0);
    GPIO_OUTPUT_SET(PIN_LED_S, 1);
    os_printf("1s To Power On\n");
    send_flag = 0;
}



void ICACHE_FLASH_ATTR
platform_init(void)
{
	devicexx_app_load();
	devicexx_app_apply_settings();
	gpio16_output_conf();
	gpio16_output_set(1);

	devicexx_io_init();


    wifi_promiscuous_enable(0);


    os_timer_disarm(&temer_10s);//20S没有收到数据，重新上电
    os_timer_setfn(&temer_10s, (os_timer_func_t *)delay_power_on, NULL);


    os_timer_disarm(&timer_3S);
    os_timer_setfn(&timer_3S, (os_timer_func_t *)delay_power_on, NULL);
    os_timer_arm(&timer_3S, 3000, 0);//开机
    os_printf("wait... 3s\n");
    sniffer_flag = 0;
}
