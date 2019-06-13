

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


os_timer_t upload_data;
os_timer_t bind_timeout;
uint8_t update_reboot = 0;



#define VOWSTAR_WAN_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)

uint8_t ap_ssid[32] = "qingzhu";
uint8_t  ap_pwd[32] = "12345678";
uint8_t update_host[128];
uint8_t send_order = 0;
uint8_t http_dis = 1;
uint8_t isFixedTime[1] ;

uint8_t version_type;
uint16_t version_num;
//uint8_t http_create[80]={0};

uint8_t loginName[] = "12345678901234567890123456789012";
uint8_t loginPwd[]  = "98765432109876543210987654321098";

uint8_t apmac_rssi[6][8] = {
// 0x24,0x69,0x68,0xF8,0xDA,0x9F,20,2,0x64,0x09,0x80,0x75,0xE0,0x78,61,1,0xC8,0x94,0xBB,0x51,0x8C,0x84,73,8
};
//uint8_t meng[] = "AT*MENGINFO=0\r\n";

uint8_t cclk[] = "AT+CCLK?\r\n";
uint8_t ipr[] = "AT+IPR=0\r\n";
uint8_t at[] = "AT\r\n";
uint8_t read_adc[] = "AT+ZADC?\r\n";
uint8_t con_led[] = "AT+ZCONTLED=1\r\n";

uint8_t at_cfun[] = "AT+CFUN=0\r\n";
//uint8_t e_power_off[] = "AT+ZTURNOFF\r\n";
uint8_t e_power_off[] = "AT+ZRST\r\n";
uint8_t get_cesq[] = "AT+CESQ\r\n";

//uint8_t rx_fifo[2048];

#ifdef test_environment
    uint8_t http_create[] = "AT+EHTTPCREATE=0,41,41,\"\"http://221.122.119.226:8098/\",,,0,,0,,0,\"\r\n";
#else
    uint8_t http_create[] = "AT+EHTTPCREATE=0,40,40,\"\"http://47.105.207.228:8098/\",,,0,,0,,0,\"\r\n";
#endif

//uint8_t edns[] = "AT+EDNS=\"wpupgrade.devicexx.com\"\r\n";//
//uint8_t edns[] = "AT+EDNS=\"bc.qzbdata.com\"\r\n";//  清竹大数据 域名

uint8_t http_con[] = "AT+EHTTPCON=0\r\n";

uint8_t http_post_apmac [] = "AT+EHTTPSEND=0,326,326,\"0,1,14,\"/location/wifi\",0,,16,\"application/json\",273,7B226D6163223a223030303030303030303030302C30302C323430303B3030303030303030303030302C30302C323430303B3030303030303030303030302C30302C323430303B3030303030303030303030302C30302C323430303B3030303030303030303030302C30302C323430303B3030303030303030303030302C30302C323430303B227D,\"\r\n";
                       //     AT+EHTTPSEND=0,326,326,\"0,1,14,\"/location/wifi\",0,,16,\"application/json\",273,7B226D6163223a223630444138333341373946302C37362C323362C323431323B4443464531383545384643462C38332C323431323B4443464531383531433946432C38332C323431323B3634303545393232413642382C38392C323431323B4338334133354236393738312C38372C323431373B227D,"

uint8_t http_get_tag [] = "AT+EHTTPSEND=0,293,293,\"0,1,21,\"/MacGather/getCollect\",0,,16,\"application/json\",233,7b226465766963654964223a2251696e677a68752d5651303030303030303030303030222c2276657273696f6e223a22302e303030222c22776569223a22303030302e3030303030222c226c6e67223a2230303030302e3030303030222c2274696d65223a22303030303030303030303030227d,\"\r\n";
                                                                                                                  // { " d e v i c e I d " : " Q i n g z h u - V Q 0 0 0 0 0 0 0 0 0 0 0 0 " , " v e r s i o n " : " 0 . 0 0 0 " , " w e i " : " 0 0 0 0 . 0 0 0 0 0 " , " l n g " : " 0 0 0 0 0 . 0 0 0 0 0 " , " t i m e " : " 0 0 0 0 0 0 0 0 0 0 0 0 " }
                                                                                                             //2953
uint8_t http_send0[] = "AT+EHTTPSEND=1,3015,442,\"0,1,22,\"/MacGather/submitValue\",0,,16,\"application/json\",2953,7b226465766963654964223a2251696e677a68752d5651454346414243314139463137222c2274696d657374616d70223a22303030303030303030303030222c22636f6c6c6563744964223a223030303030303030303030222c226c6f6e676974756465223a2230303030302e3030303030222c226c61746974756465223a22303030302e3030303030222c226d61635f737472223a223030303030303030303030302c3030303030303030303030302c3030303030303030303030302c\"\r\n";//388
uint8_t http_send1[] = "AT+EHTTPSEND=1,3015,494,\"3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C\"\r\n";
uint8_t http_send2[] = "AT+EHTTPSEND=1,3015,494,\"3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C\"\r\n";
uint8_t http_send3[] = "AT+EHTTPSEND=1,3015,494,\"3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C\"\r\n";
uint8_t http_send4[] = "AT+EHTTPSEND=1,3015,494,\"3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C\"\r\n";
uint8_t http_send5[] = "AT+EHTTPSEND=1,3015,494,\"3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C3030303030303030303030302C\"\r\n";
uint8_t http_send6[] = "AT+EHTTPSEND=0,3015,103,\"3030303030303030303030302c303030303030303030303030222c2266697865644964223a224744303030303030303030227d,\"\r\n";


uint8_t http_discon[] = "AT+EHTTPDISCON=0\r\n";
uint8_t http_destroy[] = "AT+EHTTPDESTROY=1\r\n";

uint8_t probe_flag = 0;
uint8_t creat_flag = 0;

uint8_t gnrmc_gps_flag = 0;
at_state_t at_state = 0;
post_state_t post_state = 0;
uint16_t z_adc = 0;
uint8_t shut_down_flag = 0;
uint8_t get_tag_flag = 0;
uint8_t update_firmware_flag= 1;
uint8_t nb_signal_bad = 1;
uint8_t connected_wifi=0;
uint8_t scan_qz = 0;

uint8_t wifi_net = 0;
uint8  test_str[1512];
uint8  sector_str[4096];

uint16 sector_str_upload_flag_bit;
uint8 mac_inrom_flag=0;
uint8 all_sector_upload_done=1;
uint16 sector_str_totlen=0;
uint8_t first_get_resq = 1;

uint8_t zgmode[] = "AT+ZGMODE=2\r\n";
uint8_t zgnmea[] = "AT+ZGNMEA=2\r\n";
uint8_t zgrun[] = "AT+ZGRUN=2\r\n";
uint8_t zgstop[] = "AT+ZGRUN=0\r\n";

uint8_t zgdata[] = "AT+ZGDATA \r\n";

uint8_t wifi_version_temp[5] ;//0.000
uint8_t parameter_version[]  = "0.000";

uint8_t parameter_deviceId[] = "Qingzhu-VQ112233445566";

//                              QZBData
//uint8_t                    = "vankia-01-68c63a8a2420";

//parameter_longitude=11618.00871;
//parameter_latitude=3958.98470;
//parameter_timestamp=034639110119;
uint8_t parameter_timestamp[] = "111111111111";//都是1,都为0会与tag重叠//修改tag
//                              181101,185415
uint8_t fixedId[] = "GD000000000";


uint8_t parameter_tag[] = "00000000000";//CJ,JD
uint8_t parameter_longitude[] = "00000.00000";
//                              11618.03708
uint8_t parameter_latitude[] = "0000.00000";
//                             3958.98966

os_timer_t rtc_test_t;

typedef struct {
    uint64  time_acc;
    uint32  time_base;
}RTC_TIMER_DEMO;
RTC_TIMER_DEMO  rtc_time;
uint32  rtc_t1, rtc_t2;
uint32  cal1, cal2;
uint64 sys_time_base,sys_time_add;


LOCAL system_status_t local_system_status = {
	.version_type = 0,
	.version_num = 0,
	.used_sector = 0,
	.sector_updata_flag = 0,
	.task_id="CJ000000000",
	.times="111111111111",
};



uint8_t uart_receive_at[2048];
uint8_t uart_fifo_flag = 0;


void read_sector(void );
void send_flash_mac(void);

void ICACHE_FLASH_ATTR
devicexx_app_save(void)
{
    system_param_save_with_protect(
        (DEVICEXX_APP_START_SEC),
        (void *)(&local_system_status),
        sizeof(local_system_status));
}

void ICACHE_FLASH_ATTR
app_save(uint8 flag)
{
    all_sector_upload_done=0;
    local_system_status.sector_updata_flag=flag;
    devicexx_app_save();
}

void ICACHE_FLASH_ATTR
save_stakid(uint8 * task_id)
{
    if(os_strstr(local_system_status.task_id,task_id))
    {
        os_printf("same task id");

    }else
    {
        os_memcpy(local_system_status.task_id,task_id,11);
        os_memcpy(parameter_tag,task_id,11);

        devicexx_app_save();
    }
}

void ICACHE_FLASH_ATTR
delay_power_off()
{

    os_printf("cfun Off \n");

    os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
    queue_uart_send(at_cfun,os_strlen(at_cfun));
    os_printf("send %s\n",at_cfun);
    at_state = ZCFUN;

}


void ICACHE_FLASH_ATTR
ota_finished_callback(void *arg)
{
    struct upgrade_server_info *update = arg;
        if (update->upgrade_flag == true)
        {
            update_reboot = 1;
            os_printf("OTA  Success ! rebooting!\n");
            local_system_status.version_num = version_num;
        	devicexx_app_save();
        }else
        {
            update_reboot = 0;
            os_printf("OTA failed!\n");
        }
        delay_power_off();
}



//LOCAL void ICACHE_FLASH_ATTR
//get_white_list_callback(void * ctx, char * response_body, size_t response_body_size, int http_status, char * response_headers)
//{
//    uint16_t i;
//    os_printf("%s: status:%d\n", __func__, http_status);
//    if ((NULL != response_body) && (0 != response_body_size) && (NULL != response_headers)) {
//        if (200 == http_status) {
//            // Process and try encrypt data
//            for(i=0;i<response_body_size;i++)
//            {
//                os_printf("%c",response_body[i]);
//            }
//        } else {
//            response_body[response_body_size] = '\0';
//            os_printf("%s: error:\n%s\n", __func__, response_body);
//        }
//    }
//}



void ICACHE_FLASH_ATTR
ota_start_upgrade(const char *server_ip, uint16_t port,const char *path)
//ota_start_upgrade(uint16_t port,const char *path)
{
    char file[20];
    if(update_firmware_flag)
    {
        os_memcpy(file,"user0.0.000.bin",15);
        file[15]='\0';
        //获取系统的目前加载的是哪个bin文件
        uint8_t userBin = system_upgrade_userbin_check();
        //0：user1  1:user2
        os_printf("userbin = %d\n",userBin);
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


        file[6] = version_type + 48;
        file[8] = version_num/100 + 48;
        file[9] = version_num%100/10 + 48;
        file[10] = version_num%100%10 + 48;

        os_printf("%c",file[8]);os_printf("%c",file[9]);os_printf("%c",file[10]);
    }else
    {
        os_memcpy(file,"white_list_10_1.txt",19);
        file[19]='\0';
    }

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
    update->check_times = 90000;
    //从 4M *1024 =4096申请内存
    update->url = (uint8 *)os_zalloc(4096);

    //打印下請求地址
//    os_printf("Http Server Address:%d.%d.%d.%d ,port: %d,filePath: %s,fileName: %s \n",
//    		IP2STR(update->ip), update->port, path, file);

    //拼接完整的 URL去请求服务器
//    os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: "IPSTR":%d\r\n"	"Connection: keep-alive\r\n" "\r\n",
//			               path, file, IP2STR(update->ip), update->port);


    if(update_firmware_flag)
    {
        os_sprintf((char*) update->url, "GET /%s%s HTTP/1.1\r\n" "Host: %s:%d\r\n"  "Connection: keep-alive\r\n" "\r\n",
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
    }else
    {
//        os_sprintf((char*) update->url, "%s%s\r\n" "Connection: keep-alive\r\n" "\r\n",
//                               path, file, update->port );
        os_sprintf((char*) update->url, "%s\r\n" ,
                               path);
        // Build URL

        if (NULL != update->url) {
//            void * ctx = NULL;
            // Perform a http get request
//            http_get(ctx, update->url, "Content-Type: text/plain", get_white_list_callback);
            os_printf("%s: url %s\n", __func__, update->url);
        } else {
            os_printf("%s: not enough memory\r\n", __func__);
        }

        // Free memory
        if (update->url)
            os_free(update->url);
    }
}



void ICACHE_FLASH_ATTR
power_on()
{

    GPIO_OUTPUT_SET(PIN_POWER, 1);
#ifdef OLED_VERSION
    GPIO_OUTPUT_SET(PIN_LED_S, 0);
#endif
    os_printf("Power On\n");
    if(shut_down_flag == 1)
    {
        shut_down_flag = 0;
        system_deep_sleep_set_option(1);
        system_deep_sleep(0);
    }else
    {
        if(update_reboot == 1)
        {
            system_upgrade_reboot();
            update_reboot = 0;
        }
        else
            system_restart();
    }
}



void ICACHE_FLASH_ATTR
rtc_count(void)
{
    system_rtc_mem_read( 64, &rtc_time, sizeof(rtc_time) );
    rtc_t2  = system_get_rtc_time();
    cal2    = system_rtc_clock_cali_proc();

    rtc_time.time_acc += ( ( (uint64) (rtc_t2 - rtc_time.time_base) ) * ( (uint64)
                                          ( (cal2 * 1000) >> 12) ) );

    os_printf( "sys_time_base : %lld S\r\n", sys_time_base);
    os_printf( "sys_time_add  : %lld S\r\n", sys_time_add);

//    os_printf( "power on time : %lld.%02lld S\r\n", (rtc_time.time_acc / 10000000) / 100,
//           (rtc_time.time_acc / 10000000) % 100 );
    rtc_time.time_base = rtc_t2;
    system_rtc_mem_write( 64, &rtc_time, sizeof(rtc_time) );
}

void ICACHE_FLASH_ATTR
resolution_times( char * buffer )
{
    uint8_t i;
    uint8_t * date_addr = NULL;

    uint8_t mouth_ab[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};
    uint8_t mouth_num[12][3]={"01","02","03","04","05","06","07","08","09","10","11","12"};

    char *p,*n;

    for(i=0;i<12;i++)
    {
        p=mouth_ab[i];
        n=mouth_num[i];
        if(strstr(buffer,p))
        {
            os_memcpy(parameter_timestamp+8,n,2);//月
        }
    }

    date_addr = strstr(buffer,"Date:");

    os_memcpy(parameter_timestamp+6,date_addr+11,2);//日
    os_memcpy(parameter_timestamp+10,date_addr+20,2);//年
    os_memcpy(parameter_timestamp,date_addr+23,2);//时
    os_memcpy(parameter_timestamp+2,date_addr+26,2);//分
    os_memcpy(parameter_timestamp+4,date_addr+29,2);//秒

    rtc_count();
    sys_time_base=(rtc_time.time_acc / 10000000) / 100;
//    034639110119
//    date_addr = NULL;
    os_printf("parameter_timestamp %s\n",parameter_timestamp);

    os_memcpy(local_system_status.times,parameter_timestamp,12);
    devicexx_app_save();
}

uint8_t ICACHE_FLASH_ATTR
strtoi(uint8_t *str)
{
    return ((str[0]-48)*10+str[1]-48);
}

void ICACHE_FLASH_ATTR
itostr(uint8_t *num , uint8_t *str)
{

    str[0]=*num/10+48;
    str[1]=*num%10+48;

}



void ICACHE_FLASH_ATTR
update_timestamp(void)
{
    uint8  iY, iM, iD, iH, iMin, iS;
    uint8_t sY[2],sM[2],sD[2],sH[2],sMin[2],sS[2];


//    os_memcpy(parameter_timestamp,"235958311216",os_strlen(parameter_timestamp));
    os_printf("parameter_timestamp:%s\n",parameter_timestamp);

    os_memcpy(sY,parameter_timestamp+10,2);
    os_memcpy(sM,parameter_timestamp+8,2);
    os_memcpy(sD,parameter_timestamp+6,2);
    os_memcpy(sH,parameter_timestamp,2);
    os_memcpy(sMin,parameter_timestamp+2,2);
    os_memcpy(sS,parameter_timestamp+4,2);

    iY = strtoi(sY);
    iM = strtoi(sM);
    iD = strtoi(sD);
    iH = strtoi(sH);
    iMin = strtoi(sMin);
    iS = strtoi(sS);
    os_printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);
//    st1 = 0;
    rtc_count();
    sys_time_add=(rtc_time.time_acc / 10000000) / 100;


    iS+=(sys_time_add-sys_time_base);
    if(iS>59){
        iMin=iMin+iS/60;
        iS%=60;
        if(iMin>59){
            iH=iH+iMin/60;
            iMin%=60;
            if(iH>23){
                iD=iD+iH/24;
                iH%=24;
                switch (iM)
                {
                    case 1:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 2:
                    {
                        if((iY%4==0 && iY%100!=0) || iY%400==0){//闰年
                            if(iD>29){
                                iD-=29;
                                iM++;
                            }
                        }else{
                            if(iD>28){
                                iD-=28;
                                iM++;
                            }
                        }
                    }break;
                    case 3:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 4:
                    {
                        if(iD>30){
                            iD-=30;
                            iM++;
                        }
                    }break;
                    case 5:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 6:
                    {
                        if(iD>30){
                            iD-=30;
                            iM++;
                        }
                    }break;
                    case 7:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 8:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 9:
                    {
                        if(iD>30){
                            iD-=30;
                            iM++;
                        }
                    }break;
                    case 10:
                    {
                        if(iD>31){
                            iD-=31;
                            iM++;
                        }
                    }break;
                    case 11:
                    {
                        if(iD>30){
                            iD-=30;
                            iM++;
                        }
                    }break;
                    case 12:
                    {
                        if(iD>31){
                            iD-=31;
                            iM=1;
                            iY++;
                        }
                    }break;
                }
            }
        }
    }

    os_printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);

    itostr(&iY,sY);
    itostr(&iM,sM);
    itostr(&iD,sD);
    itostr(&iH,sH);
    itostr(&iMin,sMin);
    itostr(&iS,sS);


    os_memcpy(parameter_timestamp+10,sY,2);
    os_memcpy(parameter_timestamp+8,sM,2);
    os_memcpy(parameter_timestamp+6,sD,2);
    os_memcpy(parameter_timestamp,sH,2);
    os_memcpy(parameter_timestamp+2,sMin,2);
    os_memcpy(parameter_timestamp+4,sS,2);

    os_memcpy(local_system_status.times,parameter_timestamp,12);
    devicexx_app_save();


    os_printf("parameter_timestamp:%s\n",parameter_timestamp);
}

void ICACHE_FLASH_ATTR
update_tag()
{
    os_printf("version_num:%d\n",local_system_status.version_num);
    http_get_tag[185] = local_system_status.version_num/100 + 48;
    http_get_tag[187] = local_system_status.version_num%100/10 + 48;
    http_get_tag[189]= local_system_status.version_num%100%10 + 48;
}


void ICACHE_FLASH_ATTR
what_do(uint8_t times)
{

    switch (send_order)
    {

    case 0:

        os_timer_disarm(&check_id_timer);
        os_timer_arm(&check_id_timer, 10000, 0);


        break;

    case 1://发请求绑定
//        queue_uart_send(http_post_bind,os_strlen(http_post_bind));
//        os_printf("http_post_bind %s\n",http_post_bind);
//
//        os_timer_disarm(&bind_timeout);
//        os_timer_setfn(&bind_timeout, (os_timer_func_t *) bind_timeout_cb, NULL);
//        os_timer_arm(&bind_timeout, 15000, 0);
//
//        at_state = WAIT;
        break;

    case 2://发100个mac
//        if( 0 == os_strcmp(parameter_longitude,"00000.00000") && 0 == os_strcmp(parameter_latitude,"0000.00000") )
//        {
//            at_state = NONE;
//        }else
        {
            queue_uart_send(http_send0,os_strlen(http_send0));
            os_printf("send0 %s\n",http_send0);
            at_state = HTTP_SEND;
        }
        break;

    case 3://发6个ap mac

//        uint8_t http_post_apmac_tt [] = "AT+EHTTPSEND=0,326,326,\"0,1,14,\"/location/wifi\",0,,16,\"application/json\",273,7B226D6163223a224534413743353541463530322C35362C323431323B4436314133463242324244312C36392C323436323B3241413032423231343044442C37352C323431323B4538344530363730323243302C38312C323435373B3743373636383638343343302C38322C323433373B4443463039453832414537302C38352C323431323B227D,\"\r\n";
//        os_memcpy(http_post_apmac,http_post_apmac_tt,os_strlen(http_post_apmac_tt));
        queue_uart_send(http_post_apmac,os_strlen(http_post_apmac));
        os_printf("send %s\n",http_post_apmac);

        at_state = WAIT;

        if(http_dis == 0)
        {
            os_timer_disarm(&delay_discon_timer);
            os_timer_arm(&delay_discon_timer, 20000, 0);//20秒后发送
        }

        os_timer_disarm(&delay_update_timer);
        os_timer_arm(&delay_update_timer, 40000, 0);//20秒后发送


        break;
    case 4://再次获取任务id

        update_tag();
        queue_uart_send(http_get_tag,os_strlen(http_get_tag));
        os_printf("send %s\n",http_get_tag);
        at_state = WAIT;
        break;

    default:
        break;
    }
}
void ICACHE_FLASH_ATTR
get_rssi()
{
    os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
    queue_uart_send(get_cesq,os_strlen(get_cesq));
    os_printf("send get_cesq %s\n",get_cesq);

}

void ICACHE_FLASH_ATTR
create_http(uint8_t times)
{

    os_timer_disarm(&restart_nb);
    os_timer_arm(&restart_nb, 180000, 1);//3分钟后没有收到数据，重启
    send_order = times;
    os_printf("send_order %d\n",send_order);
    os_printf("http_dis %d\n",http_dis);

    if(send_order == 0 ||  creat_flag == 0)
    {
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_create,os_strlen(http_create));
        os_printf("send http_create %s\n",http_create);

    }else
    {
        if(http_dis == 1)
        {
            os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            queue_uart_send(http_con,os_strlen(http_con));
            os_printf("send http_create %s\n",http_con);
            at_state = HTTP_CON;
        }else
        {
            what_do(send_order);
        }
    }


    os_printf("at_state %d\n",at_state);
}
void ICACHE_FLASH_ATTR
ap_str_ascii_str(char * ap_str)
{
    os_printf("wifi status %d\r\n",wifi_station_get_connect_status());
    os_printf("scan_qz:%d\n",scan_qz);

    if (wifi_station_get_connect_status() == STATION_GOT_IP) {
        scan_qz=1;
    }else
    {
        scan_qz=0;

        uint16_t len = os_strlen(ap_str);
        os_printf("len=%d\n",len);
        uint8_t str[len*2];
        uint16_t i;


        for(i=0;i<len*2;i++)
        {
            str[i] = (ap_str[i/2] >> 4) & 0xf;
            str[i+1] = ap_str[i/2] & 0xf;
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

    //    os_printf("ap_ascii %s\n",str);
        os_memcpy(http_post_apmac+93,str, len*2);
        os_printf("ap_ascii %s\n",http_post_apmac);
        create_http(3);
    }
}


//
//cmd 1      2       3    4   5
//deviceId  version wei lng time
void ICACHE_FLASH_ATTR
strhex_to_str(char * strhex, uint8 cmd)
{
    uint16_t len = os_strlen(strhex);
    uint8_t str[len*2];
    uint16_t i;


    for(i=0;i<len*2;i++)
    {
        str[i] = (strhex[i/2] >> 4) & 0xf;
        str[i+1] = strhex[i/2] & 0xf;
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

    switch (cmd)
    {
		case 1://deviceId
		{
			os_memcpy(http_get_tag+110, str,  len*2);
		}
		break;
		case 2://version
		{



		}
		break;
		case 3://wei
		{
			os_memcpy(http_get_tag+208, str,  len*2);
		}
		break;
		case 4://lng
		{
			 os_memcpy(http_get_tag+246, str, len*2);
		}
		break;
		case 5://time
		{
			 os_memcpy(http_get_tag+288, str, len*2);
		}
		break;
    }
}




void ICACHE_FLASH_ATTR
int_to_str(uint16_t num , uint8_t *str)
{
    if(num<10)
        str[0]=num+48;
    else if(num<100)
    {
        str[0]=num/10+48;
        str[1]=num%10+48;
    }else if(num<1000)
    {
        str[0]=num/100+48;
        str[1]=num%100/10+48;
        str[2]=num%100%10+48;
    }else if(num<10000)
    {
        str[0]=num/1000+48;
        str[1]=num%1000/100+48;
        str[2]=num%1000%100/10+48;
        str[3]=num%1000%100%10+48;
    }


}




void ICACHE_FLASH_ATTR
str_ascii_str(char * body)
{
    uint8_t str[2955];
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
    len++;
    os_printf("len %d str %s \n",len,str);
    os_printf("\n");




    os_memcpy(http_send0+87,str    , 378);
    os_memcpy(http_send1+25,str+380, 494);
    os_memcpy(http_send2+25,str+874, 494);
    os_memcpy(http_send3+25,str+1368,494);
    os_memcpy(http_send4+25,str+1862,494);
    os_memcpy(http_send5+25,str+2356,494);
    os_memcpy(http_send6+25,str+2850,50 );


    os_printf("send0 %s\n",http_send0);
    os_printf("send1 %s\n",http_send1);
    os_printf("send2 %s\n",http_send2);
    os_printf("send3 %s\n",http_send3);
    os_printf("send4 %s\n",http_send4);
    os_printf("send5 %s\n",http_send5);
    os_printf("send6 %s\n",http_send6);

    create_http(2);



}

void ICACHE_FLASH_ATTR
update_data()//上传mac数据
{
    VOWSTAR_WAN_DEBUG("\r\n\r\n\r\n%s: request\r\n", __func__);

    update_timestamp();
    if (wifi_station_get_connect_status() == STATION_GOT_IP) {
        scan_qz=1;
    }else scan_qz=0;


    if(scan_qz == 1)
    {
        uint8_t * body = (uint8_t *) os_zalloc(os_strlen(JSON_FIX_MAC) +
                                               os_strlen(parameter_deviceId) +
                                               os_strlen(parameter_timestamp) +
                                               os_strlen(parameter_tag) +
                                               os_strlen(parameter_longitude) +
                                               os_strlen(parameter_latitude) +
                                               os_strlen(sta_str)+
                                               os_strlen(fixedId));
        if (body == NULL) {
            VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
            return;
        }

//        uint8_t i;
//        for(i=0;i<12;i++)
//        {
//            if(parameter_timestamp[i]>57 || parameter_timestamp[i]<48)
//            {
//                os_memcpy(parameter_timestamp,"000000000000",os_strlen(parameter_timestamp));
//                break;
//            }
//        }

        os_sprintf(body,
                   JSON_FIX_MAC,
                   parameter_deviceId,
                   parameter_timestamp,
                   parameter_tag,
                   parameter_longitude,
                   parameter_latitude,
                   sta_str,
                   fixedId);

    uint8_t * url = NULL;


#ifdef test_environment
        url = "http://221.122.119.226:8098/MacGather/submitValue";
#else
        url = "http://47.105.207.228:8098/MacGather/submitValue";
#endif
        void * ctx = NULL;
        http_post(ctx, url, "Content-Type:application/json\r\n", (const char *) body, os_strlen(body), post_callback);

        if (body)
            os_free(body);
//        url = NULL;
//        ctx = NULL;
    }else
    {

        if(os_strlen(sta_str)<11)
        {
            led_state = 5;
            devicexx_io_led_timer_tick();
            os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            queue_uart_send(read_adc,os_strlen(read_adc));
            at_state = ZADC;
        }else
        {
            uint8_t * all_str = "100000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,";
            all_str[0]='0';
            os_memcpy(all_str+(1300-os_strlen(sta_str)),sta_str,os_strlen(sta_str));
            uint8_t * body2 = (uint8_t *) os_zalloc(os_strlen(JSON_FIX_MAC) +
                                                   os_strlen(parameter_deviceId) +
                                                   os_strlen(parameter_timestamp) +
                                                   os_strlen(parameter_tag) +
                                                   os_strlen(parameter_longitude) +
                                                   os_strlen(parameter_latitude) +
                                                   os_strlen(all_str)+
                                                   os_strlen(fixedId));
            if (body2 == NULL) {
                VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
                return;
            }

            os_sprintf(body2,
                       JSON_FIX_MAC,
                       parameter_deviceId,
                       parameter_timestamp,
                       parameter_tag,
                       parameter_longitude,
                       parameter_latitude,
                       all_str,
                       fixedId);
            str_ascii_str(body2);

            if (body2)
                os_free(body2);
        }
    }


}


void ICACHE_FLASH_ATTR
nbiot_http_post()
{


    os_printf("Gather done\n");
    sni_temp = 0;

    wifi_promiscuous_enable(0);
    wifi_unregister_send_pkt_freedom_cb();

    os_printf("wifi status %d\r\n",wifi_station_get_connect_status());



    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
    if(get_gps == 1)
    {
        post_state = IDLE;
        os_timer_disarm(&upload_data);
        os_timer_setfn(&upload_data, (os_timer_func_t *)update_data, NULL);
        os_timer_arm(&upload_data, 21000, 0);//开始上传数据

	}
        os_timer_disarm(&scan_timer);
        os_timer_arm(&scan_timer, 3000, 0);

}




void ICACHE_FLASH_ATTR
check_id(void)
{
    update_tag();
    //再次获取id
    if (wifi_station_get_connect_status() == STATION_GOT_IP) {
        scan_qz=1;
    }else scan_qz=0;
    if(scan_qz == 1)
    {
        //            "deviceId":"Qingzhu-VQ000000000000","version":"0.000","wei":"0000.00000","lng":"00000.00000","time":"000000000000"}
        // Build Body's JSON string

        uint8_t * body = (uint8_t *) os_zalloc(os_strlen(JSON_GET_TAG) +
                                               os_strlen(parameter_deviceId) +
                                               os_strlen(parameter_version) +
                                               os_strlen(parameter_latitude) +
                                               os_strlen(parameter_longitude) +
                                               os_strlen(parameter_timestamp)
                                               );

        if (body == NULL) {
            VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
            return;
        }

        os_sprintf(body,
                   JSON_GET_TAG,
                   parameter_deviceId,
                   parameter_version,
                   parameter_latitude,
                   parameter_longitude,
                   parameter_timestamp);

        os_printf("body:%s\n",body);

        uint8_t * url = NULL;
#ifdef test_environment
        url = "http://221.122.119.226:8098/MacGather/getCollect";
#else
        url = "http://47.105.207.228:8098/MacGather/getCollect";
#endif
        void * ctx = NULL;
        http_post(ctx, url, "Content-Type:application/json\r\n", (const char *) body, os_strlen(body), post_callback);

        url = NULL;
        ctx = NULL;
        if (body)
            os_free(body);
    }else
    {
        create_http(4);
    }

}

void ICACHE_FLASH_ATTR
http_disc(void)
{
    if(http_dis == 0)
    {
        queue_uart_send(http_discon,os_strlen(http_discon));
        os_printf("send %s\n",http_discon);
        http_dis = 1;
        at_state = WAIT;
    }
}


void ICACHE_FLASH_ATTR
start(void)
{
    led_state = 1;
    devicexx_io_led_timer_tick();
    probe_flag = 1;
    gnrmc_gps_flag = 0;

    os_timer_disarm(&checkTimer_wifistate); //取消定时器定时
    os_timer_disarm(&wait_nb_con_timer);
    os_timer_disarm(&scan_timer);

    sniffer_init();
    sniffer_init_in_system_init_done();
}




//void ICACHE_FLASH_ATTR
//update_white_list(void)
//{
//
//        tcp_client_init(ap_ssid,ap_pwd);//OTA升级
//
//
//}

void ICACHE_FLASH_ATTR
check_update_firmware(uint8 type, uint8 num ,char *url)
{
    os_printf("type:%d,num:%d\n",type,num);
    if(local_system_status.version_type == type && local_system_status.version_num < num)
    {
        os_memcpy(update_host,url,os_strlen(url));
        tcp_client_init(ap_ssid,ap_pwd);//OTA升级
        post_state = OTA;
    }else
    {
        os_printf("wait...\n");
        os_timer_disarm(&check_id_timer);
        os_timer_arm(&check_id_timer, 100000, 0);
    }

}

void ICACHE_FLASH_ATTR
update_firmware(void)
{
    uint8_t * version_addr = NULL;
    uint8_t version_temp[11] ;//302e303031



    version_addr = strstr(uart_receive_at,"76657273696f6e");//version

    os_memcpy(version_temp,version_addr+20,10);
    version_temp[10] = '\0';
    os_printf("version_temp %s\n",&version_temp);
    version_type = version_temp[1]-48;
    version_num = (version_temp[5]-48)*100 + (version_temp[7]-48)*10 + version_temp[9]-48;
    os_printf("version_type %d version_num %d\n",version_type,version_num);

    os_printf("local_system_status.version_num %d\n",local_system_status.version_num);
    if(local_system_status.version_type == version_type && local_system_status.version_num < version_num)
    {

        uint8_t url_temp[128] ;
        uint8_t * url_addr = NULL;
        uint8_t url_len, i;

//        local_system_status.version_num = version_num;
        url_addr = strstr(uart_receive_at,"75726c");//url,缓存服务器返回的URL，存放新固件的服务器
        url_len = version_addr-url_addr-18;
        os_memcpy(url_temp,url_addr+12,url_len);

        url_temp[url_len] = '\0';

        for(i=0;i<url_len;i++) if(url_temp[i]<58) url_temp[i]-=48; else url_temp[i]-=87;
        for(i=0;i<url_len/2;i++) update_host[i] = ((url_temp[i*2] << 4) + url_temp[1+i*2]);

        os_printf("update_host %s \n",update_host);

        //传递ap_ssid ap_pwd
        tcp_client_init(ap_ssid,ap_pwd);//OTA升级
        url_addr = NULL;
    }else
    {
        os_timer_disarm(&check_id_timer);
        os_timer_arm(&check_id_timer, 120000, 1);
    }
    version_addr = NULL;
}


void ICACHE_FLASH_ATTR
receive_ok(void)
{
    switch (at_state)
    {
    case LED:
        //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(read_adc,os_strlen(read_adc));
        os_printf("send %s\n",read_adc);
        at_state = ZADC;
        break;

    case ZADC:  //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
        z_adc = (uart_receive_at[6]-48)*100+(uart_receive_at[7]-48)*10+uart_receive_at[8]-48;

        if(z_adc>544)
        {
            //获取时间
            queue_uart_send(cclk,os_strlen(cclk));
            os_printf("send %s\n",cclk);
            at_state = CCLK;
        }else delay_power_off();
        break;


    case CCLK:

        os_printf("clk parameter_timestamp %s \n",parameter_timestamp);


        //查询 ESP8266 WiFi station 接口连接 AP 的状态
        if (wifi_station_get_connect_status() == STATION_GOT_IP) {
            scan_qz=1;
            if(creat_flag == 0)
                create_http(0);
            check_id();
        }else{
            //等待回应：未绑定/定位错误
            if(creat_flag == 0)
                create_http(0);
            else//再次获取id
            {
                create_http(4);
            }
        }
        break;

    case ZCFUN:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(e_power_off,os_strlen(e_power_off));
        os_printf("send %s\n",e_power_off);
        at_state = ZOFF;
        break;

    case ZOFF:
        os_printf("NB turned off, now system turn off\n");
//            delay_power_on();//关机
//        shut_down_flag = 1;
//                os_timer_disarm(&temer_90s);
        power_on();
        break;

    case ZGMODE:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(zgnmea,os_strlen(zgnmea));
        os_printf("send %s\n",zgnmea);
        at_state = ZGNMEA;
        break;

    case ZGSTOP:
//                zgmode[10] = '1';
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(zgmode,os_strlen(zgmode));
//                os_printf("send %s\n",zgmode);

        os_timer_disarm(&restart_nb);

//                start();

        break;


    case ZGNMEA:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(zgrun,os_strlen(zgrun));
        os_printf("send %s\n",zgrun);
        at_state = ZGRUN;
        if(http_dis == 0)
        {
            queue_uart_send(http_discon,os_strlen(http_discon));
            os_printf("send %s\n",http_discon);
            http_dis = 1;
        }
        break;

    case ZGRUN:
        start();
        at_state = WAIT;
        break;

    case HTTP_CON:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        http_dis = 0;
        what_do(send_order);


        break;

    case HTTP_SEND:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_send1,os_strlen(http_send1));
        os_printf("send1 %s\n",http_send1);
        at_state = HTTP_SEND1;
        break;

    case HTTP_SEND1:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_send2,os_strlen(http_send2));
        os_printf("send2 %s\n",http_send2);
        at_state = HTTP_SEND2;
        break;

    case HTTP_SEND2:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_send3,os_strlen(http_send3));
        os_printf("send3 %s\n",http_send3);
        at_state = HTTP_SEND3;
        break;

    case HTTP_SEND3:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_send4,os_strlen(http_send4));
        os_printf("send4 %s\n",http_send4);
        at_state = HTTP_SEND4;
        break;

    case HTTP_SEND4:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_send5,os_strlen(http_send5));
        os_printf("send5 %s\n",http_send5);
        at_state = HTTP_SEND5;
        break;

    case HTTP_SEND5:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
//        if(isFixedTime[0] == 48)
//        {
//            queue_uart_send(http_send7,os_strlen(http_send7));
//            os_printf("send6 %s\n",http_send7);
//        }else
//        {
            queue_uart_send(http_send6,os_strlen(http_send6));
            os_printf("send6 %s\n",http_send6);
//        }

        os_printf("send done\n");
        gnrmc_gps_flag = 0;
        at_state = WAIT;
        break;

    case HTTP_DISCON:
    case HTTP_DESTROY:
        os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
        queue_uart_send(http_con,os_strlen(http_con));
        os_printf("send %s\n",http_con);
        at_state = HTTP_CON;
        break;

    default:
        break;

    }
}

void ICACHE_FLASH_ATTR
get_indoor_position(void)
{

    uint8_t i;
    uint8_t * lon_addr = NULL;
    uint8_t mmac_str[73];
//7b22636f6465223a2230222c226c6f6e223a2231313631382e3032393832222c226c6174223a22333935392e3030323539222c226d6163223a22323436393638463844413946463845373145314545303538384341423845414136303238227d
//7b22636f6465223a2230222c226c6f6e223a2231313631382e3031353939222c226c6174223a22333935392e3030313331222c226d6163223a22463845373145314544444238323436393638463844413946463845373145314545303538227d
// { " c o d e " : " 0 " , " l o n " : " 1 1 6 1 8 . 0 1 5 9 9 " , " l a t " : " 3 9 5 9 . 0 0 1 3 1 " , " m a c " : " F 8 E 7 1 E 1 E D D B 8 2 4 6 9 6 8 F 8 D A 9 F F 8 E 7 1 E 1 E E 0 5 8 " }
    lon_addr = strstr(uart_receive_at,"2c226c6f6e22");//"lon"
    mmac_str[72]='\0';

    os_memcpy(mmac_str, lon_addr+96 , 72);
    os_printf("mmac_str %s\n",&mmac_str);

    if(strstr(uart_receive_at,"226c6f6e223a2230303030302e303030303022"))
    {

    }else{
        for(i=0;i<11;i++)
        {
            parameter_longitude[i] = *(lon_addr+17+i*2);
        }
        parameter_longitude[5] = '.';
        for(i=0;i<10;i++)
        {
            parameter_latitude[i] = *(lon_addr+57+i*2);
        }
        parameter_latitude[4] = '.';
    }
    os_printf("parameter_longitude=%s\nparameter_latitude=%s\n",parameter_longitude,parameter_latitude);

    os_memcpy(http_get_tag+246, lon_addr+16, 22);
    os_memcpy(http_get_tag+208, lon_addr+56, 20);


    for(i=0;i<72;i++)
    {
        mmac_str[i]-=48;
    }

    for(i=0;i<36;i++)
    {
        mmac_str[i]=mmac_str[i*2]*16+mmac_str[i*2+1];
        if(mmac_str[i]<58)
            mmac_str[i]-=48;
        else
            mmac_str[i]-=55;

    }
    for(i=0;i<36;i++)
    {
        mmac_str[i]=mmac_str[i*2]*16+mmac_str[i*2+1];
    }
//                for(i=0;i<18;i++)
//                    os_printf("mmac_str %x\n",mmac_str[i]);

    for(i=0;i<6;i++)
    {
        apmac_rssi[0][i]=mmac_str[i];
        apmac_rssi[1][i]=mmac_str[i+6];
        apmac_rssi[2][i]=mmac_str[i+12];
    }
    for(i=0;i<3;i++)
    {
        os_printf( "%2d-----%02X:%02X:%02X:%02X:%02X:%02X ,rssi:%d ,channel:%d\r\n", i,
               apmac_rssi[i][0], apmac_rssi[i][1], apmac_rssi[i][2],
               apmac_rssi[i][3], apmac_rssi[i][4], apmac_rssi[i][5],
               apmac_rssi[i][6],apmac_rssi[i][7] );
    }

    lon_addr = NULL;
    post_state = IDLE;
    os_timer_disarm(&upload_data);
    os_timer_setfn(&upload_data, (os_timer_func_t *)update_data, NULL);
    os_timer_arm(&upload_data, 2000, 0);//开始上传数据
}



//void ICACHE_FLASH_ATTR
//qz_dns(void)//解析清竹域名
//{
//    uint8_t total_len,i;
//    char * ip_addr = NULL;
//    char * ok_addr = NULL;
//
//    uint8_t dns_ip[] = "000.000.000.000";
//    uint8_t http_create_before_half[] = "AT+EHTTPCREATE=0,40,40,\"\"http://";
//    uint8_t http_create_after_half[] = ":8098/\",,,0,,0,,0,\"\r\n";
//
//    ip_addr = strstr(uart_receive_at,"IPV4:");
//    ok_addr = strstr(uart_receive_at,"OK");
//    os_memcpy(dns_ip, ip_addr+5, ok_addr-ip_addr-8);
//
//
//    total_len = ok_addr-ip_addr-9+26;
//    dns_ip[ok_addr-ip_addr-9]='\0';
////    for(i=0;i<ok_addr-ip_addr-9;i++)
////    {
////        os_printf("dns_ip=%d\n",dns_ip[i]);
////    }
//    os_printf("dns_ip=%s=%d\n",&dns_ip,total_len);
//
//    http_create_before_half[17] = total_len/10+48;
//    http_create_before_half[18] = total_len%10+48;
//    http_create_before_half[20] = http_create_before_half[17];
//    http_create_before_half[21] = http_create_before_half[18];
//    os_sprintf(http_create, "%s%s", http_create_before_half, dns_ip);
//
//    os_memcpy(http_create+total_len+6,http_create_after_half,21);
//
//}

void ICACHE_FLASH_ATTR
uart_receive(const uint8_t * pdata, uint16_t length)
{
    if (wifi_station_get_connect_status() == STATION_GOT_IP) {
        scan_qz=1;
    }else scan_qz=0;
    if(scan_qz == 0)
    {
        os_timer_disarm(&check_id_timer);
        os_timer_arm(&check_id_timer, 120000, 1);

        os_timer_disarm(&restart_nb);
        os_timer_arm(&restart_nb, 180000, 1);//3分钟后没有收到数据，重启
    }


	os_printf("+++++++++++UART Data received+++++++++++++\n");
	uint8_t end[1] = {'\0'};
	os_memcpy(pdata+length,end,1);

	os_printf("rev_len %d :%s \r\n",length, pdata);
	os_printf("----------------------------------------------\n");
    os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
    os_memcpy(uart_receive_at,pdata,length);

    {

        if(os_strstr(uart_receive_at,"Date:"))
        {
            resolution_times(uart_receive_at);
        }

//"+CESQ: 22,6,255,255,12,45"
//        rev_len 69 :$GNRMC,,V,,,,,,,,,,N,V*37
//        AT+CESQ
//        +CESQ: 0,0,255,255,12,12
//
//        OK

        if(os_strstr(uart_receive_at,"+CESQ:"))
        {
            uint8_t at_cesq_back[23];
            os_memset(at_cesq_back,'\0',23);

            uint8_t * addr_cesq = NULL;
            addr_cesq = strstr(uart_receive_at,"+CESQ:");
            os_memcpy(at_cesq_back,addr_cesq,22);
//            addr_cesq = NULL;

            uint8_t * addr1 = NULL;
            uint8_t * addr2 = NULL;
            addr1 = strstr(at_cesq_back,":");
            addr2 = strstr(at_cesq_back,",");
            os_printf("addr1:%c\naddr2:%c\naddr2-addr1:%d\n",addr1[0],addr2[0],addr2-addr1);
            if(addr2-addr1 == 3 || (*(addr2-2))==0x39)
            {
                os_printf("nb_signal_bad\r\n");
                nb_signal_bad = 1;
//                post_state=CHECK_ID;
//                switch_to_wifi();
            }
            else
            {
                os_printf("nb_signal_ok\r\n");
                nb_signal_bad = 0;

//                nbiot_http_post();


            }

            nbiot_http_post();

            addr1 = NULL;
            addr2 = NULL;


            if (wifi_station_get_connect_status() == STATION_GOT_IP) {
                connected_wifi = 1;
            }else connected_wifi = 0;
        }

        // Send back what received
    //	uart_send(pdata, length);
//        os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//        os_memcpy(uart_receive_at,pdata,length);

        if(os_strstr(uart_receive_at,"+CPIN: READY"))
        {
            os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            queue_uart_send(ipr,os_strlen(ipr));
            at_state = WAIT;
        }

        if(os_strstr(uart_receive_at,"+IP:"))
        {

            os_timer_disarm(&wait_nb_con_timer);

            uint8_t i;

            os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            queue_uart_send(con_led,os_strlen(con_led));
            at_state = LED;
            os_printf("send %s\n",con_led);


            return;
        }


        if(os_strstr(uart_receive_at,"+EHTTPCREAT:"))
        {
            creat_flag = 1;
            if(uart_receive_at[81] == 1)
            {
                queue_uart_send(http_destroy,os_strlen(http_destroy));
                os_printf("send %s\n",http_destroy);
                at_state = HTTP_DESTROY;
            }
            os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            queue_uart_send(http_con,os_strlen(http_con));
            os_printf("send %s\n",http_con);
            http_dis = 0;
            at_state = HTTP_CON;
            return;
        }

        if(os_strstr(uart_receive_at,"+EHTTPERR:0"))//0是客户端编号
        {
            http_dis = 1;
            return;
        }



//        if(os_strstr(uart_receive_at,"IPV4:"))
//        {
//            uint8_t i;
//            qz_dns();
//
//
//            os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//            queue_uart_send(con_led,os_strlen(con_led));
//            at_state = LED;
//            os_printf("send %s\n",con_led);
//
//            for(i=0;i<12;i++)
//                parameter_timestamp[i] = '0';
//            return;
//        }


        if(os_strstr(uart_receive_at,"ERROR"))
        {

            os_printf("error\n");
            os_printf("at_state = %d\n",at_state);
            switch (at_state)
            {

//            case EDNS:
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(edns,os_strlen(edns));
//                os_printf("send %s\n",edns);
//                break;


            case ZOFF:
                delay_power_off();
                break;
//
//            case HTTP_CON:
////                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
////                queue_uart_send(http_discon,os_strlen(http_discon));
////                os_printf("discon %s\n",http_discon);
////                at_state = HTTP_DISCON;
////                break;
////

////
////            case HTTP_DESTROY:
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(http_destroy,os_strlen(http_destroy));
//                os_printf("send %s\n",http_destroy);
//                at_state = ZGMODE;
//                break;

            default:
                delay_power_off();
                break;
            }
        }

        if(os_strstr(uart_receive_at,"OK"))
        {

            os_printf("at_state = %d\n",at_state);
            receive_ok();
        }



        if(os_strstr(uart_receive_at,"+EHTTPNMIC:"))//服务器有响应
        {
            nb_signal_bad=0;
            if(os_strstr(uart_receive_at,"2264657363223a2273756363657373227d"))
            {
                os_printf("{\"success\"}\n");
                os_printf("{\"success\"}\n");
                os_printf("{\"success\"}\n");
                os_printf("{\"success\"}\n");
                if(mac_inrom_flag==0)
                    sector_str[sector_str_upload_flag_bit]='0';
                mac_inrom_flag=0;
                if(all_sector_upload_done==0)
                {
                    connected_wifi=0;
                    send_flash_mac();
                }else
                {
                    led_state = 5;
                    devicexx_io_led_timer_tick();
    //                send_flag = 0;
                    //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
                    os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
                    queue_uart_send(read_adc,os_strlen(read_adc));
                    at_state = ZADC;
                }

            }else if(os_strstr(uart_receive_at,"e59cb0e79086e58cbae59f9fe5a496")||os_strstr(uart_receive_at,"e5bd93e5898de697a0e4bbbbe58aa1")||os_strstr(uart_receive_at,"e58f82e695b0e4b8bae7a9ba"))
            {
                //地理区域外         当前无任务       参数为空
                os_timer_disarm(&check_id_timer);
                os_timer_arm(&check_id_timer, 120000, 1);
                led_state = 5;
                devicexx_io_led_timer_tick();

            }else if(os_strstr(uart_receive_at,"636f6c6c6563744964"))//获得id,查询版本
            {
                os_timer_disarm(&err_restart);
                os_timer_arm(&err_restart, 240000, 0);

                if(all_sector_upload_done==0 )
                {
                    read_sector();
                }else
                {
                    uint8_t i,istime[1],issite[1];
                    uint8_t temp_tag[] = "1234567890123456789012";
                    char * cid_addr = NULL;
                    char * fid_addr = NULL;


                    cid_addr = strstr(uart_receive_at,"22636f6c6c656374496422");//"collectId"
                    os_memcpy(temp_tag,cid_addr+26,22);

                    os_memcpy(istime,cid_addr+87,1);
                    os_memcpy(issite,cid_addr+121,1);
                    os_printf("istime %d , issite %d\n",istime[0],issite[0]);

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

                    fid_addr = strstr(uart_receive_at,"226669786564496422");//"fixedId"
                    os_memcpy(isFixedTime,fid_addr+79,1);

                    cid_addr = NULL;
                    fid_addr = NULL;

                    os_printf("isFixedTime %d\n",isFixedTime[0]);

                    save_stakid(parameter_tag);
                    at_state = NONE;
                    os_printf("tag %s\n",parameter_tag);

                    if((0 != os_strcmp(parameter_tag,"00000000000") && istime[0] == 48) || isFixedTime[0] == 48)//不等于00000000
                    {
                        get_tag_flag = 1;//获得任务ID标志
                        os_timer_disarm(&check_id_timer);


                        queue_uart_send(zgmode,os_strlen(zgmode));
                        os_printf("send %s\n",zgmode);
                        at_state = ZGMODE;


                    }else if(istime[0] == 49 || issite[0] == 49 )//此时没有任务，再判断是否能升级。0在；1不在
                    {
                        os_timer_disarm(&restart_nb);

                        wifi_state = 0;
                        update_firmware();


                    }
                    if(0 == os_strcmp(parameter_tag,"00000000000") || istime[0] == 49)//等于00000000
                    {
                        get_tag_flag=0;

                        if(http_dis == 0)
                        {

                            queue_uart_send(http_discon,os_strlen(http_discon));
                            os_printf("discon %s\n",http_discon);
                            http_dis = 1;
                        }
                    }
                    os_timer_disarm(&restart_nb);
                    os_timer_arm(&restart_nb, 185000, 1);//3分钟后没有收到数据，重启
                }

            }
            else if(os_strstr(uart_receive_at,"2c226c6f6e223a22"))//返回室内定位经纬度
            {
                get_indoor_position();

            }

            else if(os_strstr(uart_receive_at,"e5ae9ae4bd8de99499e8afaf"))//定位错误
            {
                queue_uart_send(zgmode,os_strlen(zgmode));
                os_printf("send %s\n",zgmode);
                at_state = ZGMODE;

            }
            else if(os_strstr(uart_receive_at,"e8aebee5a487e69caae4bdbfe794a8")||os_strstr(uart_receive_at,"e8aebee5a487e69caae7bb91e5ae9a"))//设备未使用：数据库没有这个设备；未绑定：数据库有设备但是为关联代理商
            {


                os_timer_disarm(&restart_nb);

                wifi_state = 0;
                update_firmware();

//                bind_flag = 0;
//                at_state = WAIT;
                os_timer_disarm(&check_id_timer);
                os_timer_arm(&check_id_timer, 120000, 1);
#ifdef OLED_VERSION
#else
//                led_state = 3;
//                devicexx_io_led_timer_tick();
#endif
//                wifi_set_opmode(SOFTAP_MODE);
//                vowstar_set_ssid_prefix("Qingzhu_VQ_");
//                tcp_server();

//                os_timer_disarm(&restart_nb);

            }
            else
            {
//                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
//                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
//                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
//                os_printf("{\"code\":\"1\",\"desc\":\"error\"}\n");
                os_printf("{\"error\"}\n");
                os_printf("{\"error\"}\n");
                os_printf("{\"error\"}\n");
                os_printf("{\"error\"}\n");
                update_tag();
                queue_uart_send(http_get_tag,os_strlen(http_get_tag));
                os_printf("send %s\n",http_get_tag);
                at_state = WAIT;
//                //检测电池电压 v*5.7,max(4.3V)=755,min(3.1V)=544
//                os_memset(uart_receive_at,'\0',sizeof(char)*2048);
//                queue_uart_send(read_adc,os_strlen(read_adc));
//                at_state = ZADC;
//                os_printf("read adc %s\n",read_adc);
            }

            os_printf("tag %s\n",parameter_tag);

            return;

        }


        if(os_strstr(uart_receive_at,"GNRMC"))
        {
        	uint8_t i;
            gnrmc_gps_flag++;

            if(uart_receive_at[17] == 'A')
            {
//                gnrmc_gps_flag = 0;
                get_gps = 1;
#ifdef OLED_VERSION
#else
                GPIO_OUTPUT_SET(PIN_GPS_S, 0);
#endif

                uint8_t FixedTime[1] = {0};
                char * gnrmc_addr = NULL;

                    //03 46 39 11 01 19
                gnrmc_addr = strstr(uart_receive_at,"$GNRMC");//"fixedId"


//   $GNRMC,080457.14,A,3958.99017,N,11618.03480,E,1.770,,070319,,,A,V*16
//                os_printf("receive %s\n",uart_receive_at);
//临时
                os_memcpy(parameter_longitude,gnrmc_addr + 32, os_strlen(parameter_longitude));
                os_memcpy(parameter_latitude, gnrmc_addr + 19 , os_strlen(parameter_latitude));

                strhex_to_str(parameter_longitude,4);
                strhex_to_str(parameter_latitude,3);




                if(os_strstr(parameter_timestamp,"."))
                {
                	for(i=0;i<12;i++)
                		parameter_timestamp[i] = '0';
                }


                os_memcpy(parameter_timestamp, gnrmc_addr + 7 , 6);
                os_memcpy(parameter_timestamp+6, gnrmc_addr + 53 ,6);

                gnrmc_addr = NULL;

                strhex_to_str(parameter_timestamp,5);


                os_printf("parameter_longitude=%s\nparameter_latitude=%s\nparameter_timestamp=%s\ncreat_flag=%d\n"
                                        ,parameter_longitude,parameter_latitude,parameter_timestamp,creat_flag);

//                start();
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

                get_gps = 0;
#ifdef OLED_VERSION
#else
                GPIO_OUTPUT_SET(PIN_GPS_S, 1);
#endif
//                at_state = WAIT;
//                for(i=0;i<12;i++)
//                    parameter_timestamp[i] = '0';
    //            os_printf("parameter_timestamp %s\n",parameter_timestamp);
            }

            os_printf("at_state = %d,gnrmc_gps_flag = %d\n",at_state,gnrmc_gps_flag);


            if(gnrmc_gps_flag > 110 )
            {
                gnrmc_gps_flag = 0;
                http_dis = 1;
                queue_uart_send(read_adc,os_strlen(read_adc));
                at_state = ZADC;

                os_memset(uart_receive_at,'\0',sizeof(uart_receive_at));
            }
        }


        os_printf("%s: memory left=%d\r\n", __func__, system_get_free_heap_size());
	}

}



uint16_t start_sector=518;

uint8_t test_str_num=0;
uint16 used_len=0;

void ICACHE_FLASH_ATTR
write_to_flash(void)
{
    uint8 *str_len="10001";

    //如果还有不同的，存入最新扇区
    if(os_strlen(sta_str)>12)
    {
        os_memset(sector_str,'\0',sizeof(sector_str));
        os_printf("SpiFlashReadResult:%d\n",spi_flash_read((start_sector+local_system_status.used_sector)*4*1024,(uint32 *)sector_str,4096));
        os_printf("read flash sector_str:\n%s\n",sector_str);
        //计算扇区总长度
        used_len=0;
        if(mac_inrom_flag==0)
            while(sector_str[used_len]!=0xff)
                used_len++;
        if(used_len>4096)
        {
            used_len=4096;
        }
    //    used_len = os_strlen(str121);
        os_printf("used_len:%d\n",used_len);


        all_sector_upload_done=0;
        local_system_status.sector_updata_flag=0;
        devicexx_app_save();
//        uint16 body_len = os_strlen(JSON_FIX_MAC)-14 +
//                os_strlen(parameter_deviceId) +
//                os_strlen(parameter_timestamp) +
//                os_strlen(parameter_tag) +
//                os_strlen(parameter_longitude) +
//                os_strlen(parameter_latitude) +
//                os_strlen(sta_str)+
//                os_strlen(fixedId);
//
//
//        uint8_t * body = (uint8_t *) os_zalloc(body_len);
//
//        if (body == NULL) {
//            VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
//            return;
//        }
//
//
//        str_len[0]=49;
//        str_len[1]=body_len/1000+48;
//        str_len[2]=(body_len%1000)/100+48;
//        str_len[3]=(body_len%100)/10+48;
//        str_len[4]=body_len%10+48;
//
//        os_printf("body_len:%d\n",body_len);
//
//        os_sprintf(body, JSON_FIX_MAC, parameter_deviceId, parameter_timestamp, parameter_tag, parameter_longitude, parameter_latitude, sta_str, fixedId);
//        os_memcpy(sector_str+used_len,str_len,5);
//        os_memcpy(sector_str+used_len+5,body,body_len);

//        os_printf("3write to flash sector_str:\n%s\n",sector_str);

//        spi_flash_erase_sector(start_sector+local_system_status.used_sector);
//        spi_flash_write((start_sector+local_system_status.used_sector)*4*1024,(uint32 *)sector_str,4096);

//        if (body)
//            os_free(body);
        if(os_strlen(test_str)>4096-used_len)
        {
            os_printf("used_sector %d is full !!\n",local_system_status.used_sector);
            local_system_status.used_sector++;
            if(local_system_status.used_sector>=1010)
                local_system_status.used_sector=1009;
            devicexx_app_save();
            used_len=0;
            spi_flash_erase_sector(start_sector+local_system_status.used_sector);
            os_memset (sector_str,255,sizeof(sector_str));
        }

        os_memcpy(sector_str+used_len,test_str,os_strlen(test_str));
        os_printf("memory left=%d\r\n",system_get_free_heap_size());
        os_printf("read flash sector_str:\n%s\n",sector_str);
        spi_flash_erase_sector(start_sector+local_system_status.used_sector);
        os_printf("SpiFlashWriteResult:%d\n",spi_flash_write((start_sector+local_system_status.used_sector)*4*1024,(uint32 *)sector_str,4096));


    }

}
void ICACHE_FLASH_ATTR
Deduplication(void)//去重，缓存
{
    wifi_promiscuous_enable(0);
    wifi_unregister_send_pkt_freedom_cb();
    wifi_station_disconnect();

    os_printf("Gather done\n");

    update_timestamp();
#if HOP_JUMP_ENABLE
    os_timer_disarm(&channelHop_timer);
#endif


//    os_printf( "mac string %s\n", sta_str );
    uint8 j;
    uint8 *temp_macstr="123456789012";


//    uint8 temp_macstr[]="DAA119360DB2";
//    uint8_t  sector_str[4097];
//    os_memset (sector_str,255,4096);
//    sector_str[4096]='\0';
    //10155{\"timestamp\":\"004323150319\",\"collectId\":\"CJ599214549\",\"longitude\":\"00000.00000\",\"latitude\":\"0000.00000\",\"mac_str\":\"DAA119360DB2,7836CC8BA321,3EEEBF010CAB\"}
//    uint8_t * str= "10155{\"timestamp\":\"004323150319\",\"collectId\":\"CJ599214549\",\"longitude\":\"00000.00000\",\"latitude\":\"0000.00000\",\"mac_str\":\"DAA119360DB2,7836CC8BA321,3EEEBF010CAB\"}10168{\"timestamp\":\"2059A07880F8\",\"collectId\":\"CJ599214549\",\"longitude\":\"00000.00000\",\"latitude\":\"0000.00000\",\"mac_str\":\"7072CF090056,7836CC8BA321,3EEEBF010CAB,5A6D4ECC79D0\"}10142{\"timestamp\":\"004323150319\",\"collectId\":\"CJ599214549\",\"longitude\":\"00000.00000\",\"latitude\":\"0000.00000\",\"mac_str\":\"432119360DB2,5A6D4ECC79D0\"}";
//    uint8_t * str= "10155{\"timestamp\":\"004323150319\",\"collectId\":\"CJ599214549\",\"longitude\":\"00000.00000\",\"latitude\":\"0000.00000\",\"mac_str\":\"AAA119360DB2,0836CC8BA321,EEEEBF010CAB\"}";
//10155{"timestamp":"004323150319","collectId":"CJ599214549

    //    spi_flash_read(sector*4*1024,(uint32 *)test_str,4096);
    uint16 i;

    uint8_t temp_cache_ap[1300];
    os_memset (temp_cache_ap,'\0',sizeof(temp_cache_ap));
    uint8 *sector_tag_id="CJ000000000";


    os_printf("sta_str:\n%s\n",sta_str);

    os_printf("1used_sector %d\n",local_system_status.used_sector);
    os_printf("sni_temp:%d\n",sni_temp);
    for(i=0;i<=local_system_status.used_sector;i++)
    {
        os_memset(sector_str,'\0',sizeof(sector_str));
//      读出1个扇区
//        os_printf("sector:%d\n",i+start_sector);
//        os_printf("SpiFlashReadResult:%d\n",spi_flash_read((i+start_sector)*4*1024,(uint32 *)sector_str,4096));
        spi_flash_read((i+start_sector)*4*1024,(uint32 *)sector_str,4096);
//        os_printf("sector_str:\n%s\n",sector_str);


        if(os_strstr(sector_str,"collectId"))
        {
            uint8 * tag_id_addr=NULL;
            tag_id_addr = strstr(sector_str,"collectId");
            os_memcpy(sector_tag_id,tag_id_addr+12,11);
            tag_id_addr=NULL;
        }
//        os_printf("sector_tag_id:%s\n",sector_tag_id);

        if(os_strstr(sector_tag_id,parameter_tag) )//任务ID相同
        {
//            os_printf("str:%s\n",str);

            for(j=0;j<=sni_temp;j++)
            {
//                os_printf("j:%d\n",j);
                os_memcpy(temp_macstr,sta_str+j*13,12);

//                os_printf("temp_macstr%d---%s\n",j,temp_macstr);
                if(os_strstr(sector_str,temp_macstr))
                {
//                    os_printf("temp_macstr%d-----------------%s\n",j,temp_macstr);
//              DAA119360DB2,7836CC8BA321,3EEEBF010CAB\,123419360DB2,7836CC8BA321,3EEEBF010CAB,5A6D4ECC79D0\,432119360DB2,5A6D4ECC79D0

                }
                else
                {
                    os_memcpy(temp_cache_ap+j*13,temp_macstr,12);
                    temp_cache_ap[12+j*13]=',';
//                    os_printf("temp_cache_ap:%s\n",temp_cache_ap);

                }
            }
            temp_cache_ap[1299]='\0';
            os_printf("temp_cache_ap len:%d\n",os_strlen(temp_cache_ap));

//            sni_temp=test_str_num;
//            test_str_num=0;
            os_memset (sta_str,'\0',1300);
            os_memcpy(sta_str,temp_cache_ap,os_strlen(temp_cache_ap));
//            os_printf("temp_cache_ap len:%d\n",os_strlen(temp_cache_ap));
//            os_printf("sta_str:\n%s\n",sta_str);
//            os_memset (temp_cache_ap,'\0',sizeof(char)*(os_strlen(temp_cache_ap)));
        }
    }
    os_printf("sta_str:\n%s\n",sta_str);
    //所有扇区都比较完

    if(os_strlen(sta_str)>12)
        mac_inrom_flag=1;

    uint16 body_len = os_strlen(JSON_FIX_MAC)-14 +
            os_strlen(parameter_deviceId) +
            os_strlen(parameter_timestamp) +
            os_strlen(parameter_tag) +
            os_strlen(parameter_longitude) +
            os_strlen(parameter_latitude) +
            os_strlen(sta_str)+
            os_strlen(fixedId);


    uint8_t * body = (uint8_t *) os_zalloc(body_len);

    if (body == NULL) {
        VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
        return;
    }



    uint8 *str_len="10001";
    str_len[0]=49;
    str_len[1]=body_len/1000+48;
    str_len[2]=(body_len%1000)/100+48;
    str_len[3]=(body_len%100)/10+48;
    str_len[4]=body_len%10+48;

    os_printf("body_len:%d\n",body_len);

    os_sprintf(body, JSON_FIX_MAC, parameter_deviceId, parameter_timestamp, parameter_tag, parameter_longitude, parameter_latitude, sta_str, fixedId);


    os_memset (test_str,'\0',sizeof(test_str));
    os_memcpy(test_str,str_len,5);
    os_memcpy(test_str+5,body,body_len);
    if (body)
        os_free(body);

    os_printf("test_str:\n%s\n",test_str);

    sni_temp = 0;

//    nbiot_http_post();
    get_rssi();

}
void ICACHE_FLASH_ATTR
sector_upload_done(void)
{
    sector_str_upload_flag_bit=0;
    sector_str_totlen=0;
    spi_flash_erase_sector(start_sector+local_system_status.used_sector);

    if(local_system_status.used_sector==0)
    {
        os_printf("all sector send done\n");
        all_sector_upload_done=1;
        local_system_status.sector_updata_flag=1;
    }else
    {
        local_system_status.used_sector--;
        os_printf("used_sector:%d\n",local_system_status.used_sector);
        os_memset(sector_str,'\0',sizeof(sector_str));
        os_printf("SpiFlashReadResult:%d\n",spi_flash_read((start_sector+local_system_status.used_sector)*4*1024,(uint32 *)sector_str,4096));
        os_printf("sector_str:\n%s\n",sector_str);
    }
    devicexx_app_save();
    post_state = CHECK_ID;
    check_id();
    return;
}


void ICACHE_FLASH_ATTR
send_flash_mac(void)
{
    uint16 len=0;

    os_printf("%s call\r\n", __func__);
    os_printf("sector_str_:%d\n", sector_str[sector_str_totlen]);

    os_printf("sector_str_totlen:%d\nsector_str:\n%s\n",sector_str_totlen,sector_str);

    if(sector_str[sector_str_totlen] != 255 && sector_str[sector_str_totlen] != '\0' && sector_str_totlen<4096)
    {
        len=(sector_str[sector_str_totlen+1]-48)*1000+(sector_str[sector_str_totlen+2]-48)*100
              +(sector_str[sector_str_totlen+3]-48)*10+sector_str[sector_str_totlen+4]-48;

        os_printf("len:%d\n",len);
        os_printf("totlen:%d\n",sector_str_totlen);
        if(sector_str[0+sector_str_totlen]==49)//本字段未上传
        {
            sector_str_upload_flag_bit = sector_str_totlen;
            sector_str_totlen+=5;
            os_printf("totlen:%d\n",sector_str_totlen);
            os_memset (test_str,'\0',sizeof(test_str));
            os_memcpy(test_str,sector_str+sector_str_totlen,len);

            sector_str_totlen+=len;

            if(sector_str_totlen>4096)
            {
                os_printf("sector_str_totlen>4096\n");
                sector_upload_done();
            }

            os_printf("test_str:%s\n",test_str);

            if(connected_wifi==0)
            {
                uint8 * mac_str_addr=NULL;
                uint8 * timestamp_addr=NULL;
                uint8 * tag_addr=NULL;
                uint8 * longitude_addr=NULL;
                uint8 * latitude_addr=NULL;
                uint8 * fixedId_addr=NULL;
                uint8_t temp_timestamp[] = "111111111111";
                uint8_t temp_tag[] = "00000000000";
                uint8_t temp_longitude[] = "00000.00000";
                uint8_t temp_latitude[] = "0000.00000";
                uint8_t temp_fixedId[] = "GD000000000";
                mac_str_addr = strstr(test_str,"mac_str");
                timestamp_addr = strstr(test_str,"timestamp");
                tag_addr = strstr(test_str,"collectId");
                longitude_addr = strstr(test_str,"longitude");
                latitude_addr = strstr(test_str,"latitude");
                fixedId_addr = strstr(test_str,"fixedId");


                os_memcpy(temp_timestamp,timestamp_addr+12,12);
                os_memcpy(temp_tag,tag_addr+12,11);
                os_memcpy(temp_longitude,longitude_addr+12,11);
                os_memcpy(temp_latitude,latitude_addr+11,10);
                os_memcpy(temp_fixedId,fixedId_addr+10,11);




                os_memset(sta_str,'\0',sizeof(uint8)*1300);
                os_memcpy(sta_str,mac_str_addr+10,fixedId_addr-mac_str_addr-13);

                mac_str_addr=NULL;
                timestamp_addr=NULL;
                tag_addr=NULL;
                longitude_addr=NULL;
                latitude_addr=NULL;
                fixedId_addr=NULL;

                uint8_t * all_str = "010000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,000000000000,";
                all_str[1]='0';
				os_memcpy(all_str+(1300-os_strlen(sta_str)),sta_str,os_strlen(sta_str));
                uint8_t * body = (uint8_t *) os_zalloc(os_strlen(JSON_FIX_MAC) +
                                                       os_strlen(parameter_deviceId) +
                                                       os_strlen(parameter_timestamp) +
                                                       os_strlen(parameter_tag) +
                                                       os_strlen(parameter_longitude) +
                                                       os_strlen(parameter_latitude) +
                                                       os_strlen(all_str)+
                                                       os_strlen(fixedId));
                if (body == NULL) {
                    VOWSTAR_WAN_DEBUG("%s: not enough memory\r\n", __func__);
                    return;
                }

                os_sprintf(body,JSON_FIX_MAC,parameter_deviceId,temp_timestamp,temp_tag,
                           temp_longitude,temp_latitude,all_str,temp_fixedId);

                str_ascii_str(body);
                if (body)
                    os_free(body);

            }else
            {
                uint8_t * url = NULL;
    #ifdef test_environment
                url = "http://221.122.119.226:8098/MacGather/submitValue";
    #else
                url = "http://47.105.207.228:8098/MacGather/submitValue";
    #endif
                void * ctx = NULL;
                http_post(ctx, url, "Content-Type:application/json\r\n", (const char *) test_str, os_strlen(test_str), post_callback);
                url = NULL;
                ctx = NULL;
            }
        }else{
            sector_str_totlen=sector_str_totlen+5+len;

            if(sector_str_totlen>4096)
            {
                os_printf("this sector send done\n");
                sector_upload_done();
            }
            post_state = CHECK_ID;
            check_id();
//            send_flash_mac();
        }

    }else if(mac_inrom_flag==1)
    {
        mac_inrom_flag=0;
        post_state = CHECK_ID;
        check_id();

    }else
    {
        os_printf("this sector send done\n");
        sector_upload_done();
    }
}
void ICACHE_FLASH_ATTR
read_sector(void )
{
    os_printf("%s call\r\n", __func__);
    os_printf(" used_sector:%d\n mac_inrom_flag:%d\n",local_system_status.used_sector,mac_inrom_flag);
    os_memset(sector_str,'\0',sizeof(sector_str));
    if(mac_inrom_flag==0)
    {
        os_printf("SpiFlashReadResult:%d\n",spi_flash_read((start_sector+local_system_status.used_sector)*4*1024,(uint32 *)sector_str,4096));
        os_printf("sector_str:\n%s\n",sector_str);
    }else
    {
        os_memcpy(sector_str,test_str,os_strlen(test_str));
    }

    send_flash_mac();
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
    os_printf( "rtc time init...\r\n" );
    rtc_time.time_acc   = 0;
    rtc_time.time_base  = system_get_rtc_time();
    os_printf( "time base : %d \r\n", rtc_time.time_base );

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
        local_system_status.used_sector = 0;
        local_system_status.sector_updata_flag=1;
        os_memcpy(local_system_status.task_id, "CJ000000000",11);
        os_memcpy(local_system_status.times, "111111111111",12);

	}
//    local_system_status.version_num = 5;

	if(local_system_status.times[0]>58 || local_system_status.times[0]<48 )
	    os_memcpy(local_system_status.times, "111111111111",12);
	all_sector_upload_done=local_system_status.sector_updata_flag;
	os_memcpy(parameter_tag,local_system_status.task_id,11);
    os_memcpy(parameter_timestamp,local_system_status.times,12);

    parameter_version[0] = local_system_status.version_type + 48;
    parameter_version[2] = local_system_status.version_num/100 + 48;
    parameter_version[3] = local_system_status.version_num%100/10 + 48;
    parameter_version[4] = local_system_status.version_num%100%10 + 48;
    devicexx_app_save();

	os_printf("version_type:%d  version_num:%d\n",local_system_status.version_type,local_system_status.version_num);
	os_printf("used_sector:%d\n",local_system_status.used_sector);
	os_printf("all_sector_upload_done:%d\n",all_sector_upload_done);
	os_printf("local_system_status.task_id:%s\n",local_system_status.task_id);
	os_printf("local_system_status.times:%s\n",local_system_status.times);

}





