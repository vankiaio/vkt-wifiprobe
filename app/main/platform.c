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

os_timer_t delay_discon_timer;
os_timer_t delay_update_timer;
os_timer_t scan_timer;
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

static void ICACHE_FLASH_ATTR
scan_done(void *arg, STATUS status)
{

    uint8_t i=0,j=0,sum=0;
    uint8_t temp_pond[8],temp_rssi=99;
    uint8_t temp_apstr[114];
//    char temp[128];
//    uint8 tem_ssid[33];
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


    ap_str_ascii_str(ap_str);

}

void ICACHE_FLASH_ATTR
wifi_scan()
{
    wifi_station_scan(&config, scan_done);
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

    wifi_set_opmode(STATION_MODE);

    os_timer_disarm(&scan_timer);
    os_timer_setfn(&scan_timer, (os_timer_func_t *)wifi_scan, NULL);

    os_timer_disarm(&delay_update_timer);
    os_timer_setfn(&delay_update_timer, (os_timer_func_t *)update_data, NULL);

    os_timer_disarm(&delay_discon_timer);
    os_timer_setfn(&delay_discon_timer, (os_timer_func_t *)http_disc, NULL);
//    {//test
//        uint16_t i;
//        char * fid_addr = NULL;
//        uint8_t http_get_tag1 [] = "AT+EHTTPSEND=0,312,312,\"0,1,21,\"/MacGather/getCollect\",0,,16,\"application/json\",233,7b226465766963654964223a2251696e677a68752d5651303030303030303030303030222c2276657273696f6e223a22302e303030222c22776569223a22303030302e3030303030222c226c6e67223a2230303030302e3030303030222c2274696d65223a22303030303030303030303030227d,\"\r\n";
//            //03 46 39 11 01 19
//    	for(i=0;i<os_strlen(http_get_tag1);i++)
//    	{
//    		os_printf("http_get_tag1[%3d]=%c\n",i ,http_get_tag1[i]);
//    	}
//    }
//    sniffer_init();
//    sniffer_init_in_system_init_done();
    get_gps = 0;

}
