
//#include "driver/uart.h"  //串口0需要的头文件
#include "osapi.h"  //串口1需要的头文件
#include "user_interface.h" //WIFI连接需要的头文件
#include "espconn.h"//TCP连接需要的头文件
#include "mem.h" //系统操作需要的头文件
#include "ets_sys.h"
#include "gpio.h"
#include "tcpclient.h"
#include "devicexx_app.h"
#include "queue_uart.h"
//#include "at_custom.h"

os_timer_t checkTimer_wifistate;
//struct espconn user_tcp_conn;
uint8_t upgrade_tcp;
uint8_t wifi_state = 0;
uint8_t connet_flag = 0;
uint8_t count = 0;

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
   "<form action=\"\" method=\"post\">"\
   "<input type=\"text\" id=\"loginName\"  placeholder=\"请输入您的用户名\"><br/>"\
   "<input type=\"password\" id=\"loginPwd\"  placeholder=\"请输入您的密码\"><br/>"\
   "<button type=\"button\" id=\"loginbtn\" >绑定</button>"
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
   "            }, 800);"\
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


struct espconn tcpserver;
uint8_t http_data[] = {
    "HTTP/1.1 200 OK\r\n"\
    "Content-Type:text/plain;charset=UTF-8\r\n"\
    "Content-Disposition:attachment;filename=1.txt\r\n"\
    "\r\n"\
    "get data"\
    "\r\n"
};


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




//
//void ICACHE_FLASH_ATTR uart_tcp_send(void *arg)
//{
//
//    struct espconn *pespconn = arg;
//    espconn_sent(pespconn, "8266", strlen("8266"));
//}
//void ICACHE_FLASH_ATTR user_tcp_sent_cb(void *arg)  //发送
//{
//	os_printf("send data succe ！");
//}
//void ICACHE_FLASH_ATTR user_tcp_discon_cb(void *arg)  //断开
//{
//	os_printf("disconnected ok！");
//}
//void ICACHE_FLASH_ATTR user_tcp_recv_cb(void *arg,  //接收
//		char *pdata, unsigned short len) {
//
//	os_printf("received data：%s\r\n", pdata);
////	espconn_sent((struct espconn *) arg, "0", strlen("0"));
//
//	uart_send(pdata,len);
//
//}
//void ICACHE_FLASH_ATTR user_tcp_recon_cb(void *arg, sint8 err) //注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
//{
//	os_printf("connected error, error code %d\r\n", err);
//	espconn_connect((struct espconn *) arg);
//}
//void ICACHE_FLASH_ATTR user_tcp_connect_cb(void *arg)  //注册 TCP 连接成功建立后的回调函数
//{
//	struct espconn *pespconn = arg;
//	espconn_regist_recvcb(pespconn, user_tcp_recv_cb);  //接收
//	espconn_regist_sentcb(pespconn, user_tcp_sent_cb);  //发送
//	espconn_regist_disconcb(pespconn, user_tcp_discon_cb);  //断开
//	espconn_sent(pespconn, "8266", strlen("8266"));
//    queue_uart_send(zgmode,os_strlen(zgmode));
//    at_state = ZGMODE;
//}
//
//void ICACHE_FLASH_ATTR my_station_init(struct ip_addr *remote_ip,
//		struct ip_addr *local_ip, int remote_port) {
//	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));  //分配空间
//	user_tcp_conn.type = ESPCONN_TCP;  //设置类型为TCP协议
//	os_memcpy(user_tcp_conn.proto.tcp->local_ip, local_ip, 4);
//	os_memcpy(user_tcp_conn.proto.tcp->remote_ip, remote_ip, 4);
////	user_tcp_conn.proto.tcp->local_port = espconn_port();  //本地端口
//	user_tcp_conn.proto.tcp->local_port = 5000;  //本地端口
//
//	os_printf("local port：%d\r\n", user_tcp_conn.proto.tcp->local_port);
//
//	user_tcp_conn.proto.tcp->remote_port = remote_port;  //目标端口
//	//注册连接成功回调函数和重新连接回调函数
//	espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb);//注册 TCP 连接成功建立后的回调函数
//	espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb);//注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连
//	//启用连接
//	espconn_connect(&user_tcp_conn);
//}

void Check_WifiState(void) {

	uint8 getState;
	getState = wifi_station_get_connect_status();

	os_printf("%s called  wifi status %d\r\n",__func__,getState);

	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP) {
		os_printf("WIFI Connected！\r\n");
		os_timer_disarm(&checkTimer_wifistate);
		const char remote_ip[4] = { 192, 178, 1, 107 };//目标IP地址,必须要先从手机获取，否则连接失败.

		if(upgrade_tcp == 1)
		{
            ota_start_upgrade(remote_ip, 80, "");
		}else
		{

//    		struct ip_info info;
//    		wifi_get_ip_info(STATION_IF, &info);	//查询 WiFi模块的 IP 地址
//    		my_station_init((struct ip_addr *) remote_ip, &info.ip, 5000);//连接到目标服务器的6000端口
		}
	}else
	    wifi_state++;
	if(wifi_state == 20)
	{
	    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
	    wifi_state = 0;
	}
}

void tcp_client_init()	//初始化
{

	wifi_set_opmode(0x01);	//设置为STATION模式

	struct station_config stationConf;
	os_strcpy(stationConf.ssid, "Yajiehui3");	  //改成你自己的   路由器的用户名
	os_strcpy(stationConf.password, "yajiehui2016188"); //改成你自己的   路由器的密码

	wifi_station_set_config(&stationConf);	//设置WiFi station接口配置，并保存到 flash
	wifi_station_connect();	//连接路由器

	upgrade_tcp = 1;

	os_timer_disarm(&checkTimer_wifistate);	//取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState, NULL);	//设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, 1);	//启动定时器，单位：毫秒

}

