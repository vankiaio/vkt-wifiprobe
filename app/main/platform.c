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
os_timer_t timer_90s;


void ICACHE_FLASH_ATTR
delay_power_on()
{
    os_timer_disarm(&timer_1S);
    os_timer_setfn(&timer_1S, (os_timer_func_t *)power_on, NULL);
    os_timer_arm(&timer_1S, 1000, 0);

    GPIO_OUTPUT_SET(PIN_POWER, 0);

    os_printf("1s To Power On\n");

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


    os_timer_disarm(&timer_90s);//20S没有收到数据，重新上电
    os_timer_setfn(&timer_90s, (os_timer_func_t *)delay_power_on, NULL);


    os_timer_disarm(&timer_3S);
    os_timer_setfn(&timer_3S, (os_timer_func_t *)delay_power_on, NULL);
//    os_timer_arm(&timer_3S, 3000, 0);//开机
//    os_printf("wait... 3s\n");
    gnrmc_gps_flag = 0;
//    send_flag = 0;
}
