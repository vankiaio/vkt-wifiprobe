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
#include "tcpclient.h"
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

os_timer_t timer_4S;
os_timer_t restart_nb;
os_timer_t check_id_timer;
os_timer_t timer_300s;

os_timer_t timer_wait_con_wifi;


os_timer_t F6x8_timer;
os_timer_t qingzhu_timer;
os_timer_t vankia_timer;

os_timer_t delay_discon_timer;
os_timer_t delay_update_timer;
os_timer_t scan_timer;
os_timer_t wait_nb_con_timer;

uint8_t ap_black_list[256];
struct scan_config config = { NULL, NULL, 0, 0, 0, 100};
uint8_t *  ap_str = "000000000000,00,2400;000000000000,00,2400;000000000000,00,2400;000000000000,00,2400;000000000000,00,2400;000000000000,00,2400;";
struct scan_inf {
    uint8_t ap_mac_pond[128][6];
    uint8 rssi[128];
    uint8 channel[128];
}scan_inf;


void ICACHE_FLASH_ATTR
delay_power_on()
{
    os_timer_disarm(&timer_1S);
    os_timer_setfn(&timer_1S, (os_timer_func_t *)power_on, NULL);
    os_timer_arm(&timer_1S, 1000, 0);

    GPIO_OUTPUT_SET(PIN_POWER, 0);

    os_printf("1s To Power On\n");

}

//void ICACHE_FLASH_ATTR
//vankia_logo()
//{
//    OLED_CLS();
//  //    显示位图，自左上角0,0，位置分辨率点起，至第128个分辨率点、第8行的右下角，显示图片“”的位图数据
//    OLED_DrawBMP(0, 0, 128, 8, vankia_BMP);
//
//    os_timer_arm(&qingzhu_timer, 1000, 0);
//}
//
//void ICACHE_FLASH_ATTR
//qingzhu_logo()
//{
//    OLED_CLS();
//  //    显示位图，自左上角0,0，位置分辨率点起，至第128个分辨率点、第8行的右下角，显示图片“”的位图数据
//    OLED_DrawBMP(0, 0, 128, 8, qingzhu_BMP);
//
//    os_timer_arm(&F6x8_timer, 1000, 0);
//}
//
//
//
//void ICACHE_FLASH_ATTR
//F6x8_logo()
//{
//    OLED_CLS();
//
//    OLED_ShowStr(0, 0, "1234567890abcdef", 2);
//    OLED_ShowStr(0, 2, "gklmnopqrstuvwxy", 2);
//    OLED_ShowStr(0, 4, "z!@#$%^&*()_+|/.", 2);
//    OLED_ShowStr(0, 6, "ABCDEFGHIGKLMNOP", 2);
//
//    os_timer_arm(&vankia_timer, 3000, 0);
//}



void ICACHE_FLASH_ATTR
switch_to_wifi()
{
    uint8_t *ssid="qingzhu";
    uint8_t *pwd = "12345678";
    os_strcpy(stationConf.ssid, ssid);
    os_strcpy(stationConf.password, pwd);
    os_printf(" ssid %s\n password %s\n",stationConf.ssid,stationConf.password);
//    wifi_station_set_config_current(&stationConf);  //设置WiFi station接口配置，不保存到 flash
//    注意不能使用上一句，我出现连接不上路由器情况

    wifi_station_set_config(&stationConf);  //设置WiFi station接口配置，并保存到 flash
//    wifi_set_opmode(STATION_MODE);  //设置为STATION模式
    wifi_station_connect(); //连接路由器

    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
    os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState, NULL);   //设置定时器回调函数
    os_timer_arm(&checkTimer_wifistate, 1000, 1);   //启动定时器，单位：毫秒

}

void ICACHE_FLASH_ATTR
wait_con_wifi(void)
{
    ap_str_ascii_str(ap_str);
}

static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{

    os_printf("wifi status %d\r\n",wifi_station_get_connect_status());
    post_state = AP_MAC;
    uint8_t i=0,j=0,sum=0;
    uint8_t temp_pond[8],temp_rssi=99;
    uint8_t temp_apstr[114];
    char temp[128];
    uint8 tem_ssid[33];
    if (status == OK)
    {
        j=0;
        struct bss_info *bss_link = (struct bss_info *)arg;
        os_printf("status:%d pointer:%p\n", status, arg);
        while (bss_link != NULL)
        {
            scan_inf.rssi[j] = bss_link->rssi*(-1);
            scan_inf.channel[j] = bss_link->channel;
            for(i=0;i<6;i++)
                scan_inf.ap_mac_pond[j][i] = bss_link->bssid[i];

            if(os_strstr(bss_link->ssid,"qingzhu"))
            {
                os_printf("%s find\n",bss_link->ssid);
                scan_qz = 1;

            }

            bss_link = bss_link->next.stqe_next;
            j++;


//            os_memset(tem_ssid, 0, 33);
//            if (os_strlen(bss_link->ssid) <= 32)
//            {
//              os_memcpy(tem_ssid, bss_link->ssid, os_strlen(bss_link->ssid));
//            }
//            else
//            {
//              os_memcpy(tem_ssid, bss_link->ssid, 32);
//            }
//
//            os_sprintf(temp,"+CWLAP:(%d,\"%s\",%d,\""MACSTR"\",%d)\r\n",
//                       bss_link->authmode, tem_ssid, bss_link->rssi,
//                       MAC2STR(bss_link->bssid),bss_link->channel);
//            os_printf("%s",temp);
        }
        sum = j;



//        for(i=0;i<j;i++)
//        {
//            os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
//                       scan_inf.ap_mac_pond[i][0], scan_inf.ap_mac_pond[i][1], scan_inf.ap_mac_pond[i][2],
//                       scan_inf.ap_mac_pond[i][3], scan_inf.ap_mac_pond[i][4], scan_inf.ap_mac_pond[i][5],
//                       scan_inf.rssi[i],scan_inf.channel[i] );
//        }
    }

    for(i=0;i<sum-1;++i)//n个数,总共需要进行n-1次
    {                 //n-1个数排完,第一个数一定已经归位
        //每次会将最大(升序)或最小(降序)放到最后面

        for(j=0;j<sum-i-1;++j)
        {
            if(scan_inf.rssi[j]>scan_inf.rssi[j+1])
            {
                temp_rssi=scan_inf.rssi[j];
                temp_pond[0] = scan_inf.ap_mac_pond[j][0];
                temp_pond[1] = scan_inf.ap_mac_pond[j][1];
                temp_pond[2] = scan_inf.ap_mac_pond[j][2];
                temp_pond[3] = scan_inf.ap_mac_pond[j][3];
                temp_pond[4] = scan_inf.ap_mac_pond[j][4];
                temp_pond[5] = scan_inf.ap_mac_pond[j][5];
                temp_pond[7] = scan_inf.channel[j];

                scan_inf.rssi[j]=scan_inf.rssi[j+1];
                scan_inf.ap_mac_pond[j][0] = scan_inf.ap_mac_pond[j+1][0];
                scan_inf.ap_mac_pond[j][1] = scan_inf.ap_mac_pond[j+1][1];
                scan_inf.ap_mac_pond[j][2] = scan_inf.ap_mac_pond[j+1][2];
                scan_inf.ap_mac_pond[j][3] = scan_inf.ap_mac_pond[j+1][3];
                scan_inf.ap_mac_pond[j][4] = scan_inf.ap_mac_pond[j+1][4];
                scan_inf.ap_mac_pond[j][5] = scan_inf.ap_mac_pond[j+1][5];
                scan_inf.channel[j] = scan_inf.channel[j+1];


                scan_inf.rssi[j+1]=temp_rssi;
                scan_inf.ap_mac_pond[j+1][0] = temp_pond[0];
                scan_inf.ap_mac_pond[j+1][1] = temp_pond[1];
                scan_inf.ap_mac_pond[j+1][2] = temp_pond[2];
                scan_inf.ap_mac_pond[j+1][3] = temp_pond[3];
                scan_inf.ap_mac_pond[j+1][4] = temp_pond[4];
                scan_inf.ap_mac_pond[j+1][5] = temp_pond[5];
                scan_inf.channel[j+1] = temp_pond[7];

            }
        }
    }

//    for(i=0;i<sum;i++)
//    {
//        os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
//                   scan_inf.ap_mac_pond[i][0], scan_inf.ap_mac_pond[i][1], scan_inf.ap_mac_pond[i][2],
//                   scan_inf.ap_mac_pond[i][3], scan_inf.ap_mac_pond[i][4], scan_inf.ap_mac_pond[i][5],
//                   scan_inf.rssi[i],scan_inf.channel[i] );
//    }

    for ( i = 0; i < 3; i++ )
    {
        for ( j = 0; j < sum; j++ )
        {
            if (( scan_inf.ap_mac_pond[j][0] == apmac_rssi[i][0])&&
                 (scan_inf.ap_mac_pond[j][1] == apmac_rssi[i][1])&&
                 (scan_inf.ap_mac_pond[j][2] == apmac_rssi[i][2])&&
                 (scan_inf.ap_mac_pond[j][3] == apmac_rssi[i][3])&&
                 (scan_inf.ap_mac_pond[j][4] == apmac_rssi[i][4])&&
                 (scan_inf.ap_mac_pond[j][5] == apmac_rssi[i][5]) )
            {
//                apmac_rssi[i][6] = scan_inf.rssi[j];
                os_printf("i=%d\n",i);

                temp_pond[0] =  scan_inf.ap_mac_pond[j][0];
                temp_pond[1] =  scan_inf.ap_mac_pond[j][1];
                temp_pond[2] =  scan_inf.ap_mac_pond[j][2];
                temp_pond[3] =  scan_inf.ap_mac_pond[j][3];
                temp_pond[4] =  scan_inf.ap_mac_pond[j][4];
                temp_pond[5] =  scan_inf.ap_mac_pond[j][5];
                temp_pond[6] =  scan_inf.rssi[j];
                temp_pond[7] =  scan_inf.channel[j];


                scan_inf.ap_mac_pond[j][0] = scan_inf.ap_mac_pond[i][0];
                scan_inf.ap_mac_pond[j][1] = scan_inf.ap_mac_pond[i][1];
                scan_inf.ap_mac_pond[j][2] = scan_inf.ap_mac_pond[i][2];
                scan_inf.ap_mac_pond[j][3] = scan_inf.ap_mac_pond[i][3];
                scan_inf.ap_mac_pond[j][4] = scan_inf.ap_mac_pond[i][4];
                scan_inf.ap_mac_pond[j][5] = scan_inf.ap_mac_pond[i][5];
                scan_inf.rssi[j]    = scan_inf.rssi[i];
                scan_inf.channel[j] = scan_inf.channel[i];

                scan_inf.ap_mac_pond[i][0] = temp_pond[0];
                scan_inf.ap_mac_pond[i][1] = temp_pond[1];
                scan_inf.ap_mac_pond[i][2] = temp_pond[2];
                scan_inf.ap_mac_pond[i][3] = temp_pond[3];
                scan_inf.ap_mac_pond[i][4] = temp_pond[4];
                scan_inf.ap_mac_pond[i][5] = temp_pond[5];
                scan_inf.rssi[i]    = temp_pond[6];
                scan_inf.channel[i] = temp_pond[7];

                break;
            }
        }
    }

//    for(i=0;i<sum;i++)
//    {
//    os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
//               scan_inf.ap_mac_pond[i][0], scan_inf.ap_mac_pond[i][1], scan_inf.ap_mac_pond[i][2],
//               scan_inf.ap_mac_pond[i][3], scan_inf.ap_mac_pond[i][4], scan_inf.ap_mac_pond[i][5],
//               scan_inf.rssi[i],scan_inf.channel[i] );
//    }
    for(i=0;i<6;i++)
    {
        apmac_rssi[i][0] = scan_inf.ap_mac_pond[i][0];
        apmac_rssi[i][1] = scan_inf.ap_mac_pond[i][1];
        apmac_rssi[i][2] = scan_inf.ap_mac_pond[i][2];
        apmac_rssi[i][3] = scan_inf.ap_mac_pond[i][3];
        apmac_rssi[i][4] = scan_inf.ap_mac_pond[i][4];
        apmac_rssi[i][5] = scan_inf.ap_mac_pond[i][5];
        apmac_rssi[i][6] = scan_inf.rssi[i];
        apmac_rssi[i][7] = scan_inf.channel[i];

        os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
                   apmac_rssi[i][0], apmac_rssi[i][1], apmac_rssi[i][2],
                   apmac_rssi[i][3], apmac_rssi[i][4], apmac_rssi[i][5],
                   apmac_rssi[i][6],apmac_rssi[i][7] );
//        os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
//                   scan_inf.ap_mac_pond[i][0], scan_inf.ap_mac_pond[i][1], scan_inf.ap_mac_pond[i][2],
//                   scan_inf.ap_mac_pond[i][3], scan_inf.ap_mac_pond[i][4], scan_inf.ap_mac_pond[i][5],
//                   scan_inf.rssi[i],scan_inf.channel[i] );
    }



    for ( j = 0; j < 6; j++ )
    {
        for ( i = 0; i < 12; i++ )
        {
            temp_apstr[i+j*18]      = (apmac_rssi[j][i/2] >> 4) & 0xf;
            temp_apstr[i+1+j*18]    =  apmac_rssi[j][i/2] & 0xf;
            i++;
        }

        temp_apstr[i+j*18]      =  apmac_rssi[j][i/2]/10;
        temp_apstr[i+1+j*18]    =  apmac_rssi[j][i/2]%10;
        i=i+2;
        temp_apstr[i+j*18]   =  2;//手动添加2和4
        temp_apstr[i+1+j*18] =  4;
        i=i+2;                   //跨过2和4
        temp_apstr[i+j*18]   =  (apmac_rssi[j][(i-2)/2]+1)/2;
        temp_apstr[i+1+j*18] = ((apmac_rssi[j][(i-2)/2]+1)%2)*5+2;

    }




    for ( i = 0; i < 114; i++ )
    {
        if ( temp_apstr[i] < 10 )
            temp_apstr[i] += 48;
        else
            temp_apstr[i] += 55;
    }

    for ( i = 0; i < 6; i++ )
    {
        os_memcpy( ap_str+  i*21, temp_apstr+i*18, 12);
        os_memcpy( ap_str+13+i*21, temp_apstr+12+i*18, 2);
        os_memcpy( ap_str+18+i*21, temp_apstr+16+i*18, 2);
    }

    os_printf( "mac string %s\n", ap_str );
    if(scan_qz == 1)
    {
        if(STATION_GOT_IP != wifi_station_get_connect_status()){
            switch_to_wifi();

            os_timer_disarm(&restart_nb);
        }
    }

    os_timer_arm(&timer_wait_con_wifi, 15000, 0);
}





void ICACHE_FLASH_ATTR
get_device_id()
{
    uint8_t src[6], i, mac_address_str[12];
    if (wifi_get_macaddr(STATION_IF, src))//获取mac地址
    {
        for(i=0; i< 12; i++)
        {
            mac_address_str[i]   = (src[i/2] >> 4) & 0xf;
            mac_address_str[i+1] =  src[i/2] & 0xf;
            i++;
        }

        for(i=0; i< 12; i++){
            if( mac_address_str[i] < 10)
                mac_address_str[i] += 48;
            else
                mac_address_str[i] += 55;
        }
        os_memcpy(parameter_deviceId + 10, mac_address_str, 12);

        os_printf("deviceid %s\n",parameter_deviceId);
        strhex_to_str(parameter_deviceId,1);


    }else system_restart();
}


void ICACHE_FLASH_ATTR
wifi_scan()
{
    wifi_station_scan(&config, scan_done);
}

void ICACHE_FLASH_ATTR
erase_sector(void)
{
    uint16 i;
    if(0)
        for(i=0;i<505;i++)
            spi_flash_erase_sector(i+513);
}

void ICACHE_FLASH_ATTR
platform_init(void)
{
	devicexx_app_load();
	devicexx_app_apply_settings();
	gpio16_output_conf();
	gpio16_output_set(1);

	erase_sector();

	devicexx_io_init();
	get_device_id();
	update_timestamp();

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



    os_timer_disarm(&restart_nb);//20S没有收到数据，重新上电
    os_timer_setfn(&restart_nb, (os_timer_func_t *)delay_power_on, NULL);



    gnrmc_gps_flag = 0;

//    wifi_set_opmode(STATION_MODE);


    os_timer_disarm(&timer_wait_con_wifi);
    os_timer_setfn(&timer_wait_con_wifi, (os_timer_func_t *)wait_con_wifi, NULL);

    os_timer_disarm(&scan_timer);
    os_timer_setfn(&scan_timer, (os_timer_func_t *)wifi_scan, NULL);

    os_timer_disarm(&delay_update_timer);
    os_timer_setfn(&delay_update_timer, (os_timer_func_t *)update_data, NULL);

    os_timer_disarm(&delay_discon_timer);
    os_timer_setfn(&delay_discon_timer, (os_timer_func_t *)http_disc, NULL);
//    {//test
//    uint8 * uart_receive_at = "+CESQ: 99,6,255,255,12,45";
//
//    if(os_strstr(uart_receive_at,"+CESQ"))
//    {
//
//        uint8_t * addr1 = NULL;
//        uint8_t * addr2 = NULL;
//
//        addr1 = strstr(uart_receive_at,":");
//        addr2 = strstr(uart_receive_at,",");
//
//        os_printf("len=%c\n",*(addr2-2));
//    }
//    }
//    sniffer_init();
//    sniffer_init_in_system_init_done();
//    wifi_scan();
    post_state = CHECK_ID;
    os_timer_disarm(&wait_nb_con_timer);
    os_timer_setfn(&wait_nb_con_timer, (os_timer_func_t *)switch_to_wifi, NULL);
    os_timer_arm(&wait_nb_con_timer, 60000, 0);

    get_gps = 0;

}
