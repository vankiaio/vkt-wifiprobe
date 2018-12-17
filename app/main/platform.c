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
#include "upgrade.h"
#include "tcpclient.h"
#include "sniffer.h"

os_timer_t timer_1S;
os_timer_t timer_3S;
os_timer_t timer_4S;


LOCAL void ICACHE_FLASH_ATTR
devicexx_state_changed_callback(devicexx_state_t state)
{
	if (devicexx_state() != state)
		PLATFORM_DEBUG("Platform: devicexx_state error \r\n");

	switch (state) {
	case WIFI_IDLE:
		PLATFORM_DEBUG("Platform: WIFI_IDLE\r\n");
		break;
	case WIFI_SMARTLINK_START:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_START\r\n");
		devicexx_io_set_led_state(3);
		break;

	case WIFI_SMARTLINK_LINKING:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_LINKING\r\n");
		break;
	case WIFI_SMARTLINK_FINDING:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_FINDING\r\n");
		break;
	case WIFI_SMARTLINK_TIMEOUT:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_TIMEOUT\r\n");
		// devicexx_force_aplink();
		// devicexx_io_set_led_state(4);
		break;
	case WIFI_SMARTLINK_GETTING:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_GETTING\r\n");
		break;
	case WIFI_SMARTLINK_OK:
		PLATFORM_DEBUG("Platform: WIFI_SMARTLINK_OK\r\n");
		break;
	case WIFI_AP_OK:
		PLATFORM_DEBUG("Platform: WIFI_AP_OK\r\n");
		break;
	case WIFI_AP_ERROR:
		PLATFORM_DEBUG("Platform: WIFI_AP_ERROR\r\n");
		break;
	case WIFI_AP_STATION_OK:
		PLATFORM_DEBUG("Platform: WIFI_AP_STATION_OK\r\n");
		break;
	case WIFI_AP_STATION_ERROR:
		PLATFORM_DEBUG("Platform: WIFI_AP_STATION_ERROR\r\n");
		break;
	case WIFI_STATION_OK:
		PLATFORM_DEBUG("Platform: WIFI_STATION_OK\r\n");
		break;
	case WIFI_STATION_ERROR:
		PLATFORM_DEBUG("Platform: WIFI_STATION_ERROR\r\n");
		break;
	case DEVICEXX_CONNECTING:
		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTING\r\n");
		break;
	case DEVICEXX_CONNECTING_ERROR:
		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTING_ERROR\r\n");
		break;
	case DEVICEXX_CONNECTED:
		PLATFORM_DEBUG("Platform: DEVICEXX_CONNECTED \r\n");
		devicexx_io_set_led_state(devicexx_io_get_relay_state());
		break;
	case DEVICEXX_DISCONNECTED:
		PLATFORM_DEBUG("Platform: DEVICEXX_DISCONNECTED\r\n");
		break;
	default:
		break;
	}
}
void ICACHE_FLASH_ATTR
ota_finished_callback(void *arg)
{
    struct upgrade_server_info *update = arg;
        if (update->upgrade_flag == true)
        {
            os_printf("OTA  Success ! rebooting!\n");
            system_upgrade_reboot();
        }else
        {
            os_printf("OTA failed!\n");
//            upgrade_tcp = 0;
            Check_WifiState();

//            os_timer_arm(&timer_3S, 3000, 0);//����
        }
}

void ICACHE_FLASH_ATTR
ota_start_upgrade(const char *server_ip, uint16_t port,const char *path)
//ota_start_upgrade(uint16_t port,const char *path)
{
    const char *file;
    //��ȡϵͳ��Ŀǰ���ص����ĸ�bin�ļ�
    uint8_t userBin = system_upgrade_userbin_check();

    switch (userBin) {

    //�����鵱ǰ���Ǵ���user1�ļ����ļ�����ô��ȡ�ľ���user2.bin
    case UPGRADE_FW_BIN1:
        file = "user2.4096.new.6.bin";
        break;

        //�����鵱ǰ���Ǵ���user2�ļ����ļ�����ô��ȡ�ľ���user1.bin
    case UPGRADE_FW_BIN2:
        file = "user1.4096.new.6.bin";
        break;

        //�����鶼���ǣ����ܴ˿̲���OTA��bin�̼�
    default:
        os_printf("Fail read system_upgrade_userbin_check! \n\n");
        return;
    }

    struct upgrade_server_info* update =
            (struct upgrade_server_info *) os_zalloc(sizeof(struct upgrade_server_info));
    update->pespconn = (struct espconn *) os_zalloc(sizeof(struct espconn));
    //���÷�������ַ
    os_memcpy(update->ip, server_ip, 4);
    //���÷������˿�
    update->port = port;
    //����OTA�ص�����
    update->check_cb = ota_finished_callback;
    //���ö�ʱ�ص�ʱ��
    update->check_times = 20000;
    //�� 4M *1024 =4096�����ڴ�
    update->url = (uint8 *)os_zalloc(4096);

    //��ӡ��Ո���ַ
//    os_printf("Http Server Address:%d.%d.%d.%d ,port: %d,filePath: %s,fileName: %s \n",
//    		IP2STR(update->ip), update->port, path, file);

    //ƴ�������� URLȥ���������
//    os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: "IPSTR":%d\r\n"	"Connection: keep-alive\r\n" "\r\n",
//			               path, file, IP2STR(update->ip), update->port);

    os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: %s:%d\r\n"	"Connection: keep-alive\r\n" "\r\n",
			               path, file, update_host, update->port );


    os_printf("url %s\n",update->url);

    if (system_upgrade_start(update) == false) {
        os_printf(" Could not start upgrade\n");
        //�ͷ���Դ
        os_free(update->pespconn);
        os_free(update->url);
        os_free(update);
    } else {
        os_printf(" Upgrading...\n");
    }
}



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


    os_timer_disarm(&temer_10s);//20Sû���յ����ݣ������ϵ�
    os_timer_setfn(&temer_10s, (os_timer_func_t *)delay_power_on, NULL);


    os_timer_disarm(&timer_3S);
    os_timer_setfn(&timer_3S, (os_timer_func_t *)delay_power_on, NULL);
    os_timer_arm(&timer_3S, 3000, 0);//����
    os_printf("wait... 3s\n");
    sniffer_flag = 0;
}
