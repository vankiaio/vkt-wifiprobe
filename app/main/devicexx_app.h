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


typedef enum {
    NONE,
	LED,
    ZADC,
    ZCFUN,
    ZOFF,
    ZGMODE,
    ZGNMEA,
    ZGRUN,
    FIRCON,
    POSTBIND,
    HTTP_CON,
    HTTP_SEND,
    HTTP_SEND1,
    HTTP_SEND2,
    HTTP_SEND3,
    HTTP_SEND4,
    HTTP_SEND5,
    HTTP_SEND6,
    HTTP_DISCON,
    HTTP_DESTROY,
    WAIT,

}at_state_t;

// Export get flash size function
extern uint32_t ICACHE_FLASH_ATTR flash_rom_get_size_byte(void);
extern uint8_t at[];
extern at_state_t at_state;
extern uint8_t zgmode[] ;
extern struct espconn pespconn;
extern uint16_t gnrmc_gps_flag;
extern os_timer_t temer_10s;
extern uint8_t send_flag;
extern uint8_t bind_flag;
extern uint8_t loginName[];
extern uint8_t loginPwd[];
extern uint8_t http_create[];
extern uint8_t  update_host[128];
void power_on(void);
void update_post_bind(void);

// Save user data to last 15, 14, 13 sector of flash
#define DEVICEXX_APP_START_SEC   	((flash_rom_get_size_byte() / 4096) - 15)


typedef struct system_status_t {
	uint8_t version_type;
	uint16_t version_num;
} __attribute__((aligned(4), packed)) system_status_t;

typedef enum devicexx_app_state_t
{
	devicexx_app_state_normal,
	devicexx_app_state_smart,
	devicexx_app_state_upgrade,
	devicexx_app_state_restore,
} devicexx_app_state_t;

//uint8_t uart_receive_at[100];

void devicexx_receive(const d_object_t* object);
void devicexx_app_apply_settings(void);
void devicexx_app_load(void);
void devicexx_app_save(void);


#endif /* __USER_DEVICEXX_VIRTUAL_MCU_H__ */

