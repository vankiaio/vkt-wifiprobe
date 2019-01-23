
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
//#include "at_custom.h"

os_timer_t checkTimer_wifistate;
os_timer_t timeout_timer;
ip_addr_t esp_server_ip;
struct espconn tcpserver;
struct station_config stationConf;
//uint8_t upgrade_tcp;
uint8_t wifi_state = 0;
uint8_t connet_flag = 0;
uint8_t count = 0;

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



uint8_t http_head[] = {
   "HTTP/1.0 200 OK\r\n"\
   "Content-Type: text/html;charset=gbk\r\n"\
   "Cache-Control: private\r\n"\
   "Connection: close\r\n"\
   "\r\n"\
   "<!DOCTYPE html>"\
   "<html>"\
   "<head>"\
   "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=gbk\">"\
   "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"\
   "<meta name=\"viewport\" content=\"width=device-width,height=device-height,inital-scale=1.0,maximum-scale=1.0,user-scalable=no;\">"\
   "<meta name=\"renderer\" content=\"webkit|ie-comp|ie-stand\">"\
   "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\">"\
   "<meta name=\"apple-mobile-web-app-status-bar-style\" content=\"black\">"\
   "<meta name=\"format-detection\" content=\"telephone=no\">"\
   "<meta content=\"email=no\" name=\"format-detection\" />"\
   "<title></title>"\
   "</head>"\
   "<style type=\"text/css\">"\
   "body{overflow-x:hidden;}"\
   "img{width: 100%;display: block;}"\
   "</style>"\
   "<body>"\
   "<form action=\"\" method=\"post\" style=\"width:100%;text-align:center;margin-top: 10%;\">"\
   "<h3><span style=\"color:BlueViolet\">清竹大数据</span>SmartBox</h3>"
   "<input type=\"text\" id=\"loginName\"  placeholder=\"请输入您的用户名\" style=\"width:200px;height:30px\"><br/></br>"\
   "<input type=\"password\" id=\"loginPwd\"  placeholder=\"请输入您的密码\" style=\"width:200px;height:30px\"><br/></br>"\
   "<button type=\"button\" id=\"loginbtn\" style=\"width:100px;height:30px\">绑定</button>"
   "<div id='tip'>                     </div>"\
   "</form>"\
   "<script language=\"javascript\">"\
   "var loginName = document.getElementById('loginName');"\
   "var loginPwd = document.getElementById('loginPwd');"\
   "var i = 0;"\
   "function reset() {"\
   "    var xhr = new XMLHttpRequest();"\
   "    xhr.open('GET', 'http://192.168.4.1/getAnswer', true);"\
   "    xhr.send();"\
   "    xhr.onreadystatechange = function() {"\
   "        if (xhr.status == 200) {"\
   "            if((xhr.responseText!=null)&&(xhr.responseText!='')&&!(xhr.responseText.indexOf('answer')>=0)){"\
   "                         i=1;"\
   "                         var tip = document.getElementById('tip');"\
   "                         tip.innerHTML=xhr.responseText;"\
   "                      }"\
   "        }"\
   "    };"\
   "}"\
   "function resetTime() {"\
   "    var xhr = new XMLHttpRequest();"\
   "    xhr.open('POST', 'http://192.168.4.1', true);"\
   "    xhr.send('loginName=' + loginName.value + '&loginPwd=' + loginPwd.value+\"&submit=\");"\
   "    xhr.onreadystatechange = function() {"\
   "        if (xhr.status == 200) {"\
   "            setInterval(function () {"\
   "                if(i==0){"\
   "                    reset();"\
   "                }"\
   "            }, 2000);"\
   "        }"\
   "    };"\
   "}"\
   "var oDiv = document.getElementById('loginbtn');"\
   "oDiv.onclick = function() {"\
   "    i = 0;"\
   "    var tip = document.getElementById('tip');"\
   "    tip.innerHTML='<p>验证中请稍后</p>';"\
   "    resetTime();}"\
   "</script>"\
   "</body>"\
   "</html>"\
   ""
};


uint8_t http_answer[] = {
    "HTTP/1.0 200 OK\r\n"\
    "Content-Type: text/html;charset=gbk\r\n"\
    "Cache-Control: private\r\n"\
    "Connection: close\r\n"\
    "\r\n"\
    "<!DOCTYPE html>"\
    "<html>"\
    "<head>"\
    "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=gbk\">"\
    "</head>"\
    "<body>"\
    "<p>answer             </p>"\
    "</body>"\
    "</html>"\
};

uint8_t http_response[] = {
    "HTTP/1.0 200 OK\r\n"\
    "Content-Type: text/html;charset=gbk\r\n"\
    "Cache-Control: private\r\n"\
    "Connection: close\r\n"\
};


void ICACHE_FLASH_ATTR
bind_timeout(void)
{
    char *not_match="连接超时";
    os_memcpy(http_answer+206,not_match,8);
//    os_printf("http_head = %s\n",http_answer);
}

void ICACHE_FLASH_ATTR
tcpserver_recon_cb(void *arg, sint8 errType)//异常断开回调
{
    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("\r\n异常断开");
}


void ICACHE_FLASH_ATTR
tcpserver_discon_cb(void *arg)//正常断开回调
{
    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("\r\n正常断开");
}


void ICACHE_FLASH_ATTR
tcpclient_sent_cb(void *arg)//发送回调
{
    struct espconn *pespconn = (struct espconn *)arg;


    if(connet_flag == 0)
    {
        os_printf("回调断开\n");
        espconn_disconnect(pespconn);//断开连接
    }
    os_printf("\r\n发送回调");
}




void ICACHE_FLASH_ATTR
tcpserver_recv(void *arg, char *pdata, unsigned short len)//接收函数
{
    count++;
    os_printf("\n\n+++++++++++++++++++count = %d\n",count);

    char * http_flg = NULL;
    char * name_addr = NULL;
    char * pwd_addr  = NULL;
    char * sub_addr  = NULL;
    uint8_t i,name_len = 0, pwd_len = 0;

    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("\r\n接收函数,%s\n",pdata);

    http_flg = strstr(pdata,"loginName");

    if (http_flg != NULL)
    {


        name_addr = strstr(pdata,"loginName");
        pwd_addr = strstr(pdata,"&loginPwd");
        sub_addr = strstr(pdata,"&submit=");

        name_len = pwd_addr-name_addr-10;
        pwd_len  = sub_addr-pwd_addr-10;

        os_printf("\nname_len=%d,pwd_len=%d\n",name_len,pwd_len);
        os_memcpy(loginName,name_addr+10,name_len);
        os_memcpy(loginPwd,pwd_addr+10,pwd_len);

        loginName[name_len] = '\0';
        loginPwd[pwd_len] = '\0';
        os_printf("\n%s    %s\n",loginName,loginPwd);

        if(0!=os_strcmp(loginName,"") || 0!=os_strcmp(loginPwd,"")) {
            connet_flag = 0;
            update_post_bind();
            espconn_send(pespconn,http_response,sizeof(http_response));
        }

    } else {
        http_flg = strstr(pdata,"getAnswer");

        if(http_flg != NULL){
            os_printf("发现 getAnswer\n");
            char *default_str="answer        ";

            os_timer_disarm(&timeout_timer); //取消定时器定时
            os_timer_setfn(&timeout_timer, (os_timer_func_t *) bind_timeout, NULL);   //设置定时器回调函数
            os_timer_arm(&timeout_timer, 10000, 0);   //启动定时器，单位：毫秒

            espconn_send(pespconn,http_answer,sizeof(http_answer));

            os_memcpy(http_answer+206,default_str,14);

        }else {
                espconn_send(pespconn,http_head,sizeof(http_head));
        }
    }
}

void ICACHE_FLASH_ATTR
tcpserver_listen(void *arg)//服务器被链接回调
{
    struct espconn *pespconn = (struct espconn *)arg;

    espconn_regist_reconcb(pespconn, tcpserver_recon_cb);//开启异常断开回调
    espconn_regist_disconcb(pespconn, tcpserver_discon_cb);//开启正常断开回调
    espconn_regist_recvcb(pespconn, tcpserver_recv);//开启接收回调
    espconn_regist_sentcb(pespconn, tcpclient_sent_cb);//开启发送成功回调
}


void ICACHE_FLASH_ATTR
tcp_server(void)//开启tcp服务器
{
    tcpserver.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    tcpserver.proto.tcp->local_port = 80;//监听本地端口号

    tcpserver.type = ESPCONN_TCP;
    tcpserver.state = ESPCONN_NONE;

    espconn_regist_connectcb(&tcpserver, tcpserver_listen);//链接成功回调
    espconn_accept(&tcpserver);//开启TCP服务器
    espconn_regist_time(&tcpserver, 30, 0);//设置服务器超时时间为1秒


    os_printf("Hello Esp8266!\r\n");

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
		ota_start_upgrade(remote_ip, 80, "");
	}
}


void ICACHE_FLASH_ATTR
Check_WifiState(void) {

	uint8 getState;
	getState = wifi_station_get_connect_status();

	os_printf("%s called  wifi status %d\r\n",__func__,getState);

	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP) {
		os_printf("WIFI Connected！\r\n");
		wifi_station_set_config(&stationConf);//保存到 flash
		wifi_state = 0;
		os_timer_disarm(&checkTimer_wifistate);

//		ota_start_upgrade(remote_ip, 80, "");

		HTTPCLIENT_DEBUG("DNS request\n");
		request_args_t * req  = (request_args_t *) os_malloc(sizeof(request_args_t));
		req->hostname         = strdup(update_host);
		req->port             = 80;
		req->secure           = false;
		req->method           = strdup("POST");
		req->path             = strdup("");
		req->headers          = strdup("Content-Type:text/plain");
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

	}else
	{
	    wifi_state++;
	}
	if(wifi_state == 180)
	{
	    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
	    wifi_state = 0;
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



