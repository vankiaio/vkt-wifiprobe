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
#include "c_types.h"
#include "user_config.h"
#include "user_interface.h"
#include "platform.h"
#include "uart.h"
#include "i2c_oled.h"
#include "mem.h"
#include "gpio16.h"
#include "espconn.h"
#include "espnow_app.h"
#include "devicexx.h"
#include "devicexx_io.h"
#include "devicexx_app.h"
#include "platform.h"
#include "i2c_oled.h"
#include "i2c_oled_fonts.h"

#include "sniffer.h"

os_timer_t timer_1S;
os_timer_t timer_3S;
os_timer_t timer_4S;
os_timer_t restart_nb;
os_timer_t check_id_timer;
os_timer_t timer_300s;


os_timer_t F6x8_timer;
os_timer_t qingzhu_timer;
os_timer_t vankia_timer;

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
vankia_logo()
{
    OLED_CLS();
  //    显示位图，自左上角0,0，位置分辨率点起，至第128个分辨率点、第8行的右下角，显示图片“”的位图数据
    OLED_DrawBMP(0, 0, 128, 8, vankia_BMP);

    os_timer_arm(&qingzhu_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR
qingzhu_logo()
{
    OLED_CLS();
  //    显示位图，自左上角0,0，位置分辨率点起，至第128个分辨率点、第8行的右下角，显示图片“”的位图数据
    OLED_DrawBMP(0, 0, 128, 8, qingzhu_BMP);

    os_timer_arm(&F6x8_timer, 1000, 0);
}



void ICACHE_FLASH_ATTR
F6x8_logo()
{
    OLED_CLS();

    OLED_ShowStr(0, 0, "1234567890abcdef", 2);
    OLED_ShowStr(0, 2, "gklmnopqrstuvwxy", 2);
    OLED_ShowStr(0, 4, "z!@#$%^&*()_+|/.", 2);
    OLED_ShowStr(0, 6, "ABCDEFGHIGKLMNOP", 2);

    os_timer_arm(&vankia_timer, 3000, 0);
}


void ICACHE_FLASH_ATTR
platform_init(void)
{
	devicexx_app_load();
	devicexx_app_apply_settings();
	gpio16_output_conf();
	gpio16_output_set(1);

	devicexx_io_init();


#ifdef OLED_VERSION
    i2c_master_gpio_init();
    oled_init();

    os_timer_setfn(&qingzhu_timer, (os_timer_func_t *)qingzhu_logo, NULL);
    os_timer_arm(&qingzhu_timer, 1000, 0);
    os_timer_disarm(&qingzhu_timer);


    os_timer_setfn(&vankia_timer, (os_timer_func_t *)vankia_logo, NULL);
    os_timer_arm(&vankia_timer, 1000, 0);
    os_timer_disarm(&vankia_timer);


    os_timer_setfn(&F6x8_timer, (os_timer_func_t *)F6x8_logo, NULL);
    os_timer_arm(&F6x8_timer, 1000, 0);
    os_timer_disarm(&F6x8_timer);

    os_timer_arm(&qingzhu_timer, 1000, 0);
#endif

    wifi_promiscuous_enable(0);

    os_timer_disarm(&check_id_timer);
    os_timer_setfn(&check_id_timer, (os_timer_func_t *)check_id, NULL);

//    os_timer_disarm(&timer_300s);
//    os_timer_setfn(&timer_300s, (os_timer_func_t *)check_gps, NULL);

    os_timer_disarm(&restart_nb);//20S没有收到数据，重新上电
    os_timer_setfn(&restart_nb, (os_timer_func_t *)delay_power_on, NULL);


    os_timer_disarm(&timer_3S);
    os_timer_setfn(&timer_3S, (os_timer_func_t *)delay_power_on, NULL);
//    os_timer_arm(&timer_3S, 3000, 0);//开机
//    os_printf("wait... 3s\n");
    gnrmc_gps_flag = 0;
//    send_flag = 0;


//    sniffer_init();
//    sniffer_init_in_system_init_done();
}
