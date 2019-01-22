/******************************************************************************
 * Copyright 2015 Vowstar Co., Ltd.
 *
 * FileName: devicexx_app.c
 *
 * Description: Simulate MCU in ESP8266 kit.
 *
 * Modification history:
 *     2015/06/09, v1.0 create this file.
*******************************************************************************/
#include "user_config.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "devicexx.h"
#include "devicexx_io.h"
#include "queue_uart.h"
#include "devicexx_app.h"
#include "espconn.h"
#include "tcpclient.h"
#include "upgrade.h"
#include "httpclient.h"
#include "cJSON.h"
#include "platform.h"
#include "sniffer.h"
#include "espconn.h"


os_timer_t delay_2s;




#define VOWSTAR_WAN_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)

uint8_t ap_ssid[32] = "qingzhu";
uint8_t  ap_pwd[32] = "12345678";
uint8_t  update_host[128];

uint8_t loginName[] = "12345678901234567890123456789012";
uint8_t loginPwd[]  = "98765432109876543210987654321098";


uint8_t cclk[] = "AT+CCLK?\r\n";
uint8_t ipr[] = "AT+IPR=<115200>\r\n";
uint8_t at[] = "AT\r\n";
uint8_t read_adc[] = "AT+ZADC?\r\n";
uint8_t con_led[] = "AT+ZCONTLED=1\r\n";

uint8_t at_cfun[] = "AT+CFUN=0\r\n";
uint8_t e_power_off[] = "AT+ZTURNOFF\r\n";
//uint8_t rx_fifo[2048];

uint8_t parsing_ip[] = "IPV4:";

#if 0
uint8_t http_create[] = "AT+EHTTPCREATE=0,40,40,\"\"http://119.23.146.207:1380/\",,,0,,0,,0,\"\r\n";
#else
uint8_t http_create[] = "AT+EHTTPCREATE=0,41,41,\"\"http://221.122.119.226:8098/\",,,0,,0,,0,\"\r\n";
#endif
//uint8_t rev_http_create[] = "+EHTTPCREAT:0";
uint8_t http_con[] = "AT+EHTTPCON=0\r\n";

uint8_t http_post_bind [] = "AT+EHTTPSEND=0,312,312,\"0,1,12,\"/device/sign\",0,,16,\"application/json\",261,7B226465766963654964223A223741526A6F34713361634E6557544746336D52577358222C226C6F67696E4E616D65223A223132333435363738393031323334353637383930313233343536373839303132222C226C6F67696E507764223A223132333435363738393031323334353637383930313233343536373839303132227D,\"\r\n";


uint8_t http_get_tag [] = "AT+EHTTPSEND=0,137,137,\"0,0,117,\"/MacGather/submitValue?deviceId=Vankia-WP-112233445566&version=0.000&lng=00000.00000&wei=0000.00000&time=000000000000\",0,,0,,0,,\"\r\n";
//uint8_t http_get_tag [] = "AT+EHTTPSEND=0,137,137,"0,0,117,"/MacGather/submitValue?deviceId=Vankia-WP-112233445566&version=0.000&lng=11618.03931&wei=3958.99464&time=034639110119\",0,,0,,0,,\"\r\n";
//AT+EHTTPSEND=0,118,118,"0,0,99,"/MacGather/submitValue?deviceId=7ARjo4q3acNeWTGF3mRWsX&version=0.000&lng=00000.00000&lat=0000.00000",0,,0,,0,,"

uint8_t http_send0[] = "AT+EHTTPSEND=1,2973,448,\"0,1,22,\"/MacGather/submitValue\",0,,16,\"application/json\",2911,7B226465766963654964223A2256616E6B69612D57502D454346414243314139463234222C2274696D657374616D70223A2231382F31312F30312C31383A35343A3135222C22636F6C6C6563744964223A22434A303030303030303032222C226C6F6E676974756465223A223131362E333133323838222C226C61746974756465223A2233392E393930373631222C226D61635F737472223A223031303035453746464646412C3031303035453030303046432C4343423841383035363438342C\"\r\n";//388
uint8_t http_send1[] = "AT+EHTTPSEND=1,2973,494,\"3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C\"\r\n";
uint8_t http_send2[] = "AT+EHTTPSEND=1,2973,494,\"3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C\"\r\n";
uint8_t http_send3[] = "AT+EHTTPSEND=1,2973,494,\"3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C\"\r\n";
uint8_t http_send4[] = "AT+EHTTPSEND=1,2973,494,\"3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C\"\r\n";
uint8_t http_send5[] = "AT+EHTTPSEND=1,2973,494,\"3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C3131323233333434353536362C\"\r\n";
uint8_t http_send6[] = "AT+EHTTPSEND=0,2973,55,\"3131323233333434353536362C313132323333343435353636227d,\"\r\n";

uint8_t http_discon[] = "AT+EHTTPDISCON=0\r\n";
uint8_t http_destroy[] = "AT+EHTTPDESTROY=1\r\n";

uint8_t probe_flag = 0;
uint8_t creat_flag = 0;
//uint8_t send_flag = 0;
uint8_t gnrmc_gps_flag = 0;
at_state_t at_state = 0;
uint16_t z_adc = 0;
uint8_t shut_down_flag = 0;
uint8_t get_tag_flag = 0;
//uint8_t bind_flag = 0;
//uint8_t bound_flag = 1;


uint8_t zgmode[] = "AT+ZGMODE=2\r\n";
uint8_t zgnmea[] = "AT+ZGNMEA=2\r\n";
uint8_t zgrun[] = "AT+ZGRUN=2\r\n";
uint8_t zgstop[] = "AT+ZGRUN=0\r\n";

uint8_t zgdata[] = "AT+ZGDATA \r\n";



uint8_t parameter_deviceId[] = "Qingzhu-VQ112233445566";
//                              QZBData
//uint8_t                    = "vankia-01-68c63a8a2420";

//parameter_longitude=11618.00871;
//parameter_latitude=3958.98470;
//parameter_timestamp=034639110119;
uint8_t parameter_timestamp[] = "111111111111";//都是1,都为0会与tag重叠
//                              18/11/01,18:54:15


uint8_t parameter_tag[] = "00000000000";//CJ,JD
uint8_t parameter_longitude[] = "00000.00000";
//                              11618.03708
uint8_t parameter_latitude[] = "0000.00000";
//                             3958.98966
#define JSON_DEVICE_MAC "{\"deviceId\":\"%s\",\"timestamp\":\"%s\",\"collectId\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"mac_str\":\"%s\"}"

#define JSON_POST_BIND  "{\"deviceId\":\"%s\",\"loginName\":\"%s\",\"loginPwd\":\"%s\"}"



LOCAL system_status_t local_system_status = {
	.version_type = 0,
	.version_num = 0,
};



uint8_t uart_receive_at[2048];
uint8_t uart_fifo_flag = 0;



void ICACHE_FLASH_ATTR
ota_finished_callback(void *arg)
{
    struct upgrade_server_info *update = arg;
        if (update->upgrade_flag == true)
        {

        	devicexx_app_save();
            os_printf("OTA  Success ! rebooting!\n");

            system_upgrade_reboot();
        }else
        {
            os_printf("OTA failed!\n");
//            upgrade_tcp = 0;
            Check_WifiState();

//            os_timer_arm(&timer_3S, 3000, 0);//开机
        }
}

void ICACHE_FLASH_ATTR
ota_start_upgrade(const char *server_ip, uint16_t port,const char *path)
//ota_start_upgrade(uint16_t port,const char *path)
{
    char file[16] = "user0.0.000.bin";
    //获取系统的目前加载的是哪个bin文件
    uint8_t userBin = system_upgrade_userbin_check();

    switch (userBin) {

    //如果检查当前的是处于user1的加载文件，那么拉取的就是user2.bin
    case UPGRADE_FW_BIN1:
        file[4] = '2';
        break;

        //如果检查当前的是处于user2的加载文件，那么拉取的就是user1.bin
    case UPGRADE_FW_BIN2:
    	file[4] = '1';
        break;

        //如果检查都不是，可能此刻不是OTA的bin固件
    default:
        os_printf("Fail read system_upgrade_userbin_check! \n\n");
        return;
    }


    file[6] = local_system_status.version_type + 48;
    file[8] = local_system_status.version_num/100 + 48;
    file[9] = local_system_status.version_num%100/10 + 48;
    file[10] = local_system_status.version_num%100%10 + 48;

    struct upgrade_server_info* update =
            (struct upgrade_server_info *) os_zalloc(sizeof(struct upgrade_server_info));
    update->pespconn = (struct espconn *) os_zalloc(sizeof(struct espconn));
    //设置服务器地址
    os_memcpy(update->ip, server_ip, 4);
    //设置服务器端口
    update->port = port;
    //设置OTA回调函数
    update->check_cb = ota_finished_callback;
    //设置定时回调时间
    update->check_times = 30000;
    //从 4M *1024 =4096申请内存
    update->url = (uint8 *)os_zalloc(4096);

    //打印下請求地址
//    os_printf("Http Server Address:%d.%d.%d.%d ,port: %d,filePath: %s,fileName: %s \n",
//    		IP2STR(update->ip), update->port, path, file);

    //拼接完整的 URL去请求服务器
//    os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: "IPSTR":%d\r\n"	"Connection: keep-alive\r\n" "\r\n",
//			               path, file, IP2STR(update->ip), update->port);

    os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: %s:%d\r\n"	"Connection: keep-alive\r\n" "\r\n",
			               path, file, update_host, update->port );


    os_printf("url %s\n",update->url);

    if (system_upgrade_start(update) == false) {
        os_printf(" Could not start upgrade\n");
        //释放资源
        os_free(update->pespconn);
        os_free(update->url);
        os_free(update);
    } else {
        os_printf(" Upgrading...\n");
    }
}



void ICACHE_FLASH_ATTR
power_on()
{

    GPIO_OUTPUT_SET(PIN_POWER, 1);
    GPIO_OUTPUT_SET(PIN_LED_S, 0);
    os_printf("Power On\n");
    if(shut_down_flag == 1)
    {
        shut_down_flag = 0;
        system_deep_sleep_set_option(1);
        system_deep_sleep(0);
    }
}

void ICACHE_FLASH_ATTR
delay_power_off()
{

    os_printf("cfun Off \n");

    os_memset(uart_receive_at,'\0',sizeof(char)*2048);
    queue_uart_send(at_cfun,os_strlen(at_cfun));
    os_printf("send %s\n",at_cfun);
    at_state = ZCFUN;

}

void ICACHE_FLASH_ATTR
create_http(uint8_t times)
{
    os_printf("create_http times %d\n",times);
//    os_printf("creat_flag %d\n",creat_flag);
//    if(creat_flag == 0)//开机以来第一次创建客户端
//    {
//
//        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//        queue_uart_send(http_create,os_strlen(http_create));
//        os_printf("send http_create %s\n",http_create);
////修改请求绑定    creat_flag = 1;
//
//    }else
//    {
//        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//        queue_uart_send(http_con,os_strlen(http_con));
//        os_printf("send %s\n",http_con);
//        at_state = HTTP_CON;
//    }
    switch (times)
    {
    case 0:
        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
        queue_uart_send(http_create,os_strlen(http_create));
        os_printf("send http_create %s\n",http_create);
        break;

    case 1://发请求绑定
        queue_uart_send(http_post_bind,os_strlen(http_post_bind));
        os_printf("http_post_bind %s\n",http_post_bind);
        at_state = WAIT;

        break;

    case 2://发100个mac

        queue_uart_send(http_send0,os_strlen(http_send0));
        os_printf("send0 %s\n",http_send0);
        at_state = HTTP_SEND;
        break;


    default:
        break;
    }
}


void ICACHE_FLASH_ATTR
str_ascii_str(char * body)
{
    uint8_t str[2900];
    uint16_t i;
    uint16_t len = os_strlen(body);

    for(i=0;i<len*2;i++)
    {
        str[i] = (body[i/2] >> 4) & 0xf;
        str[i+1] = body[i/2] & 0xf;
        i++;
    }

    for(i=0;i<len*2;i++)
    {
        if(str[i]<10)
        {
            str[i]+= 48;
        }else
        {
            str[i]+= 55;
        }
    }
    str[len*2] = ',';
    str[len*2+1] = '\"';
    str[len*2+2] = '\0';
    os_printf("len %d str %s \n",len,str);
    os_printf("\n");

    os_memcpy(http_send0+87,str    , 377);
    os_memcpy(http_send1+25,str+380, 494);
    os_memcpy(http_send2+25,str+874, 494);
    os_memcpy(http_send3+25,str+1368,494);
    os_memcpy(http_send4+25,str+1862,494);
    os_memcpy(http_send5+25,str+2356,494);
    os_memcpy(http_send6+24,str+2850,56 );
    http_send6[78] = ',';
    http_send6[79] = '\"';

    os_printf("send0 %s\n",http_send0);
    os_printf("send1 %s\n",http_send1);
    os_printf("send2 %s\n",http_send2);
    os_printf("send3 %s\n",http_send3);
    os_printf("send4 %s\n",http_send4);
    os_printf("send5 %s\n",http_send5);
    os_printf("send6 %s\n",http_send6);

    create_http(2);

//    send_flag = 0;
}

void ICACHE_FLASH_ATTR
update_data()
{
    VOWSTAR_WAN_DEBUG("%s: request\r\n", __func__);
    // Build Body's JSON string
    uint8_t * body = (uint8_t *) os_zalloc(os_strlen(JSON_DEVICE_MAC) +
                                           os_strlen(parameter_deviceId) +
                                           os_strlen(parameter_timestamp) +
                                           os_strlen(parameter_tag) +
                                           os_strlen(parameter_longitude) +
                                           os_strlen(parameter_latitude) +
                                           os_strlen(mac_str)
                                           );
    if (body == NULL) {
        VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
        return;
    }

    os_sprintf(body,
               JSON_DEVICE_MAC,
               parameter_deviceId,
               parameter_timestamp,
               parameter_tag,
               parameter_longitude,
               parameter_latitude,
               mac_str);

    // Perform a Device++ request
//    dxx_http_request(parameter_timestamp, body, device_post_callback);
    VOWSTAR_WAN_DEBUG("%s: body %s\n", __func__, body);
    str_ascii_str(body);
    // Free memory
    if (body)
        os_free(body);
}


void ICACHE_FLASH_ATTR
str_ascii_str_bind(char * bind_body)
{
    uint8_t str[2048];
    uint16_t i;
    uint16_t len = os_strlen(bind_body);
    uint16_t data_len,json_len;

    for(i=0;i<len*2;i++)
    {
        str[i] = (bind_body[i/2] >> 4) & 0xf;
        str[i+1] = bind_body[i/2] & 0xf;
        i++;
    }

    for(i=0;i<len*2;i++)
    {
        if(str[i]<10)
        {
            str[i]+= 48;
        }else
        {
            str[i]+= 55;
        }
    }
    str[len*2] = ',';
    str[len*2+1] = '\"';
    str[len*2+2] = '\r';
    str[len*2+3] = '\n';
    str[len*2+4] = '\0';

    os_printf("len %d str %s \n",len,str);
    os_printf("\n");

    os_memcpy(http_post_bind+ 75, str, len*2+4);
    os_memset(http_post_bind+79+len*2,'\0',sizeof(char)*(os_strlen(http_post_bind)-79-len*2));


    json_len = len*2+1;
    data_len = json_len+51;
    os_printf("data_len %d,json_len %d,\n",data_len,json_len);

    http_post_bind[15] = data_len/100 +48;
    http_post_bind[16] = data_len%100/10 +48;
    http_post_bind[17] = data_len%100%10 +48;
    http_post_bind[19] = http_post_bind[15];
    http_post_bind[20] = http_post_bind[16];
    http_post_bind[21] = http_post_bind[17];

    http_post_bind[71] = json_len/100 +48;
    http_post_bind[72] = json_len%100/10 +48;
    http_post_bind[73] = json_len%100%10 +48;

    os_printf("http_post_bind15=%c,16=%c,17=%c,71=%c,72=%c,73=%c\n",http_post_bind[15],http_post_bind[16],http_post_bind[17],http_post_bind[71],http_post_bind[72],http_post_bind[73]);


//AT+EHTTPSEND=0,235,235,"0,1,12,"/device/sign",0,,16,"application/json",184,7B226465766963654964223A223741526A6F34713361634E6557544746336D52577358222C226C6F67696E4E616D65223A223132333435363738393031323334353637383930222C226C6F67696E507764223A223233373930227D,"

    create_http(1);


//    send_flag = 0;
}
void ICACHE_FLASH_ATTR
update_post_bind()
{

    os_printf("create len %d\n",os_strlen(http_post_bind));
    VOWSTAR_WAN_DEBUG("%s: request\r\n", __func__);
    // Build Body's JSON string
    uint8_t * bind_body = (uint8_t *) os_zalloc(os_strlen(JSON_POST_BIND) +
                                           os_strlen(parameter_deviceId) +
                                           os_strlen(loginName) +
                                           os_strlen(loginPwd)
                                           );
    if (bind_body == NULL) {
        VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
        return;
    }

    os_sprintf(bind_body,
               JSON_POST_BIND,
               parameter_deviceId,
               loginName,
               loginPwd);

    // Perform a Device++ request
//    dxx_http_request(parameter_timestamp, body, device_post_callback);
    VOWSTAR_WAN_DEBUG("%s: body %s\n", __func__, bind_body);

    str_ascii_str_bind(bind_body);
    // Free memory
    if (bind_body)
        os_free(bind_body);
}


void ICACHE_FLASH_ATTR
nbiot_http_post()
{
    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
    os_timer_disarm(&delay_2s);
    os_timer_setfn(&delay_2s, (os_timer_func_t *)update_data, NULL);
    os_timer_arm(&delay_2s, 2500, 0);//开始上传数据
}


void ICACHE_FLASH_ATTR
uart_send(const uint8_t * buffer, uint16_t length)
{
	os_printf("UART Data send >>>\n");
	uint16_t i = 0;

	for (i = 0; i < length; i++) {
		os_printf("%02X ", buffer[i]);
	}

	os_printf("\n");

	queue_uart_send((char *)buffer, length);


	os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
}

int ICACHE_FLASH_ATTR
dxx_http_process(char * body, size_t body_len)
{
//	if ((NULL != body) && (0 != body_len)) {
		VOWSTAR_WAN_DEBUG("%s: body size:%d\r\n", __func__, body);

//		if (0 == dxx_message_ckeck_json_object(body, body_len)) {
		dxx_message_ckeck_json_object(body, body_len);
			body[body_len] = '\0';
			VOWSTAR_WAN_DEBUG("%s: body:\r\n", __func__);
			VOWSTAR_WAN_DEBUG("%s\r\n", body);
			return 0;
//		}
//	}
}
//void ICACHE_FLASH_ATTR
//device_ota_callback(void * ctx, char * response_body, size_t response_body_size, int http_status, char * response_headers)
//{
//	os_printf("%s: status:%d\n", __func__, http_status);
//	if (200 == http_status) {
//		dxx_http_process(response_body, response_body_size);
//	}
//}

void ICACHE_FLASH_ATTR
check_id(void)
{
    //再次获取id
    queue_uart_send(http_get_tag,os_strlen(http_get_tag));
    os_printf("send %s\n",http_get_tag);
    at_state = WAIT;
}



void ICACHE_FLASH_ATTR
uart_receive(const uint8_t * pdata, uint16_t length)
{

    os_timer_disarm(&timer_90s);
    os_timer_arm(&timer_90s, 90000, 1);//90s后没有收到数据，重启

	os_printf("+++++++++++UART Data received+++++++++++++\n");
	uint8_t end[1] = {'\0'};
	os_memcpy(pdata+length,end,1);

	os_printf("rev_len %d :%s \r\n",length, pdata);
	os_printf("----------------------------------------------\n");
    os_memset(uart_receive_at,'\0',sizeof(char)*512);
    os_memcpy(uart_receive_at,pdata,length);

//	if(length == 100)
//	{
//	    if(uart_fifo_flag==0)
//	        os_printf("+++++++++++++++UART Data received++++++++++++++++\n");
//	    os_printf("rev_len %d :%s \r\n",length, pdata);
//	    os_memcpy(uart_receive_at+(uart_fifo_flag*100),pdata,100);
//	    uart_fifo_flag++;
//	}else
    {
//	    os_memcpy(uart_receive_at+(uart_fifo_flag*100),pdata,length);
//	    os_printf("rev_len %d :%s \r\n",uart_fifo_flag*100+length, uart_receive_at);
//	    os_printf("----------------------------------------------\n");
//	    uart_fifo_flag = 0;

        // Send back what received
    //	uart_send(pdata, length);
//        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//        os_memcpy(uart_receive_at,pdata,length);


        if(os_strstr(uart_receive_at,"+IP:"))
        {

            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
            queue_uart_send(con_led,os_strlen(con_led));
            at_state = LED;
            os_printf("send %s\n",con_led);

        }


        if(os_strstr(uart_receive_at,"+EHTTPCREAT:"))
        {
            creat_flag = 1;
    //        http_send0[25] = uart_receive_at[81];
    //        http_con[12] = uart_receive_at[81];
            if(uart_receive_at[81] == 1)
            {
                queue_uart_send(http_destroy,os_strlen(http_destroy));
                os_printf("send %s\n",http_destroy);
                at_state = HTTP_DESTROY;
            }
            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
            queue_uart_send(http_con,os_strlen(http_con));
            os_printf("send %s\n",http_con);

            at_state = HTTP_CON;
        }

        if(os_strstr(uart_receive_at,"+EHTTPERR:0,-2"))
        {
            at_state = NONE;
            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
            queue_uart_send(http_con,os_strlen(http_con));
            os_printf("http_con %s\n",http_con);
        }


        if(os_strstr(uart_receive_at,"ERROR"))
        {

            os_printf("error\n");
            os_printf("at_state = %d\n",at_state);
            switch (at_state)
            {
            case ZOFF:
                delay_power_off();
                break;

            case HTTP_CON:
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(http_discon,os_strlen(http_discon));
//                os_printf("discon %s\n",http_discon);
//                at_state = HTTP_DISCON;
//                break;
//
//            case HTTP_DISCON:
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(http_con,os_strlen(http_con));
//                os_printf("send %s\n",http_con);
//                at_state = HTTP_DESTROY;
//                break;
//
//            case HTTP_DESTROY:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_destroy,os_strlen(http_destroy));
                os_printf("send %s\n",http_destroy);
                at_state = ZGMODE;
                break;

            default:
                break;
            }
        }

//        if(os_strstr(uart_receive_at,"ZGDATA: READY"))
//        {
//            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//            queue_uart_send(zgrun,os_strlen(zgrun));
//            os_printf("send %s\n",zgrun);
//            return;
//        }
//
//        if(os_strstr(uart_receive_at,"ZGDATA: NOT READY"))
//        {
//            zgdata[9] = '?';
//            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//            queue_uart_send(zgdata,os_strlen(zgdata));
//            os_printf("send %s\n",zgdata);
//            return;
//        }



        if(os_strstr(uart_receive_at,"OK"))
        {
            char * clk_addr = NULL;
            uint8_t temp_clk[] = "14/09/11,01:54:15";
            os_printf("at_state = %d\n",at_state);
            switch (at_state)
            {
            case LED:
                //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(read_adc,os_strlen(read_adc));
                os_printf("send %s\n",read_adc);
                at_state = ZADC;
                break;

            case ZADC:  //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
                z_adc = (uart_receive_at[6]-48)*100+(uart_receive_at[7]-48)*10+uart_receive_at[8]-48;
                uint8_t src[6], i, mac_address_str[12];
                if(z_adc>544)
                {
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
                        os_memcpy(http_get_tag + 65, parameter_deviceId, 22);
                        os_printf("deviceid %s\n",parameter_deviceId);



                        //获取时间
                        queue_uart_send(cclk,os_strlen(cclk));
                        os_printf("send %s\n",cclk);
                        at_state = CCLK;

                    }else system_restart();
                }else delay_power_off();
                break;


            case CCLK:

//                clk_addr = strstr(uart_receive_at,"+CCLK:");
//                os_memcpy(temp_clk,clk_addr+8,19);
//                os_printf("temp_clk %s \n",temp_clk);
//                parameter_timestamp[0] = temp_clk[9];
//                parameter_timestamp[1] = temp_clk[10];
//                parameter_timestamp[2] = temp_clk[12];
//                parameter_timestamp[3] = temp_clk[13];
//                parameter_timestamp[4] = temp_clk[15];
//                parameter_timestamp[5] = temp_clk[16];
//                parameter_timestamp[6] = temp_clk[6];
//                parameter_timestamp[7] = temp_clk[7];
//                parameter_timestamp[8] = temp_clk[3];
//                parameter_timestamp[9] = temp_clk[4];
//                parameter_timestamp[10] = temp_clk[0];
//                parameter_timestamp[11] = temp_clk[1];
                os_printf("clk parameter_timestamp %s \n",parameter_timestamp);

                //等待回应：未绑定/定位错误
                if(creat_flag == 0)
                    create_http(0);
                else//再次获取id
                {
                    queue_uart_send(http_get_tag,os_strlen(http_get_tag));
                    os_printf("send %s\n",http_get_tag);
                    at_state = WAIT;
                }
                break;

            case ZCFUN:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(e_power_off,os_strlen(e_power_off));
                os_printf("send %s\n",e_power_off);
                at_state = ZOFF;
                break;

            case ZOFF:
                os_printf("NB turned off, now system turn off\n");
    //            delay_power_on();//关机
                shut_down_flag = 1;
//                os_timer_disarm(&temer_90s);
                power_on();
                break;

            case ZGMODE:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(zgnmea,os_strlen(zgnmea));
                os_printf("send %s\n",zgnmea);
                at_state = ZGNMEA;
                break;

            case ZGSTOP:
                zgmode[10] = '1';
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(zgmode,os_strlen(zgmode));
                os_printf("send %s\n",zgmode);


                break;


            case ZGNMEA:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(zgrun,os_strlen(zgrun));
                os_printf("send %s\n",zgrun);

                at_state = NONE;
                break;

            case HTTP_CON:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//            if(bind_flag == 1)    //修改请求绑定
//                {
//                    queue_uart_send(http_post_bind,os_strlen(http_post_bind));
//                    os_printf("http_post_bind %s\n",http_post_bind);
//                    at_state = WAIT;
//                }
//                else
                if(get_tag_flag == 0)//0
                {


                	http_get_tag[98] = local_system_status.version_num/100 + 48;
                	http_get_tag[99] = local_system_status.version_num%100/10 + 48;
                	http_get_tag[100]= local_system_status.version_num%100%10 + 48;

                    queue_uart_send(http_get_tag,os_strlen(http_get_tag));
                    os_printf("send %s\n",http_get_tag);
                    at_state = WAIT;
                }
                else if(probe_flag == 0)
                    at_state = NONE;
                else
                {
                    queue_uart_send(http_send0,os_strlen(http_send0));
                    os_printf("send0 %s\n",http_send0);
                    at_state = HTTP_SEND;
                }
                break;

            case HTTP_SEND:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send1,os_strlen(http_send1));
                os_printf("send1 %s\n",http_send1);
                at_state = HTTP_SEND1;
                break;

            case HTTP_SEND1:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send2,os_strlen(http_send2));
                os_printf("send2 %s\n",http_send2);
                at_state = HTTP_SEND2;
                break;

            case HTTP_SEND2:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send3,os_strlen(http_send3));
                os_printf("send3 %s\n",http_send3);
                at_state = HTTP_SEND3;
                break;

            case HTTP_SEND3:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send4,os_strlen(http_send4));
                os_printf("send4 %s\n",http_send4);
                at_state = HTTP_SEND4;
                break;

            case HTTP_SEND4:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send5,os_strlen(http_send5));
                os_printf("send5 %s\n",http_send5);
                at_state = HTTP_SEND5;
                break;

            case HTTP_SEND5:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_send6,os_strlen(http_send6));
                os_printf("send6 %s\n",http_send6);
                os_printf("send done\n");
                gnrmc_gps_flag = 0;
                at_state = NONE;
                break;

            case HTTP_DISCON:
            case HTTP_DESTROY:
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(http_con,os_strlen(http_con));
                os_printf("send %s\n",http_con);
                at_state = HTTP_CON;
                break;

            default:
                break;

            }
        }



        if(os_strstr(uart_receive_at,"+EHTTPNMIC:"))//服务器有响应
        {
            if(os_strstr(uart_receive_at,"22636f6465223a2230222c2264657363223a227375636365737322"))
            {
                os_printf("{\"code\":\"0\",\"desc\":\"success\"}\n");
                os_printf("{\"code\":\"0\",\"desc\":\"success\"}\n");
                os_printf("{\"code\":\"0\",\"desc\":\"success\"}\n");
                os_printf("{\"code\":\"0\",\"desc\":\"success\"}\n");
                led_state = 5;
                devicexx_io_led_timer_tick();
//                send_flag = 0;
                //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(read_adc,os_strlen(read_adc));
                at_state = ZADC;
                os_printf("read adc %s\n",read_adc);

            }else if(os_strstr(uart_receive_at,"e59cb0e79086e58cbae59f9fe5a496")||os_strstr(uart_receive_at,"e5bd93e5898de697a0e4bbbbe58aa1")||os_strstr(uart_receive_at,"e58f82e695b0e4b8bae7a9ba"))
            {
                //地理区域外         当前无任务       参数为空
                os_timer_disarm(&timer_60s);
                os_timer_arm(&timer_60s, 6000, 1);
//                delay_power_off();//关机
            }else if(os_strstr(uart_receive_at,"636f6c6c6563744964"))//获得id,查询版本
            {

                uint8_t i,istime[1],issite[1];
                uint8_t temp_tag[] = "1234567890123456789012";
                char * cid_addr = NULL;



                wifi_set_opmode(STATION_MODE);

                cid_addr = strstr(uart_receive_at,"22636f6c6c656374496422");//"collectId"
                os_memcpy(temp_tag,cid_addr+26,22);
                os_memcpy(istime,cid_addr+85,2);
                os_memcpy(issite,cid_addr+115,2);
                os_printf("istime %d , issite %d\n",istime[0],issite[0]);
//22636f6c6c6563744964223a22434a303030303030303034222c226973436f727265637454696d65223a302c2269734c6f636174696f6e223a312c
                for(i=0;i<22;i++)
                {
                    if(temp_tag[i]<58)
                    {
                        temp_tag[i]-=48;
                    }else
                    {
                        temp_tag[i]-=87;
                    }
                }
                for(i=0;i<11;i++)
                {
                    parameter_tag[i] = ((temp_tag[i*2] << 4) + temp_tag[1+i*2]);
    //                os_printf("%d,,%d\n",uart_receive_at[94+i*2],uart_receive_at[95+i*2]);
    //                os_printf("parameter_tag[%d]\n",parameter_tag[i]);
                }

                at_state = NONE;
                os_printf("tag %s\n",parameter_tag);

                if(0 != os_strcmp(parameter_tag,"00000000000") && istime[0] == 48)//不等于00000000
                {
                    get_tag_flag = 1;//获得任务ID标志
                    os_timer_disarm(&timer_60s);
                    queue_uart_send(zgmode,os_strlen(zgmode));
                    os_printf("send %s\n",zgmode);
                    at_state = ZGMODE;
                }else if(istime[0] == 49 || issite[0] == 49 )//此时没有任务，再判断是否能升级
                {
                    os_timer_disarm(&timer_90s);

                    uint8_t * version_addr = NULL;
                    uint8_t version_temp[11] ;//302e303031
                    uint8_t version_type;
                    uint16_t version_num;


                    version_addr = strstr(pdata,"76657273696f6e");//version

                    os_memcpy(version_temp,version_addr+20,10);
                    version_temp[10] = '\0';
                    os_printf("version_temp %s\n",version_temp);
                    version_type = version_temp[1]-48;
                    version_num = (version_temp[5]-48)*100 + (version_temp[7]-48)*10 + version_temp[9]-48;
                    os_printf("version_type %d version_num %d\n",version_type,version_num);

                    os_printf("local_system_status.version_num %d\n",local_system_status.version_num);
                    if(local_system_status.version_type == version_type && local_system_status.version_num < version_num)
                    {

                        uint8_t url_temp[128] ;
                        uint8_t * url_addr = NULL;
                        uint8_t url_len, i;

                        local_system_status.version_num = version_num;
                        url_addr = strstr(pdata,"75726c");//url
                        url_len = version_addr-url_addr-18;
                        os_memcpy(url_temp,url_addr+12,url_len);
                        url_temp[url_len] = '\0';

                        for(i=0;i<url_len;i++) if(url_temp[i]<58) url_temp[i]-=48; else url_temp[i]-=87;
                        for(i=0;i<url_len/2;i++) update_host[i] = ((url_temp[i*2] << 4) + url_temp[1+i*2]);

                        os_printf("update_host %s \n",update_host);

                        //传递ap_ssid ap_pwd
                        tcp_client_init(ap_ssid,ap_pwd);//OTA升级
                    }
                }

                os_timer_disarm(&timer_90s);
                os_timer_arm(&timer_90s, 90000, 1);//20s后没有收到数据，重启


            }
            else if(os_strstr(uart_receive_at,"e5ae9ae4bd8de99499e8afaf"))//定位错误
            {
                queue_uart_send(zgmode,os_strlen(zgmode));
                os_printf("send %s\n",zgmode);
                at_state = ZGMODE;

            }
            else if(os_strstr(uart_receive_at,"e8aebee5a487e69caae4bdbfe794a8")||os_strstr(uart_receive_at,"e8aebee5a487e69caae7bb91e5ae9a"))//设备未使用：数据库没有这个设备；未绑定：数据库有设备但是为关联代理商
            {
//                bind_flag = 0;
                at_state = WAIT;

                led_state = 3;
//                devicexx_io_led_timer_tick();
                wifi_set_opmode(SOFTAP_MODE);
                vowstar_set_ssid_prefix("Qingzhu_VQ_");
                tcp_server();

                os_timer_disarm(&timer_90s);

            }else if(os_strstr(uart_receive_at,"e8aebee5a487e5b7b2e7bb91e5ae9a"))//
            {
                os_timer_disarm(&timer_90s);

                char *binded="设备已绑定,请先解绑";
                os_memcpy(http_answer+206,binded,19);

            }else if(os_strstr(uart_receive_at,"e8b4a6e58fb7e5af86e7a081e4b88de58cb9e9858d"))//账号密码不匹配
            {
                os_timer_disarm(&timer_90s);

                char *not_match="账号密码不匹配";
                os_memcpy(http_answer+206,not_match,14);
                os_printf("http_head = %s\n",http_answer);
            }else if(os_strstr(uart_receive_at,"62696e6473756363657373"))//设备绑定成功
            {
                char *bind_success="设备绑定成功";
                os_memcpy(http_answer+206,bind_success,12);
                os_printf("http_head = %s\n",http_answer);
                led_state = 5;
//                devicexx_io_led_timer_tick();
                //再次获取id
                queue_uart_send(http_get_tag,os_strlen(http_get_tag));
                os_printf("send %s\n",http_get_tag);
                at_state = WAIT;

                os_timer_disarm(&devicexx_io_led_timer);
            }else

            {
                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");

                //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(read_adc,os_strlen(read_adc));
                at_state = ZADC;
                os_printf("read adc %s\n",read_adc);
            }

            os_printf("tag %s\n",parameter_tag);

//            os_printf("send ok %s\n",uart_receive_at);
            os_memset(uart_receive_at,'\0',sizeof(char)*2048);

//            if(0 != os_strcmp(parameter_tag,"00000000"))
//            {
//            	get_tag_flag  = 1;
////                bind_flag = 1;
//                led_state = 5;
//                queue_uart_send(zgmode,os_strlen(zgmode));
//                os_printf("send %s\n",zgmode);
//                at_state = ZGMODE;
//
//                os_timer_disarm(&temer_10s);
//                os_timer_arm(&temer_10s, 20000, 1);//20s后没有收到数据，重启
//            }

            return;

        }


        if(os_strstr(uart_receive_at,"GNRMC"))
        {
            gnrmc_gps_flag++;

            if(uart_receive_at[17] == 'A')
            {
                GPIO_OUTPUT_SET(PIN_GPS_S, 0);
                os_printf("receive %s\n",uart_receive_at);
//临时
                os_memcpy(parameter_longitude,uart_receive_at + 32, os_strlen(parameter_longitude));
                os_memcpy(parameter_latitude, uart_receive_at + 19 , os_strlen(parameter_latitude));

                os_memcpy(http_get_tag+106, parameter_longitude, os_strlen(parameter_longitude));
                os_memcpy(http_get_tag+122, parameter_latitude , os_strlen(parameter_latitude));

                os_memcpy(parameter_timestamp, uart_receive_at + 7 , 6);
                os_memcpy(parameter_timestamp+6, uart_receive_at + 53 ,6);
                os_memcpy(http_get_tag + 138, parameter_timestamp , 12);


                os_printf("parameter_longitude=%s\nparameter_latitude=%s\nparameter_timestamp=%s\ncreat_flag=%d\n"
                                        ,parameter_longitude,parameter_latitude,parameter_timestamp,creat_flag);

//                if(creat_flag == 0)
//                    create_http(0);
//                else
//                {
//                    //再次获取id
//                    queue_uart_send(http_get_tag,os_strlen(http_get_tag));
//                    os_printf("send %s\n",http_get_tag);
//                    at_state = WAIT;
//                }


            }else
            {
                uint8_t i;
                GPIO_OUTPUT_SET(PIN_GPS_S, 1);
                at_state = WAIT;
                for(i=0;i<12;i++)
                    parameter_timestamp[i] = '0';
    //            os_printf("parameter_timestamp %s\n",parameter_timestamp);
            }

            os_printf("at_state = %d,gnrmc_gps_flag = %d\n",at_state,gnrmc_gps_flag);
            if(get_tag_flag == 1 && gnrmc_gps_flag > 15)
            {

                led_state = 1;
                devicexx_io_led_timer_tick();
                probe_flag = 1;
                gnrmc_gps_flag = 0;

                sniffer_init();
                sniffer_init_in_system_init_done();
            }

            if(gnrmc_gps_flag > 120)//2分钟无定位，启用wifi基站定位
            {
                gnrmc_gps_flag = 0;
                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
                queue_uart_send(zgstop,os_strlen(zgstop));
                os_printf("send %s\n",zgstop);
                at_state = ZGSTOP;
            }

        }

        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
        os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	}

}

void ICACHE_FLASH_ATTR
devicexx_receive(const d_object_t* object)
{
	d_object_t * data = devicexx_object_create();

	if (devicexx_receive_get(object, "s")) {
		os_printf("cloud try get hardware value: s\r\n");
		devicexx_add_int(data, "s", devicexx_io_get_relay_state());
		devicexx_send(data);
	}

	if (devicexx_receive_int(object, "s")) {
		os_printf("cloud try set hardware int value: s\r\n");
		devicexx_io_set_relay_state((uint8_t) devicexx_value_int(object, "s"));
		devicexx_io_set_led_state(devicexx_io_get_relay_state());
		devicexx_add_int(data, "s", devicexx_io_get_relay_state());
		devicexx_send(data);
	}

	devicexx_object_delete(data);
	os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
}

void ICACHE_FLASH_ATTR
devicexx_app_apply_settings(void)
{
	// Set uart receive callback
	uart_receive_callback_register(queue_uart_input);
	queue_uart_receive_callback_register(uart_receive);

}


void ICACHE_FLASH_ATTR
devicexx_app_load(void)
{
	os_printf("version_type %d  version_num %d\n",local_system_status.version_type,local_system_status.version_num);

	system_param_load(
	    (DEVICEXX_APP_START_SEC),
	    0,
	    (void *)(&local_system_status),
	    sizeof(local_system_status));
//flash 擦除置1, 出厂第一次启动给type num 赋值
	if(local_system_status.version_type == 0xff && local_system_status.version_num == 0xffff)
	{
		local_system_status.version_type = 0;
		local_system_status.version_num = 0;
		devicexx_app_save();
	}
	os_printf("version_type %d  version_num %d\n",local_system_status.version_type,local_system_status.version_num);
}

void ICACHE_FLASH_ATTR
devicexx_app_save(void)
{
	system_param_save_with_protect(
	    (DEVICEXX_APP_START_SEC),
	    (void *)(&local_system_status),
	    sizeof(local_system_status));
}




