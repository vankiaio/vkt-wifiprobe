#ifndef __DEVICEXX_APP_H__
#define __DEVICEXX_APP_H__

#if defined(GLOBAL_DEBUG_ON)
#define DEVICEXX_APP_DEBUG_ON
#endif
#if defined(DEVICEXX_APP_DEBUG_ON)
#define DEVICEXX_APP_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define DEVICEXX_APP_DEBUG(format, ...)
#endif

#include "devicexx.h"

#define test_environment

#define JSON_DEVICE_MAC "{\"deviceId\":\"%s\",\"timestamp\":\"%s\",\"collectId\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"mac_str\":\"%s\"}"

#define JSON_FIX_MAC    "{\"deviceId\":\"%s\",\"timestamp\":\"%s\",\"collectId\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"mac_str\":\"%s\",\"fixedId\":\"%s\"}"

#define JSON_GET_TAG    "{\"deviceId\":\"%s\",\"version\":\"%s\",\"wei\":\"%s\",\"lng\":\"%s\",\"time\":\"%s\"}"

#define JSON_AP_MAC     "{\"mac\":\"%s\"}"


typedef enum {
    NONE,
    EDNS,
	LED,
    ZADC,
    CCLK,
    ZCFUN,
    ZOFF,
    ZGMODE,
    ZGMODE1,
    ZGNMEA,
    ZGNMEA1,//10
    ZGDATA,
    ZGRUN,
    ZGSTOP,
    FIRCON,
    POSTBIND,
    HTTP_CON,
    HTTP_SEND,
    HTTP_SEND1,
    HTTP_SEND2,
    HTTP_SEND3,//20
    HTTP_SEND4,
    HTTP_SEND5,
    HTTP_SEND6,
    HTTP_DISCON,
    HTTP_DESTROY,
    CSQ,
    WAIT,

}at_state_t;


typedef enum {
    IDLE,
    OTA,
    CHECK_ID,
    AP_MAC,
    UPMAC,

}post_state_t;

extern post_state_t post_state;


// Export get flash size function
extern uint32_t ICACHE_FLASH_ATTR flash_rom_get_size_byte(void);
extern uint8_t at[];
extern at_state_t at_state;
extern uint8_t zgmode[] ;
extern struct espconn pespconn;
extern uint8_t gnrmc_gps_flag;
extern uint8_t update_firmware_flag;
extern uint8_t nb_signal_bad;
extern uint8_t connected_wifi;
extern uint8_t scan_qz;
extern uint8_t wifi_net;
extern uint8  sector_str[4096];
extern uint16 sector_str_upload_flag_bit;

extern uint8 all_sector_upload_done;
extern uint16  sector_flag_bit;
extern uint8 mac_inrom_flag;

extern uint8_t version_type;
extern uint16_t version_num;

extern uint8_t fixedId[];
extern uint8_t isFixedTime[] ;

extern uint8_t parameter_version[];
extern uint8_t parameter_deviceId[];
extern uint8_t parameter_timestamp[] ;
extern uint8_t parameter_tag[] ;
extern uint8_t parameter_longitude[] ;
extern uint8_t parameter_latitude[] ;




//extern uint8_t send_flag;
//extern uint8_t bind_flag;
extern uint8_t loginName[];
extern uint8_t loginPwd[];
extern uint8_t http_create[];
extern uint8_t  update_host[128];
extern void update_firmware(void);
extern uint8_t apmac_rssi[6][8];


extern uint8_t read_adc[];


void ap_str_ascii_str(char * body);
void power_on(void);
//void update_post_bind(void);
void check_id(void);
void http_disc(void);
void check_gps(void);
void nbiot_http_post(void);
void update_data(void);
void Deduplication(void);
void write_to_flash(void);
void get_rssi(void);
void delay_power_off();

// Save user data to last 15, 14, 13 sector of flash
//#define DEVICEXX_APP_START_SEC   	((flash_rom_get_size_byte() / 4096) - 15)
#define DEVICEXX_APP_START_SEC 1011

typedef struct system_status_t {
	uint8_t version_type;
	uint16_t version_num;
	uint16_t used_sector;
	uint8_t sector_updata_flag;
	uint8 task_id[11];
    uint8 times[12];
} __attribute__((aligned(4), packed)) system_status_t;


typedef enum devicexx_app_state_t
{
	devicexx_app_state_normal,
	devicexx_app_state_smart,
	devicexx_app_state_upgrade,
	devicexx_app_state_restore,
} devicexx_app_state_t;

//uint8_t uart_receive_at[100];

void devicexx_app_apply_settings(void);
void devicexx_app_load(void);
void devicexx_app_save(void);
void app_save(uint8 flag);
void save_stakid(uint8 * task_id);

#endif /* __USER_DEVICEXX_VIRTUAL_MCU_H__ */

