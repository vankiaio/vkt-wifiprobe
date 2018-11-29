/******************************************************************************
 * Copyright 2015 Vowstar Co., Ltd.
 *
 * FileName: main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/1/23, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "user_config.h"
#include "user_interface.h"
#include "platform.h"
#include "uart.h"
#include "main.h"

#define RESET "\033[0m"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"



struct softap_config softap_cfg;
uint8 ssid[]="vankia_8266";         //wifi√˚
uint8 password[]="";     //wifi√‹¬Î

void ICACHE_FLASH_ATTR
user_set_softap_config(void)
{
    wifi_softap_get_config(&softap_cfg); // Get config first.
    wifi_set_opmode(SOFTAP_MODE);           //…Ë÷√Œ™AP MODE
    os_strcpy(softap_cfg.ssid, ssid);          //ssid√˚≥∆
    os_strcpy(softap_cfg.password, password);  //√‹¬Î
    softap_cfg.authmode = AUTH_WPA_WPA2_PSK;
    softap_cfg.ssid_len = 0; // or its actual length
    softap_cfg.max_connection = 4; // how many stations can connect to ESP8266 softAP at most.
    wifi_softap_set_config(&softap_cfg);      //…Ë÷√WIFI’ ∫≈∫Õ√‹¬Î
}

void ICACHE_FLASH_ATTR system_init_done()
{
    // Reset linux, MACOS or Windows cygwin terminal
    // Because serial port may lost data when boot, output CRLF.

     //Draw vankia logo

    os_printf("\r\n\r\n\r\n\
    /$$    /$$    /~$      /$$     /$$  /$$   /$$/  /$$$$$$     /~$           \r\n\
   | $$   | $$   / $$$    | $$$   | $$ | $$  /$$/  |_  $$_/    / $$$          \r\n\
   | $$   | $$  / $$\\ $$  | $$$$  | $$ | $$ /$$/     | $$     / $$\\ $$      \r\n\
   |  $$ / $$  / $$  \\ $$ | $$ $$ | $$ | $$$$$/      | $$    / $$  \\ $$     \r\n\
    \\  $$ $$/ | $$$$$$$$$ | $$\\ $$| $$ | $$\\ $$      | $$   | $$$$$$$$$    \r\n\
     \\  $$$/  | $$ __  $$ | $$ \\ $$ $$ | $$ \\ $$     | $$   | $$ __  $$    \r\n\
      \\  $/   | $$   | $$ | $$  \\ $$$  | $$  \\ $$   /$$$$$$ | $$   | $$    \r\n\
       \\_/    |__/   |__/ |__/   \\__/  |__/   \\__/ |______/ |__/   |__/    \r\n\r\n\r\n");




    // Set color to green
    os_printf(KGRN);
    os_printf("\r\nSDK version:%s\r\n", system_get_sdk_version());
    os_printf("\r\nready\r\n");
    // Set color to normal
    os_printf(KNRM);
    // Init platform
    platform_init();

}



ICACHE_FLASH_ATTR void main(int argc, char *argv[])
{
    // Set baudrate
    // UART_SetPrintPort(UART0);
    // uart_div_modify(0, 80 * 1000000 / 115200);


#define DEV_MODE 1
#if DEV_MODE
    // Set the port to print log info.
    UART_SetPrintPort(UART1);
    // Set UART and LOG baudrate
    uart_init(BIT_RATE_9600, BIT_RATE_115200);

#else
    // Set the port to print log info.
    UART_SetPrintPort(UART0);
    // Swap UART Port
//    system_uart_swap();
    // Set UART and LOG baudrate
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
#endif

    os_delay_us(100);

    // Set Wi-Fi mode
//    wifi_set_opmode(SOFTAP_MODE);//STATIONAP_MODE
//    vowstar_set_ssid_prefix("Vankia_WP_");
    //    user_set_softap_config();
    system_init_done_cb(system_init_done);
}
