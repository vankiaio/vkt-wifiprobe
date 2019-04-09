
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "ets_sys.h"
#include "gpio.h"
#include "tcpclient.h"
#include "devicexx_app.h"
#include "queue_uart.h"
#include "httpclient.h"
#include "cJSON.h"
#include "platform.h"
//#include "source.h"
//#include "at_custom.h"

os_timer_t checkTimer_wifistate;

ip_addr_t esp_server_ip;
struct espconn tcpserver;
struct station_config stationConf;
//uint8_t upgrade_tcp;
uint8_t wifi_state = 0;
uint8_t connet_flag = 0;
uint8_t count = 0;

//typedef void (* http_callback_t)(void * ctx, char * response_body, size_t body_size, int http_status, char * response_headers);

typedef struct request_args_t {
	char            * hostname;
	int             port;
	bool            secure;
	char            * method;
	char            * path;
	char            * headers;
	char            * post_data;
	int             post_size;
	char            * buffer;
	int             buffer_size;
	void            * ctx;
	http_callback_t user_callback;
} request_args_t;




void ICACHE_FLASH_ATTR
post_callback(void * ctx, char * response_body, size_t response_body_size, int http_status, char * response_headers)
{
    uint16_t i;
    os_printf("%s: status:%d\n", __func__, http_status);
    if(http_status == -1)
    {
        scan_qz=0;
        wifi_station_disconnect();
        return;
    }
    if ((NULL != response_body) && (0 != response_body_size) && (NULL != response_headers)) {
        if (200 == http_status) {
            // Process and try encrypt data
            for(i=0;i<response_body_size-1;i++)
            {
                os_printf("%c",response_body[i]);
            }
            os_printf("\n");

//            // Parse root data
            cJSON * json_root = cJSON_Parse(response_body);

            // Check json root
            if ((NULL != json_root) && (cJSON_Object == json_root->type)) {
                cJSON * json_code = cJSON_GetObjectItem(json_root, "code");


                cJSON * json_desc = cJSON_GetObjectItem(json_root, "desc");


                cJSON * json_lon = cJSON_GetObjectItem(json_root, "lon");
                cJSON * json_lat = cJSON_GetObjectItem(json_root, "lat");

                cJSON * json_mac = cJSON_GetObjectItem(json_root, "mac");
                cJSON * json_excludeMac = cJSON_GetObjectItem(json_root, "excludeMac");
//

                cJSON * json_url = cJSON_GetObjectItem(json_root, "url");
                cJSON * json_version = cJSON_GetObjectItem(json_root, "version");
                cJSON * json_collectId = cJSON_GetObjectItem(json_root, "collectId");
                cJSON * json_isCorrectTime = cJSON_GetObjectItem(json_root, "isCorrectTime");
                cJSON * json_isLocation = cJSON_GetObjectItem(json_root, "isLocation");
//
                cJSON * json_fixedId = cJSON_GetObjectItem(json_root, "fixedId");
                cJSON * json_isFixedTime = cJSON_GetObjectItem(json_root, "isFixedTime");



                if(NULL != json_code       && NULL != json_collectId    && NULL != json_isCorrectTime &&
                   NULL != json_isLocation && NULL != json_url          && NULL != json_version &&
                   NULL != json_fixedId    && NULL != json_isFixedTime)
                {
                    os_printf("json_code:%s\n",         json_code->valuestring);
                    os_printf("json_collectId:%s\n",    json_collectId->valuestring);
                    os_printf("json_url:%s\n",          json_url->valuestring);
                    os_printf("json_isCorrectTime:%s\n",json_isCorrectTime->valuestring);
                    os_printf("json_isLocation:%s\n",   json_isLocation->valuestring);
                    os_printf("json_version:%s\n",      json_version->valuestring);
                    os_printf("json_fixedId:%s\n",      json_fixedId->valuestring);
                    os_printf("json_isFixedTime:%s\n",  json_isFixedTime->valuestring);

                    uint8_t version_temp[5] ;//0.000
                    os_memcpy(version_temp,json_version->valuestring,5);
                    version_type=version_temp[0]-48;
                    version_num =(version_temp[2]-48)*100+(version_temp[3]-48)*10+version_temp[4]-48;


                    if((0 != os_strcmp((json_collectId->valuestring),"00000000000")) &&
                            (0 != os_strcmp((json_isCorrectTime->valuestring),"1")))
                    {

                        os_memcpy(parameter_tag,json_collectId->valuestring,os_strlen(parameter_tag));
                        queue_uart_send(zgmode,os_strlen(zgmode));
                        os_printf("send %s\n",zgmode);
                        at_state = ZGMODE;

                    }else if(0 == os_strcmp((json_isCorrectTime->valuestring),"1"))
                    {
                        check_update_firmware(version_type,version_num,json_url->valuestring);
                    }else
                    {
                        os_timer_disarm(&check_id_timer);
                        os_timer_arm(&check_id_timer, 120000, 1);
                    }

                    if((0 != os_strcmp((json_fixedId->valuestring),"00000000000")) &&
                       (0 != os_strcmp((json_isFixedTime->valuestring),"1")))       //开通固定画像功能
                    {
                        isFixedTime[0]=48;
                        os_memcpy(fixedId,json_fixedId->valuestring,os_strlen(fixedId));
                    }


                }else if(NULL != json_code       && NULL != json_lon    && NULL != json_lat &&
                         NULL != json_mac        && NULL != json_excludeMac)
                {
                    os_printf("json_code:%s\n",         json_code->valuestring);
                    os_printf("json_lon:%s\n",          json_lon->valuestring);
                    os_printf("json_lat:%s\n",          json_lat->valuestring);
                    os_printf("json_mac:%s\n",          json_mac->valuestring);
                    os_printf("json_excludeMac:%s\n",   json_excludeMac->valuestring);

                    os_memcpy(parameter_latitude,json_lat->valuestring,os_strlen(parameter_latitude));
                    os_memcpy(parameter_longitude,json_lon->valuestring,os_strlen(parameter_longitude));

                    os_printf("update_data called\n");
                    update_data();

                }else if(NULL != json_code && NULL != json_desc)
                {
                    if(0 == os_strcmp(json_desc->valuestring,"success") || 0 == os_strcmp(json_desc->valuestring,"false"))
                    {
                        os_timer_disarm(&delay_update_timer);
                        post_state = CHECK_ID;
                        check_id();
                    }

                }


                os_timer_disarm(&restart_nb);

            } else {
                os_printf("%s: Error when parse JSON\r\n", __func__);
            }
            cJSON_Delete(json_root);
        } else {
            response_body[response_body_size] = '\0';
            os_printf("%s: error:\n%s\n", __func__, response_body);
        }
    }
}







LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_dns_found(const char *name, ip_addr_t *ipaddr, void *arg) {
	struct espconn *pespconn = (struct espconn *)arg;
	if (ipaddr != NULL)
	{
		os_printf("user_esp_platform_dns_found %d.%d.%d.%d\n",
				*((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1), *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));

		uint8_t remote_ip[4] = {0, 0, 0, 0};//目标IP地址,必须要先从手机获取，否则连接失败.
		remote_ip[0] = *((uint8 *)&ipaddr->addr);
		remote_ip[1] = *((uint8 *)&ipaddr->addr + 1);
		remote_ip[2] = *((uint8 *)&ipaddr->addr + 2);
		remote_ip[3] = *((uint8 *)&ipaddr->addr + 3);

//		os_printf("ip %d %d %d %d\n",remote_ip[0],remote_ip[1],remote_ip[2],remote_ip[3]);
//升级
		if(update_firmware_flag)
		{
		    ota_start_upgrade(remote_ip, 80, "");
		}else
//		ota_start_upgrade(remote_ip, 80, "http://wpupgrade.devicexx.com/");
	        ota_start_upgrade(remote_ip, 80, "http://api.seniverse.com/v3/weather/daily.json?key=rrpd2zmqkpwlsckt&location=guangzhou&language=en&unit=c&start=0&days=3 ");
	}
}


void ICACHE_FLASH_ATTR
Check_WifiState(void) {

	uint8 getState;
	getState = wifi_station_get_connect_status();

	os_printf("%s called  wifi status %d\r\n",__func__,getState);

	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP) {
	    scan_qz=1;
        wifi_state = 0;
	    switch(post_state)
	    {
	        case OTA:
	        {
                os_printf("WIFI Connected！\r\n");
                wifi_station_set_config(&stationConf);//保存到 flash

                os_timer_disarm(&checkTimer_wifistate);

        //		ota_start_upgrade(remote_ip, 80, "");
//        		os_memcpy(update_host,"wpupgrade.devicexx.com",22);
        //		update_host[22] = '\0';
//                os_memcpy(update_host,"api.seniverse.com",17);
//                update_host[17] = '\0';
                HTTPCLIENT_DEBUG("DNS request\n");
                request_args_t * req  = (request_args_t *) os_malloc(sizeof(request_args_t));
                req->hostname         = strdup(update_host);
                req->port             = 80;
                req->secure           = false;
                req->method           = strdup("POST");
                req->path             = strdup("");
        		req->headers          = strdup("Content-Type:text/plain");//Content-Type: text/html;charset=utf-8
                req->post_data    	  = NULL;
                req->post_size        = 0;
                req->buffer_size      = 1;
                req->buffer           = (char *)os_malloc(1);
                req->buffer[0]        = '\0'; // Empty string
                req->user_callback    = NULL;
                req->ctx              = NULL;

                err_t error = espconn_gethostbyname((struct espconn *)req, update_host, &esp_server_ip, user_esp_platform_dns_found);
                os_printf("err %d\n",error);

                os_free(req);
                req = NULL;

	        }break;

            case CHECK_ID:
            {
                check_id();
                os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
            }break;

            case AP_MAC:
            {
                uint8_t * body = (uint8_t *) os_zalloc(os_strlen(JSON_AP_MAC) + os_strlen(ap_str) );

                if (body == NULL) {
                    os_printf("%s: not enough memory\r\n", __func__);
                    return;
                }
                os_sprintf(body, JSON_AP_MAC, ap_str);

                post_state = AP_MAC;
                uint8_t * url = NULL;
                url = "http://221.122.119.226:8098/location/wifi";
                void * ctx = NULL;
                http_post(ctx, url, "Content-Type:application/json\r\n", (const char *) body, os_strlen(body), post_callback);

                if(body)
                    os_free(body);

                os_printf("wifi status %d\r\n",wifi_station_get_connect_status());


                os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
            }break;
	    }
	}else
	{
	    wifi_state++;
	    os_printf("wifi_state %d\n",wifi_state);
	}
	if(wifi_state > 60)
	{
	    scan_qz=0;
	    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时

	    wifi_state = 0;
//        sniffer_init();
//        sniffer_init_in_system_init_done();
	}
}

void ICACHE_FLASH_ATTR
tcp_client_init(uint8_t ssid[32], uint8_t pwd[32])	//初始化
{
	os_strcpy(stationConf.ssid, ssid);	  //改成你自己的   路由器的用户名
	os_strcpy(stationConf.password, pwd); //改成你自己的   路由器的密码
    os_printf(" ssid %s\n password %s\n",stationConf.ssid,stationConf.password);
//	wifi_station_set_config_current(&stationConf);	//设置WiFi station接口配置，不保存到 flash
//    注意不能使用上一句，我出现连接不上路由器情况
	wifi_station_set_config(&stationConf);  //设置WiFi station接口配置，并保存到 flash
    wifi_set_opmode(STATION_MODE);  //设置为STATION模式
	wifi_station_connect();	//连接路由器

//	upgrade_tcp = 1;

	os_timer_disarm(&checkTimer_wifistate);	//取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState, NULL);	//设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, 1);	//启动定时器，单位：毫秒

}


