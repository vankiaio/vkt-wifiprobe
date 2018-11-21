/******************************************************************************
 * Copyright 2016 Vankia Co.,Ltd.
 *
 * FileName: devicexx.c
 *
 * Description: Device++ Platform Interface
 *
 * Modification history:
 *     2016/06/24, v1.1
*******************************************************************************/

#ifndef __DEVICEXX_H__
#define __DEVICEXX_H__

#include "time.h"
#include "c_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct d_object_t {
    struct d_object_t *next, *prev;
    struct d_object_t *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} d_object_t;

typedef enum {
    WIFI_IDLE,                      /* Wi-Fi空闲，常出现在设备刚刚启动且没有连接过任何SSID时 */
    WIFI_SMARTLINK_START,           /* 设备进入智能连接状态 */
    WIFI_SMARTLINK_LINKING,         /* 设备已经通过智能连接获得SSID和密码，开始连接 */
    WIFI_SMARTLINK_FINDING,         /* 设备正在进行智能连接，正在查找手机所在的信道与发出的特征包 */
    WIFI_SMARTLINK_TIMEOUT,         /* 设备智能连接超时 */
    WIFI_SMARTLINK_GETTING,         /* 设备正在进行智能连接，已经锁定手机信道，正在解码数据包 */
    WIFI_SMARTLINK_OK,              /* 设备智能连接成功 */
    WIFI_AP_CONFIG,                 /* 设备正在AP配网模式下 */
    WIFI_AP_CONFIG_OK,              /* 设备AP配网成功 */
    WIFI_AP_OK,                     /* 设备在AP模式 */
    WIFI_AP_ERROR,                  /* 设备在AP模式并发生了某些错误 */
    WIFI_AP_STATION_OK,             /* 设备在AP-STATION混合模式 */
    WIFI_AP_STATION_ERROR,          /* 设备在AP-STATION混合模式并发生了某些错误 */
    WIFI_STATION_OK,                /* 设备在STATION模式 */
    WIFI_STATION_ERROR,             /* 设备在STATION模式并发生了某些错误 */
    DEVICEXX_CONNECTING,             /* 正在连接Device++云 */
    DEVICEXX_CONNECTING_ERROR,       /* 连接Device++云失败 */
    DEVICEXX_CONNECTED,              /* 已连接Device++云 */
    DEVICEXX_DISCONNECTED,           /* 与Device++云的连接断开 */
    DEVICEXX_STATE_MAX,              /* Device++ SDK状态个数 */
} devicexx_state_t;

typedef void (*devicexx_callback_t)();
typedef void (*devicexx_receive_raw_callback_t)(void* arg, const uint8_t *pdata, uint32_t length);
// typedef void (*devicexx_receive_raw_callback_t)(const uint8_t *pdata, uint32_t length);
typedef void (*devicexx_receive_callback_t)(const d_object_t* object);
typedef void (*devicexx_state_callback_t)(devicexx_state_t state);

int              devicexx_run(void);
int              devicexx_stop(void);
void             devicexx_on_connected(devicexx_callback_t callback);
void             devicexx_on_disconnected(devicexx_callback_t callback);
void             devicexx_on_receive(devicexx_receive_callback_t callback);
void             devicexx_on_receive_raw(devicexx_receive_raw_callback_t callback);
void             devicexx_on_sent(devicexx_callback_t callback);
bool             devicexx_send_raw(const uint8_t *pdata, uint32_t length);
bool             devicexx_send(const d_object_t* object);
void             devicexx_on_state_changed(devicexx_state_callback_t callback);
devicexx_state_t devicexx_state();
void             devicexx_system_recovery();
void             devicexx_unbind();
void             devicexx_force_smartlink();
void             devicexx_force_aplink();
void             devicexx_set_ssid_prefix(const char* sPrefix);
void             devicexx_check_update();
bool             devicexx_time_ready();
bool             devicexx_set_timezone(tz_t * tz);
bool             devicexx_get_time(tm_t * tm);
bool             devicexx_is_get_object(const d_object_t* object);
bool             devicexx_is_set_object(const d_object_t* object);
bool             devicexx_receive_get(const d_object_t* object, const char * key);
bool             devicexx_receive_set(const d_object_t* object, const char * key);
bool             devicexx_receive_bool(const d_object_t* object, const char* key);
bool             devicexx_receive_double(const d_object_t* object, const char* key);
bool             devicexx_receive_int(const d_object_t* object, const char * key);
bool             devicexx_receive_string(const d_object_t* object, const char * key);
bool             devicexx_value_bool(const d_object_t* object, const char* key);
double           devicexx_value_double(const d_object_t* object, const char* key);
int              devicexx_value_int(const d_object_t* object, const char * key);
char *           devicexx_value_string(const d_object_t* object, const char * key);
bool             devicexx_add_bool(d_object_t* object, const char* key, bool value);
bool             devicexx_add_double(d_object_t* object, const char* key, double value);
bool             devicexx_add_int(d_object_t* object, const char* key, int value);
bool             devicexx_add_string(d_object_t* object, const char* key, const char* value);
d_object_t *     devicexx_object_create(void);
void             devicexx_object_delete(d_object_t* object);
uint8_t*         devicexx_get_device_id( void );

#ifdef __cplusplus
}
#endif

#endif /*  __DEVICEXX_H__ */